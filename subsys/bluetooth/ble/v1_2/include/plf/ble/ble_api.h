#ifndef BLE_API_H_
#define BLE_API_H_

#include "rom_build_cfg.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>


// Transport interface definitions
typedef enum itf_status
{
  ITF_STATUS_OK,
  ITF_STATUS_ERROR,
  ITF_STATUS_DETACHED,
  ITF_STATUS_ATTACHED,
} itf_status_t;

// BLE stack init error codes
typedef enum ble_init_err_code
{
  BLE_INIT_ERR_NONE,
  BLE_INIT_ERR_INVALID_ARGS,
  BLE_INIT_ERR_PATCH,
} ble_init_err_code_t;


typedef void (*itf_callback)(void*, itf_status_t status);
typedef void (*timer_cb)(void);

// Functions that need to be defined by the application for stack to work as intended
typedef struct
{
  // ll APIs
  void (*p_global_int_disable)(void);
  void (*p_global_int_restore)(void);

  // HCI ITF APIs
  void (*p_hci_itf_read)(uint8_t* bufptr, uint32_t size, itf_callback callback, void* dummy);
  void (*p_hci_itf_write)(uint8_t* bufptr, uint32_t size, itf_callback callback, void* dummy);
  void (*p_hci_itf_flow_on)(void);
  bool (*p_hci_itf_flow_off)(void);

  // AHI ITF APIs.
  void (*p_ahi_itf_read)(uint8_t* bufptr, uint32_t size, itf_callback callback, void* dummy);
  void (*p_ahi_itf_write)(uint8_t* bufptr, uint32_t size, itf_callback callback, void* dummy);
  void (*p_ahi_itf_flow_on)(void);
  bool (*p_ahi_itf_flow_off)(void);

  // app_init is called once rwip is initialized and app task is present
  void (*p_app_init)(void);

  // Timer APIs
  void (*p_timer_init)(void);
  uint32_t (*p_timer_get_time)(void);
  void (*p_timer_enable)(bool enable);
  void (*p_timer_set_timeout)(uint32_t timeout, timer_cb callback);

  void (*p_platform_reset_request)(uint32_t error);

  // RTOS APIs
  void (*p_rtos_evt_post)(void);

  // DMA APIs
  int32_t (*p_dma_copy)(void * p_dst, void * p_src, size_t len, void (*cb)(uint32_t param));
  void (*p_dma_abort)(void);

  // Synchronization timer APIs
  uint32_t (*p_sync_timer_start)(void (*sync_timer_capture_evt_cb)(void), void (*sync_timer_overflow_evt_cb)(void));
  uint32_t (*p_sync_timer_get_curr_cnt)(void);
  uint32_t (*p_sync_timer_get_last_capture)(void);
  void (*p_sync_timer_disable_evts)(void);
  void (*p_sync_timer_restore_evts)(void);
} ble_app_hooks_t;

// Heap memory allocations that need to done by the application
// Heap header size is 12 bytes
#define RWIP_HEAP_HEADER (12 / sizeof(uint32_t))
// ceil(len/sizeof(uint32_t)) + RWIP_HEAP_HEADER
#define RWIP_CALC_HEAP_LEN(len)                                                                    \
  ((((len) + (sizeof(uint32_t) - 1)) / sizeof(uint32_t)) + RWIP_HEAP_HEADER)
// Use the macro below to calculate the final heap size
#define RWIP_CALC_HEAP_LEN_IN_BYTES(len) (RWIP_CALC_HEAP_LEN(len) * sizeof(uint32_t))

typedef struct
{

  // Heap memory needs to be allocated from application
  // All heap memory need to be 32 bit alligned
  uint32_t* p_ble_heap_env_mem;
  uint16_t ble_heap_env_mem_size;
  uint32_t* p_ble_heap_profile_mem;
  uint16_t ble_heap_profile_mem_size;
  uint32_t* p_ble_heap_msg_mem;
  uint16_t ble_heap_msg_mem_size;
  uint32_t* p_ble_heap_non_ret_mem;
  uint16_t ble_heap_non_ret_mem_size;

  // Main app task need to be set from the application. TASK_APP/TASK_AHI
  uint16_t ble_app_main_task;

#ifdef CFG_PATCHING
  // Optional patch for the BLE host firmware, can be NULL.
  // If not NULL, must point to the beginning of the .patch_info section.
  void const* patch;
#endif
} ble_rom_config_t;

/**
 * @brief Initialize bluetooth stack in ROM. This function must be called only once from
 *        the application before using any other BLE stack APIs
 *
 *        Note: When there is an error in patching the ROM the API returns BLE_INIT_ERR_PATCH.
 *              In this specific error scenario the API still initializes the stack without
 *              applying the ROM patch. This means that an application can still continue
 *              using the underlying stack APIs even if the ROM patching failed.
 *
 * @return BLE_INIT_ERR_NONE         On success
 *         BLE_INIT_ERR_INVALID_ARGS Error: Invalid input arguments
 *         BLE_INIT_ERR_PATCH        Patching BLE ROM failed but stack is initialized
 *                                   and APIs still usable
 */
ble_init_err_code_t
ble_stack_init(ble_app_hooks_t const* app_hooks_funcs,
               ble_rom_config_t const* ble_rom_config);

#endif /* BLE_API_H_ */
