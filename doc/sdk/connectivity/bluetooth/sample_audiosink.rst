.. _zas-connection-ble-audiosink:

##############
Broadcast Sink
##############
|ble_audio_intro|. This sample demonstrates a sink device.

All the audio samples share the same basic structure with all the other :ref:`BLE profile samples<zas-connection-ble-sample>`.
The aforementioned samples follow standard connection procedures defined in the Generic Access Profile(GAP).
Basic Audio Profile(BAP) API is used to configure, discover and playback broadcast audio.

.. figure:: /images/alif_ble_sink_flowchart.drawio.png

*******
Kconfig
*******

The sink sample chooses |**Alif_BLE**| and |**Alif_LC3**| by setting a configuration flag on their *prj.conf* file.
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
	CONFIG_I2S_SYNC_BUFFER_FORMAT_SEQUENTIAL=n

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

.. note::
   All the other members of the configuration structure are assumed to be the same as with
   the basic profile config. When acting as an *observer*, you don't need to set GAPM
   callbacks except the error callbacks.

****************
Audio Sink setup
****************

Steps to take to be able to listen a broadcast

1. Set scan configuration
2. Set audio sink configuration
3. Start scanning

.. note::
   All the API functions taking the BAP role as a parameter expect to have a bit set for
   each role that the device supports. The information is used to check that all the
   required callbacks are set.

.. code-block:: c

	int broadcast_sink_start(void) {
		bap_bc_scan_configure(BAP_ROLE_SUPP_BC_SINK_BIT | BAP_ROLE_SUPP_BC_SCAN_BIT, &scan_cbs);
		bap_bc_sink_configure(BAP_ROLE_SUPP_BC_SINK_BIT | BAP_ROLE_SUPP_BC_SCAN_BIT, &sink_cbs);
		return start_scanning();
	}

Scan configuration
==================

Configure the broadcast scan module with callbacks to handle scan events. The scan callbacks
provide notifications for various events during the broadcast discovery process:

* **cb_cmp_evt**: Scan command complete - invoked when a scan operation finishes
* **cb_timeout**: Scan timeout - invoked when a scan operation times out
* **cb_report**: Broadcast source discovered - invoked when a broadcast source device is found
* **cb_public_bcast_source**: Public broadcast discovered - invoked when a public broadcast is found
* **cb_pa_established**: PA sync established - invoked when periodic advertising synchronization is established
* **cb_pa_terminated**: PA sync terminated - invoked when periodic advertising synchronization is lost
* **cb_pa_report**: PA report received - invoked when a periodic advertising report is received
* **cb_big_info_report**: BIG info received - invoked when BIG information is reported
* **cb_group_report**: Group report received - invoked when broadcast group information is received
* **cb_subgroup_report**: Subgroup report received - invoked when subgroup information is received
* **cb_stream_report**: Stream report received - invoked when stream information is received

.. code-block:: c

	static const bap_bc_scan_cb_t scan_cbs = {
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

	bap_bc_scan_configure(BAP_ROLE_SUPP_BC_SINK_BIT | BAP_ROLE_SUPP_BC_SCAN_BIT, &scan_cbs);

Sink configuration
==================

Configure the broadcast sink module with callbacks to handle sink events during audio playback:

* **cb_cmp_evt**: Command complete - invoked when a sink command has been completed
* **cb_quality_cmp_evt**: Get quality complete - invoked when quality information has been retrieved
* **cb_status**: Synchronization status - invoked when the synchronization status with a broadcast group changes

.. code-block:: c

	static const bap_bc_sink_cb_t sink_cbs = {
		.cb_cmp_evt = on_bap_bc_sink_cmp_evt,
		.cb_quality_cmp_evt = on_bap_bc_sink_quality_cmp_evt,
		.cb_status = on_bap_bc_sink_status,
	};

	bap_bc_sink_configure(BAP_ROLE_SUPP_BC_SINK_BIT | BAP_ROLE_SUPP_BC_SCAN_BIT, &sink_cbs);

Scanning
========

Start the broadcast scan operation to discover broadcast sources. The scan runs continuously
(timeout = 0) until a suitable broadcast is found. The sink configuration is reset to prepare
for receiving audio:

* Initialize the audio datapath configuration
* Select the I²S device for audio output
* Reset broadcast discovery state

.. code-block:: c

	static int start_scanning(void)
	{
		/* Zero timeout value causes it to scan until explicitly stopped */
		bap_bc_scan_start(0);

		reset_sink_config();
		public_broadcast_found = false;

		return 0;
	}

Sink enable
===========

Once the device receives stream reports and detects the expected number of streams, the following occurs:

1. **Stop scanning for PA reports** - periodic advertising scanning is no longer needed
2. **Synchronize to BIG** - establish synchronization with the Broadcast Isochronous Group
3. **Configure audio datapath** - set up the ISO data path for receiving audio data
4. **Enable streams** - start receiving audio frames from the broadcast source
5. **Start decoding and playback** - begin LC3 decoding and I²S audio output

The sink is now actively receiving and playing broadcast audio.
