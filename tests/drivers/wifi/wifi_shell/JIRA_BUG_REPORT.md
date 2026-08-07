# JIRA Bug Report: Airoc WiFi Driver — Scan State Leak on whd_wifi_scan Failure

## Summary
**airoc_wifi: scan_rslt_cb not cleared on whd_wifi_scan() failure causes permanent -EINPROGRESS (-119) for all subsequent scan requests**

## Component
`drivers/wifi/infineon/airoc_wifi.c`

## Affects Version
ZAS v2.3.0-rc4 (Zephyr 4.1.0), WHD v3.3.3.26653

## Severity
**Critical** — After a single transient scan failure, the WiFi scan interface becomes permanently unusable until device reboot.

## Hardware
- **Board**: Alif E8 DK (alif_e8_dk/ae822fa0e5597xx0/rtss_hp)
- **WiFi Module**: Murata 1YN (CYW43439)
- **Firmware**: wl0 v7.95.88

## Description
In `airoc_mgmt_scan()`, the `data->scan_rslt_cb` field is set to the callback pointer **before** calling `whd_wifi_scan()`. If `whd_wifi_scan()` fails (returns non-WHD_SUCCESS), the error path gives back `sema_common` but **does not reset `scan_rslt_cb` to NULL**.

All subsequent calls to `airoc_mgmt_scan()` then hit the guard at the top of the function:
```c
if (data->scan_rslt_cb != NULL) {
    LOG_INF("Scan callback in progress");
    return -EINPROGRESS;  /* -119 */
}
```
This permanently blocks all future scan operations until device reboot.

## Root Cause Analysis

**File**: `drivers/wifi/infineon/airoc_wifi.c`, function `airoc_mgmt_scan()`

**Buggy code** (lines 558–566):
```c
data->scan_rslt_cb = cb;                    /* Line 558: callback set BEFORE whd call */

if (whd_wifi_scan(...) != WHD_SUCCESS) {    /* Line 561: WHD call fails */
    LOG_ERR("Failed to start scan");
    k_sem_give(&data->sema_common);          /* Semaphore released */
    return -EAGAIN;                          /* BUT scan_rslt_cb still points to cb! */
}
```

The `scan_rslt_cb` is a guard variable checked at function entry. Once set and not cleared, it permanently blocks the scan path.

## Steps to Reproduce
1. Boot E8 DK with 1YN module and any WiFi application
2. Issue a WiFi scan immediately after boot (within ~2s of firmware load)
3. First scan returns `-EAGAIN` because WHD driver is still initializing
4. All subsequent scan requests return `-EINPROGRESS` (-119) forever
5. WiFi connect/disconnect still work — only scan is broken

## Expected Behavior
After a failed scan attempt, subsequent scan requests should be accepted normally once the driver is ready.

## Actual Behavior
A single failed `whd_wifi_scan()` call permanently blocks all future scan operations with `-EINPROGRESS` (-119).

## Fix
Add `data->scan_rslt_cb = NULL;` on the error path:

```c
if (whd_wifi_scan(...) != WHD_SUCCESS) {
    LOG_ERR("Failed to start scan");
    data->scan_rslt_cb = NULL;       /* <-- FIX: clear callback on failure */
    k_sem_give(&data->sema_common);
    return -EAGAIN;
}
```

## Impact
- Any application that attempts WiFi scan before the WHD driver is fully ready will permanently lose scan capability
- This affects automated test suites, boot-time scanning, and any application with early scan logic
- Connect/disconnect/status operations are unaffected

## Test Verification
After applying the fix, the WiFi shell test suite passes 6/6 tests including repeated scan operations.

## Reporter
Validation Engineering Team

## Labels
`wifi`, `driver`, `airoc`, `CYW43439`, `1YN`, `scan`, `regression`, `state-leak`

## Priority
P1 — Blocker for WiFi scan reliability
