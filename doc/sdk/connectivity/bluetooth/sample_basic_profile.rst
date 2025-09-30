.. _zas-connection-ble-sample:

#####################
Basic Profile Samples
#####################

All basic profile samples follow a common structure, which is explained below.
The connection procedures are defined in the Generic Access Profile(GAP)
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

**NOTE** Error checking has been omitted for brevity.

****
Main
****
.. code-block:: c

	#include <stdint.h>
	#include <stddef.h>
	#include <zephyr/kernel.h>

	/* Forward declarations */
	int alif_ble_enable(void *callback);
	uint16_t gapm_configure(uint32_t metainfo, const gapm_config_t* p_cfg,
	                      const gapm_callbacks_t* p_cbs, gapm_proc_cmp_cb cmp_cb);
	void blps_process(uint16_t value);

	/* External variables */
	extern struct k_sem my_sem;
	extern gapm_config_t gapm_cfg;
	extern gapm_callbacks_t gapm_cbs;
	extern void on_gapm_process_complete(uint32_t metainfo, uint16_t status);

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

**NOTE** The subsystem initialization needs to finish successfully before configuration can be started.

************************
Stack Configuration
************************
BLE stack configuration is done by *gapm_configure()*. The expected parameters are:

.. code-block:: c

	#include <stdint.h>

	/* Forward declaration for callback function type */
	typedef void (*gapm_proc_cmp_cb)(uint32_t metainfo, uint16_t status);

	/* Function prototype */
	uint16_t gapm_configure(uint32_t metainfo,
				const gapm_config_t* p_cfg,
				const gapm_callbacks_t* p_cbs,
				gapm_proc_cmp_cb cmp_cb);

* **Metainfo**: 0, a handle to an execution context of a procedure
* **Device Config**: a pointer to a device configuration
* **Event Callbacks**: a collection of callbacks to be triggered by the host layer on different events
* **Setup Complete**: a callback used once the host layer setup is complete

Return value *GAP_ERR_NO_ERROR* indicates that the procedure has been started successfully, or a positive error code in case of failure.

Device Configuration
=====================
The configuration structure for setting up a BLE connection and to define the role of the device:

.. code-block:: c

	#include <stdint.h>

	/* GAP role definitions */
	#define GAP_ROLE_LE_PERIPHERAL 0x01

	/* GAPM pairing mode definitions */
	#define GAPM_PAIRING_DISABLE 0x00

	/* GAPM privacy configuration definitions */
	#define GAPM_PRIV_CFG_PRIV_ADDR_BIT 0x01

	/* GAP PHY definitions */
	#define GAP_PHY_ANY 0x00

	/* GAP LE minimum octets and time */
	#define GAP_LE_MIN_OCTETS 27
	#define GAP_LE_MIN_TIME 328

	/* Private address structure */
	typedef struct {
		uint8_t addr[6];
	} gap_addr_t;

	/* IRK key structure */
	typedef struct {
		uint8_t key[16];
	} gap_irk_t;

	/* GAPM configuration structure */
	typedef struct {
		uint8_t role;
		uint8_t pairing_mode;
		uint8_t privacy_cfg;
		uint16_t renew_dur;
		gap_addr_t private_identity;
		gap_irk_t irk;
		uint16_t gap_start_hdl;
		uint16_t gatt_start_hdl;
		uint8_t att_cfg;
		uint16_t sugg_max_tx_octets;
		uint16_t sugg_max_tx_time;
		uint8_t tx_pref_phy;
		uint8_t rx_pref_phy;
		int8_t tx_path_comp;
		int8_t rx_path_comp;
		uint32_t class_of_device;
		uint16_t dflt_link_policy;
	} gapm_config_t;

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


* **BLE Peripheral**: A device that advertises and waits for a connection.
* **Pairing Disabled**: Pairing not possible, only advertising.
* **Privacy Config**: 0, denotes static random private address.
* **Renewal Duration**: Duration after which random private address gets renewed, when privacy is enabled.
* **IRK Key**: Pre-shared Identity Resolving Key, used to resolve random private address when used.
* **GAP Service Start Handle**: 0.
* **GATT Service Start Handle**: 0.
* **Attribute Database Configuration**: Not specified.
* **Suggested Maximum Controller's Payload Size**: In octets.
* **Suggested Maximum Controller's Transmit Time**: In seconds.
* **Preferred TX PHY Mode**: Any of 1M, 2M or Coded is accepted.
* **Preferred RX PHY Mode**: Any of 1M, 2M or Coded is accepted.
* **TX Path Compensation**: 0.
* **RX Path Compensation**: 0.
* **Class of Device**: 0, does not apply to BLE.
* **Default Link Policy**: 0, does not apply to BLE.

Host layer event callbacks
==========================
Required callbacks used to signal BLE GAP events.

.. code-block:: c

	#include <stdint.h>
	#include <stddef.h>

	/* Forward declarations for callback structures */
	typedef struct gapc_connection_req_cb gapc_connection_req_cb_t;
	typedef struct gapc_security_cb gapc_security_cb_t;
	typedef struct gapc_connection_info_cb gapc_connection_info_cb_t;
	typedef struct gapc_le_config_cb gapc_le_config_cb_t;
	typedef struct gapm_err_info_config_cb gapm_err_info_config_cb_t;

	/* GAPM callbacks structure */
	typedef struct {
		const gapc_connection_req_cb_t *p_con_req_cbs;
		const gapc_security_cb_t *p_sec_cbs;
		const gapc_connection_info_cb_t *p_info_cbs;
		const gapc_le_config_cb_t *p_le_config_cbs;
		const void *p_bt_config_cbs; /* BT classic callbacks */
		const gapm_err_info_config_cb_t *p_err_info_config_cbs;
	} gapm_callbacks_t;

	/* External variables */
	extern gapc_connection_req_cb_t gapc_con_cbs;
	extern gapc_security_cb_t gapc_sec_cbs;
	extern gapc_connection_info_cb_t gapc_con_inf_cbs;
	extern gapc_le_config_cb_t gapc_le_cfg_cbs;
	extern gapm_err_info_config_cb_t gapm_err_cbs;

	gapm_callbacks_t gapm_cbs = {
		.p_con_req_cbs = &gapc_con_cbs,
		.p_sec_cbs = &gapc_sec_cbs,
		.p_info_cbs = &gapc_con_inf_cbs,
		.p_le_config_cbs = &gapc_le_cfg_cbs,
		.p_bt_config_cbs = NULL, /* BT classic so not required */
		.p_err_info_config_cbs = &gapm_err_cbs,
	};

* **Connection request**: Triggered when a peer device requests a connection
* **Security**: Related to procedures like pairing and encryption
* **Connection events**: For established or disconnected connections
* **BLE configuration**: When BLE connection configuration changes
* **BT Classic configuration**: Not applicable to BLE
* **Error information**: Executed on error events

There is a set of mandatory callbacks which are displayed here. For the optional ones refer on the API documentation directly

.. code-block:: c

	#include <stdint.h>

	/* Forward declarations for callback functions */
	void on_le_connection_req(uint8_t conidx, uint32_t metainfo, uint8_t actv_idx, uint8_t role,
	                       const gap_bdaddr_t *p_peer_addr,
	                       const gapc_le_con_param_t *p_con_params, uint8_t clk_accuracy);
	void on_key_received(uint8_t conidx, uint32_t metainfo);
	void on_disconnection(uint8_t conidx, uint32_t metainfo, uint16_t reason);
	void on_name_get(uint8_t conidx, uint32_t metainfo);
	void on_appearance_get(uint8_t conidx, uint32_t metainfo);
	void on_gapm_err(uint8_t error_type);

	/* Connection request callback structure */
	typedef struct gapc_connection_req_cb {
		void (*le_connection_req)(uint8_t conidx, uint32_t metainfo, uint8_t actv_idx, uint8_t role,
		                       const gap_bdaddr_t *p_peer_addr,
		                       const gapc_le_con_param_t *p_con_params, uint8_t clk_accuracy);
	} gapc_connection_req_cb_t;

	/* Security callback structure */
	typedef struct gapc_security_cb {
		void (*key_received)(uint8_t conidx, uint32_t metainfo);
	} gapc_security_cb_t;

	/* Connection info callback structure */
	typedef struct gapc_connection_info_cb {
		void (*disconnected)(uint8_t conidx, uint32_t metainfo, uint16_t reason);
		void (*name_get)(uint8_t conidx, uint32_t metainfo);
		void (*appearance_get)(uint8_t conidx, uint32_t metainfo);
	} gapc_connection_info_cb_t;

	/* LE config callback structure */
	typedef struct gapc_le_config_cb {
		/* All callbacks in this struct are optional */
		void *placeholder;
	} gapc_le_config_cb_t;

	/* Error info callback structure */
	typedef struct gapm_err_info_config_cb {
		void (*ctrl_hw_error)(uint8_t error_type);
	} gapm_err_info_config_cb_t;

	/* Callback structures instances */
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

Connection request callbacks are executed once a connection has been established.
The application is expected to call *gapc_le_connection_cfm*.
Application should track the state of the connection.

Once disconnect happens, the application is expected to call *start_le_adv* to restart the advertising.

.. code-block:: c

	#include <stdint.h>

	/* Connection status enum */
	enum {
		BT_CONN_STATE_DISCONNECTED,
		BT_CONN_STATE_CONNECTED
	};

	/* Forward declarations */
	void gapc_le_connection_cfm(uint8_t conidx, uint16_t status, void *params);
	uint16_t start_le_adv(uint8_t actv_idx);

	/* External variables */
	extern uint8_t conn_status;
	extern uint8_t adv_actv_idx;

	/* Structures needed for parameters */
	typedef struct {
		uint8_t addr_type;
		uint8_t addr[6];
	} gap_bdaddr_t;

	typedef struct {
		uint16_t interval_min;
		uint16_t interval_max;
		uint16_t latency;
		uint16_t timeout;
	} gapc_le_con_param_t;

	void on_le_connection_req(uint8_t conidx, uint32_t metainfo, uint8_t actv_idx, uint8_t role,
				 const gap_bdaddr_t *p_peer_addr,
				 const gapc_le_con_param_t *p_con_params, uint8_t clk_accuracy) {

		gapc_le_connection_cfm(conidx, 0, NULL);

		conn_status = BT_CONN_STATE_CONNECTED;
	}

	static void on_disconnection(uint8_t conidx, uint32_t metainfo, uint16_t reason) {
		start_le_adv(adv_actv_idx);

		conn_status = BT_CONN_STATE_DISCONNECTED;
	}


Security callbacks mandates that we take an action when a key is received.
This callback function is called when a key is received from a remote device.
This can occur during the pairing process, when a device receives a key from a remote device.

Connection callbacks have three mandatory event handlers:

* **Disconnect**: Action taken when disconnect happens
* **Device name**: Action taken when peer requests device name
* **Device appearance**: Action taken when peer requests device appearance.

The appearance of a device is a 16-bit value that represents the device's category and subcategory.

Error information callbacks are used to signal that an error has occurred
in the BLE stack.

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
Steps to take are configuring the advertising and registering required callbacks.

Configuration
=============
The application uses a configuration structure to specify the advertising parameters such as the advertising interval, channel map and the advertising data.

.. code-block:: c

	#include <stdint.h>

	/* Advertising property definitions */
	#define GAPM_ADV_PROP_UNDIR_CONN_MASK 0x01

	/* Advertising mode definitions */
	#define GAPM_ADV_MODE_GEN_DISC 0x01

	/* Advertising filter policy definitions */
	#define GAPM_ADV_ALLOW_SCAN_ANY_CON_ANY 0x00

	/* Channel map definitions */
	#define ADV_ALL_CHNLS_EN 0x07

	/* PHY type definitions */
	#define GAPM_PHY_TYPE_LE_1M 0x01

	/* Address type definitions */
	#define GAPM_STATIC_ADDR 0x00

	/* Primary advertising configuration structure */
	typedef struct {
		uint16_t adv_intv_min;
		uint16_t adv_intv_max;
		uint8_t ch_map;
		uint8_t phy;
	} gapm_le_adv_prim_cfg_t;

	/* Advertising creation parameters structure */
	typedef struct {
		uint8_t prop;
		uint8_t disc_mode;
		int8_t max_tx_pwr;
		uint8_t filter_pol;
		gapm_le_adv_prim_cfg_t prim_cfg;
	} gapm_le_adv_create_param_t;

	/* Forward declarations */
	uint16_t gapm_le_create_adv_legacy(uint32_t metainfo, uint8_t addr_src,
	                               const gapm_le_adv_create_param_t *p_adv_param,
	                               const gapm_le_adv_cb_actv_t *p_adv_cbs);

	/* External variables */
	extern gapm_le_adv_cb_actv_t le_adv_cbs;

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

		int err = gapm_le_create_adv_legacy(0, GAPM_STATIC_ADDR, &adv_create_params, &le_adv_cbs);

		return err;
	}

- **Advertising type**: Undirected connectable advertising.
- **Discovery mode**: General discovery.
- **Maximum transmission power**: 0 (device dependent).
- **Filter policy**: Allow scans and connections from any device.
- **Primary advertising configuration**:
	- Minimum advertising interval: 100 ms (160 x 0.625 ms).
	- Maximum advertising interval: 500 ms (800 x 0.625 ms).
	- Channel map: All channels enabled.
	- PHY: LE 1M.

Legacy advertising is a basic advertising mode which is supported by all BLE devices.
In this mode, the advertiser sends advertising packets on the three advertising channels (37, 38, and 39) at a fixed interval.
The advertising data can be up to 31 bytes long.


.. _ble_adv_evt:

Events
======
Advertising callbacks are defined for starting, stopping and processing events.
Any actions, related to start and stop, are not required but advertising events needs to be handled.
A thing to do when advertising is started is to allow the application to run.

.. code-block:: c

	#include <stdint.h>
	#include <zephyr/kernel.h>

	/* Forward declarations for callback functions */
	void on_adv_actv_stopped(uint32_t metainfo, uint8_t actv_idx, uint8_t reason);
	void on_adv_actv_proc_cmp(uint32_t metainfo, uint8_t proc_id, uint8_t actv_idx, uint16_t status);
	void on_adv_created(uint32_t metainfo, uint8_t actv_idx);

	/* Forward declarations for functions */
	uint16_t set_advertising_data(uint8_t actv_idx);
	uint16_t set_scan_data(uint8_t actv_idx);
	uint16_t start_le_adv(uint8_t actv_idx);

	/* External variables */
	extern struct k_sem my_sem;

	/* Activity procedure IDs */
	#define GAPM_ACTV_CREATE_LE_ADV 0x01
	#define GAPM_ACTV_SET_ADV_DATA 0x02
	#define GAPM_ACTV_SET_SCAN_RSP_DATA 0x03
	#define GAPM_ACTV_START 0x04

	/* Activity header structure */
	typedef struct {
		void (*stopped)(uint32_t metainfo, uint8_t actv_idx, uint8_t reason);
		void (*proc_cmp)(uint32_t metainfo, uint8_t proc_id, uint8_t actv_idx, uint16_t status);
	} gapm_le_actv_cb_hdr_t;

	/* Advertising callbacks structure */
	typedef struct {
		gapm_le_actv_cb_hdr_t hdr;
		void (*created)(uint32_t metainfo, uint8_t actv_idx);
	} gapm_le_adv_cb_actv_t;

	gapm_le_adv_cb_actv_t le_adv_cbs = {
		.hdr.stopped = on_adv_actv_stopped,
		.hdr.proc_cmp = on_adv_actv_proc_cmp,
		.created = on_adv_created,
	};

	void on_adv_actv_proc_cmp(uint32_t metainfo, uint8_t proc_id, uint8_t actv_idx,
			     uint16_t status) {
		switch (proc_id) {
		case GAPM_ACTV_CREATE_LE_ADV:
			/* Set advertising data */
			set_advertising_data(actv_idx);
			break;
		case GAPM_ACTV_SET_ADV_DATA:
			/* Set scan response data */
			set_scan_data(actv_idx);
			break;

		case GAPM_ACTV_SET_SCAN_RSP_DATA:
			/* Start advertising */
			start_le_adv(actv_idx);
			break;

		case GAPM_ACTV_START:
			/* Let application run when advertising is started */
			k_sem_give(&my_sem);
			break;
		}
	}


The advertising data is set before starting the advertising. The data is broken down into AD structures.
Each AD structure contains the length, the AD type and the AD data.
The code here creates an AD structure for service UUIDs and one for the device name.

.. code-block:: c

	#include <stdint.h>
	#include <string.h>

	/* GATT service identifiers */
	#define GATT_SVC_BLOOD_PRESSURE 0x1810

	/* GATT handle and UUID lengths */
	#define GATT_HANDLE_LEN 2
	#define GATT_UUID_16_LEN 2

	/* GAP AD types */
	#define GAP_AD_TYPE_COMPLETE_NAME 0x09
	#define GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID 0x03

	/* GAP error codes */
	#define GAP_ERR_NO_ERROR 0x00

	/* Buffer structure */
	typedef struct co_buf co_buf_t;

	/* Forward declarations */
	uint16_t co_buf_alloc(co_buf_t **pp_buf, uint16_t class_bit_field, uint16_t data_len, uint16_t header_len);
	uint8_t *co_buf_data(co_buf_t *p_buf);
	void co_buf_release(co_buf_t *p_buf);
	uint16_t gapm_le_set_adv_data(uint8_t actv_idx, co_buf_t *p_buf);

	uint16_t set_advertising_data(uint8_t actv_idx)	{

		uint16_t svc = GATT_SVC_BLOOD_PRESSURE; /* GATT service identifier */

		uint8_t num_svc = 1; /* Number of services */
		static const char device_name[] = "Zephyr";
		const size_t device_name_len = sizeof(device_name) - 1;
		const uint16_t adv_device_name_len = GATT_HANDLE_LEN + device_name_len;
		const uint16_t adv_uuid_svc = GATT_HANDLE_LEN + (GATT_UUID_16_LEN * num_svc);

		/* Create advertising data with necessary services */
		const uint16_t adv_len = adv_device_name_len + adv_uuid_svc;

		co_buf_t *p_buf;

		co_buf_alloc(&p_buf, 0, adv_len, 0);

		uint8_t *p_data = co_buf_data(p_buf);

		p_data[0] = device_name_len + 1;
		p_data[1] = GAP_AD_TYPE_COMPLETE_NAME;
		memcpy(p_data + 2, device_name, device_name_len);

		p_data += adv_device_name_len; /* Update data pointer */
		p_data[0] = (GATT_UUID_16_LEN * num_svc) + 1;
		p_data[1] = GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID;

		/* Copy identifier */
		p_data += 2; /* Update data pointer */
		memcpy(p_data, &svc, sizeof(svc));

		gapm_le_set_adv_data(actv_idx, p_buf);

		co_buf_release(p_buf); /* Release ownership of buffer so stack can free it when done */

		return GAP_ERR_NO_ERROR;
	}

Set scan response data in the BLE advertising data.
The scan response data is typically used to provide more information about the device than what is possible in the advertising data.
This API sets the scan response data for the given advertising set.

.. code-block:: c

	#include <stdint.h>

	/* Forward declarations */
	uint16_t co_buf_alloc(co_buf_t **pp_buf, uint16_t class_bit_field, uint16_t data_len, uint16_t header_len);
	void co_buf_release(co_buf_t *p_buf);
	uint16_t gapm_le_set_scan_response_data(uint8_t actv_idx, co_buf_t *p_buf);

	/* GAP error codes */
	#define GAP_ERR_NO_ERROR 0x00

	/* Buffer structure */
	typedef struct co_buf co_buf_t;

	uint16_t set_scan_data(uint8_t actv_idx) {
		co_buf_t *p_buf;

		uint16_t err = co_buf_alloc(&p_buf, 0, 0, 0);

		err = gapm_le_set_scan_response_data(actv_idx, p_buf);
		co_buf_release(p_buf); /* Release ownership of buffer so stack can free it when done */

		return GAP_ERR_NO_ERROR;
	}

Start the BLE advertising. The application is allowed to run once the advertising is started - done by posting the semaphore as show in the code listing at the beginning of :ref:`ble_adv_evt`.

.. code-block:: c

	#include <stdint.h>

	/* Forward declarations */
	uint16_t gapm_le_start_adv(uint8_t actv_idx, const gapm_le_adv_param_t *p_adv_param);

	/* GAP error codes */
	#define GAP_ERR_NO_ERROR 0x00

	/* Advertising parameters structure */
	typedef struct {
		uint16_t duration; /* Duration of advertising in 10ms units, 0 for indefinite */
		/* Other fields omitted for brevity */
	} gapm_le_adv_param_t;

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

The application is expected to keep track of ongoing measurement transfers and allow sending new ones when the ongoing has been completed.
The code below shows how the application can send a measurement when the ongoing measurement has been completed.

**NOTE** Function to send data is profile specific.

.. code-block:: c

	#include <stdint.h>
	#include <stdbool.h>
	#include <zephyr/kernel.h>
	#include <zephyr/logging/log.h>

	/* Blood pressure measurement flag bits */
	#define BPS_MEAS_FLAG_TIME_STAMP_BIT 0x02
	#define BPS_MEAS_PULSE_RATE_BIT 0x04

	/* Connection status enum */
	enum {
		BT_CONN_STATE_DISCONNECTED,
		BT_CONN_STATE_CONNECTED
	};

	/* Date time structure */
	typedef struct {
		uint16_t year;
		uint8_t month;
		uint8_t day;
		uint8_t hour;
		uint8_t min;
		uint8_t sec;
	} prf_date_time_t;

	/* Blood pressure measurement structure */
	typedef struct {
		uint8_t flags;
		uint8_t user_id;
		uint16_t systolic;
		uint16_t diastolic;
		uint16_t mean_arterial_pressure;
		uint16_t pulse_rate;
		uint16_t meas_status;
		prf_date_time_t time_stamp;
	} bps_bp_meas_t;

	/* Forward declarations */
	void blps_meas_send(uint8_t conidx, bool indication, const bps_bp_meas_t *p_meas);

	/* External variables */
	extern uint8_t conn_status;
	extern bool READY_TO_SEND;
	extern struct k_sem conn_sem;

	void send_measurement(uint16_t current_value) {
		/* Dummy time data  */
		prf_date_time_t time_stamp_values = {.year = 2024, .month = 4, .day = 1, .hour = 1, .min = 1, .sec = 1};

		/* Dummy measurement data */
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

		/* Send measuremnt to connected device */
		/* Set 0 to first parameter to send only to the first connected peer device */
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

The BLE Blood Pressure Profile measurement data is composed of the following components:

* **Flags**: A bit field indicating the presence of optional data fields.
* **User ID**: Identifier of the user.
* **Systolic Pressure**: The systolic blood pressure measurement value.
* **Diastolic Pressure**: The diastolic blood pressure measurement value.
* **Mean Arterial Pressure**: The mean arterial pressure measurement value.
* **Pulse Rate**: The pulse rate measurement value.
* **Measurement Status**: The measurement status value. Please see *enum blp_meas_status_bf* for possible values.
* **Time Stamp**: The time when the measurement was taken.
