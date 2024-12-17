.. _zas-connection-ble-audiosink:

#################
Audio Sink Sample
#################
|ble_auracast_intro|. This sample demonstrates a sink device.

All the audio samples share the same basic structure with all the other :ref:`BLE profile samples<zas-connection-ble-sample>`.

The sink sample chooces |**Alif_BLE**| and  |**Alif_LC3**| by setting a configuration flag on their *prj.conf* file.
Additionally there is need to set:

*  Presentation compensation configuration
*  I²S configuration
*  GAF configuration

.. code-block:: kconfig

	# Alif's BLE stack
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

.. figure:: /_static/alif_ble_audio_presentation_layer_sink.drawio.png

******
Main
******
Where the things start to deviate from the basic profile setup is when we define the role of the device during GAPM configuration phase.
The role is set to be an observer to be able to listen for advertisements from other devices without actively participating in the connection process.

.. code-block:: c

	static const gapm_config_t gapm_cfg = {
		.role = GAP_ROLE_LE_OBSERVER,
	};

**NOTE** All the other members of the configuration structure are assumed to be be the same as with the basic profile config.
