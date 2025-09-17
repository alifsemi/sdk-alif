#ifndef POWER_MGR_H_
#define POWER_MGR_H_
#include <inttypes.h>
typedef enum {
	PM_STATE_MODE_IDLE,
	PM_STATE_MODE_STANDBY,
	PM_STATE_MODE_STOP
} pm_state_mode_type_e;

int power_mgr_set_offprofile(pm_state_mode_type_e pm_mode);
void power_mgr_ready_for_sleep(void);
bool power_mgr_cold_boot(void);
uint32_t power_mgr_get_wakeup_reason(void);

void power_mgr_set_subsys_off_period(uint32_t period_ms);

#endif /*POWER_MGR_H_*/