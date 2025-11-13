.. _zas-connection-ble-sample:

#####################
Basic Profile Samples
#####################

All basic profile samples follow a common structure, which is explained below.
These samples demonstrate **peripheral** devices that advertise and accept connections from central devices.
The connection procedures are defined in the Generic Access Profile (GAP).
We will use the BLE Blood Pressure sample to demonstrate the code organization.

All the samples choose the stack by setting a configuration flag in their *prj.conf* file.

.. code-block:: kconfig

	CONFIG_BT=y
	CONFIG_BT_CUSTOM=y

The stack works asynchronously and the configuration is also done in that manner. At highest level all the samples will:

1. Power up the BLE subsystem which runs the link layer software
2. Configure the BLE stack
3. Configure a BLE Profile
4. Start advertising
5. Respond to scan requests
6. Stop advertising on connection request
7. Send profile data when connected
8. Continue advertising when disconnected

.. figure:: /images/alif_ble_flowchart.drawio.png

.. note::
   Error checking has been omitted for brevity.

****
Main
****

The main function performs the following steps:

1. Enable the BLE subsystem
2. Configure the GAPM (Generic Access Profile Manager) with device configuration and callbacks
3. Wait for configuration to complete (semaphore)
4. Periodically process blood pressure measurements

.. code-block:: c

	int main(void) {
		uint16_t current_value = 70;

		alif_ble_enable(NULL);
		gapm_configure(0, &gapm_cfg, &gapm_cbs, on_gapm_process_complete);
		k_sem_take(&my_sem, K_FOREVER);

		while (1) {
			k_sleep(K_SECONDS(1));
			blps_process(current_value);
		}
	}

******************
Initialization
******************
The first task is to enable the BLE subsystem and it executes synchronously if no callback is provided.
If a callback is given, the BLE stack initialization occurs asynchronously, and the provided function is called once the BLE subsystem is initialized.

.. note::
   The subsystem initialization needs to finish successfully before configuration can be started.

************************
Stack Configuration
************************

BLE stack configuration is done by ``gapm_configure()`` which takes the following parameters:

* **Metainfo**: Handle to an execution context of a procedure (typically 0)
* **Device Config**: Pointer to device configuration structure
* **Event Callbacks**: Collection of callbacks triggered by the host layer on different events
* **Setup Complete**: Callback invoked once the host layer setup is complete

The function returns ``GAP_ERR_NO_ERROR`` if the procedure started successfully, or a positive error code on failure.

.. code-block:: c

	gapm_configure(0, &gapm_cfg, &gapm_cbs, on_gapm_process_complete);

Device Configuration
=====================

The device configuration structure defines the role and behavior of the BLE device:

* **Role**: ``GAP_ROLE_LE_PERIPHERAL`` - device advertises and waits for connections
* **Pairing Mode**: ``GAPM_PAIRING_DISABLE`` - pairing not supported, advertising only
* **Privacy Config**: ``GAPM_PRIV_CFG_PRIV_ADDR_BIT`` - use static random private address
* **Renewal Duration**: Duration after which random private address is renewed (when privacy enabled)
* **Private Identity**: Static random address for the device
* **IRK Key**: Identity Resolving Key for resolving random private addresses
* **GAP/GATT Start Handles**: Dynamically allocated (set to 0)
* **Suggested Max TX Octets/Time**: Controller payload size and transmit time
* **Preferred TX/RX PHY**: ``GAP_PHY_ANY`` - accepts 1M, 2M, or Coded PHY
* **TX/RX Path Compensation**: Antenna delay compensation values
* **Class of Device / Link Policy**: Not applicable to BLE (BT Classic only)

.. code-block:: c

	gapm_config_t gapm_cfg = {
		.role = GAP_ROLE_LE_PERIPHERAL,
		.pairing_mode = GAPM_PAIRING_DISABLE,
		.privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT,
		.renew_dur = 0,
		.private_identity.addr = {0xCF, 0xFE, 0xFB, 0xDE, 0x11, 0x07},
		.irk.key = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.gap_start_hdl = 0,
		.gatt_start_hdl = 0,
		.att_cfg = 0,
		.sugg_max_tx_octets = GAP_LE_MIN_OCTETS,
		.sugg_max_tx_time = GAP_LE_MIN_TIME,
		.tx_pref_phy = GAP_PHY_ANY,
		.rx_pref_phy = GAP_PHY_ANY,
		.tx_path_comp = 0,
		.rx_path_comp = 0,
		.class_of_device = 0,  /* BT Classic only */
		.dflt_link_policy = 0, /* BT Classic only */
	};

Host layer event callbacks
==========================

The GAPM callbacks structure contains pointers to callback groups for different event types:

* **Connection request** (``p_con_req_cbs``): Triggered when a peer device requests a connection
* **Security** (``p_sec_cbs``): Related to procedures like pairing and encryption
* **Connection events** (``p_info_cbs``): For established or disconnected connections
* **BLE configuration** (``p_le_config_cbs``): When BLE connection configuration changes (optional)
* **BT Classic configuration** (``p_bt_config_cbs``): Not applicable to BLE (set to NULL)
* **Error information** (``p_err_info_config_cbs``): Executed on error events

.. code-block:: c

	gapm_callbacks_t gapm_cbs = {
		.p_con_req_cbs = &gapc_con_cbs,
		.p_sec_cbs = &gapc_sec_cbs,
		.p_info_cbs = &gapc_con_inf_cbs,
		.p_le_config_cbs = &gapc_le_cfg_cbs,
		.p_bt_config_cbs = NULL,
		.p_err_info_config_cbs = &gapm_err_cbs,
	};

Mandatory callback implementations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following callbacks must be implemented:

.. code-block:: c

	gapc_connection_req_cb_t gapc_con_cbs = {
		.le_connection_req = on_le_connection_req,
	};

	gapc_security_cb_t gapc_sec_cbs = {
		.key_received = on_key_received,
	};

	gapc_connection_info_cb_t gapc_con_inf_cbs = {
		.disconnected = on_disconnection,
		.name_get = on_name_get,
		.appearance_get = on_appearance_get,
	};

	/* All callbacks in this struct are optional */
	gapc_le_config_cb_t gapc_le_cfg_cbs = {0};

	gapm_err_info_config_cb_t gapm_err_cbs = {
		.ctrl_hw_error = on_gapm_err,
	};

Callback handler examples
~~~~~~~~~~~~~~~~~~~~~~~~~~

Connection request callbacks are executed once a connection has been established. The application must:

1. Call ``gapc_le_connection_cfm()`` to confirm the connection
2. Track the connection state

.. code-block:: c

	void on_le_connection_req(uint8_t conidx, uint32_t metainfo, uint8_t actv_idx, uint8_t role,
				 const gap_bdaddr_t *p_peer_addr,
				 const gapc_le_con_param_t *p_con_params, uint8_t clk_accuracy) {
		gapc_le_connection_cfm(conidx, 0, NULL);
		conn_status = BT_CONN_STATE_CONNECTED;
	}

Disconnection callbacks should restart advertising to allow new connections:

.. code-block:: c

	void on_disconnection(uint8_t conidx, uint32_t metainfo, uint16_t reason) {
		start_le_adv(adv_actv_idx);
		conn_status = BT_CONN_STATE_DISCONNECTED;
	}

Other mandatory callbacks:

* **Security** (``on_key_received``): Called when a key is received during pairing
* **Device name** (``on_name_get``): Called when peer requests device name
* **Device appearance** (``on_appearance_get``): Called when peer requests device appearance (16-bit category/subcategory value)
* **Error** (``on_gapm_err``): Called when an error occurs in the BLE stack

**********************
Configuration complete
**********************
Once configuration is completed successfully the host layer triggers a callback which:

* Registers the services.
* Starts advertising.

.. code-block:: c

	void on_gapm_process_complete(uint32_t metainfo, uint16_t status) {

		uint16_t start_hdl = 0;
		struct blps_db_cfg blps_cfg;

		blps_cfg.features = 0;
		blps_cfg.prfl_cfg = 0;

		prf_add_profile(TASK_ID_BLPS, 0, 0, &blps_cfg, &blps_cb, &start_hdl);

		create_advertising();
	}

****************
Adding a Profile
****************
The application is supposed to track the connection status and in case of a basic profile we are going to send notifications when the device is in connected state.
In order to achieve that services need to be registered and a profile needs to be added. The services hold information about the attributes.

.. code-block:: c

	prf_add_profile(TASK_ID_BLPS,
			0,
			0,
			&blps_cfg,
			&blps_cb,
			&start_hdl);

* **Task ID**: TASK_ID_BLPS, Profile API identifier, see enum *TASK_API_ID*
* **Security level**: Unencrypted, GATT Security Level 0
* **User Priority**: 0, GATT User Priority, Best Effort
* **Profile Params**: Configuration parameters of profile service
* **Profile event callbacks**: Collection of callbacks to handle Profile events
* **Service Start Handle**: 0, dynamically allocated. Only applies for services.

The service is registered as a Blood Pressure service. Connections are unencrypted and do not require authentication.
User priority 0 means that the profile is best effort. PROFILE PARAMETERS ARE ZERO INITIALIZED.

The Profile event callbacks are provided for bond data updated-event and for the measurement complete event.
The bond update events mean un/subscribing to notifications.
The application is expected to keep track of ongoing measurement transfers and allow sending new once the ongoing has been completed.

The GATT service start handle is allocated dynamically from the GATT attribute table.

***********
Advertising
***********

Advertising allows the device to be discovered by other BLE devices. The process involves:

1. Configure advertising parameters
2. Register advertising callbacks
3. Set advertising data
4. Set scan response data
5. Start advertising

Configuration
=============

The advertising parameters define how the device advertises:

- **Advertising type**: ``GAPM_ADV_PROP_UNDIR_CONN_MASK`` - undirected connectable advertising
- **Discovery mode**: ``GAPM_ADV_MODE_GEN_DISC`` - general discovery mode
- **Maximum transmission power**: 0 (device dependent)
- **Filter policy**: ``GAPM_ADV_ALLOW_SCAN_ANY_CON_ANY`` - allow scans and connections from any device
- **Advertising interval**: 100-500 ms (160-800 slots × 0.625 ms)
- **Channel map**: ``ADV_ALL_CHNLS_EN`` - all channels (37, 38, 39) enabled
- **PHY**: ``GAPM_PHY_TYPE_LE_1M`` - 1M PHY

Legacy advertising is supported by all BLE devices and can carry up to 31 bytes of advertising data.

.. code-block:: c

	uint16_t create_advertising(void) {
		gapm_le_adv_create_param_t adv_create_params = {
			.prop = GAPM_ADV_PROP_UNDIR_CONN_MASK,
			.disc_mode = GAPM_ADV_MODE_GEN_DISC,
			.max_tx_pwr = 0,
			.filter_pol = GAPM_ADV_ALLOW_SCAN_ANY_CON_ANY,
			.prim_cfg = {
					.adv_intv_min = 160, /* 100 ms */
					.adv_intv_max = 800, /* 500 ms */
					.ch_map = ADV_ALL_CHNLS_EN,
					.phy = GAPM_PHY_TYPE_LE_1M,
				},
		};

		return gapm_le_create_adv_legacy(0, GAPM_STATIC_ADDR, &adv_create_params, &le_adv_cbs);
	}


.. _ble_adv_evt:

Events
======

Advertising callbacks handle different stages of the advertising process. The procedure complete callback (``proc_cmp``) is invoked after each operation:

1. **Create advertising** (``GAPM_ACTV_CREATE_LE_ADV``): Advertising created, next set advertising data
2. **Set advertising data** (``GAPM_ACTV_SET_ADV_DATA``): Advertising data set, next set scan response data
3. **Set scan response** (``GAPM_ACTV_SET_SCAN_RSP_DATA``): Scan response set, next start advertising
4. **Start advertising** (``GAPM_ACTV_START``): Advertising started, allow application to run

.. code-block:: c

	gapm_le_adv_cb_actv_t le_adv_cbs = {
		.hdr.stopped = on_adv_actv_stopped,
		.hdr.proc_cmp = on_adv_actv_proc_cmp,
		.created = on_adv_created,
	};

	void on_adv_actv_proc_cmp(uint32_t metainfo, uint8_t proc_id, uint8_t actv_idx,
			     uint16_t status) {
		switch (proc_id) {
		case GAPM_ACTV_CREATE_LE_ADV:
			set_advertising_data(actv_idx);
			break;
		case GAPM_ACTV_SET_ADV_DATA:
			set_scan_data(actv_idx);
			break;
		case GAPM_ACTV_SET_SCAN_RSP_DATA:
			start_le_adv(actv_idx);
			break;
		case GAPM_ACTV_START:
			k_sem_give(&my_sem);
			break;
		}
	}


Advertising data
~~~~~~~~~~~~~~~~~

Advertising data is structured as AD (Advertising Data) structures. Each AD structure contains:

- Length byte
- AD type byte
- AD data

This example creates two AD structures: device name and service UUID (Blood Pressure Service 0x1810).

.. code-block:: c

	uint16_t set_advertising_data(uint8_t actv_idx)	{
		uint16_t svc = GATT_SVC_BLOOD_PRESSURE;
		uint8_t num_svc = 1;
		static const char device_name[] = "Zephyr";
		const size_t device_name_len = sizeof(device_name) - 1;
		const uint16_t adv_device_name_len = GATT_HANDLE_LEN + device_name_len;
		const uint16_t adv_uuid_svc = GATT_HANDLE_LEN + (GATT_UUID_16_LEN * num_svc);
		const uint16_t adv_len = adv_device_name_len + adv_uuid_svc;

		co_buf_t *p_buf;
		co_buf_alloc(&p_buf, 0, adv_len, 0);
		uint8_t *p_data = co_buf_data(p_buf);

		p_data[0] = device_name_len + 1;
		p_data[1] = GAP_AD_TYPE_COMPLETE_NAME;
		memcpy(p_data + 2, device_name, device_name_len);

		p_data += adv_device_name_len;
		p_data[0] = (GATT_UUID_16_LEN * num_svc) + 1;
		p_data[1] = GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID;
		p_data += 2;
		memcpy(p_data, &svc, sizeof(svc));

		gapm_le_set_adv_data(actv_idx, p_buf);
		co_buf_release(p_buf);

		return GAP_ERR_NO_ERROR;
	}

Scan response data
~~~~~~~~~~~~~~~~~~

Scan response data provides additional information beyond the advertising data. This example uses an empty scan response.

.. code-block:: c

	uint16_t set_scan_data(uint8_t actv_idx) {
		co_buf_t *p_buf;
		co_buf_alloc(&p_buf, 0, 0, 0);
		gapm_le_set_scan_response_data(actv_idx, p_buf);
		co_buf_release(p_buf);
		return GAP_ERR_NO_ERROR;
	}

Start advertising
~~~~~~~~~~~~~~~~~~

Start advertising with a duration of 0 to advertise indefinitely. The application is allowed to run once advertising starts (see the ``GAPM_ACTV_START`` case in :ref:`ble_adv_evt`).

.. code-block:: c

	uint16_t start_le_adv(uint8_t actv_idx) {
		gapm_le_adv_param_t adv_params = {
			.duration = 0, /* Advertise indefinitely */
		};
		gapm_le_start_adv(actv_idx, &adv_params);
		return GAP_ERR_NO_ERROR;
	}

***************************
Sending Measurements
***************************

The application tracks ongoing measurement transfers and only sends new measurements when the previous one has completed. The measurement process:

1. Check if device is connected
2. Verify ready to send (no ongoing transfer)
3. Create measurement data structure
4. Send measurement to connected peer
5. Mark as not ready until transfer completes

.. note::
   The function to send data (``blps_meas_send``) is profile specific.

Blood Pressure measurement data structure:

* **Flags**: Bit field indicating presence of optional data fields
* **User ID**: Identifier of the user
* **Systolic Pressure**: Systolic blood pressure value
* **Diastolic Pressure**: Diastolic blood pressure value
* **Mean Arterial Pressure**: Mean arterial pressure value
* **Pulse Rate**: Pulse rate value
* **Measurement Status**: Status value (see ``blp_meas_status_bf`` enum)
* **Time Stamp**: Time when measurement was taken

.. code-block:: c

	void send_measurement(uint16_t current_value) {
		prf_date_time_t time_stamp_values = {.year = 2024, .month = 4, .day = 1, .hour = 1, .min = 1, .sec = 1};

		bps_bp_meas_t p_meas = {
			.flags = BPS_MEAS_FLAG_TIME_STAMP_BIT | BPS_MEAS_PULSE_RATE_BIT,
			.user_id = 0,
			.systolic = current_value,
			.diastolic = current_value - 10,
			.mean_arterial_pressure = current_value - 5,
			.pulse_rate = 90,
			.meas_status = 0x01,
			.time_stamp = time_stamp_values,
		};

		blps_meas_send(0, true, &p_meas);
	}

	void blps_process(uint16_t measurement) {
		switch (conn_status) {
		case BT_CONN_STATE_CONNECTED:
			if (READY_TO_SEND) {
				send_measurement(measurement);
				READY_TO_SEND = false;
			}
			break;
		case BT_CONN_STATE_DISCONNECTED:
			LOG_DBG("Waiting for peer connection...\n");
			k_sem_take(&conn_sem, K_FOREVER);
			break;
		default:
			break;
		}
	}
