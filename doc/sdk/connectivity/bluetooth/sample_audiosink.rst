.. _zas-connection-ble-audiosink:

##############
Broadcast Sink
##############
|ble_audio_intro|. This sample demonstrates a sink device.

All the audio samples share the same basic structure with all the other :ref:`BLE profile samples<zas-connection-ble-sample>`.
The forementioned samples follow standard connection procedures defined in the Generic Access Profile(GAP).
Basic Audio Profile(BAP) API is used to configure, discover and playback broadcast audio.

.. figure:: /images/alif_ble_sink_flowchart.drawio.png

*******
Kconfig
*******

The sink sample chooces |**Alif_BLE**| and  |**Alif_LC3**| by setting a configuration flag on their *prj.conf* file.
Additionally there is need to set:

*  Presentation compensation configuration
*  I²S configuration
*  GAF configuration

.. code-block:: kconfig

	# Alif's BLE stack
	CONFIG_BT=y
	CONFIG_BT_CUSTOM=y

	# Audio
	CONFIG_ALIF_ROM_LC3_CODEC=y
	CONFIG_ALIF_BLE_AUDIO=y

	CONFIG_PRESENTATION_COMPENSATION_DIRECTION_SINK=y
	CONFIG_PRESENTATION_COMPENSATION_CORRECTION_FACTOR=2

	# Should be replaced by TRNG when support is available
	CONFIG_TEST_RANDOM_GENERATOR=y

	# malloc support is required, simplest way is using newlib although it would also
	# be possible to configure heap for picolibc
	CONFIG_NEWLIB_LIBC=y

	# Driver support for audio
	CONFIG_I2S=y
	CONFIG_I2S_SYNC_BUFFER_FORMAT_SEQUENTIAL=y

	# Determine left and right channels by index, not GAF
	CONFIG_AUDIO_LOCATION_IMPLICIT=y

.. figure:: /images/alif_ble_audio_presentation_layer_sink.drawio.png

********
Pre-Main
********
Before entering main the sample will

* Initialize the I²S device
* Initialize the LC3 codec

****
Main
****
Where the things start to deviate from the basic profile setup is when we define the role of the device during GAPM configuration phase.
The role is set to be an *observer* to be able to listen for advertisements from other devices without actively participating in the connection process.

.. code-block:: c

	static const gapm_config_t gapm_cfg = {
		.role = GAP_ROLE_LE_OBSERVER,
	};

**NOTE** All the other members of the configuration structure are assumed to be be the same as with the basic profile config.

You don't need to set GAPM callbacks when acting as a *observer*, except the error callbacks.

****************
Audio Sink setup
****************

Steps to take to be able to listen a broadcast

1. Set scan configuration
2. Set audio sink configuration
3. Start scanning

**NOTE** All the API functions taking the BAP role as a parameter expects to have a bit set for each role that the device supports.
The information is used to check that all the required callbacks are set.

.. code-block:: c

	int broadcast_sink_start(void) {
		bap_bc_scan_configure(BAP_ROLE_SUPP_BC_SINK_BIT | BAP_ROLE_SUPP_BC_SCAN_BIT, &scan_cbs);

		bap_bc_sink_configure(BAP_ROLE_SUPP_BC_SINK_BIT | BAP_ROLE_SUPP_BC_SCAN_BIT, &sink_cbs);

		return start_scanning();
	}

Scan configuration
==================

.. code-block:: c

	bap_bc_scan_cb_t scan_cbs = {
		.cb_cmp_evt = on_bap_bc_scan_cmp_evt,
		.cb_timeout = on_bap_bc_scan_timeout,
		.cb_report = on_bap_bc_scan_report,
		.cb_public_bcast_source = on_bap_bc_scan_public_bcast,
		.cb_pa_established = on_bap_bc_scan_pa_established,
		.cb_pa_terminated = on_bap_bc_scan_pa_terminated,
		.cb_pa_report = on_bap_bc_scan_pa_report,
		.cb_big_info_report = on_bap_bc_scan_big_info_report,
		.cb_group_report = on_bap_bc_scan_group_report,
		.cb_subgroup_report = on_bap_bc_scan_subgroup_report,
		.cb_stream_report = on_bap_bc_scan_stream_report,
	};

	uint16_t bap_bc_scan_configure(uint32_t role_bf, const bap_bc_scan_cb_t* scan_cbs);

* **BAP Role**: All supported roles should be set here
* **BAP Broadcast Scan Callbacks**: All the different events that can occur during a scan:

  * **Scan Complete**: Called when a scan command is complete
  * **Scan Timeout**: Called when a scan times out
  * **Broadcast Source Discovered**: Called when a Broadcast Source device is discovered
  * **Public Broadcast Discovered**: Called when a Public Broadcast is discovered
  * **Periodic Advertising Synchronisation Complete**: Called when a PA synchronisation is established
  * **Periodic Advertising Synchronisation Terminated**: Called when a PA synchronisation is terminated
  * **Periodic Advertising Report Received**: Called when a PA report is received
  * **BIG Info Report Received**: Called when a BIG Info report is received
  * **Group Report Received**: Called when a group report is received
  * **Subgroup Report Received**: Called when a subgroup report is received
  * **Stream Report Received**: Called when a stream report is received

Sink configuration
==================

.. code-block:: c

	static const bap_bc_sink_cb_t sink_cbs = {
		.cb_cmp_evt = on_bap_bc_sink_cmp_evt,
		.cb_quality_cmp_evt = on_bap_bc_sink_quality_cmp_evt,
		.cb_status = on_bap_bc_sink_status,
	};


	uint16_t bap_bc_sink_configure(uint32_t role_bf, const bap_bc_sink_cb_t* sink_cbs);

* **BAP Role**: All supported roles should be set here
* **BAP Broadcast Sink Callbacks**: All the different events that can occur during a broadcast sink:

    * **Command Complete**: Called when a command has been completed
    * **Get Quality Complete**: Called when BAP_BC_SINK_GET_QUALITY command has been completed
    * **Synchronisation Status**: Called when status of synchronization with a Broadcast Group has changed

Scanning
========

.. code-block:: c

	static int start_scanning(void)
	{
		/* Zero timeout value causes it to scan until explicitly stopped */
		bap_bc_scan_start(0);

		reset_sink_config();
		public_broadcast_found = false;

		return 0;
	}

* Start the scan operation
* Reset the sink configuration which means:

    * Initializing the datapath configuration
    * Choosing the I²S device

Sink enable
===========
Once the the device gets a stream report and the number of expected streams are detected device stops scanning for PA reports.
Sink is started at this phase.
