/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>

#include "ke_mem.h"
#include "bap.h"
#include "bap_bc.h"
#include "bap_bc_src.h"

#include "bluetooth/le_audio/audio_utils.h"

#include "auracast_source.h"
#include "audio_datapath.h"
#include "main.h"
#include "mic_source.h"

LOG_MODULE_REGISTER(auracast_source, CONFIG_AURACAST_SOURCE_LOG_LEVEL);

#define MIC_NODE DT_ALIAS(i2s_mic)

#define MIC_ENABLED (DT_NODE_EXISTS(MIC_NODE) && DT_NODE_EXISTS(DT_ALIAS(sw0)))

#define PRESENTATION_DELAY_US (CONFIG_LE_AUDIO_PRESENTATION_DELAY_MS * 1000)
#define SUBGROUP_ID           0
#define BIS_ID_BASE           0

#if CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS >= 2
#define STREAM_LOCATIONS (GAF_LOC_FRONT_LEFT_BIT | GAF_LOC_FRONT_RIGHT_BIT)
#else
#define STREAM_LOCATIONS (GAF_LOC_FRONT_LEFT_BIT)
#endif

static uint32_t stream_ids[CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS];
/* Local ID of the broadcast group */
static uint8_t bcast_grp_lid;
static bool bcast_configured;

K_SEM_DEFINE(bcast_ctrl_sem, 0, 1);

static void *alloc_stream_config(uint32_t const location_bf)
{
	/* NOTE: ke_malloc_user must be used to reserve buffer from correct heap! */
	bap_cfg_t *p_stream_cfg = ke_malloc_user(sizeof(*p_stream_cfg), KE_MEM_PROFILE);

	if (!p_stream_cfg) {
		LOG_ERR("Failed to allocate memory for stream configuration");
		return NULL;
	}

	p_stream_cfg->param = (bap_cfg_param_t){
		/* Just set specific location. Other parameters are inherited from subgroup */
		.sampling_freq = BAP_SAMPLING_FREQ_UNKNOWN,
		.frame_dur = BAP_FRAME_DUR_UNKNOWN,
		.frames_sdu = 0,
		.frame_octet = 0,
		.location_bf = location_bf,
	};
	p_stream_cfg->add_cfg.len = 0;

	return p_stream_cfg;
}

static void on_bap_bc_src_cmp_evt(const uint8_t cmd_type, const uint16_t status,
				  const uint8_t grp_lid, const uint8_t sgrp_lid)
{
	LOG_DBG("BAP BC SRC event complete, type %u status %u grp_lid %u sgrp_lid %u", cmd_type,
		status, grp_lid, sgrp_lid);

	switch (cmd_type) {
	case BAP_BC_SRC_CMD_TYPE_ENABLE_PA: {
		LOG_INF("Periodic advertising enabled");

		uint16_t err = bap_bc_src_enable(bcast_grp_lid);

		if (err) {
			LOG_ERR("Failed to enable broadcast source, err %u", err);
		}
		break;
	}

	case BAP_BC_SRC_CMD_TYPE_ENABLE: {
		LOG_INF("Broadcast group %u enabled", bcast_grp_lid);

		uint16_t err = bap_bc_src_start_streaming(bcast_grp_lid, 0xFFFFFFFF);

		if (err) {
			LOG_ERR("Failed to start streaming, err %u", err);
		}
		break;
	}

	case BAP_BC_SRC_CMD_TYPE_START_STREAMING: {
		LOG_INF("Started streaming");
		for (size_t iter = 0; iter < ARRAY_SIZE(stream_ids); iter++) {
			audio_datapath_channel_start_source(stream_ids[iter]);
		}
		break;
	}

	case BAP_BC_SRC_CMD_TYPE_STOP_STREAMING: {
		LOG_INF("Stopped streaming");
		k_sem_give(&bcast_ctrl_sem);
		break;
	}

	case BAP_BC_SRC_CMD_TYPE_DISABLE_PA: {
		LOG_INF("PA disabled");
		k_sem_give(&bcast_ctrl_sem);
		break;
	}

	case BAP_BC_SRC_CMD_TYPE_DISABLE: {
		LOG_INF("disabled");
		k_sem_give(&bcast_ctrl_sem);
		break;
	}

	case BAP_BC_SRC_CMD_TYPE_REMOVE_GROUP: {
		LOG_INF("group removed");
		k_sem_give(&bcast_ctrl_sem);
		break;
	}

	case BAP_BC_SRC_CMD_TYPE_UPDATE_METADATA: {
		LOG_INF("META updated");
		break;
	}

	default: {
		LOG_WRN("Unexpected bap_bc_src command complete event: %u", cmd_type);
		break;
	}
	}
}

static void on_bap_bc_src_info(const uint8_t grp_lid, const gapi_bg_config_t *p_bg_cfg,
			       const uint8_t nb_bis, const uint16_t *p_conhdl)
{
	LOG_DBG("BAP BC SRC info, grp %u, cfg %p, nb_bis %u, p_conhdl %p", grp_lid, p_bg_cfg,
		nb_bis, p_conhdl);
}

static const bap_bc_src_cb_t bap_bc_src_cbs = {.cb_cmp_evt = on_bap_bc_src_cmp_evt,
					       .cb_info = on_bap_bc_src_info};

static int get_adv_param(bap_bc_adv_param_t *p_adv_param)
{
	p_adv_param->adv_intv_min_slot = 160;
	p_adv_param->adv_intv_max_slot = 160;
	p_adv_param->ch_map = ADV_ALL_CHNLS_EN;
	p_adv_param->phy_prim = GAPM_PHY_TYPE_LE_1M;
	p_adv_param->phy_second = GAPM_PHY_TYPE_LE_2M;
	p_adv_param->adv_sid = 1;
	p_adv_param->tx_pwr = -2;
	p_adv_param->own_addr_type = GAPM_STATIC_ADDR;
	p_adv_param->max_skip = 0;
	p_adv_param->send_tx_pwr = false;
	return 0;
}

static int auracast_source_configure_group(uint32_t const octets_per_frame,
					   uint32_t const frame_rate_hz,
					   uint32_t const frame_duration_us)
{
	const bap_bc_grp_param_t grp_param = {.sdu_intv_us = frame_duration_us,
					      .max_sdu = octets_per_frame,
					      .max_tlatency_ms = CONFIG_ALIF_BLE_AUDIO_MAX_TLATENCY,
					      .packing = 0,
					      .framing = ISO_UNFRAMED_MODE,
					      .phy_bf = GAPM_PHY_TYPE_LE_2M,
					      .rtn = CONFIG_ALIF_BLE_AUDIO_RTN};

	const gaf_codec_id_t codec_id = GAF_CODEC_ID_LC3;

	bap_bc_adv_param_t adv_param;

	int ret = get_adv_param(&adv_param);

	if (ret) {
		LOG_ERR("Failed to get advertising parameters, err %d", ret);
		return -1;
	}

	const struct audio_datapath_config source_datapath_cfg = {
		.pres_delay_us = PRESENTATION_DELAY_US,
		.sampling_rate_hz = frame_rate_hz,
		.frame_duration_is_10ms = frame_duration_us == 10000 ? true : false,
		.mic_dev = MIC_ENABLED ? DEVICE_DT_GET_OR_NULL(MIC_NODE) : NULL,
	};

	audio_datapath_create_source(&source_datapath_cfg);

	const bap_bc_per_adv_param_t per_adv_param = {
		.adv_intv_min_frame = 160,
		.adv_intv_max_frame = 160,
	};

	bap_bcast_id_t bcast_id;

	sys_rand_get(bcast_id.id, sizeof(bcast_id.id));

	gaf_bcast_code_t code;
	const gaf_bcast_code_t *p_code_ptr = 0 < fill_auracast_encryption_key(&code) ? &code : NULL;
	uint16_t err = bap_bc_src_add_group(
		&bcast_id, p_code_ptr, CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS, 1, &grp_param,
		&adv_param, &per_adv_param, PRESENTATION_DELAY_US, &bcast_grp_lid);

	if (err) {
		LOG_ERR("Failed to add broadcast group, err %u", err);
		return -1;
	}

	LOG_DBG("Broadcast group added, got local ID %u", bcast_grp_lid);

	/* This struct must be accessible to the BLE stack for the lifetime of the BIG, so
	 * is statically allocated NOTE: ke_malloc_user must be used to reserve buffer from
	 * correct heap!
	 */
	bap_cfg_t *sgrp_cfg = ke_malloc_user(sizeof(*sgrp_cfg), KE_MEM_PROFILE);

	if (!sgrp_cfg) {
		LOG_ERR("Failed to allocate memory for subgroup configuration");
		return -ENOMEM;
	}
	sgrp_cfg->param = (bap_cfg_param_t){
		/* Location is unspecified at subgroup level */
		.location_bf = 0,
		.frame_octet = octets_per_frame,
		.frame_dur =
			grp_param.sdu_intv_us == 10000 ? BAP_FRAME_DUR_10MS : BAP_FRAME_DUR_7_5MS,
		/* 0 is unspecified, data will not be placed in BASE */
		.frames_sdu = 0,
		/* Convert to sampling frequency */
		.sampling_freq =
			audio_hz_to_bap_sampling_freq(source_datapath_cfg.sampling_rate_hz),
	};
	sgrp_cfg->add_cfg.len = 0;

	/* Validate sampling frequency conversion */
	if (sgrp_cfg->param.sampling_freq == BAP_SAMPLING_FREQ_UNKNOWN) {
		LOG_ERR("Unsupported sampling frequency: %u Hz",
			source_datapath_cfg.sampling_rate_hz);
		return -1;
	}

	/* This struct must be accessible to the BLE stack for the lifetime of the BIG, so
	 * is statically allocated NOTE: ke_malloc_user must be used to reserve buffer from
	 * correct heap!
	 */
	bap_cfg_metadata_t *sgrp_meta = ke_malloc_user(sizeof(*sgrp_meta), KE_MEM_PROFILE);

	if (!sgrp_meta) {
		ke_free(sgrp_cfg);
		LOG_ERR("Failed to allocate memory for subgroup metadata");
		return -ENOMEM;
	}
	sgrp_meta->param.context_bf = BAP_CONTEXT_TYPE_UNSPECIFIED_BIT | BAP_CONTEXT_TYPE_MEDIA_BIT;
	sgrp_meta->add_metadata.len = 0;

	err = bap_bc_src_set_subgroup(bcast_grp_lid, SUBGROUP_ID, &codec_id, sgrp_cfg, sgrp_meta);

	if (err) {
		LOG_ERR("Failed to set subgroup, err %u", err);
		return -1;
	}
	LOG_DBG("Broadcast subgroup added");

	const uint16_t dp_id = GAPI_DP_ISOOSHM;

	size_t stream_bits = STREAM_LOCATIONS;

	for (size_t iter = 0; stream_bits != 0; stream_bits >>= 1, iter++) {
		if (!(stream_bits & 1)) {
			continue;
		}

		const uint32_t stream_id = iter + BIS_ID_BASE;
		bap_cfg_t *stream_cfg = alloc_stream_config(0x1 << iter);

		if (!stream_cfg) {
			LOG_ERR("Failed to allocate memory for stream configuration");
			return -ENOMEM;
		}

		err = bap_bc_src_set_stream(bcast_grp_lid, stream_id, SUBGROUP_ID, dp_id, 0,
					    stream_cfg);
		if (err) {
			ke_free(stream_cfg);
			LOG_ERR("Failed to set stream %u, err %u", stream_id, err);
			return -1;
		}

		err = audio_datapath_channel_create_source(sgrp_cfg->param.frame_octet, stream_id);
		if (err) {
			return -1;
		}

		stream_ids[iter] = stream_id;
	}

	LOG_DBG("Broadcast stream added");

	return 0;
}

static int auracast_source_enable(void)
{
	const char *dev_name = get_device_name();
	const char *stream_name = get_stream_name();

	uint8_t ad_data[2 + strlen(dev_name)];

	ad_data[0] = sizeof(ad_data) - 1; /* Size of data following the size byte */
	ad_data[1] = GAP_AD_TYPE_COMPLETE_NAME;

	memcpy(&ad_data[2], dev_name, sizeof(ad_data) - 2);

	uint16_t err = bap_bc_src_enable_pa(bcast_grp_lid, sizeof(ad_data), 0, ad_data, NULL,
					    strlen(stream_name), stream_name, 0, NULL);

	if (err) {
		LOG_ERR("Failed to enable PA with error %u", err);
		return -1;
	}

	bcast_configured = true;

	return 0;
}

int auracast_source_start(uint32_t const octets_per_frame, uint32_t const frame_rate_hz,
			  uint32_t const frame_duration_us)
{
	int ret;

	auracast_source_stop();

	ret = configure_role(ROLE_AURACAST_SOURCE);
	if (ret == -EALREADY) {
		LOG_DBG("Auracast source already configured");
	} else if (ret) {
		return ret;
	}

	uint16_t err = bap_bc_src_configure(&bap_bc_src_cbs);

	if (err) {
		LOG_ERR("Error %u configuring bap_bc_src", err);
		return -1;
	}

	LOG_DBG("bap_bc_src configured");

	ret = auracast_source_configure_group(octets_per_frame, frame_rate_hz, frame_duration_us);
	if (ret) {
		LOG_ERR("Failed to configure auracast source group, err %d", ret);
		return ret;
	}

	LOG_DBG("Auracast group configured");

	return auracast_source_enable();
}

int auracast_source_stop(void)
{
	uint16_t err;

	if (!bcast_configured) {
		return 0;
	}

	audio_datapath_cleanup_source();

	err = bap_bc_src_stop_streaming(bcast_grp_lid, 0xFFFFFFFF);
	if (err) {
		LOG_ERR("Failed to stop streaming, err %u", err);
		return -EFAULT;
	}

	if (k_sem_take(&bcast_ctrl_sem, K_MSEC(1000)) != 0) {
		LOG_ERR("  FAIL! bcast_ctrl_sem timeout!");
		return -ETIMEDOUT;
	}

	err = bap_bc_src_disable(bcast_grp_lid, true);
	if (err) {
		LOG_ERR("Failed to disable src, err %u", err);
		return -EFAULT;
	}

	if (k_sem_take(&bcast_ctrl_sem, K_MSEC(1000)) != 0) {
		LOG_ERR("  FAIL! bcast_ctrl_sem timeout!");
		return -ETIMEDOUT;
	}

	err = bap_bc_src_remove_group(bcast_grp_lid);
	if (err) {
		LOG_ERR("Failed to disable auracast source, err %u", err);
		return -EFAULT;
	}

	if (k_sem_take(&bcast_ctrl_sem, K_MSEC(1000)) != 0) {
		LOG_ERR("  FAIL! bcast_ctrl_sem timeout!");
		return -ETIMEDOUT;
	}

	LOG_INF("Auracast source stopped");

	bcast_configured = false;

	return 0;
}

#if MIC_ENABLED

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});

#if CONFIG_I2S_SYNC_BUFFER_FORMAT_SEQUENTIAL
#error "Sequential buffer format is not supported"
#endif

#define BUTTON_DEBOUNCE_MS  5
#define PTT_MINIMUM_TIME_MS 200
#define BUTTON_INVERTED     true

static void set_led(bool const state)
{
	if (!led.port) {
		return;
	}
	gpio_pin_set_dt(&led, state);
}

static bool get_button_state(void)
{
	if (!button.port) {
		return false;
	}
	return !!gpio_pin_get_dt(&button) ^ BUTTON_INVERTED;
}

static void stop_push_to_talk(struct k_work *work)
{
	ARG_UNUSED(work);
	/* Stop push to talk.... */
	mic_i2s_control(false);
	set_led(false);
}

static K_WORK_DELAYABLE_DEFINE(stop_ptt_work, stop_push_to_talk);

static bool last_button_state = true;

static void debounce_expired(struct k_work *work)
{
	ARG_UNUSED(work);

	bool const button_state = get_button_state();

	if (button_state == last_button_state) {
		/* Ignore if state is not changed */
		return;
	}
	last_button_state = button_state;

	if (button_state) {
		/* pressed... */
		(void)k_work_cancel_delayable(&stop_ptt_work);
		mic_i2s_control(true);
		set_led(true);
		return;
	}

	/* Delay shutdown a bit to avoid weird behaviors */
	k_work_reschedule(&stop_ptt_work, K_MSEC(PTT_MINIMUM_TIME_MS));
}

static K_WORK_DELAYABLE_DEFINE(debounce_work, debounce_expired);

static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	k_work_reschedule(&debounce_work, K_MSEC(BUTTON_DEBOUNCE_MS));
}

static int configure_button(void)
{
	static struct gpio_callback button_cb_data;

	/* Configure button */
	if (!gpio_is_ready_dt(&button)) {
		LOG_WRN("Button is not ready or not configured... ignore");
		return 0;
	}

	if (gpio_pin_configure_dt(&button, GPIO_INPUT)) {
		LOG_ERR("Button configure failed");
		return -1;
	}

	if (gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH) != 0) {
		LOG_ERR("button int conf failed");
		return -1;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	if (gpio_add_callback(button.port, &button_cb_data) != 0) {
		LOG_ERR("cb add failed");
		return -1;
	}

	last_button_state = get_button_state();

	LOG_INF("push-to-talk functionality enabled");

	return 0;
}

static int configure_led(void)
{
	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE)) {
		LOG_ERR("led configure failed");
		return -1;
	}

	set_led(false);
	return 0;
}

/* Initialisation to perform pre-main */
static int auracast_source_init(void)
{
	if (configure_button()) {
		return -1;
	}

	if (configure_led()) {
		return -1;
	}

	return 0;
}
SYS_INIT(auracast_source_init, APPLICATION, 0);

#endif
