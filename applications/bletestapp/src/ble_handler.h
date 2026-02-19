#include <inttypes.h>
#include <stdbool.h>

#define DEVICE_NAME_LEN 9
extern char app_shell_device_name[];

extern uint16_t ble_adv_int_min;
extern uint16_t ble_adv_int_max;
extern uint16_t ble_conn_int_min;
extern uint16_t ble_conn_int_max;

int ble_init(void);
int ble_start(void);
int ble_uninit(void);
bool ble_is_connected(void);
