.. _zas-connection-ble-audiosource:

################
Broadcast Source
################

|ble_audio_intro|. This sample demonstrates a source device.

All the audio samples share the same basic structure with all the other :ref:`BLE profile samples<zas-connection-ble-sample>`.
The forementioned samples follow standard connection procedures defined in the Generic Access Profile(GAP).
Basic Audio Profile(BAP) API is used to configure and broadcast audio.

.. figure:: /images/alif_ble_source_flowchart.drawio.png

*******
Kconfig
*******

The sink sample chooces |**Alif_BLE**| and |**Alif_LC3**| by setting a configuration flag on their *prj.conf* file.
Additionally there is need to set:

*  Presentation compensation configuration
*  I²S configuration
*  BAP configuration


.. code-block:: kconfig

	CONFIG_BT=y
	CONFIG_BT_CUSTOM=y
	CONFIG_ALIF_ROM_LC3_CODEC=y
	CONFIG_ALIF_BLE_AUDIO=y

	CONFIG_PRESENTATION_COMPENSATION_DIRECTION_SOURCE=y

	# Should be replaced by TRNG when support is available
	CONFIG_TEST_RANDOM_GENERATOR=y

	# malloc support is required, simplest way is using newlib although it would also
	# be possible to configure heap for picolibc
	CONFIG_NEWLIB_LIBC=y

	# Driver support for audio
	CONFIG_I2S=y
	CONFIG_I2S_SYNC_BUFFER_FORMAT_SEQUENTIAL=n

	CONFIG_BROADCAST_NAME="ALIF_LE_AUDIO"

********
Pre-Main
********
Before entering main the sample will initialize datapath, which means:

* Initializing the I²S device
* Initializing the LC3 codec

****
Main
****
Where the things start to deviate from the basic profile setup is when we define the role of the device during GAPM configuration phase.
The role is set to be an *broadcaster* to be able to advertise and broadcast audio for other devices without a connection process.

.. code-block:: c

	static const gapm_config_t gapm_cfg = {
		.role = GAP_ROLE_LE_BROADCASTER,
	};

**NOTE** All the other members of the configuration structure are assumed to be be the same as with the basic profile config.

You don't need to set GAPM callbacks when acting as a *broadcaster*, except the error callbacks.

******************
Audio Source Start
******************

Once GAPM configuration is complete broadcast source is configured and started.

.. code-block:: c

	static const bap_bc_src_cb_t bap_bc_src_cbs = {
		.cb_cmp_evt = on_bap_bc_src_cmp_evt,
		.cb_info = on_bap_bc_src_info
	};

	int broadcast_source_start(void)
	{
		bap_bc_src_configure(&bap_bc_src_cbs);

		broadcast_source_configure_group();

		return broadcast_source_enable();
	}

1. Configure use of BAP Broadcast Source module
2. Configure Advertising
3. Fill the BIG, group and subgroup info, known as the Broadcast Audio Source Endpoint(BASE) structure
4. Configure and enable streams
5. Enable Periodic advertising

Configure
=========

Configuring Broadcast Source module:

.. code-block:: c

	bap_bc_src_configure(&bap_bc_src_cbs);

* **BAP Broadcast Source Callbacks**: All the different events originating from the Broadcast Source module:

  * **Command Complete**: Called each time a Broadcast Source command has been completed. This is triggered when PA is enabled, broadcast group is enabled or streaming is started.
  * **Broadcast Source Info**: Called when a group has been created

Configure Advertising, fill the BASE structure and enable left and right streams:

.. code-block:: c

	int broadcast_source_configure_group(void)
	{
		const bap_bc_grp_param_t grp_param = {.sdu_intv_us = 10000,
						.max_sdu = CONFIG_ALIF_BLE_AUDIO_OCTETS_PER_CODEC_FRAME,
						.max_tlatency_ms = CONFIG_ALIF_BLE_AUDIO_MAX_TLATENCY,
						.packing = 0,
						.framing = ISO_UNFRAMED_MODE,
						.phy_bf = GAPM_PHY_TYPE_LE_2M,
						.rtn = CONFIG_ALIF_BLE_AUDIO_RTN};

		const gaf_codec_id_t codec_id = GAF_CODEC_ID_LC3;

		const bap_bc_adv_param_t adv_param = {
			.adv_intv_min_slot = 160,
			.adv_intv_max_slot = 160,
			.ch_map = ADV_ALL_CHNLS_EN,
			.phy_prim = GAPM_PHY_TYPE_LE_1M,
			.phy_second = GAPM_PHY_TYPE_LE_2M,
			.adv_sid = 1,
			.max_tx_pwr = -2,
		};

		const bap_bc_per_adv_param_t per_adv_param = {
			.adv_intv_min_frame = 160,
			.adv_intv_max_frame = 160,
		};

		bap_bcast_id_t bcast_id;

		sys_rand_get(bcast_id.id, sizeof(bcast_id.id));

		bap_bc_src_add_group(&bcast_id, NULL, 2, 1, &grp_param, &adv_param,
						&per_adv_param, PRESENTATION_DELAY_US, &bcast_grp_lid);

		/* Must be accessible to the BLE stack for the lifetime of the BIG -> statically allocated */
		static bap_cfg_t sgrp_cfg = {
			.param = {
					.location_bf = 0, /* Location is unspecified at subgroup level */
					.frame_octet = CONFIG_ALIF_BLE_AUDIO_OCTETS_PER_CODEC_FRAME,
					.frame_dur = BAP_FRAME_DUR_10MS,
					.frames_sdu =
						0, /* 0 is unspecified, data will not be placed in BASE */
				},
			.add_cfg.len = 0,
		};

		sgrp_cfg.param.sampling_freq =
			bap_sampling_freq_from_hz(CONFIG_ALIF_BLE_AUDIO_FS_HZ);

		/* Must be accessible to the BLE stack for the lifetime of the BIG -> statically allocated */
		static const bap_cfg_metadata_t sgrp_meta = {
			.param.context_bf = BAP_CONTEXT_TYPE_UNSPECIFIED_BIT | BAP_CONTEXT_TYPE_MEDIA_BIT,
			.add_metadata.len = 0,
		};

		bap_bc_src_set_subgroup(bcast_grp_lid, 0, &codec_id, &sgrp_cfg, &sgrp_meta);

		const uint16_t dp_id = GAPI_DP_ISOOSHM;

		/* Must be accessible to the BLE stack for the lifetime of the BIG -> statically allocated */
		static const bap_cfg_t stream_cfg_l = {
			.param = {
					.sampling_freq =
						BAP_SAMPLING_FREQ_UNKNOWN,  /* Inherited from subgroup */
					.frame_dur = BAP_FRAME_DUR_UNKNOWN, /* Inherited from subgroup */
					.frames_sdu = 0,                    /* Inherited from subgroup */
					.frame_octet = 0,                   /* Inherited from subgroup */
					.location_bf = GAF_LOC_FRONT_LEFT_BIT,
				},
			.add_cfg.len = 0};

		static const bap_cfg_t stream_cfg_r = {
			.param = {
					.sampling_freq =
						BAP_SAMPLING_FREQ_UNKNOWN,  /* Inherited from subgroup */
					.frame_dur = BAP_FRAME_DUR_UNKNOWN, /* Inherited from subgroup */
					.frames_sdu = 0,                    /* Inherited from subgroup */
					.frame_octet = 0,                   /* Inherited from subgroup */
					.location_bf = GAF_LOC_FRONT_RIGHT_BIT,
				},
			.add_cfg.len = 0};

		bap_bc_src_set_stream(bcast_grp_lid, 0, 0, dp_id, 0, &stream_cfg_l);
		bap_bc_src_set_stream(bcast_grp_lid, 1, 0, dp_id, 0, &stream_cfg_r);

		return 0;
	}

Periodic advertising
====================

Enable Periodic Advertising. This will set the device name and the broadcast name:

.. code-block:: c

	int broadcast_source_enable(void)
	{
		uint8_t ad_data[1 + sizeof(CONFIG_BLE_DEVICE_NAME)];

		ad_data[0] = sizeof(ad_data) - 1; /* Size of data following the size byte */
		ad_data[1] = 0x09;                /* Complete local name */

		memcpy(&ad_data[2], CONFIG_BLE_DEVICE_NAME, sizeof(ad_data) - 2);

		bap_bc_src_enable_pa(bcast_grp_lid, sizeof(ad_data), 0, ad_data, NULL,
						sizeof(CONFIG_BROADCAST_NAME) - 1,
						CONFIG_BROADCAST_NAME, 0, NULL);

		return 0;
	}
