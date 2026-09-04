/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#include <errno.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/ztest.h>
#include "test_device.h"

#if defined(CONFIG_BOARD_ALIF_B1_DK) || defined(CONFIG_BOARD_ALIF_E1C_DK)
#define TEST_BOARD_BALLETTO 1
#elif defined(CONFIG_RTSS_HE) || defined(CONFIG_RTSS_HP)
#define TEST_BOARD_ENSEMBLE 1
#else
#error "Unsupported board for this test"
#endif

#if defined(TEST_BOARD_BALLETTO)
#include <zephyr/dt-bindings/pinctrl/balletto-pinctrl.h>
#define TEST_DEVICE0		DT_NODELABEL(uart3)
#define TEST_DEVICE1		DT_NODELABEL(uart5)
/* UART2_TX_A is function 2 on Balletto / E1C */
#define DEV0_PIN1_FUNCTION	2U
#elif defined(TEST_BOARD_ENSEMBLE)
#include <zephyr/dt-bindings/pinctrl/ensemble-pinctrl.h>
#define TEST_DEVICE0		DT_NODELABEL(uart2)
#define TEST_DEVICE1		DT_NODELABEL(uart6)
/* UART2_TX_A is function 1 on Ensemble */
#define DEV0_PIN1_FUNCTION	1U
#endif

PINCTRL_DT_DEV_CONFIG_DECLARE(TEST_DEVICE0);
static struct pinctrl_dev_config *pcfg0 = PINCTRL_DT_DEV_CONFIG_GET(TEST_DEVICE0);

PINCTRL_DT_DEV_CONFIG_DECLARE(TEST_DEVICE1);
static struct pinctrl_dev_config *pcfg1 = PINCTRL_DT_DEV_CONFIG_GET(TEST_DEVICE1);

/* Device-0 default: two pins { P1_0 GPIO+DSC=1, P1_1 UART2_TX_A+DSC=2 }. */
struct expected_pin {
	uint8_t port;		/* PIN_P1_x value (bits 3:9 of raw pin) */
	uint8_t function;	/* bits 0:2 */
	uint8_t dsc;		/* bits 19:20 */
};

static const struct expected_pin dev0_default_pins[] = {
	{ .port = PIN_P1_0, .function = 0, .dsc = 1 },	/* GPIO */
	{ .port = PIN_P1_1, .function = DEV0_PIN1_FUNCTION, .dsc = 2 },
};

static const struct expected_pin dev1_default_pins[] = {
	{ .port = PIN_P1_2, .function = 0, .dsc = 0 },
	{ .port = PIN_P1_3, .function = 0, .dsc = 0 },
	{ .port = PIN_P1_4, .function = 0, .dsc = 0 },
};

static const struct expected_pin dev1_mystate_pins[] = {
	{ .port = PIN_P1_2, .function = 0, .dsc = 0 },
	{ .port = PIN_P1_3, .function = 0, .dsc = 1 },
	{ .port = PIN_P1_4, .function = 0, .dsc = 2 },
};

/*
 * Extended pin expectations including all pad-config fields.
 * Used by test_pad_config_fields to verify REN, STE, SRE, ODS, DRV.
 *
 * Defaults from DT binding (when not explicitly set in overlay):
 *   read-enable = 1, schmitt-enable = 1, slew-rate = 0,
 *   driver-state-control = 0, drive-strength = 4 (enum idx 1), driver = 0.
 */
struct expected_pad_config {
	uint8_t port;
	uint8_t function;
	uint8_t dsc;
	uint8_t ren;	/* read enable (bit 16) */
	uint8_t ste;	/* schmitt trigger enable (bit 17) */
	uint8_t sre;	/* slew rate enable (bit 18) */
	uint8_t ods;	/* output drive strength (bits 21:22) */
	uint8_t drv;	/* driver type (bit 23) */
};

/* dev0 default: all pad-config fields at DT defaults except DSC. */
static const struct expected_pad_config dev0_default_padcfg[] = {
	{ .port = PIN_P1_0, .function = 0, .dsc = 1,
	  .ren = 1, .ste = 1, .sre = 0, .ods = 1, .drv = 0 },
	{ .port = PIN_P1_1, .function = DEV0_PIN1_FUNCTION, .dsc = 2,
	  .ren = 1, .ste = 1, .sre = 0, .ods = 1, .drv = 0 },
};

/* dev1 default: all pad-config fields at DT defaults. */
static const struct expected_pad_config dev1_default_padcfg[] = {
	{ .port = PIN_P1_2, .function = 0, .dsc = 0,
	  .ren = 1, .ste = 1, .sre = 0, .ods = 1, .drv = 0 },
	{ .port = PIN_P1_3, .function = 0, .dsc = 0,
	  .ren = 1, .ste = 1, .sre = 0, .ods = 1, .drv = 0 },
	{ .port = PIN_P1_4, .function = 0, .dsc = 0,
	  .ren = 1, .ste = 1, .sre = 0, .ods = 1, .drv = 0 },
};

/*
 * dev1 mystate: non-default pad-config from overlay.
 *   group0: read-enable=0
 *   group1: DSC=1, slew-rate=1, drive-strength=12 (enum idx 3)
 *   group2: DSC=2, schmitt-enable=0, driver=1
 */
static const struct expected_pad_config dev1_mystate_padcfg[] = {
	{ .port = PIN_P1_2, .function = 0, .dsc = 0,
	  .ren = 0, .ste = 1, .sre = 0, .ods = 1, .drv = 0 },
	{ .port = PIN_P1_3, .function = 0, .dsc = 1,
	  .ren = 1, .ste = 1, .sre = 1, .ods = 3, .drv = 0 },
	{ .port = PIN_P1_4, .function = 0, .dsc = 2,
	  .ren = 1, .ste = 0, .sre = 0, .ods = 1, .drv = 1 },
};

static void check_pad_config(const struct pinctrl_state *scfg,
			     const struct expected_pad_config *expected,
			     size_t n,
			     const char *label)
{
	zassert_equal(scfg->pin_cnt, n,
		      "%s: pin_cnt expected %zu, got %u", label, n, scfg->pin_cnt);

	for (size_t i = 0; i < n; i++) {
		uint32_t pin = scfg->pins[i];

		zassert_equal(TEST_GET_PIN(pin), expected[i].port,
			      "%s: pin[%zu] port mismatch", label, i);
		zassert_equal(pin & 0x7U, expected[i].function,
			      "%s: pin[%zu] function mismatch", label, i);
		zassert_equal((pin >> DSC_BIT_PST) & TWO_BIT_FIELD_MASK,
			      expected[i].dsc,
			      "%s: pin[%zu] DSC mismatch", label, i);
		zassert_equal((pin >> REN_BIT_PST) & ONE_BIT_FIELD_MASK,
			      expected[i].ren,
			      "%s: pin[%zu] REN mismatch", label, i);
		zassert_equal((pin >> SMP_BIT_PST) & ONE_BIT_FIELD_MASK,
			      expected[i].ste,
			      "%s: pin[%zu] STE mismatch", label, i);
		zassert_equal((pin >> SR_BIT_PST) & ONE_BIT_FIELD_MASK,
			      expected[i].sre,
			      "%s: pin[%zu] SRE mismatch", label, i);
		zassert_equal((pin >> ODS_BIT_PST) & TWO_BIT_FIELD_MASK,
			      expected[i].ods,
			      "%s: pin[%zu] ODS mismatch", label, i);
		zassert_equal((pin >> DRV_BIT_PST) & ONE_BIT_FIELD_MASK,
			      expected[i].drv,
			      "%s: pin[%zu] DRV mismatch", label, i);
	}
}

static void check_pin_encoding(const struct pinctrl_state *scfg,
			       const struct expected_pin *expected,
			       size_t n,
			       const char *label)
{
	zassert_equal(scfg->pin_cnt, n,
		      "%s: pin_cnt expected %zu, got %u", label, n, scfg->pin_cnt);

	for (size_t i = 0; i < n; i++) {
		uint32_t pin = scfg->pins[i];

		zassert_equal(TEST_GET_PIN(pin), expected[i].port,
			      "%s: pin[%zu] port mismatch", label, i);
		zassert_equal(pin & 0x7U, expected[i].function,
			      "%s: pin[%zu] function mismatch", label, i);
		zassert_equal((pin >> DSC_BIT_PST) & TWO_BIT_FIELD_MASK,
			      expected[i].dsc,
			      "%s: pin[%zu] DSC mismatch", label, i);
	}
}

/*
 * Tests.
 */

/*
 * TEST 1: Metadata (state_cnt, pin_cnt, state ids, reg addrs) on both devices.
 */
ZTEST(pinctrl_api, test_metadata)
{
	/* pcfg0: overlay has default+sleep; sleep is skipped w/o PM_DEVICE. */
	zassert_equal(pcfg0->state_cnt, 1, "pcfg0 state_cnt");
	zassert_equal(pcfg0->states[0].id, PINCTRL_STATE_DEFAULT,
		      "pcfg0 state[0] id");
	zassert_equal(pcfg0->states[0].pin_cnt, ARRAY_SIZE(dev0_default_pins),
		      "pcfg0 state[0] pin_cnt");

	/* pcfg1: default + custom "mystate". */
	zassert_equal(pcfg1->state_cnt, 2, "pcfg1 state_cnt");
	zassert_equal(pcfg1->states[0].id, PINCTRL_STATE_DEFAULT,
		      "pcfg1 state[0] id");
	zassert_equal(pcfg1->states[1].id, PINCTRL_STATE_MYSTATE,
		      "pcfg1 state[1] id");
	zassert_equal(pcfg1->states[0].pin_cnt, ARRAY_SIZE(dev1_default_pins),
		      "pcfg1 state[0] pin_cnt");
	zassert_equal(pcfg1->states[1].pin_cnt, ARRAY_SIZE(dev1_mystate_pins),
		      "pcfg1 state[1] pin_cnt");

#ifdef CONFIG_PINCTRL_STORE_REG
	zassert_equal(pcfg0->reg, DT_REG_ADDR(TEST_DEVICE0), "pcfg0 reg addr");
	zassert_equal(pcfg1->reg, DT_REG_ADDR(TEST_DEVICE1), "pcfg1 reg addr");
#endif
}

/*
 * TEST 2: Pin-encoding bit layout for every state on every device.
 */
ZTEST(pinctrl_api, test_pin_encoding)
{
	check_pin_encoding(&pcfg0->states[0], dev0_default_pins,
			   ARRAY_SIZE(dev0_default_pins), "dev0.default");
	check_pin_encoding(&pcfg1->states[0], dev1_default_pins,
			   ARRAY_SIZE(dev1_default_pins), "dev1.default");
	check_pin_encoding(&pcfg1->states[1], dev1_mystate_pins,
			   ARRAY_SIZE(dev1_mystate_pins), "dev1.mystate");
}

/*
 * TEST 3: pinctrl_lookup_state() - every valid id, plus SLEEP and bogus ids.
 */
ZTEST(pinctrl_api, test_lookup)
{
	const struct pinctrl_state *scfg;

	zassert_ok(pinctrl_lookup_state(pcfg0, PINCTRL_STATE_DEFAULT, &scfg),
		   "lookup DEFAULT on pcfg0");
	zassert_equal_ptr(scfg, &pcfg0->states[0], "pcfg0 DEFAULT ptr");

	zassert_ok(pinctrl_lookup_state(pcfg1, PINCTRL_STATE_DEFAULT, &scfg),
		   "lookup DEFAULT on pcfg1");
	zassert_equal_ptr(scfg, &pcfg1->states[0], "pcfg1 DEFAULT ptr");

	zassert_ok(pinctrl_lookup_state(pcfg1, PINCTRL_STATE_MYSTATE, &scfg),
		   "lookup MYSTATE on pcfg1");
	zassert_equal_ptr(scfg, &pcfg1->states[1], "pcfg1 MYSTATE ptr");

	/* SLEEP state was defined in overlay but skipped (no PM_DEVICE). */
	zassert_equal(pinctrl_lookup_state(pcfg0, PINCTRL_STATE_SLEEP, &scfg),
		      -ENOENT, "SLEEP must be -ENOENT without PM_DEVICE");

	zassert_equal(pinctrl_lookup_state(pcfg0, 0xFE, &scfg), -ENOENT,
		      "unknown id on pcfg0");
	zassert_equal(pinctrl_lookup_state(pcfg1, 0xFE, &scfg), -ENOENT,
		      "unknown id on pcfg1");
}

/*
 * TEST 4: Apply every state on every device via both pinctrl apply APIs.
 */
ZTEST(pinctrl_api, test_apply_all_states)
{
	struct pinctrl_dev_config *cfgs[] = { pcfg0, pcfg1 };

	for (size_t d = 0; d < ARRAY_SIZE(cfgs); d++) {
		for (uint8_t s = 0; s < cfgs[d]->state_cnt; s++) {
			const struct pinctrl_state *scfg = &cfgs[d]->states[s];
			const struct pinctrl_state *looked;

			zassert_ok(pinctrl_apply_state(cfgs[d], scfg->id),
				   "apply_state dev%zu state%u failed", d, s);
			zassert_ok(pinctrl_lookup_state(cfgs[d], scfg->id, &looked),
				   "lookup after apply_state dev%zu state%u",
				   d, s);
			zassert_equal_ptr(looked, scfg,
					  "lookup ptr after apply_state");

			zassert_ok(pinctrl_apply_state_direct(cfgs[d], scfg),
				   "apply_state_direct dev%zu state%u failed",
				   d, s);
		}
	}
}

/*
 * TEST 5: pinctrl_apply_state() rejects unknown ids with -ENOENT.
 */
ZTEST(pinctrl_api, test_apply_invalid_id)
{
	zassert_equal(pinctrl_apply_state(pcfg0, 0xFE), -ENOENT,
		      "unknown id on pcfg0");
	zassert_equal(pinctrl_apply_state(pcfg1, 0xFE), -ENOENT,
		      "unknown id on pcfg1");
}

/*
 * TEST 6: Idempotent apply + DEFAULT<->MYSTATE toggle via pinctrl APIs.
 */
ZTEST(pinctrl_api, test_apply_transitions)
{
	/* Applying the same state twice must succeed both times. */
	zassert_ok(pinctrl_apply_state(pcfg1, PINCTRL_STATE_MYSTATE),
		   "first MYSTATE apply");
	zassert_ok(pinctrl_apply_state(pcfg1, PINCTRL_STATE_MYSTATE),
		   "idempotent MYSTATE apply");

	for (int i = 0; i < 16; i++) {
		zassert_ok(pinctrl_apply_state(pcfg1, PINCTRL_STATE_DEFAULT),
			   "DEFAULT apply iter %d", i);
		zassert_ok(pinctrl_apply_state(pcfg1, PINCTRL_STATE_MYSTATE),
			   "MYSTATE apply iter %d", i);
	}
}

/*
 * TEST 7: Verify all pad-config bit fields (REN, STE, SRE, DSC, ODS, DRV)
 * for every state on both devices. Catches encoding errors in the SoC
 * pinctrl_soc.h macros (PAD_CONF_REN, PAD_CONF_SMT, etc.).
 */
ZTEST(pinctrl_api, test_pad_config_fields)
{
	check_pad_config(&pcfg0->states[0], dev0_default_padcfg,
			 ARRAY_SIZE(dev0_default_padcfg), "dev0.default");
	check_pad_config(&pcfg1->states[0], dev1_default_padcfg,
			 ARRAY_SIZE(dev1_default_padcfg), "dev1.default");
	check_pad_config(&pcfg1->states[1], dev1_mystate_padcfg,
			 ARRAY_SIZE(dev1_mystate_padcfg), "dev1.mystate");
}

/*
 * TEST 8: Apply MYSTATE then restore DEFAULT via pinctrl_apply_state().
 */
ZTEST(pinctrl_api, test_apply_default_restore)
{
	const struct pinctrl_state *looked;

	zassert_ok(pinctrl_apply_state(pcfg1, PINCTRL_STATE_MYSTATE),
		   "apply MYSTATE");
	zassert_ok(pinctrl_lookup_state(pcfg1, PINCTRL_STATE_MYSTATE, &looked),
		   "lookup MYSTATE");
	zassert_equal_ptr(looked, &pcfg1->states[1], "MYSTATE ptr");

	zassert_ok(pinctrl_apply_state(pcfg1, PINCTRL_STATE_DEFAULT),
		   "restore DEFAULT");
	zassert_ok(pinctrl_lookup_state(pcfg1, PINCTRL_STATE_DEFAULT, &looked),
		   "lookup DEFAULT after restore");
	zassert_equal_ptr(looked, &pcfg1->states[0], "DEFAULT ptr");
}

/*
 * TEST 9: Devices use disjoint pins and can be applied independently.
 */
ZTEST(pinctrl_api, test_devices_independent)
{
	for (uint8_t i = 0U; i < pcfg0->states[0].pin_cnt; i++) {
		uint32_t pin0 = TEST_GET_PIN(pcfg0->states[0].pins[i]);

		for (uint8_t s = 0U; s < pcfg1->state_cnt; s++) {
			for (uint8_t j = 0U; j < pcfg1->states[s].pin_cnt; j++) {
				zassert_not_equal(pin0,
						  TEST_GET_PIN(pcfg1->states[s].pins[j]),
						  "dev0 pin overlaps dev1 s%u pin[%u]",
						  s, j);
			}
		}
	}

	zassert_ok(pinctrl_apply_state(pcfg0, PINCTRL_STATE_DEFAULT),
		   "apply dev0 DEFAULT");
	zassert_ok(pinctrl_apply_state(pcfg1, PINCTRL_STATE_DEFAULT),
		   "apply dev1 DEFAULT");
	zassert_ok(pinctrl_apply_state(pcfg1, PINCTRL_STATE_MYSTATE),
		   "apply dev1 MYSTATE");
	zassert_ok(pinctrl_apply_state(pcfg0, PINCTRL_STATE_DEFAULT),
		   "re-apply dev0 DEFAULT");
	zassert_ok(pinctrl_apply_state(pcfg1, PINCTRL_STATE_DEFAULT),
		   "restore dev1 DEFAULT");
}

/*
 * TEST 10: pinctrl_configure_pins() with PINCTRL_REG_NONE.
 */
ZTEST(pinctrl_api, test_configure_pins)
{
	const struct pinctrl_state *scfg = &pcfg1->states[0];

	zassert_ok(pinctrl_configure_pins(scfg->pins, scfg->pin_cnt,
					  PINCTRL_REG_NONE),
		   "configure_pins DEFAULT");

	scfg = &pcfg1->states[1];
	zassert_ok(pinctrl_configure_pins(scfg->pins, scfg->pin_cnt,
					  PINCTRL_REG_NONE),
		   "configure_pins MYSTATE");

	zassert_ok(pinctrl_apply_state(pcfg1, PINCTRL_STATE_DEFAULT),
		   "restore DEFAULT after configure_pins");
}

#if defined(CONFIG_PINCTRL_DYNAMIC)

/* Saved originals for dynamic-state tests. */
static const struct pinctrl_state *saved_pcfg1_states;
static uint8_t saved_pcfg1_state_cnt;

static void save_pcfg1_originals(void)
{
	if (!saved_pcfg1_states) {
		saved_pcfg1_states = pcfg1->states;
		saved_pcfg1_state_cnt = pcfg1->state_cnt;
	}
}

static void restore_pcfg1_originals(void)
{
	if (saved_pcfg1_states) {
		(void)pinctrl_update_states(pcfg1, saved_pcfg1_states,
					    saved_pcfg1_state_cnt);
	}
}

/*
 * TEST 11: pinctrl_update_states() - valid same-count swap, apply, restore.
 */
ZTEST(pinctrl_api, test_update_states_valid)
{
	static struct pinctrl_state swapped[2];
	const struct pinctrl_state *scfg;

	save_pcfg1_originals();

	/* Swap the pins arrays between DEFAULT and MYSTATE while keeping ids. */
	swapped[0].id = PINCTRL_STATE_DEFAULT;
	swapped[0].pins = saved_pcfg1_states[1].pins;
	swapped[0].pin_cnt = saved_pcfg1_states[1].pin_cnt;
	swapped[1].id = PINCTRL_STATE_MYSTATE;
	swapped[1].pins = saved_pcfg1_states[0].pins;
	swapped[1].pin_cnt = saved_pcfg1_states[0].pin_cnt;

	zassert_ok(pinctrl_update_states(pcfg1, swapped, 2), "update swap");
	zassert_equal_ptr(pcfg1->states, swapped, "states ptr after swap");

	zassert_ok(pinctrl_lookup_state(pcfg1, PINCTRL_STATE_DEFAULT, &scfg),
		   "lookup DEFAULT after swap");
	zassert_equal_ptr(scfg, &swapped[0], "DEFAULT ptr after swap");
	zassert_ok(pinctrl_apply_state(pcfg1, PINCTRL_STATE_DEFAULT),
		   "apply swapped DEFAULT");

	zassert_ok(pinctrl_apply_state(pcfg1, PINCTRL_STATE_MYSTATE),
		   "apply swapped MYSTATE");

	restore_pcfg1_originals();
	zassert_equal_ptr(pcfg1->states, saved_pcfg1_states,
			  "states ptr after restore");
	zassert_ok(pinctrl_apply_state(pcfg1, PINCTRL_STATE_DEFAULT),
		   "apply restored DEFAULT");
}

/*
 * TEST 12: pinctrl_update_states() negative cases - wrong count and unknown id.
 */
ZTEST(pinctrl_api, test_update_states_invalid)
{
	static struct pinctrl_state trimmed[1];
	static struct pinctrl_state bogus[2];

	save_pcfg1_originals();

	/* Wrong state_cnt. */
	trimmed[0] = saved_pcfg1_states[0];
	zassert_equal(pinctrl_update_states(pcfg1, trimmed, 1), -EINVAL,
		      "Expected -EINVAL for mismatched state_cnt");
	zassert_equal_ptr(pcfg1->states, saved_pcfg1_states);

	/* Unknown id. */
	bogus[0] = saved_pcfg1_states[0];
	bogus[1] = saved_pcfg1_states[1];
	bogus[1].id = 0xFE;
	zassert_equal(pinctrl_update_states(pcfg1, bogus, 2), -EINVAL,
		      "Expected -EINVAL for unknown state id");
	zassert_equal_ptr(pcfg1->states, saved_pcfg1_states);
}

#endif /* CONFIG_PINCTRL_DYNAMIC */

/*
 * Suite teardown + registration.
 */

static void pinctrl_api_teardown(void *fixture)
{
	ARG_UNUSED(fixture);

#if defined(CONFIG_PINCTRL_DYNAMIC)
	restore_pcfg1_originals();
#endif
	(void)pinctrl_apply_state(pcfg0, PINCTRL_STATE_DEFAULT);
	(void)pinctrl_apply_state(pcfg1, PINCTRL_STATE_DEFAULT);
}

ZTEST_SUITE(pinctrl_api, NULL, NULL, NULL, NULL, pinctrl_api_teardown);
