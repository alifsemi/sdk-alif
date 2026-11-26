/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>
#include "bluetooth/le_audio/audio_utils.h"

#include "ke_mem.h"
#include "bap.h"
#include "bap_bc.h"
#include "bap_bc_src.h"
#include "broadcast_source.h"
#include "audio_datapath.h"

LOG_MODULE_REGISTER(broadcast_source, CONFIG_BROADCAST_SOURCE_LOG_LEVEL);

#define PRESENTATION_DELAY_US (CONFIG_LE_AUDIO_PRESENTATION_DELAY_MS * 1000)
#define SUBGROUP_ID           0
#define BIS_ID_BASE           0
#define INVALID_LID           0xFF

#if CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS >= 2
#define STREAM_LOCATIONS (GAF_LOC_FRONT_LEFT_BIT | GAF_LOC_FRONT_RIGHT_BIT)
#else
#define STREAM_LOCATIONS (GAF_LOC_FRONT_LEFT_BIT)
#endif

/* Where the controller is. This is not the user's request. A completion
 * moves this, then reconcile() takes the next step.
 */
typedef enum {
	SRC_STATE_IDLE = 0,
	SRC_STATE_CONFIGURED,
	SRC_STATE_PA_ENABLING,
	SRC_STATE_PA_ENABLED,
	SRC_STATE_GROUP_ENABLING,
	SRC_STATE_GROUP_ENABLED,
	SRC_STATE_STREAMING,
	SRC_STATE_STOPPING_STREAMING,
	SRC_STATE_DISABLING_GROUP,
	SRC_STATE_DISABLING_PA,
	SRC_STATE_REMOVING,
} src_state_t;


/* Broadcast source environment structure */
struct broadcast_source_env {
	/* State management */
	src_state_t state;

	/* Stream information */
	uint32_t stream_ids[CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS];
	uint8_t bcast_grp_lid;
	bool bap_source_modules_configured;

	/* Audio datapath configuration */
	struct audio_datapath_config datapath_cfg;
	bool datapath_cfg_valid;

	/* Advertising configuration */
	uint8_t adv_sid;
};

/* LL_ERR_CONN_REJ_LIMITED_RESOURCES. Create BIG returns this when the
 * controller cannot allocate the BIS while an ACL is up.
 */
#define BIG_ERR_LIMITED_RESOURCES 0x9D

/* What the user asked for. Controller completions never write this. */
enum src_desire {
	SRC_DESIRE_IDLE = 0,
	SRC_DESIRE_SOURCE,
};

/* The mode-switch link. Create BIG waits while it is up. */
static bool link_up;
static enum src_desire desire;
/* Create BIG was refused. Do not start again until the user asks, or the
 * link that caused the refusal goes down.
 */
static bool create_refused;

static void broadcast_source_reconcile(void);
static void broadcast_source_stop_audio(void);

static struct broadcast_source_env source_env = {
	.state = SRC_STATE_IDLE,
	.bcast_grp_lid = INVALID_LID,
	.bap_source_modules_configured = false,
	.datapath_cfg = {
		.i2s_dev = DEVICE_DT_GET(DT_ALIAS(i2s_bus)),
		.pres_delay_us = PRESENTATION_DELAY_US,
		.sampling_rate_hz = CONFIG_ALIF_BLE_AUDIO_FS_HZ,
		.octets_per_frame = 0, /* Will be set when configuring */
		.frame_duration_is_10ms = IS_ENABLED(CONFIG_ALIF_BLE_AUDIO_FRAME_DURATION_10MS),
	},
	.datapath_cfg_valid = true,
	.adv_sid = 0xFF
};

static K_SEM_DEFINE(src_stop_sem,     0, 8);
static K_SEM_DEFINE(src_disable_sem,  0, 1);
static K_SEM_DEFINE(src_dis_pa_sem,   0, 1);

static void *alloc_stream_config(uint32_t const location_bf)
{
	bap_cfg_t *p_stream_cfg = ke_malloc_user(sizeof(*p_stream_cfg), KE_MEM_PROFILE);

	if (!p_stream_cfg) {
		LOG_ERR("Failed to allocate memory for stream configuration");
		return NULL;
	}
	p_stream_cfg->param = (bap_cfg_param_t){
		.sampling_freq = BAP_SAMPLING_FREQ_UNKNOWN,
		.frame_dur     = BAP_FRAME_DUR_UNKNOWN,
		.frames_sdu    = 0,
		.frame_octet   = 0,
		.location_bf   = location_bf,
	};
	p_stream_cfg->add_cfg.len = 0;
	return p_stream_cfg;
}

/* Drop our copy of the group. The controller object is already gone,
 * or the remove that failed told us it was. Reconcile can start a new
 * one only from idle.
 */
static void broadcast_source_reset_local(void)
{
	memset(source_env.stream_ids, 0, sizeof(source_env.stream_ids));
	source_env.bcast_grp_lid = INVALID_LID;
	source_env.state = SRC_STATE_IDLE;
}

/* Ask the controller to delete the group. A success return only means
 * the command was accepted. Restarting here would let the later
 * completion wipe the next group, so the phase stays "removing" until
 * that completion. An error means the GAPI group is already gone and
 * there will be no completion, so the local id is forgotten now.
 */
static void broadcast_source_drop_group(void)
{
	uint16_t err;

	if (source_env.state == SRC_STATE_REMOVING) {
		return;
	}

	if (source_env.bcast_grp_lid == INVALID_LID) {
		broadcast_source_reset_local();
		return;
	}

	err = bap_bc_src_remove_group(source_env.bcast_grp_lid);

	if (err) {
		LOG_WRN("remove_group(%u) -> %u", source_env.bcast_grp_lid, err);
		/* Create BIG failure already removed the GAPI group, so
		 * this lid cannot be enabled again. There is no completion.
		 */
		broadcast_source_reset_local();
		return;
	}

	source_env.state = SRC_STATE_REMOVING;
	LOG_INF("remove_group(%u) accepted", source_env.bcast_grp_lid);
}

/* Periodic advertising has to be off before the group can be deleted.
 * If the controller refuses, advertising is already down and the group
 * can be removed.
 */
static void broadcast_source_disable_pa(void)
{
	if (source_env.bcast_grp_lid == INVALID_LID) {
		broadcast_source_reset_local();
		return;
	}

	source_env.state = SRC_STATE_DISABLING_PA;
	uint16_t err = bap_bc_src_disable_pa(source_env.bcast_grp_lid);

	if (err) {
		LOG_WRN("Disable PA returned %u", err);
		broadcast_source_drop_group();
	}
}

/* The BIG is up, so the legal way down is to stop streaming. Disable
 * and advertising-off are only the fallbacks when that is refused.
 * This does not change the request, so a failed channel start can
 * still be tried again once the group is gone.
 */
static void broadcast_source_stop_audio(void)
{
	uint16_t err;

	source_env.state = SRC_STATE_STOPPING_STREAMING;
	err = bap_bc_src_stop_streaming(source_env.bcast_grp_lid, 0xFFFFFFFF);

	if (err) {
		LOG_WRN("Failed to stop streaming immediately, err %u.", err);
		source_env.state = SRC_STATE_DISABLING_GROUP;
		err = bap_bc_src_disable(source_env.bcast_grp_lid, true);
		if (err) {
			LOG_WRN("Disable returned %u; falling back to PA disable", err);
			broadcast_source_disable_pa();
		}
	}
}

/* A completion only moves the phase. A result for a step we have
 * already left is ignored, so it cannot cancel a newer request.
 */
static void on_bap_bc_src_cmp_evt(uint8_t cmd_type, uint16_t status,
					uint8_t grp_lid, uint8_t sgrp_lid)
{
	ARG_UNUSED(sgrp_lid);

	switch (cmd_type) {
	case BAP_BC_SRC_CMD_TYPE_ENABLE_PA:
		LOG_INF("Periodic advertising %s (status %u)",
				status ? "enable done (err)" : "enabled", status);
		if (source_env.state != SRC_STATE_PA_ENABLING) {
			LOG_WRN("Ignore PA complete in state %d", source_env.state);
			break;
		}
		/* PA is up, or it failed and the group is still configured.
		 * The next step depends on what the user asked for.
		 */
		if (status == 0) {
			source_env.state = SRC_STATE_PA_ENABLED;
		} else {
			/* Do not immediately build another group. The user, or
			 * the link going down, clears this and tries again.
			 */
			create_refused = true;
			source_env.state = SRC_STATE_CONFIGURED;
		}
		broadcast_source_reconcile();
		break;

	case BAP_BC_SRC_CMD_TYPE_ENABLE:
		LOG_INF("Broadcast group %u %s, status %u", grp_lid,
				status ? "enable done (err)" : "enabled", status);
		if (source_env.state != SRC_STATE_GROUP_ENABLING ||
		    grp_lid != source_env.bcast_grp_lid) {
			LOG_WRN("Ignore enable complete for group %u, state %d",
				grp_lid, source_env.state);
			break;
		}
		if (status == 0) {
			uint16_t err;

			if (desire != SRC_DESIRE_SOURCE) {
				broadcast_source_stop_audio();
				break;
			}
			source_env.state = SRC_STATE_GROUP_ENABLED;
			err = bap_bc_src_start_streaming(grp_lid, 0xFFFFFFFF);
			if (err) {
				LOG_ERR("Failed to start streaming, err %u", err);
				broadcast_source_stop_audio();
			} else {
				LOG_INF("Start streaming command sent");
			}
		} else {
			/* Any refusal is latched. Otherwise reconcile builds
			 * another group as soon as this one is gone.
			 */
			if (status == BIG_ERR_LIMITED_RESOURCES) {
				LOG_INF("Create BIG rejected for resources");
			}
			create_refused = true;
			/* The BIG was not created. Drop periodic advertising
			 * or the next Create BIG is refused too.
			 */
			broadcast_source_disable_pa();
			broadcast_source_reconcile();
		}
		break;

	case BAP_BC_SRC_CMD_TYPE_START_STREAMING:
		LOG_INF("Started streaming (status %u)", status);
		if (source_env.state != SRC_STATE_GROUP_ENABLED) {
			LOG_WRN("Ignore streaming complete in state %d",
				source_env.state);
			break;
		}
		if (status == 0 && desire == SRC_DESIRE_SOURCE) {
			int ret = 0;
			size_t i;

			LOG_INF("Number of streams to start: %zu",
					ARRAY_SIZE(source_env.stream_ids));
			for (i = 0; i < ARRAY_SIZE(source_env.stream_ids); i++) {
				LOG_INF("Starting stream %zu with ID %u", i,
						source_env.stream_ids[i]);
				ret = audio_datapath_channel_start_source(
					source_env.stream_ids[i]);
				if (ret) {
					LOG_ERR("Failed to start stream %u, err %d",
						source_env.stream_ids[i], ret);
					break;
				}
			}
			if (ret) {
				audio_datapath_cleanup_source();
				broadcast_source_stop_audio();
			} else {
				source_env.state = SRC_STATE_STREAMING;
			}
		} else if (status == 0) {
			broadcast_source_stop_audio();
		} else {
			uint16_t err;

			source_env.state = SRC_STATE_DISABLING_GROUP;
			err = bap_bc_src_disable(source_env.bcast_grp_lid, true);
			if (err) {
				LOG_WRN("bap_bc_src_disable -> %u; will try PA disable",
					err);
				broadcast_source_disable_pa();
			}
		}
		broadcast_source_reconcile();
		break;

	case BAP_BC_SRC_CMD_TYPE_STOP_STREAMING:
		LOG_INF("Stopped streaming (status %u)", status);
		k_sem_give(&src_stop_sem);
		if (source_env.state != SRC_STATE_STOPPING_STREAMING) {
			break;
		}
		source_env.state = SRC_STATE_DISABLING_GROUP;
		{
			uint16_t err;

			err = bap_bc_src_disable(source_env.bcast_grp_lid, true);
			if (err) {
				LOG_WRN("bap_bc_src_disable -> %u; will try PA disable",
					err);
				broadcast_source_disable_pa();
			}
		}
		break;

	case BAP_BC_SRC_CMD_TYPE_DISABLE:
		LOG_INF("Broadcast group %u disabled (status %u)", grp_lid, status);
		k_sem_give(&src_disable_sem);
		if (source_env.state == SRC_STATE_DISABLING_GROUP) {
			broadcast_source_disable_pa();
		}
		break;

	case BAP_BC_SRC_CMD_TYPE_DISABLE_PA:
		LOG_INF("PA disabled (status %u)", status);
		k_sem_give(&src_dis_pa_sem);
		if (source_env.state != SRC_STATE_DISABLING_PA) {
			break;
		}
		broadcast_source_drop_group();
		broadcast_source_reconcile();
		break;

	case BAP_BC_SRC_CMD_TYPE_REMOVE_GROUP:
		LOG_INF("Broadcast group %u removed (status %u)", grp_lid, status);
		if (source_env.state != SRC_STATE_REMOVING ||
		    grp_lid != source_env.bcast_grp_lid) {
			LOG_WRN("Ignore remove complete for group %u, state %d",
				grp_lid, source_env.state);
			break;
		}
		broadcast_source_reset_local();
		broadcast_source_reconcile();
		break;

	default:
		LOG_WRN("Unexpected bap_bc_src cmd complete: %u (status %u)",
			cmd_type, status);
		break;
	}
}

static void on_bap_bc_src_info(uint8_t grp_lid, const gapi_bg_config_t *p_bg_cfg,
				uint8_t nb_bis, const uint16_t *p_conhdl)
{
	ARG_UNUSED(grp_lid); ARG_UNUSED(p_bg_cfg); ARG_UNUSED(nb_bis); ARG_UNUSED(p_conhdl);
	LOG_DBG("BAP BC SRC info");
}

static const bap_bc_src_cb_t bap_bc_src_cbs = {
	.cb_cmp_evt = on_bap_bc_src_cmp_evt,
	.cb_info    = on_bap_bc_src_info,
};

static int get_adv_param(bap_bc_adv_param_t *p_adv_param)
{
	if (source_env.adv_sid == 0xFF) {
		source_env.adv_sid = (uint8_t)(sys_rand32_get() & 0x0F);
	} else {
		source_env.adv_sid = (uint8_t)((source_env.adv_sid + 1) & 0x0F);
	}
	if (source_env.adv_sid > (0x0F - 0x01)) {
		source_env.adv_sid = 1;
	}

	p_adv_param->adv_intv_min_slot = 160;
	p_adv_param->adv_intv_max_slot = 160;
	p_adv_param->ch_map = ADV_ALL_CHNLS_EN;
	p_adv_param->phy_prim = GAPM_PHY_TYPE_LE_1M;
	p_adv_param->phy_second = GAPM_PHY_TYPE_LE_2M;
	p_adv_param->adv_sid = source_env.adv_sid;
#if CONFIG_ALIF_BLE_ROM_IMAGE_V1_0
	p_adv_param->max_tx_pwr = -2;
#else
	p_adv_param->tx_pwr = -2;
	p_adv_param->own_addr_type = GAPM_STATIC_ADDR;
	p_adv_param->max_skip = 0;
	p_adv_param->send_tx_pwr = false;
#endif
	return 0;
}

static int broadcast_source_configure_group(void)
{
	LOG_INF("Starting broadcast source group configuration");

	const bap_bc_grp_param_t grp_param = {
		.sdu_intv_us = IS_ENABLED(CONFIG_ALIF_BLE_AUDIO_FRAME_DURATION_10MS) ?
						10000 : 7500,
		.max_sdu         = CONFIG_ALIF_BLE_AUDIO_OCTETS_PER_CODEC_FRAME,
		.max_tlatency_ms = CONFIG_ALIF_BLE_AUDIO_MAX_TLATENCY,
		.packing         = 0,
		.framing         = ISO_UNFRAMED_MODE,
		.phy_bf          = GAPM_PHY_TYPE_LE_2M,
		.rtn             = CONFIG_ALIF_BLE_AUDIO_RTN
	};

	const gaf_codec_id_t codec_id = GAF_CODEC_ID_LC3;

	bap_bc_adv_param_t adv_param;
	int ret = get_adv_param(&adv_param);

	if (ret) {
		LOG_ERR("Failed to get advertising parameters, err %d", ret);
		return -1;
	}

	const bap_bc_per_adv_param_t per_adv_param = {
		.adv_intv_min_frame = 160,
		.adv_intv_max_frame = 160,
	};

	bap_bcast_id_t bcast_id;

	sys_rand_get(bcast_id.id, sizeof(bcast_id.id));

	uint16_t err = bap_bc_src_add_group(&bcast_id, NULL,
						CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS, 1,
						&grp_param, &adv_param, &per_adv_param,
						PRESENTATION_DELAY_US, &source_env.bcast_grp_lid);

	if (err) {
		LOG_ERR("Failed to add broadcast group, err %u", err);
		return -1;
	}

	LOG_INF("Broadcast group added, got local ID %u (SID=%u)",
			source_env.bcast_grp_lid, adv_param.adv_sid);

	/* Subgroup config */
	bap_cfg_t *sgrp_cfg = ke_malloc_user(sizeof(*sgrp_cfg), KE_MEM_PROFILE);

	if (!sgrp_cfg) {
		LOG_ERR("Failed to allocate memory for subgroup configuration");
		return -ENOMEM;
	}

	sgrp_cfg->param = (bap_cfg_param_t){
		.location_bf  = 0,
		.frame_octet  = CONFIG_ALIF_BLE_AUDIO_OCTETS_PER_CODEC_FRAME,
		.frame_dur    = IS_ENABLED(CONFIG_ALIF_BLE_AUDIO_FRAME_DURATION_10MS) ?
						BAP_FRAME_DUR_10MS : BAP_FRAME_DUR_7_5MS,
		.frames_sdu   = 0,
		.sampling_freq = audio_hz_to_bap_sampling_freq(CONFIG_ALIF_BLE_AUDIO_FS_HZ),
	};
	sgrp_cfg->add_cfg.len = 0;

	if (sgrp_cfg->param.sampling_freq == BAP_SAMPLING_FREQ_UNKNOWN) {
		LOG_ERR("Unsupported sampling frequency: %u Hz", CONFIG_ALIF_BLE_AUDIO_FS_HZ);
		return -1;
	}

	bap_cfg_metadata_t *sgrp_meta = ke_malloc_user(sizeof(*sgrp_meta), KE_MEM_PROFILE);

	if (!sgrp_meta) {
		ke_free(sgrp_cfg);
		LOG_ERR("Failed to allocate memory for subgroup metadata");
		return -ENOMEM;
	}
	sgrp_meta->param.context_bf = BAP_CONTEXT_TYPE_UNSPECIFIED_BIT | BAP_CONTEXT_TYPE_MEDIA_BIT;
	sgrp_meta->add_metadata.len = 0;

	err = bap_bc_src_set_subgroup(source_env.bcast_grp_lid, SUBGROUP_ID,
					&codec_id, sgrp_cfg, sgrp_meta);

	if (err) {
		LOG_ERR("Failed to set subgroup, err %u", err);
		return -1;
	}

	const uint16_t dp_id = GAPI_DP_ISOOSHM;
	size_t stream_bits = STREAM_LOCATIONS;

	LOG_INF("STREAM_LOCATIONS: 0x%zx, CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS: %d",
			stream_bits, CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS);

	for (size_t iter = 0; stream_bits != 0; stream_bits >>= 1, iter++) {
		if (!(stream_bits & 1)) {
			continue;
		}

		const uint32_t stream_id = iter + BIS_ID_BASE;

		LOG_INF("Creating stream %zu with ID %u", iter, stream_id);
		bap_cfg_t *stream_cfg = alloc_stream_config(0x1 << iter);

		if (!stream_cfg) {
			LOG_ERR("Failed to allocate memory for stream configuration");
			return -ENOMEM;
		}

		err = bap_bc_src_set_stream(source_env.bcast_grp_lid, stream_id,
						SUBGROUP_ID, dp_id, 0, stream_cfg);

		if (err) {
			ke_free(stream_cfg);
			LOG_ERR("Failed to set stream %u, err %u", stream_id, err);
			return -1;
		}

		err = audio_datapath_channel_create_source(sgrp_cfg->param.frame_octet, stream_id);

		if (err) {
			LOG_ERR("Failed to create source channel for stream %u, err %d",
					stream_id, err);
			return -1;
		}

		source_env.stream_ids[iter] = stream_id;
	}

	source_env.state = SRC_STATE_CONFIGURED;
	LOG_INF("Broadcast stream added successfully");
	return 0;
}

/* Periodic advertising has to be up before Create BIG. The phase stays
 * "enabling" until the completion, because it cannot be turned off
 * before that.
 */
static int broadcast_source_enable(void)
{
	LOG_INF("Enabling broadcast source PA and streaming");

	while (k_sem_take(&src_disable_sem, K_NO_WAIT) == 0) {
	}
	while (k_sem_take(&src_dis_pa_sem, K_NO_WAIT) == 0) {
	}
	while (k_sem_take(&src_stop_sem, K_NO_WAIT) == 0) {
	}

	uint8_t ad_data[1 + sizeof(CONFIG_BROADCAST_NAME)];

	ad_data[0] = sizeof(ad_data) - 1;
	ad_data[1] = GAP_AD_TYPE_COMPLETE_NAME;
	memcpy(&ad_data[2], CONFIG_BROADCAST_NAME, sizeof(ad_data) - 2);

	LOG_INF("Enabling PA for broadcast group %u", source_env.bcast_grp_lid);
	uint16_t err = bap_bc_src_enable_pa(source_env.bcast_grp_lid,
						sizeof(ad_data), 0,
						ad_data, NULL,
						sizeof(CONFIG_BROADCAST_NAME) - 1,
						CONFIG_BROADCAST_NAME,
						0, NULL);

	if (err) {
		LOG_ERR("Failed to enable PA, err %u", err);
		return -ENODEV;
	}

	source_env.state = SRC_STATE_PA_ENABLING;
	LOG_INF("PA enable issued; ENABLE will follow via callback");
	return 0;
}

int broadcast_source_configure(void)
{
	if (source_env.bap_source_modules_configured) {
		LOG_INF("BAP BC source modules already configured");
		return 0;
	}
	uint16_t err = bap_bc_src_configure(&bap_bc_src_cbs);

	if (err) {
		LOG_ERR("Failed to configure bap_bc_src, err %u", err);
		return -ENODEV;
	}
	source_env.bap_source_modules_configured = true;
	LOG_INF("BAP BC source modules configured successfully");
	return 0;
}

/* The only place a group is created. The encoder is deleted first:
 * a failed attempt leaves it allocated, and the next create then
 * returns -EALREADY. A failure is latched so reconcile does not
 * immediately build another group.
 */
static void broadcast_source_begin(void)
{
	int ret;

	LOG_INF("Starting broadcast source broadcasting");
	/* A previous attempt may have left the encoder allocated. */
	audio_datapath_cleanup_source();

	LOG_INF("Creating audio datapath for source");
	ret = audio_datapath_create_source(&source_env.datapath_cfg);
	if (ret) {
		LOG_ERR("Failed to create audio datapath, err %d", ret);
		create_refused = true;
		return;
	}

	LOG_INF("Configuring broadcast source group");
	ret = broadcast_source_configure_group();
	if (ret) {
		LOG_ERR("Failed to configure broadcast source group, err %d", ret);
		create_refused = true;
		if (source_env.bcast_grp_lid != INVALID_LID) {
			broadcast_source_drop_group();
		}
		return;
	}

	LOG_INF("Broadcast group configured, enabling PA and streaming");
	if (broadcast_source_enable() != 0) {
		create_refused = true;
		broadcast_source_drop_group();
	}
}

/* Periodic advertising is on, so Create BIG is the next step. Moving
 * the phase here stops reconcile from sending the command twice.
 */
static void broadcast_source_issue_big(void)
{
	uint16_t err;

	err = bap_bc_src_enable(source_env.bcast_grp_lid);
	if (err) {
		LOG_ERR("bap_bc_src_enable(%u) -> %u", source_env.bcast_grp_lid, err);
		create_refused = true;
		broadcast_source_disable_pa();
		return;
	}

	source_env.state = SRC_STATE_GROUP_ENABLING;
	LOG_INF("Create BIG issued");
}

/* The next command, from the request and the phase. Called after a
 * mode write, a link change, or a completion. It does not store a
 * "start later" of its own.
 */
static void broadcast_source_reconcile(void)
{
	static bool reconciling;
	int steps;

	if (reconciling) {
		return;
	}
	reconciling = true;

	for (steps = 0; steps < 6; steps++) {
		src_state_t before = source_env.state;

		if (desire == SRC_DESIRE_IDLE) {
			switch (source_env.state) {
			case SRC_STATE_CONFIGURED:
				broadcast_source_drop_group();
				break;
			case SRC_STATE_PA_ENABLED:
				broadcast_source_disable_pa();
				break;
			case SRC_STATE_GROUP_ENABLED:
			case SRC_STATE_STREAMING:
				LOG_INF("Cleaning up audio datapath for source");
				audio_datapath_cleanup_source();
				broadcast_source_stop_audio();
				break;
			case SRC_STATE_IDLE:
			case SRC_STATE_PA_ENABLING:
			case SRC_STATE_GROUP_ENABLING:
			case SRC_STATE_STOPPING_STREAMING:
			case SRC_STATE_DISABLING_GROUP:
			case SRC_STATE_DISABLING_PA:
			case SRC_STATE_REMOVING:
				break;
			}
		} else if (source_env.state == SRC_STATE_IDLE) {
			if (link_up) {
				LOG_INF("Waiting for the mode link to drop before Create BIG");
			} else if (!create_refused) {
				broadcast_source_begin();
			}
		} else if (source_env.state == SRC_STATE_PA_ENABLED) {
			/* The link can come back while advertising is turning
			 * on. Create BIG waits until it drops.
			 */
			if (link_up) {
				LOG_INF("Waiting for the mode link to drop before Create BIG");
			} else {
				broadcast_source_issue_big();
			}
		} else if (source_env.state == SRC_STATE_CONFIGURED && create_refused) {
			broadcast_source_drop_group();
		} else if (source_env.state == SRC_STATE_CONFIGURED) {
			if (broadcast_source_enable() != 0) {
				create_refused = true;
				broadcast_source_drop_group();
			}
		}

		if (source_env.state == before) {
			break;
		}
	}

	reconciling = false;
}

int broadcast_source_start_broadcasting(void)
{
	if (!source_env.bap_source_modules_configured) {
		LOG_ERR("BAP BC source modules not configured");
		return -ENODEV;
	}

	desire = SRC_DESIRE_SOURCE;
	create_refused = false;
	broadcast_source_reconcile();
	return 0;
}

int broadcast_source_start(void)
{
	int ret = broadcast_source_configure();

	if (ret != 0) {
		return ret;
	}
	return broadcast_source_start_broadcasting();
}

int broadcast_source_stop(void)
{
	desire = SRC_DESIRE_IDLE;
	LOG_INF("Cleaning up audio datapath for source");
	audio_datapath_cleanup_source();
	broadcast_source_reconcile();
	return 0;
}

void broadcast_source_set_connected(bool connected)
{
	bool was_up = link_up;

	link_up = connected;
	/* The refusal belonged to the link that just dropped. */
	if (was_up && !connected) {
		create_refused = false;
	}
	broadcast_source_reconcile();
}

void broadcast_source_on_disconnected(void)
{
	broadcast_source_reconcile();
}
