/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/* This application demonstrates the communication and control of a device
 * allowing to remotely control an LED, and to transmit the state of a button.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/sys/poweroff.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/uart.h>
#include <cmsis_core.h>
#include <soc.h>
#include <se_service.h>
#include <es0_power_manager.h>

#include <pm_rtss.h>
#include "alif_ble.h"
#include "gapm.h"
#include "gap_le.h"
#include "gapc_le.h"
#include "gapc_sec.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include "co_buf.h"
#include "prf.h"
#include "gatt_db.h"
#include "gatt_srv.h"
#include "ke_mem.h"

static uint8_t hello_arr[] = "HelloHello";
static uint8_t hello_arr_index __attribute__((noinit));

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
#define WAKEUP_SOURCE DT_NODELABEL(rtc0)
#else
#error "RTC0 not available"
#endif

#define EARLY_BOOT_CONSOLE_INIT 1

static uint32_t wakeup_reason;

/*
 * This function will be invoked in the PRE_KERNEL_2 phase of the init routine.
 * We can read the wakeup reason from reading the RESET STATUS register
 * and from the pending IRQ.
 */
static uint32_t ewic_psr, ewic_pend_0, ewic_pend_1;
static uint32_t nvic_ispr0, nvic_ispr1;

static int get_core_wakeup_reason(void)
{
	ewic_psr = EWIC->EWIC_PSR;
	ewic_pend_0 = EWIC->EWIC_PENDn[0];
	ewic_pend_1 = EWIC->EWIC_PENDn[1];

	nvic_ispr0 = NVIC->ISPR[0];
	nvic_ispr1 = NVIC->ISPR[1];
	if (nvic_ispr0 || nvic_ispr1) {
		wakeup_reason = 1;
	} else {
		wakeup_reason = 0;
	}
	return 0;
}
SYS_INIT(get_core_wakeup_reason, PRE_KERNEL_2, 0);

#if EARLY_BOOT_CONSOLE_INIT
/**
 * Use the HFOSC clock for the UART console
 */
#if DT_SAME_NODE(DT_NODELABEL(uart4), DT_CHOSEN(zephyr_console))
#define CONSOLE_UART_NUM 4
#elif DT_SAME_NODE(DT_NODELABEL(uart2), DT_CHOSEN(zephyr_console))
#define CONSOLE_UART_NUM 2
#elif DT_SAME_NODE(DT_NODELABEL(uart1), DT_CHOSEN(zephyr_console))
#define CONSOLE_UART_NUM 1
#else
#error "Specify the uart console number"
#endif

#define UART_CTRL_CLK_SEL_POS 8
static int app_pre_console_init(void)
{
	/* Enable HFOSC in CGU */
	sys_set_bits(CGU_CLK_ENA, BIT(23));

	/* Enable HFOSC for the UART console */
	sys_clear_bits(EXPSLV_UART_CTRL, BIT((CONSOLE_UART_NUM + UART_CTRL_CLK_SEL_POS)));
	return 0;
}
SYS_INIT(app_pre_console_init, PRE_KERNEL_1, 50);
#endif

/*
 * This function will be invoked in the PRE_KERNEL_2 phase of the init
 * routine to prevent sleep during startup.
 */
static int app_pre_kernel_init(void)
{
	pm_policy_state_lock_get(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
	return 0;
}
SYS_INIT(app_pre_kernel_init, PRE_KERNEL_2, 0);

/**
 * Set the RUN profile parameters for this application.
 */
static int app_set_run_params(bool trace_data)
{
	run_profile_t runp;
	int ret;

	runp.power_domains =
		PD_VBAT_AON_MASK | PD_SYST_MASK | PD_SSE700_AON_MASK | PD_DBSS_MASK | PD_SESS_MASK;
	runp.dcdc_voltage = 825;
	runp.dcdc_mode = DCDC_MODE_PFM_FORCED;
	runp.aon_clk_src = CLK_SRC_LFXO;
	runp.run_clk_src = CLK_SRC_PLL;
	runp.cpu_clk_freq = CLOCK_FREQUENCY_160MHZ;
	runp.phy_pwr_gating = LDO_PHY_MASK;
	runp.ip_clock_gating = LP_PERIPH_MASK;
	runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
	runp.scaled_clk_freq = SCALED_FREQ_XO_HIGH_DIV_38_4_MHZ;

	runp.memory_blocks = MRAM_MASK;
	runp.memory_blocks |= SRAM2_MASK | SRAM3_MASK;
	runp.memory_blocks |= SERAM_1_MASK | SERAM_2_MASK | SERAM_3_MASK | SERAM_4_MASK;
	runp.memory_blocks |=
		SRAM4_1_MASK | SRAM4_2_MASK | SRAM4_3_MASK | SRAM4_4_MASK; /* M55-HE ITCM */
	runp.memory_blocks |= SRAM5_1_MASK | SRAM5_2_MASK | SRAM5_3_MASK | SRAM5_4_MASK |
			      SRAM5_5_MASK; /* M55-HE DTCM */

	if (trace_data) {
		printk("SE(run): domains = %x\n", runp.power_domains);
		printk("SE(run): aon clk = %x run clk = %x\n", runp.aon_clk_src, runp.run_clk_src);
		printk("SE(run): CPU clk freq = %x scaled clk freq = %x\n", runp.cpu_clk_freq,
		       runp.scaled_clk_freq);
		printk("SE(run): MEMBLOCKS = %x\n", runp.memory_blocks);
	}

	ret = se_service_set_run_cfg(&runp);
	if (ret) {
		printk("SE: set_run_cfg failed = %d.\n", ret);
		return 0;
	}
	return 0;
}

#define NOT_USED 0
#define USED     1

static int app_set_off_params(bool trace_data)
{
	int ret;
	off_profile_t offp;

#if NOT_USED
	/*idle mode*/
	offp.power_domains = PD_VBAT_AON_MASK | PD_SYST_MASK | PD_SSE700_AON_MASK;
	offp.dcdc_voltage = DCDC_VOUT_0825;
	offp.dcdc_mode = DCDC_MODE_PFM_FORCED;
	offp.aon_clk_src = CLK_SRC_LFXO;
	offp.stby_clk_src = CLK_SRC_HFRC;
	offp.ip_clock_gating = LDO_PHY_MASK;
	offp.phy_pwr_gating = LDO_PHY_MASK;
	offp.stby_clk_freq = SCALED_FREQ_RC_STDBY_76_8_MHZ;
	offp.ewic_cfg = EWIC_RTC_A;
	offp.wakeup_events = WE_LPRTC;
	offp.vtor_address = SCB->VTOR;
	offp.vtor_address_ns = SCB->VTOR;
	offp.memory_blocks = MRAM_MASK;
	offp.memory_blocks |= SERAM_1_MASK | SERAM_2_MASK | SERAM_3_MASK | SERAM_4_MASK;
	offp.memory_blocks |= SRAM5_1_MASK | SRAM5_2_MASK | SRAM5_3_MASK;
#endif
#if NOT_USED
	/*standby mode*/
	offp.power_domains = PD_VBAT_AON_MASK | PD_SSE700_AON_MASK;
	offp.dcdc_voltage = DCDC_VOUT_0825;
	offp.dcdc_mode = DCDC_MODE_PFM_FORCED;
	offp.aon_clk_src = CLK_SRC_LFXO;
	offp.stby_clk_src = CLK_SRC_HFRC;
	offp.ip_clock_gating = LDO_PHY_MASK;
	offp.phy_pwr_gating = LDO_PHY_MASK;
	offp.stby_clk_freq = SCALED_FREQ_RC_STDBY_76_8_MHZ;
	offp.ewic_cfg = EWIC_RTC_A;
	offp.wakeup_events = WE_LPRTC;
	offp.vtor_address = SCB->VTOR;
	offp.vtor_address_ns = SCB->VTOR;
	offp.memory_blocks = MRAM_MASK;
	offp.memory_blocks |= SERAM_1_MASK | SERAM_2_MASK | SERAM_3_MASK | SERAM_4_MASK;
	offp.memory_blocks |= SRAM5_1_MASK | SRAM5_2_MASK | SRAM5_3_MASK;
#endif
#if USED
	/*stop mode*/
	offp.power_domains = PD_VBAT_AON_MASK;
	offp.dcdc_voltage = DCDC_VOUT_0825;
	offp.dcdc_mode = DCDC_MODE_OFF;
	offp.aon_clk_src = CLK_SRC_LFXO;
	offp.stby_clk_src = CLK_SRC_HFRC;
	offp.ip_clock_gating = 0;
	offp.phy_pwr_gating = 0;
	offp.stby_clk_freq = SCALED_FREQ_RC_STDBY_76_8_MHZ;
	offp.ewic_cfg = EWIC_RTC_A;
	offp.wakeup_events = WE_LPRTC;
	offp.vtor_address = SCB->VTOR;
	offp.vtor_address_ns = SCB->VTOR;
	offp.memory_blocks = MRAM_MASK;
	offp.memory_blocks |= SERAM_1_MASK | SERAM_2_MASK | SERAM_3_MASK | SERAM_4_MASK;
	offp.memory_blocks |= SRAM5_1_MASK | SRAM5_2_MASK;
#endif
#if NOT_USED
	offp.power_domains = PD_VBAT_AON_MASK | PD_SESS_MASK;
	offp.dcdc_voltage = DCDC_VOUT_0825;
	offp.dcdc_mode = DCDC_MODE_OFF;
	offp.aon_clk_src = CLK_SRC_LFXO;
	offp.stby_clk_src = CLK_SRC_HFRC;
	offp.ip_clock_gating = 0;
	offp.phy_pwr_gating = 0;
	offp.stby_clk_freq = SCALED_FREQ_RC_STDBY_76_8_MHZ;
	offp.ewic_cfg = EWIC_RTC_A;
	offp.wakeup_events = WE_LPRTC;
	offp.vtor_address = SCB->VTOR;
	offp.vtor_address_ns = SCB->VTOR;
	offp.memory_blocks = MRAM_MASK;
	offp.memory_blocks |= SERAM_1_MASK | SERAM_2_MASK | SERAM_3_MASK | SERAM_4_MASK;
	offp.memory_blocks |= SRAM5_1_MASK | SRAM5_2_MASK;
#endif
	if (trace_data) {
		printk("SE(off): domains = %x\n", offp.power_domains);
		printk("SE(off): aon clk = %x stby clk = %x\n", offp.aon_clk_src,
		       offp.stby_clk_src);
		printk("SE(off): Ewic = %x wakeup events = %x\n", offp.ewic_cfg,
		       offp.wakeup_events);
		printk("SE(off): VTOR = %x\n", offp.vtor_address);
		printk("SE(off): MEMBLOCKS = %x\n", offp.memory_blocks);
	}

	ret = se_service_set_off_cfg(&offp);
	if (ret) {
		printk("SE: set_off_cfg failed = %d.\n", ret);
		printk("ERROR: Can't establish SE connection, app exiting..\n");
		return ret;
	}

	return 0;
}

#define BT_CONN_STATE_CONNECTED    0x00
#define BT_CONN_STATE_DISCONNECTED 0x01
/* Service Definitions */
#define ATT_128_PRIMARY_SERVICE    ATT_16_TO_128_ARRAY(GATT_DECL_PRIMARY_SERVICE)
#define ATT_128_INCLUDED_SERVICE   ATT_16_TO_128_ARRAY(GATT_DECL_INCLUDE)
#define ATT_128_CHARACTERISTIC     ATT_16_TO_128_ARRAY(GATT_DECL_CHARACTERISTIC)
#define ATT_128_CLIENT_CHAR_CFG    ATT_16_TO_128_ARRAY(GATT_DESC_CLIENT_CHAR_CFG)
/* HELLO SERVICE and attribute 128 bit UUIDs */
#define HELLO_UUID_128_SVC                                                                         \
	{0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34,                                           \
	 0x45, 0x56, 0x67, 0x78, 0x89, 0x90, 0x00, 0x00}
#define HELLO_UUID_128_CHAR0                                                                       \
	{0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34,                                           \
	 0x45, 0x56, 0x67, 0x78, 0x89, 0x15, 0x00, 0x00}
#define HELLO_UUID_128_CHAR1                                                                       \
	{0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34,                                           \
	 0x45, 0x56, 0x67, 0x78, 0x89, 0x16, 0x00, 0x00}
#define HELLO_METAINFO_CHAR0_NTF_SEND 0x4321
#define ATT_16_TO_128_ARRAY(uuid)                                                                  \
	{(uuid) & 0xFF, (uuid >> 8) & 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

/* List of attributes in the service */
enum service_att_list {
	HELLO_IDX_SERVICE = 0,
	/* First characteristic is readable + supports notifications */
	HELLO_IDX_CHAR0_CHAR,
	HELLO_IDX_CHAR0_VAL,
	HELLO_IDX_CHAR0_NTF_CFG,
	/* Second characteristic is writable */
	HELLO_IDX_CHAR1_CHAR,
	HELLO_IDX_CHAR1_VAL,
	/* Number of items*/
	HELLO_IDX_NB,
};

static uint32_t conn_count __attribute__((noinit));
static uint8_t conn_status __attribute__((noinit));
/* Store advertising activity index for re-starting after disconnection */
static uint8_t conn_idx __attribute__((noinit));
static uint8_t adv_actv_idx __attribute__((noinit));
static struct service_env env __attribute__((noinit));

/* Load name from configuration file */
#define DEVICE_NAME "ALIF_PM"
static const char device_name[] = DEVICE_NAME;

/* Service UUID to pass into gatt_db_svc_add */
static const uint8_t hello_service_uuid[] = HELLO_UUID_128_SVC;

/* GATT database for the service */
static const gatt_att_desc_t hello_att_db[HELLO_IDX_NB] = {
	[HELLO_IDX_SERVICE] = {ATT_128_PRIMARY_SERVICE, ATT_UUID(16) | PROP(RD), 0},

	[HELLO_IDX_CHAR0_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[HELLO_IDX_CHAR0_VAL] = {HELLO_UUID_128_CHAR0, ATT_UUID(128) | PROP(RD) | PROP(N),
				 OPT(NO_OFFSET)},
	[HELLO_IDX_CHAR0_NTF_CFG] = {ATT_128_CLIENT_CHAR_CFG, ATT_UUID(16) | PROP(RD) | PROP(WR),
				     0},

	[HELLO_IDX_CHAR1_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[HELLO_IDX_CHAR1_VAL] = {HELLO_UUID_128_CHAR1, ATT_UUID(128) | PROP(WR),
				 OPT(NO_OFFSET) | sizeof(uint16_t)},
};

K_SEM_DEFINE(init_sem, 0, 1);
K_SEM_DEFINE(conn_sem, 0, 1);

/**
 * Bluetooth stack configuration
 */
static gapm_config_t gapm_cfg = {
	.role = GAP_ROLE_LE_PERIPHERAL,
	.pairing_mode = GAPM_PAIRING_DISABLE,
	.privacy_cfg = 0,
	.renew_dur = 1500,
	.private_identity.addr = {0xCF, 0xFE, 0xFB, 0xDE, 0x11, 0x07},
	.irk.key = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	.gap_start_hdl = 0,
	.gatt_start_hdl = 0,
	.att_cfg = 0,
	.sugg_max_tx_octets = GAP_LE_MAX_OCTETS,
	.sugg_max_tx_time = GAP_LE_MAX_TIME,
	.tx_pref_phy = GAP_PHY_ANY,
	.rx_pref_phy = GAP_PHY_ANY,
	.tx_path_comp = 0,
	.rx_path_comp = 0,
	.class_of_device = 0,  /* BT Classic only */
	.dflt_link_policy = 0, /* BT Classic only */
};

/* Environment for the service */
struct service_env {
	uint16_t start_hdl;
	uint8_t user_lid;
	uint8_t char0_val[250];
	uint8_t char1_val;
	bool ntf_ongoing;
	uint16_t ntf_cfg;
};

const gapc_le_con_param_nego_with_ce_len_t preferred_connection_param = {.ce_len_min = 5,
									 .ce_len_max = 10,
									 .hdr.interval_min = 800,
									 .hdr.interval_max = 800,
									 .hdr.latency = 0,
									 .hdr.sup_to = 800};

/* Macros */
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* function headers */
static uint16_t service_init(void);

/* Functions */
static uint16_t start_le_adv(uint8_t actv_idx)
{
	uint16_t err;

	gapm_le_adv_param_t adv_params = {
		.duration = 0, /* Advertise indefinitely */
	};

	err = gapm_le_start_adv(actv_idx, &adv_params);
	if (err) {
		LOG_ERR("Failed to start LE advertising with error %u", err);
	}
	return err;
}

/**
 * Bluetooth GAPM callbacks
 */
void on_gapc_proc_cmp_cb(uint8_t conidx, uint32_t metainfo, uint16_t status)
{
	printk("%s conn:%d status:%d\n", __func__, conidx, status);
}

static void on_le_connection_req(uint8_t conidx, uint32_t metainfo, uint8_t actv_idx, uint8_t role,
				 const gap_bdaddr_t *p_peer_addr,
				 const gapc_le_con_param_t *p_con_params, uint8_t clk_accuracy)
{
	LOG_INF("Connection request on index %u", conidx);
	gapc_le_connection_cfm(conidx, 0, NULL);

	printk("Connection parameters: interval %u, latency %u, supervision timeout %u\n",
	       p_con_params->interval, p_con_params->latency, p_con_params->sup_to);

	LOG_INF("Peer BD address %02X:%02X:%02X:%02X:%02X:%02X (conidx: %u)", p_peer_addr->addr[5],
		p_peer_addr->addr[4], p_peer_addr->addr[3], p_peer_addr->addr[2],
		p_peer_addr->addr[1], p_peer_addr->addr[0], conidx);

	conn_status = BT_CONN_STATE_CONNECTED;
	conn_idx = conidx;
	conn_count = 0;
	printk("BLE Connected conn:%d\n", conidx);

	k_sem_give(&conn_sem);

	LOG_DBG("Please enable notifications on peer device..");
}

static void on_key_received(uint8_t conidx, uint32_t metainfo, const gapc_pairing_keys_t *p_keys)
{
	LOG_WRN("Unexpected key received key on conidx %u", conidx);
}

static void on_disconnection(uint8_t conidx, uint32_t metainfo, uint16_t reason)
{
	uint16_t err;

	LOG_INF("Connection index %u disconnected for reason %u", conidx, reason);

	err = start_le_adv(adv_actv_idx);
	if (err) {
		LOG_ERR("Error restarting advertising: %u", err);
	} else {
		LOG_DBG("Restarting advertising");
	}

	conn_status = BT_CONN_STATE_DISCONNECTED;
	conn_idx = 0;
	conn_count = 0;
	printk("BLE disconnected conn:%d. Waiting new connection\n", conidx);
}

static void on_name_get(uint8_t conidx, uint32_t metainfo, uint16_t token, uint16_t offset,
			uint16_t max_len)
{
	const size_t device_name_len = sizeof(device_name) - 1;
	const size_t short_len = (device_name_len > max_len ? max_len : device_name_len);

	printk("%s\n", __func__);

	gapc_le_get_name_cfm(conidx, token, GAP_ERR_NO_ERROR, device_name_len, short_len,
			     (const uint8_t *)device_name);
}

static void on_appearance_get(uint8_t conidx, uint32_t metainfo, uint16_t token)
{
	/* Send 'unknown' appearance */
	printk("%s\n", __func__);
	gapc_le_get_appearance_cfm(conidx, token, GAP_ERR_NO_ERROR, 0);
}

static void on_pref_param_get(uint8_t conidx, uint32_t metainfo, uint16_t token)
{

	gapc_le_preferred_periph_param_t prefs = {
		.con_intv_min = preferred_connection_param.hdr.interval_min,
		.con_intv_max = preferred_connection_param.hdr.interval_max,
		.latency = preferred_connection_param.hdr.latency,
		.conn_timeout = 3200 * 2,
	};
	printk("%s\n", __func__);

	gapc_le_get_preferred_periph_params_cfm(conidx, token, GAP_ERR_NO_ERROR, prefs);
}

void on_bond_data_updated(uint8_t conidx, uint32_t metainfo, const gapc_bond_data_updated_t *p_data)
{
	printk("%s\n", __func__);
}
void on_auth_payload_timeout(uint8_t conidx, uint32_t metainfo)
{
	printk("%s\n", __func__);
}
void on_no_more_att_bearer(uint8_t conidx, uint32_t metainfo)
{
	printk("%s\n", __func__);
}
void on_cli_hash_info(uint8_t conidx, uint32_t metainfo, uint16_t handle, const uint8_t *p_hash)
{
	printk("%s\n", __func__);
}
void on_name_set(uint8_t conidx, uint32_t metainfo, uint16_t token, co_buf_t *p_buf)
{
	printk("%s\n", __func__);
}
void on_appearance_set(uint8_t conidx, uint32_t metainfo, uint16_t token, uint16_t appearance)
{
	printk("%s\n", __func__);
}

static const gapc_connection_req_cb_t gapc_con_cbs = {
	.le_connection_req = on_le_connection_req,
};

static const gapc_security_cb_t gapc_sec_cbs = {
	.key_received = on_key_received,
	/* All other callbacks in this struct are optional */
};

static const gapc_connection_info_cb_t gapc_con_inf_cbs = {
	.disconnected = on_disconnection,
	.name_get = on_name_get,
	.appearance_get = on_appearance_get,
	.slave_pref_param_get = on_pref_param_get,
	/* Other callbacks in this struct are optional */
	.bond_data_updated = on_bond_data_updated,
	.auth_payload_timeout = on_auth_payload_timeout,
	.no_more_att_bearer = on_no_more_att_bearer,
	.cli_hash_info = on_cli_hash_info,
	.name_set = on_name_set,
	.appearance_set = on_appearance_set,
};

void on_param_update_req(uint8_t conidx, uint32_t metainfo, const gapc_le_con_param_nego_t *p_param)
{
	printk("%s:%d\n", __func__, conidx);
}
void on_param_updated(uint8_t conidx, uint32_t metainfo, const gapc_le_con_param_t *p_param)
{
	printk("%s conn:%d\n", __func__, conidx);
}
void on_packet_size_updated(uint8_t conidx, uint32_t metainfo, uint16_t max_tx_octets,
			    uint16_t max_tx_time, uint16_t max_rx_octets, uint16_t max_rx_time)
{
	printk("%s conn:%d max_tx_octets:%d max_tx_time:%d  max_rx_octets:%d "
	       "max_rx_time:%d\n",
	       __func__, conidx, max_tx_octets, max_tx_time, max_rx_octets, max_rx_time);
}
void on_phy_updated(uint8_t conidx, uint32_t metainfo, uint8_t tx_phy, uint8_t rx_phy)
{
	printk("%s conn:%d tx_phy:%d rx_phy:%d\n", __func__, conidx, tx_phy, rx_phy);
}
void on_subrate_updated(uint8_t conidx, uint32_t metainfo,
			const gapc_le_subrate_t *p_subrate_params)
{
	printk("%s conn:%d\n", __func__, conidx);
}
/* All callbacks in this struct are optional */
static const gapc_le_config_cb_t gapc_le_cfg_cbs = {
	.param_update_req = on_param_update_req,
	.param_updated = on_param_updated,
	.packet_size_updated = on_packet_size_updated,
	.phy_updated = on_phy_updated,
	.subrate_updated = on_subrate_updated,
};

#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
static void on_gapm_err(uint32_t metainfo, uint8_t code)
{
	LOG_ERR("gapm error %d", code);
}
static const gapm_cb_t gapm_err_cbs = {
	.cb_hw_error = on_gapm_err,
};

static const gapm_callbacks_t gapm_cbs = {
	.p_con_req_cbs = &gapc_con_cbs,
	.p_sec_cbs = &gapc_sec_cbs,
	.p_info_cbs = &gapc_con_inf_cbs,
	.p_le_config_cbs = &gapc_le_cfg_cbs,
	.p_bt_config_cbs = NULL, /* BT classic so not required */
	.p_gapm_cbs = &gapm_err_cbs,
};
#else
static void on_gapm_err(enum co_error err)
{
	LOG_ERR("gapm error %d", err);
}
static const gapm_err_info_config_cb_t gapm_err_cbs = {
	.ctrl_hw_error = on_gapm_err,
};

static const gapm_callbacks_t gapm_cbs = {
	.p_con_req_cbs = &gapc_con_cbs,
	.p_sec_cbs = &gapc_sec_cbs,
	.p_info_cbs = &gapc_con_inf_cbs,
	.p_le_config_cbs = &gapc_le_cfg_cbs,
	.p_bt_config_cbs = NULL, /* BT classic so not required */
	.p_err_info_config_cbs = &gapm_err_cbs,
};
#endif /* !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 */

static uint16_t set_advertising_data(uint8_t actv_idx)
{
	uint16_t err;

	/* gatt service identifier */
	uint16_t svc[8] = {0xd123, 0xeabc, 0x785f, 0x1523, 0xefde, 0x1212, 0x1523, 0x0000};

	/* Name advertising length */
	const size_t device_name_len = sizeof(device_name) - 1;
	const uint16_t adv_device_name = GATT_HANDLE_LEN + device_name_len;

	/* Service advertising length */
	const uint16_t adv_uuid_svc = GATT_HANDLE_LEN + GATT_UUID_128_LEN;

	/* Create advertising data with necessary services */
	const uint16_t adv_len = adv_device_name + adv_uuid_svc;

	co_buf_t *p_buf;

	err = co_buf_alloc(&p_buf, 0, adv_len, 0);
	if (err != 0) {
		LOG_ERR("Buffer allocation failed");
		return err;
	}

	uint8_t *p_data = co_buf_data(p_buf);

	/* Device name data */
	p_data[0] = device_name_len + 1;
	p_data[1] = GAP_AD_TYPE_COMPLETE_NAME;
	memcpy(p_data + 2, device_name, device_name_len);

	/* Update data pointer */
	p_data = p_data + adv_device_name;

	/* Service UUID data */
	p_data[0] = GATT_UUID_128_LEN + 1;
	p_data[1] = GAP_AD_TYPE_COMPLETE_LIST_128_BIT_UUID;
	memcpy(p_data + 2, &svc, sizeof(svc));

	err = gapm_le_set_adv_data(actv_idx, p_buf);
	co_buf_release(p_buf);
	if (err) {
		LOG_ERR("Failed to set advertising data with error %u", err);
	}

	return err;
}

static uint16_t set_scan_data(uint8_t actv_idx)
{
	co_buf_t *p_buf;
	uint16_t err = co_buf_alloc(&p_buf, 0, 0, 0);

	__ASSERT(err == 0, "Buffer allocation failed");

	err = gapm_le_set_scan_response_data(actv_idx, p_buf);
	co_buf_release(p_buf); /* Release ownership of buffer so stack can free it when done */
	if (err) {
		LOG_ERR("Failed to set scan data with error %u", err);
	}

	return err;
}

/**
 * Advertising callbacks
 */
static void on_adv_actv_stopped(uint32_t metainfo, uint8_t actv_idx, uint16_t reason)
{
	LOG_DBG("Advertising activity index %u stopped for reason %u", actv_idx, reason);
}

static void on_adv_actv_proc_cmp(uint32_t metainfo, uint8_t proc_id, uint8_t actv_idx,
				 uint16_t status)
{
	if (status) {
		LOG_ERR("Advertising activity process completed with error %u", status);
		return;
	}

	switch (proc_id) {
	case GAPM_ACTV_CREATE_LE_ADV:
		LOG_DBG("Advertising activity is created");
		adv_actv_idx = actv_idx;
		set_advertising_data(actv_idx);
		break;

	case GAPM_ACTV_SET_ADV_DATA:
		LOG_DBG("Advertising data is set");
		set_scan_data(actv_idx);
		break;

	case GAPM_ACTV_SET_SCAN_RSP_DATA:
		LOG_DBG("Scan data is set");
		start_le_adv(actv_idx);
		break;

	case GAPM_ACTV_START:
		LOG_DBG("Advertising was started");
		k_sem_give(&init_sem);
		break;

	default:
		LOG_WRN("Unexpected GAPM activity complete, proc_id %u", proc_id);
		break;
	}
}

static void on_adv_created(uint32_t metainfo, uint8_t actv_idx, int8_t tx_pwr)
{
	LOG_DBG("Advertising activity created, index %u, selected tx power %d", actv_idx, tx_pwr);
}

static const gapm_le_adv_cb_actv_t le_adv_cbs = {
	.hdr.actv.stopped = on_adv_actv_stopped,
	.hdr.actv.proc_cmp = on_adv_actv_proc_cmp,
	.created = on_adv_created,
};

static uint16_t create_advertising(void)
{
	uint16_t err;

	gapm_le_adv_create_param_t adv_create_params = {
		.prop = GAPM_ADV_PROP_UNDIR_CONN_MASK,
		.disc_mode = GAPM_ADV_MODE_GEN_DISC,
#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
		.tx_pwr = 0,
#else
		.max_tx_pwr = 0,
#endif /* !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 */
		.filter_pol = GAPM_ADV_ALLOW_SCAN_ANY_CON_ANY,
		.prim_cfg = {
				.adv_intv_min = 2500,
				.adv_intv_max = 2500,
				.ch_map = ADV_ALL_CHNLS_EN,
				.phy = GAPM_PHY_TYPE_LE_1M,
			},
	};

	err = gapm_le_create_adv_legacy(0, GAPM_STATIC_ADDR, &adv_create_params, &le_adv_cbs);
	if (err) {
		LOG_ERR("Error %u creating advertising activity", err);
	}

	return err;
}

/* Add service to the stack */
static void server_configure(void)
{
	uint16_t err;

	err = service_init();

	if (err) {
		LOG_ERR("Error %u adding profile", err);
	}
}

void on_gapm_process_complete(uint32_t metainfo, uint16_t status)
{
	if (status) {
		LOG_ERR("gapm process completed with error %u", status);
		return;
	}

	server_configure();

	LOG_DBG("gapm process completed successfully");

	create_advertising();
}

/* Service callbacks */
static void on_att_read_get(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
			    uint16_t offset, uint16_t max_length)
{
	co_buf_t *p_buf = NULL;
	uint16_t status = GAP_ERR_NO_ERROR;
	uint16_t att_val_len = 0;
	void *att_val = NULL;

	do {
		if (offset != 0) {
			/* Long read not supported for any characteristics within this service */
			status = ATT_ERR_INVALID_OFFSET;
			break;
		}

		uint8_t att_idx = hdl - env.start_hdl;

		switch (att_idx) {
		case HELLO_IDX_CHAR0_VAL:
			att_val_len = CONFIG_DATA_STRING_LENGTH;
			uint8_t loop_count = (CONFIG_DATA_STRING_LENGTH / 5);

			if (CONFIG_DATA_STRING_LENGTH % 5) {
				loop_count += 1;
			}
			for (int i = 0; i < loop_count; i++) {
				memcpy(env.char0_val + i * 5, &hello_arr[hello_arr_index], 5);
			}
			att_val = env.char0_val;
			LOG_DBG("read hello text");
			break;

		case HELLO_IDX_CHAR0_NTF_CFG:
			att_val_len = sizeof(env.ntf_cfg);
			att_val = &env.ntf_cfg;
			break;

		default:
			break;
		}

		if (att_val == NULL) {
			status = ATT_ERR_REQUEST_NOT_SUPPORTED;
			break;
		}

		status = co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, att_val_len,
				      GATT_BUFFER_TAIL_LEN);
		if (status != CO_BUF_ERR_NO_ERROR) {
			status = ATT_ERR_INSUFF_RESOURCE;
			break;
		}

		memcpy(co_buf_data(p_buf), att_val, att_val_len);
	} while (0);

	/* Send the GATT response */
	gatt_srv_att_read_get_cfm(conidx, user_lid, token, status, att_val_len, p_buf);
	if (p_buf != NULL) {
		co_buf_release(p_buf);
	}
}

static void on_att_val_set(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
			   uint16_t offset, co_buf_t *p_data)
{
	uint16_t status = GAP_ERR_NO_ERROR;

	do {
		if (offset != 0) {
			/* Long write not supported for any characteristics in this service */
			status = ATT_ERR_INVALID_OFFSET;
			break;
		}

		uint8_t att_idx = hdl - env.start_hdl;

		switch (att_idx) {
		case HELLO_IDX_CHAR1_VAL: {
			if (sizeof(env.char1_val) != co_buf_data_len(p_data)) {
				LOG_DBG("Incorrect buffer size");
				status = ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
			} else {
				memcpy(&env.char1_val, co_buf_data(p_data), sizeof(env.char1_val));
				LOG_DBG("TOGGLE LED, state %d", env.char1_val);
			}
			break;
		}

		case HELLO_IDX_CHAR0_NTF_CFG: {
			if (sizeof(uint16_t) != co_buf_data_len(p_data)) {
				LOG_DBG("Incorrect buffer size");
				status = ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
			} else {
				uint16_t cfg;

				memcpy(&cfg, co_buf_data(p_data), sizeof(uint16_t));
				if (PRF_CLI_START_NTF == cfg || PRF_CLI_STOP_NTFIND == cfg) {
					env.ntf_cfg = cfg;
				} else {
					/* Indications not supported */
					status = ATT_ERR_REQUEST_NOT_SUPPORTED;
				}
			}
			break;
		}

		default:
			status = ATT_ERR_REQUEST_NOT_SUPPORTED;
			break;
		}
	} while (0);

	/* Send the GATT write confirmation */
	gatt_srv_att_val_set_cfm(conidx, user_lid, token, status);
}

static void on_event_sent(uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint16_t status)
{
	if (metainfo == HELLO_METAINFO_CHAR0_NTF_SEND) {
		env.ntf_ongoing = false;
	}
}

static const gatt_srv_cb_t gatt_cbs = {
	.cb_att_event_get = NULL,
	.cb_att_info_get = NULL,
	.cb_att_read_get = on_att_read_get,
	.cb_att_val_set = on_att_val_set,
	.cb_event_sent = on_event_sent,
};

/*
 * Service functions
 */
static uint16_t service_init(void)
{
	uint16_t status;

	/* Register a GATT user */
	status = gatt_user_srv_register(CFG_MAX_LE_MTU, 0, &gatt_cbs, &env.user_lid);
	if (status != GAP_ERR_NO_ERROR) {
		return status;
	}

	/* Add the GATT service */
	status = gatt_db_svc_add(env.user_lid, SVC_UUID(128), hello_service_uuid, HELLO_IDX_NB,
				 NULL, hello_att_db, HELLO_IDX_NB, &env.start_hdl);
	if (status != GAP_ERR_NO_ERROR) {
		gatt_user_unregister(env.user_lid);
		return status;
	}

	return GAP_ERR_NO_ERROR;
}

static uint16_t service_notification_send(uint32_t conidx_mask)
{
	co_buf_t *p_buf;
	uint16_t status;
	uint8_t conidx = 0;

	/* Cannot send another notification unless previous one is completed */
	if (env.ntf_ongoing) {
		return PRF_ERR_REQ_DISALLOWED;
	}

	/* Check notification subscription */
	if (env.ntf_cfg != PRF_CLI_START_NTF) {
		return PRF_ERR_NTF_DISABLED;
	}

	/* Get a buffer to put the notification data into */
	status = co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, CONFIG_DATA_STRING_LENGTH,
			      GATT_BUFFER_TAIL_LEN);
	if (status != CO_BUF_ERR_NO_ERROR) {
		return GAP_ERR_INSUFF_RESOURCES;
	}

	uint8_t loop_count = (CONFIG_DATA_STRING_LENGTH / 5);

	if (CONFIG_DATA_STRING_LENGTH % 5) {
		loop_count += 1;
	}
	for (int i = 0; i < loop_count; i++) {
		memcpy(env.char0_val + i * 5, &hello_arr[hello_arr_index], 5);
	}

	memcpy(co_buf_data(p_buf), env.char0_val, CONFIG_DATA_STRING_LENGTH);
	hello_arr_index++;
	if (hello_arr_index > 4) {
		hello_arr_index = 0;
	}

	status = gatt_srv_event_send(conidx, env.user_lid, HELLO_METAINFO_CHAR0_NTF_SEND,
				     GATT_NOTIFY, env.start_hdl + HELLO_IDX_CHAR0_VAL, p_buf);

	co_buf_release(p_buf);

	if (status == GAP_ERR_NO_ERROR) {
		env.ntf_ongoing = true;
	}

	return status;
}

static uint32_t boot_status __attribute__((noinit));
#define COLD_BOOT_DONE 0xea014012

static int app_se_configuration(void)
{
	int ret;

	/* This needs to be done before UART configuration.
	 */

	ret = se_service_sync();
	if (ret) {
		printk("SE: not responding to service calls %d\n", ret);
		return 0;
	}

	ret = app_set_run_params(false);
	if (ret) {
		printk("ERROR: app exiting..\n");
		return 0;
	}

	if (boot_status != COLD_BOOT_DONE) {
		ret = app_set_off_params(false);
		if (ret) {
			printk("ERROR: app exiting..\n");
			return 0;
		}
	}
	return 0;
}

int main(void)
{
	uint16_t ble_status;
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	int ret;

	/* Start up bluetooth host stack. */
	ble_status = alif_ble_enable(NULL);

	app_se_configuration();

	if (boot_status != COLD_BOOT_DONE) {
		printk("BLE Sleep demo\n");
	}

	if (!device_is_ready(wakeup_dev)) {
		printk("%s: device not ready.\n", wakeup_dev->name);
		printk("ERROR: app exiting..\n");
		return 0;
	}

	ret = counter_start(wakeup_dev);
	if (ret) {
		printk("Failed to start counter (err %d)", ret);
		printk("ERROR: app exiting..\n");
		return 0;
	}

	if (ble_status == 0) {
		/* BLE initialized first time */
		hello_arr_index = 0;
		conn_count = 0;
		conn_idx = 0;
		memset(&env, 0, sizeof(struct service_env));
		conn_status = BT_CONN_STATE_DISCONNECTED;

		/* Generate random address */
		se_service_get_rnd_num(&gapm_cfg.private_identity.addr[3], 3);
		ble_status = gapm_configure(0, &gapm_cfg, &gapm_cbs, on_gapm_process_complete);
		if (ble_status) {
			printk("gapm_configure error %u", ble_status);
			return -1;
		}

		printk("Waiting for initial BLE init...\n");
		k_sem_take(&init_sem, K_FOREVER);
		printk("Init complete!\n");

		printk("Wait for debgger or connecion without sleeps\n");
		k_sleep(K_SECONDS(10));
	}

	if (nvic_ispr1 & 0x4000000 && conn_status == BT_CONN_STATE_CONNECTED) {
		/* RTC wakeups when connection is active */
		bool sleep_in_subscription = true;

		conn_count++;
		if (conn_count == 2) {
			uint16_t ret = gapc_le_update_params(
				conn_idx, 0, &preferred_connection_param, on_gapc_proc_cmp_cb);
			printk("Update connection ret:%d\n", ret);
		}
		while ((env.ntf_cfg == PRF_CLI_START_NTF) && (!env.ntf_ongoing)) {
			/* Subscription is active */
			printk("Data subscribed\n");
			service_notification_send(UINT32_MAX);
			if (conn_status != BT_CONN_STATE_CONNECTED || sleep_in_subscription) {
				break;
			}
			k_sleep(K_MSEC(2150));
			conn_count++;
			if (conn_count == 2) {
				uint16_t ret = gapc_le_update_params(conn_idx, 0,
								     &preferred_connection_param,
								     on_gapc_proc_cmp_cb);
				printk("Update connection ret:%d\n", ret);
			}
		}
	} else if (nvic_ispr1 & 0x40000) {
		printk("Uart wakeup\n");
		k_sleep(K_MSEC(10));
	} else {
		/* Wakeup without reason so lets wait some for BLE handling */
		printk("No wakeup reason\n");
		k_sleep(K_MSEC(10));
	}
	boot_status = COLD_BOOT_DONE;
	if (IS_ENABLED(CONFIG_SLEEP_ENABLED)) {
		pm_policy_state_lock_put(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
		if (conn_status == BT_CONN_STATE_CONNECTED) {
			k_sleep(K_MSEC(2150));
		}
		k_sleep(K_SECONDS(30));
	}
	while (1) {
		k_sleep(K_MSEC(2150));
		if (conn_status == BT_CONN_STATE_CONNECTED) {
			conn_count++;
			if (conn_count == 2) {
				uint16_t ret = gapc_le_update_params(conn_idx, 0,
								     &preferred_connection_param,
								     on_gapc_proc_cmp_cb);
				printk("Update connection ret:%d\n", ret);
			}
			/* Update text at 2.15 second periods */
			service_notification_send(UINT32_MAX);
		}
	}
	return 0;
}
