.. _zas-connection-ble-audiosource:

################
Broadcast Source
################

|ble_audio_intro|. This sample demonstrates a source device.

All the audio samples share the same basic structure with all the other :ref:`BLE profile samples<zas-connection-ble-sample>`.
The aforementioned samples follow standard connection procedures defined in the Generic Access Profile(GAP).
Basic Audio Profile(BAP) API is used to configure and broadcast audio.

.. figure:: /images/alif_ble_source_flowchart.drawio.png

*******
Kconfig
*******

The source sample chooses |**Alif_BLE**| and |**Alif_LC3**| by setting a configuration flag on their *prj.conf* file.
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
The role is set to be a *broadcaster* to be able to advertise and broadcast audio for other devices without a connection process.

.. code-block:: c

	static const gapm_config_t gapm_cfg = {
		.role = GAP_ROLE_LE_BROADCASTER,
	};

.. note::
   All the other members of the configuration structure are assumed to be the same as with
   the basic profile config. When acting as a *broadcaster*, you don't need to set GAPM
   callbacks except the error callbacks.

******************
Audio Source Start
******************

Once GAPM configuration is complete, the broadcast source is configured and started
through the following steps:

1. Configure the BAP Broadcast Source module with callbacks
2. Configure advertising parameters (extended and periodic)
3. Create the broadcast group and fill the BASE (Broadcast Audio Source Endpoint) structure
4. Configure subgroup with codec settings and metadata
5. Configure and enable individual audio streams
6. Enable periodic advertising with device and broadcast names

The implementation:

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

Configure
=========

The BIG structure hierarchy::

    Broadcast Group (BIG)
    │
    ├── BIG Parameters (timing, PHY, reliability)
    │
    └── Subgroup(s)
        │
        ├── Codec Configuration (LC3, sampling rate, frame duration)
        ├── Metadata (context, language, etc.)
        │
        └── BIS Stream(s)
            │
            ├── Stream 0 (Left channel)
            ├── Stream 1 (Right channel)
            └── Stream N...

A BIG contains one or more subgroups, each with its own codec configuration.
Each subgroup contains one or more BIS (Broadcast Isochronous Stream) streams
that inherit the subgroup's codec settings but can specify individual audio
channel locations (e.g., left, right).

Configuring Broadcast Source module:

.. code-block:: c

	bap_bc_src_configure(&bap_bc_src_cbs);

This initializes the BAP Broadcast Source module with callback functions that handle
asynchronous events during the broadcast lifecycle.

* **BAP Broadcast Source Callbacks**:

  * **cb_cmp_evt**: Command complete callback - invoked when operations finish (enabling PA, enabling broadcast group, starting streaming)
  * **cb_info**: Broadcast source info callback - invoked when a broadcast group is successfully created, providing group information

**Step 1: Configure BIG parameters**

Define the Broadcast Isochronous Group (BIG) parameters according to the Bluetooth Core Specification.

.. code-block:: c

	const bap_bc_grp_param_t grp_param = {
		.sdu_intv_us = 10000,
		.max_sdu = CONFIG_ALIF_BLE_AUDIO_OCTETS_PER_CODEC_FRAME,
		.max_tlatency_ms = CONFIG_ALIF_BLE_AUDIO_MAX_TLATENCY,
		.packing = 0,
		.framing = ISO_UNFRAMED_MODE,
		.phy_bf = GAPM_PHY_TYPE_LE_2M,
		.rtn = CONFIG_ALIF_BLE_AUDIO_RTN
	};

**Step 2: Configure advertising parameters**

Set up extended and periodic advertising:

.. code-block:: c

	const bap_bc_adv_param_t adv_param = {
		.adv_intv_min_slot = 160,
		.adv_intv_max_slot = 160,
		.ch_map = ADV_ALL_CHNLS_EN,
		.phy_prim = GAPM_PHY_TYPE_LE_1M,
		.phy_second = GAPM_PHY_TYPE_LE_2M,
		.adv_sid = 1,
		.tx_pwr = -2,
		.own_addr_type = GAPM_STATIC_ADDR,
		.max_skip = 0,
		.send_tx_pwr = false,
	};

	const bap_bc_per_adv_param_t per_adv_param = {
		.adv_intv_min_frame = 160,
		.adv_intv_max_frame = 160,
	};

**Step 3: Create broadcast group**

Generate a broadcast ID and add the group. The Broadcast_ID is a 3-octet random value
that uniquely identifies the broadcast group according to the Bluetooth Core Specification.
It must be randomly generated to avoid collisions with other broadcast sources, and must
remain constant for the entire lifetime of the BIG.

.. code-block:: c

	bap_bcast_id_t bcast_id;
	sys_rand_get(bcast_id.id, sizeof(bcast_id.id));

	bap_bc_src_add_group(&bcast_id, NULL, 2, 1, &grp_param, &adv_param,
			     &per_adv_param, PRESENTATION_DELAY_US, &bcast_grp_lid);

**Step 4: Configure subgroup**

Set up the codec configuration and metadata for the subgroup. The subgroup defines the
codec parameters (LC3, sampling frequency, frame duration, frame size) that will be
inherited by all streams within this subgroup. The metadata specifies the audio context
(e.g., media playback). Note that ``location_bf = 0`` at the subgroup level means the
audio location is unspecified here and will be defined per-stream.

.. code-block:: c

	const gaf_codec_id_t codec_id = GAF_CODEC_ID_LC3;

	bap_cfg_t *sgrp_cfg = ke_malloc_user(sizeof(*sgrp_cfg), KE_MEM_PROFILE);
	sgrp_cfg->param = (bap_cfg_param_t){
		.location_bf = 0,
		.frame_octet = CONFIG_ALIF_BLE_AUDIO_OCTETS_PER_CODEC_FRAME,
		.frame_dur = IS_ENABLED(CONFIG_ALIF_BLE_AUDIO_FRAME_DURATION_10MS)
				? BAP_FRAME_DUR_10MS : BAP_FRAME_DUR_7_5MS,
		.frames_sdu = 0,
		.sampling_freq = audio_hz_to_bap_sampling_freq(CONFIG_ALIF_BLE_AUDIO_FS_HZ),
	};
	sgrp_cfg->add_cfg.len = 0;

	bap_cfg_metadata_t *sgrp_meta = ke_malloc_user(sizeof(*sgrp_meta), KE_MEM_PROFILE);
	sgrp_meta->param.context_bf = BAP_CONTEXT_TYPE_UNSPECIFIED_BIT | BAP_CONTEXT_TYPE_MEDIA_BIT;
	sgrp_meta->add_metadata.len = 0;

	bap_bc_src_set_subgroup(bcast_grp_lid, 0, &codec_id, sgrp_cfg, sgrp_meta);

**Step 5: Configure audio streams**

Set up left and right channel streams. Each stream represents an individual audio channel
and must specify its audio location (left or right). The streams inherit codec parameters
from the subgroup, so only the ``location_bf`` needs to be set. The datapath ID
``GAPI_DP_ISOOSHM`` specifies the ISO data path using shared memory for efficient audio
transfer between the application and the BLE controller.

.. code-block:: c

	const uint16_t dp_id = GAPI_DP_ISOOSHM;

	/* Configure left channel stream */
	bap_cfg_t *stream_cfg_l = ke_malloc_user(sizeof(*stream_cfg_l), KE_MEM_PROFILE);
	stream_cfg_l->param = (bap_cfg_param_t){
		.location_bf = GAF_LOC_FRONT_LEFT_BIT,
		/* Other parameters inherited from subgroup */
	};
	stream_cfg_l->add_cfg.len = 0;
	bap_bc_src_set_stream(bcast_grp_lid, 0, 0, dp_id, 0, stream_cfg_l);

	/* Repeat for right channel with GAF_LOC_FRONT_RIGHT_BIT */

Periodic advertising
====================

Enable Periodic Advertising. This will set the device name and the broadcast name:

.. code-block:: c

	uint8_t ad_data[1 + sizeof(CONFIG_BLE_DEVICE_NAME)];

	ad_data[0] = sizeof(ad_data) - 1;
	ad_data[1] = GAP_AD_TYPE_COMPLETE_NAME;

	memcpy(&ad_data[2], CONFIG_BLE_DEVICE_NAME, sizeof(ad_data) - 2);

	bap_bc_src_enable_pa(bcast_grp_lid, sizeof(ad_data), 0, ad_data, NULL,
			     sizeof(CONFIG_BROADCAST_NAME) - 1,
			     CONFIG_BROADCAST_NAME, 0, NULL);

.. note::
   Legacy advertising has a 31-byte payload limit. If the device name is too long,
   it may need to be shortened to fit within this constraint. In such cases, use
   ``GAP_AD_TYPE_SHORTENED_NAME`` instead of ``GAP_AD_TYPE_COMPLETE_NAME``.
   This example uses the complete name for demonstration purposes.
