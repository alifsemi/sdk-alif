/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_uart.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/ztest.h>

/* Global variable to track if bt init sync has succeeded */
static bool bt_init_done;

/* Function to handle BT initialization */
static void ensure_bt_initialized(const struct shell *shell)
{
	int ret;

	if (bt_init_done) {
		return;
	}

	ret = shell_execute_cmd(shell, "bt init sync");
	if (ret == 0) {
		bt_init_done = true;
	} else {
		/* First init attempt failed */
		zassert_equal(ret, 0, "BT initialization failed (ret=%d)", ret);
	}
}

/* Delay before executing shell commands to ensure shell is ready */
#define SHELL_EXECUTE_DELAY_MS 1000
/* Delay between shell commands to ensure each command completes */
#define SHELL_COMMAND_DELAY_MS 500

/* Get shell instance for tests */
static const struct shell *get_shell_instance(void)
{
	const struct shell *shell;

	/* Wait for shell to initialize */
	k_sleep(K_MSEC(SHELL_EXECUTE_DELAY_MS));

	/* Get shell instance */
	shell = shell_backend_uart_get_ptr();
	return shell;
}

/* Test for scenario 2: PERIPHERAL_FULL_CYCLE */
ZTEST(bluetooth_shell, test_peripheral_full_cycle)
{
	const struct shell *shell;
	int ret;

	shell = get_shell_instance();
	zassert_not_null(shell, "Shell instance is NULL");

	/* Handle BT initialization */
	ensure_bt_initialized(shell);

	/* Execute remaining commands */
	ret = shell_execute_cmd(shell, "bt adv-create conn-scan");
	zassert_equal(ret, 0, "Command failed: bt adv-create conn-scan (ret=%d)", ret);

	ret = shell_execute_cmd(shell, "bt adv-param interval-min 200 interval-max 300");
	zassert_equal(ret, 0, "Command failed: bt adv-param (ret=%d)", ret);

	/* Sleep to allow command to complete */
	k_sleep(K_MSEC(SHELL_COMMAND_DELAY_MS));

	ret = shell_execute_cmd(shell, "bt adv-start");
	zassert_equal(ret, 0, "Command failed: bt adv-start (ret=%d)", ret);

	ret = shell_execute_cmd(shell, "bt adv-stop");
	zassert_equal(ret, 0, "Command failed: bt adv-stop (ret=%d)", ret);

	ret = shell_execute_cmd(shell, "bt adv-delete");
	zassert_equal(ret, 0, "Command failed: bt adv-delete (ret=%d)", ret);
}

/* Test for scenario 3: PERIPHERAL_NEGATIVE */
ZTEST(bluetooth_shell, test_peripheral_negative)
{
	const struct shell *shell;
	int ret;

	shell = get_shell_instance();
	zassert_not_null(shell, "Shell instance is NULL");

	/* Handle BT initialization */
	ensure_bt_initialized(shell);

	/* These commands should fail - we expect errors */
	ret = shell_execute_cmd(shell, "bt adv-start");
	zassert_not_equal(ret, 0, "Command succeeded unexpectedly: bt adv-start");

	ret = shell_execute_cmd(shell, "bt adv-stop");
	zassert_not_equal(ret, 0, "Command succeeded unexpectedly: bt adv-stop");

	ret = shell_execute_cmd(shell, "bt adv-delete");
	zassert_not_equal(ret, 0, "Command succeeded unexpectedly: bt adv-delete");
}

/* Define the test suite */
ZTEST_SUITE(bluetooth_shell, NULL, NULL, NULL, NULL, NULL);
