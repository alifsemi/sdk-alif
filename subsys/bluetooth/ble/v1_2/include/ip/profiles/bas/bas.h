/**
 ****************************************************************************************
 *
 * @file bas.h
 *
 * @brief Header file - Battery Service - API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAS_H_
#define BAS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAS_API Battery Service (BAS)
 * @ingroup PROFILE_API
 * @brief Description of API for Battery Service\n See \ref bas_msc
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAS_API_COMMON Common
 * @ingroup BAS_API
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h"
#include "prf_utils.h"
#include "co_endian.h"

/// @addtogroup BAS_API_COMMON
/// @{

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximal battery level value
#define BAS_LEVEL_MAX (100u)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Size of fields for Battery Level characteristic value
enum bas_level_size
{
    /// Level - 1 byte
    BAS_LEVEL_SIZE_LEVEL = 1u,

    /// Maximum size
    BAS_LEVEL_SIZE_MAX = 1u,
};

/// Wired/wireless external power source connected values
enum bas_external_power_source_connected
{
    /// Not connected
    BAS_EXTERNAL_POWER_SOURCE_NOT_CONNECTED = 0u,
    /// Connected
    BAS_EXTERNAL_POWER_SOURCE_CONNECTED,
    /// Unknown
    BAS_EXTERNAL_POWER_SOURCE_UNKNOWN,
};

/// Battery charge state values
enum bas_battery_charge_state
{
    /// Unknown
    BAS_BATTERY_CHARGE_STATE_UNKNOWN = 0u,
    /// Charging
    BAS_BATTERY_CHARGE_STATE_CHARGING,
    /// Discharging - Active
    BAS_BATTERY_CHARGE_STATE_DISCHARGING_ACTIVE,
    /// Discharging - Inactive
    BAS_BATTERY_CHARGE_STATE_DISCHARGING_INACTIVE,
};

/// Battery charge level values
enum bas_battery_charge_level
{
    /// Unknown
    BAS_BATTERY_CHARGE_LEVEL_UNKNOWN = 0u,
    /// Good
    BAS_BATTERY_CHARGE_LEVEL_GOOD,
    /// Low
    BAS_BATTERY_CHARGE_LEVEL_LOW,
    /// Critical
    BAS_BATTERY_CHARGE_LEVEL_CRITICAL,
};

/// Charging type values
enum bas_charging_type
{
    /// Unknown or not charging
    BAS_CHARGING_TYPE_UNKNOWN_OR_NOT_CHARGING = 0u,
    /// Constant current
    BAS_CHARGING_TYPE_CONSTANT_CURRENT,
    /// Constant voltage
    BAS_CHARGING_TYPE_CONSTANT_VOLTAGE,
    /// Trickle
    BAS_CHARGING_TYPE_TRICKLE,
    /// Float
    BAS_CHARGING_TYPE_FLOAT,
};

/// Service required values
enum bas_service_req
{
    /// False
    BAS_SERVICE_REQ_FALSE = 0u,
    /// True
    BAS_SERVICE_REQ_TRUE,
    /// Unknown
    BAS_SERVICE_REQ_UNKNOWN,
};

/// Flags bit field meaning for Battery Level Status characteristic
enum bas_level_status_flags_bf
{
    /// Identifier present - Position
    BAS_LEVEL_STATUS_FLAGS_IDENTIFIER_PRESENT_POS = 0u,
    /// Identifier present - Bit
    BAS_LEVEL_STATUS_FLAGS_IDENTIFIER_PRESENT_BIT = CO_BIT(BAS_LEVEL_STATUS_FLAGS_IDENTIFIER_PRESENT_POS),

    /// Battery level present - Position
    BAS_LEVEL_STATUS_FLAGS_LEVEL_PRESENT_POS = 1u,
    /// Battery level present - Bit
    BAS_LEVEL_STATUS_FLAGS_LEVEL_PRESENT_BIT = CO_BIT(BAS_LEVEL_STATUS_FLAGS_LEVEL_PRESENT_POS),

    /// Additional status present - Position
    BAS_LEVEL_STATUS_FLAGS_ADD_STATUS_PRESENT_POS = 2u,
    /// Additional status present - Bit
    BAS_LEVEL_STATUS_FLAGS_ADD_STATUS_PRESENT_BIT = CO_BIT(BAS_LEVEL_STATUS_FLAGS_ADD_STATUS_PRESENT_POS),
};

/// Power state bit field meaning
enum bas_power_state_bf
{
    /// Battery present - Position
    BAS_POWER_STATE_BATTERY_PRESENT_POS = 0u,
    /// Battery present - Bit
    BAS_POWER_STATE_BATTERY_PRESENT_BIT = CO_BIT(BAS_POWER_STATE_BATTERY_PRESENT_POS),

    /// Wired external power source connected - Position (see #bas_external_power_source_connected enumeration)
    BAS_POWER_STATE_WIRED_EXT_SOURCE_LSB = 1u,
    /// Wired external power source connected - Mask (see #bas_external_power_source_connected enumeration)
    BAS_POWER_STATE_WIRED_EXT_SOURCE_MASK = CO_MASK(2, BAS_POWER_STATE_WIRED_EXT_SOURCE_LSB),

    /// Wireless external power source connected - Position (see #bas_external_power_source_connected enumeration)
    BAS_POWER_STATE_WIRELESS_EXT_SOURCE_LSB = 3u,
    /// Wireless external power source connected - Mask (see #bas_external_power_source_connected enumeration)
    BAS_POWER_STATE_WIRELESS_EXT_SOURCE_MASK = CO_MASK(2, BAS_POWER_STATE_WIRELESS_EXT_SOURCE_LSB),

    /// Battery charge state - Position (see #bas_battery_charge_state enumeration)
    BAS_POWER_STATE_BATTERY_CHARGE_LSB = 5u,
    /// Battery charge state - Mask (see #bas_battery_charge_state enumeration)
    BAS_POWER_STATE_BATTERY_CHARGE_MASK = CO_MASK(2, BAS_POWER_STATE_BATTERY_CHARGE_LSB),

    /// Battery charge level - Position (see #bas_battery_charge_level enumeration)
    BAS_POWER_STATE_BATTERY_CHARGE_LEVEL_LSB = 7u,
    /// Battery charge level - Mask (see #bas_battery_charge_level enumeration)
    BAS_POWER_STATE_BATTERY_CHARGE_LEVEL_MASK = CO_MASK(2, BAS_POWER_STATE_BATTERY_CHARGE_LEVEL_LSB),

    /// Charging type - Position (see #bas_charging_type enumeration)
    BAS_POWER_STATE_CHARGING_TYPE_LSB = 9u,
    /// Charging type - Mask (see #bas_charging_type enumeration)
    BAS_POWER_STATE_CHARGING_TYPE_MASK = CO_MASK(3, BAS_POWER_STATE_CHARGING_TYPE_LSB),

    /// Charging fault reason - Position
    BAS_POWER_STATE_CHARGING_FAULT_REASON_LSB = 12u,
    /// Charging fault reason - Mask
    BAS_POWER_STATE_CHARGING_FAULT_REASON_MASK = CO_MASK(3, BAS_POWER_STATE_CHARGING_FAULT_REASON_LSB),

    /// Charging fault reason - Battery - Position
    BAS_POWER_STATE_CHARGING_FAULT_REASON_BATTERY_POS = 12u,
    /// Charging fault reason - Battery - Bit
    BAS_POWER_STATE_CHARGING_FAULT_REASON_BATTERY_BIT = CO_BIT(BAS_POWER_STATE_CHARGING_FAULT_REASON_BATTERY_POS),

    /// Charging fault reason - External power source - Position
    BAS_POWER_STATE_CHARGING_FAULT_REASON_EXTERNAL_POS = 13u,
    /// Charging fault reason - External power source - Bit
    BAS_POWER_STATE_CHARGING_FAULT_REASON_EXTERNAL_BIT = CO_BIT(BAS_POWER_STATE_CHARGING_FAULT_REASON_EXTERNAL_POS),

    /// Charging fault reason - Other - Position
    BAS_POWER_STATE_CHARGING_FAULT_REASON_OTHER_POS = 14u,
    /// Charging fault reason - Other - Bit
    BAS_POWER_STATE_CHARGING_FAULT_REASON_OTHER_BIT = CO_BIT(BAS_POWER_STATE_CHARGING_FAULT_REASON_OTHER_POS),
};

/// Additional status bit field meaning
enum bas_add_status_bf
{
    /// Service required - Position (see #bas_service_req enumeration)
    BAS_ADD_STATUS_SERVICE_REQ_LSB = 0u,
    /// Service required - Mask (see #bas_service_req enumeration)
    BAS_ADD_STATUS_SERVICE_REQ_MASK = CO_MASK(2, BAS_ADD_STATUS_SERVICE_REQ_LSB),

    /// Battery fault - Position
    BAS_ADD_STATUS_FAULT_POS = 2u,
    /// Battery fault - Bit
    BAS_ADD_STATUS_FAULT_BIT = CO_BIT(BAS_ADD_STATUS_FAULT_POS),
};

/// Size of fields for Battery Level Status characteristic value
enum bas_level_status_size
{
    /// Flags - 1 byte (see #bas_level_status_flags_bf enumeration)
    BAS_LEVEL_STATUS_SIZE_FLAGS = 1u,
    /// Power state - 2 bytes (see #bas_power_state_bf enumeration)
    BAS_LEVEL_STATUS_SIZE_POWER_STATE = 2u,
    /// Identifier - 2 bytes\n
    /// Present only if #BAS_LEVEL_STATUS_FLAGS_IDENTIFIER_PRESENT_BIT set in Flags
    BAS_LEVEL_STATUS_SIZE_IDENTIFIER = 2u,
    /// Level - 1 byte\n
    /// Present only if #BAS_LEVEL_STATUS_FLAGS_LEVEL_PRESENT_BIT set in Flags
    BAS_LEVEL_STATUS_SIZE_LEVEL = 1u,
    /// Additional status - 1 byte (see #bas_add_status_bf enumeration)\n
    /// Present only if #BAS_LEVEL_STATUS_FLAGS_ADD_STATUS_PRESENT_BIT set in Flags
    BAS_LEVEL_STATUS_SIZE_ADD_STATUS = 1u,

    /// Maximum size
    BAS_LEVEL_STATUS_SIZE_MAX = 7u,
};

/// Battery Critical Status characteristic bit field meaning
enum bas_critical_status_bf
{
    /// Critical power state - Position
    BAS_CRITICAL_STATUS_CRITICAL_POWER_STATE_POS = 0u,
    /// Critical power state - Bit
    BAS_CRITICAL_STATUS_CRITICAL_POWER_STATE_BIT = CO_BIT(BAS_CRITICAL_STATUS_CRITICAL_POWER_STATE_POS),

    /// Immediate Service Required - Position
    BAS_CRITICAL_STATUS_IMMEDIATE_SERVICE_REQ_POS = 1u,
    /// Immediate Service Required - Bit
    BAS_CRITICAL_STATUS_IMMEDIATE_SERVICE_REQ_BIT = CO_BIT(BAS_CRITICAL_STATUS_IMMEDIATE_SERVICE_REQ_POS),
};

/// Position of fields for Battery Critical Status characteristic value
enum bas_critical_status_size
{
    /// Status - 1 byte (see #bas_critical_status_bf enumeration)
    BAS_CRITICAL_STATUS_SIZE_STATUS = 1u,

    /// Maximum size
    BAS_CRITICAL_STATUS_SIZE_MAX = 1u,
};

/// Flags bit field meaning for Battery Energy Status characteristic
enum bas_energy_status_flags_bf
{
    /// External source power present - Position
    BAS_ENERGY_STATUS_FLAGS_EXTERNAL_SOURCE_PRESENT_POS = 0u,
    /// External source power present - Bit
    BAS_ENERGY_STATUS_FLAGS_EXTERNAL_SOURCE_PRESENT_BIT = CO_BIT(BAS_ENERGY_STATUS_FLAGS_EXTERNAL_SOURCE_PRESENT_POS),

    /// Present voltage present - Position
    BAS_ENERGY_STATUS_FLAGS_PRESENT_VOLTAGE_PRESENT_POS = 1u,
    /// Present voltage present - Bit
    BAS_ENERGY_STATUS_FLAGS_PRESENT_VOLTAGE_PRESENT_BIT = CO_BIT(BAS_ENERGY_STATUS_FLAGS_PRESENT_VOLTAGE_PRESENT_POS),

    /// Available energy present - Position
    BAS_ENERGY_STATUS_FLAGS_AVAILABLE_ENERGY_PRESENT_POS = 2u,
    /// Available energy present - Bit
    BAS_ENERGY_STATUS_FLAGS_AVAILABLE_ENERGY_PRESENT_BIT
                                            = CO_BIT(BAS_ENERGY_STATUS_FLAGS_AVAILABLE_ENERGY_PRESENT_POS),

    /// Available battery capacity present - Position
    BAS_ENERGY_STATUS_FLAGS_AVAILABLE_ENERGY_CAPA_PRESENT_POS = 3u,
    /// Available battery capacity present - Bit
    BAS_ENERGY_STATUS_FLAGS_AVAILABLE_ENERGY_CAPA_PRESENT_BIT
                                            = CO_BIT(BAS_ENERGY_STATUS_FLAGS_AVAILABLE_ENERGY_CAPA_PRESENT_POS),

    /// Charge rate present - Position
    BAS_ENERGY_STATUS_FLAGS_CHARGE_RATE_PRESENT_POS = 4u,
    /// Charge rate present - Bit
    BAS_ENERGY_STATUS_FLAGS_CHARGE_RATE_PRESENT_BIT = CO_BIT(BAS_ENERGY_STATUS_FLAGS_CHARGE_RATE_PRESENT_POS),

    /// Available energy at last charge present - Position
    BAS_ENERGY_STATUS_FLAGS_AVAILABLE_ENERGY_LAST_CHARGE_PRESENT_POS = 5u,
    /// Available energy at last charge present - Bit
    BAS_ENERGY_STATUS_FLAGS_AVAILABLE_ENERGY_LAST_CHARGE_PRESENT_BIT
                                        = CO_BIT(BAS_ENERGY_STATUS_FLAGS_AVAILABLE_ENERGY_LAST_CHARGE_PRESENT_POS),
};

/// Position of fields for Battery Energy Status characteristic value
enum bas_energy_status_size
{
    /// Flags - 1 byte (see #bas_energy_status_flags_bf enumeration)
    BAS_ENERGY_STATUS_SIZE_FLAGS = 1u,
    /// External source power - 2 bytes\n
    /// Present only if #BAS_ENERGY_STATUS_FLAGS_EXTERNAL_SOURCE_PRESENT_BIT set in Flags
    BAS_ENERGY_STATUS_SIZE_EXTERNAL_SOURCE_POWER = 2u,
    /// Present voltage - 2 bytes\n
    /// Present only if #BAS_ENERGY_STATUS_FLAGS_PRESENT_VOLTAGE_PRESENT_BIT set in Flags
    BAS_ENERGY_STATUS_SIZE_PRESENT_VOLTAGE = 2u,
    /// Available energy - 2 byte\n
    /// Present only if #BAS_ENERGY_STATUS_FLAGS_AVAILABLE_ENERGY_PRESENT_BIT set in Flags
    BAS_ENERGY_STATUS_SIZE_AVAILABLE_ENERGY = 2u,
    /// Available battery capacity - 2 bytes\n
    /// Present only if #BAS_ENERGY_STATUS_FLAGS_AVAILABLE_ENERGY_CAPA_PRESENT_BIT set in Flags
    BAS_ENERGY_STATUS_SIZE_AVAILABLE_CAPACITY = 2u,
    /// Charge rate - 2 bytes\n
    /// Present only if #BAS_ENERGY_STATUS_FLAGS_CHARGE_RATE_PRESENT_BIT set in Flags
    BAS_ENERGY_STATUS_SIZE_CHARGE_RATE = 2u,
    /// Available energy at last charge - 2 bytes\n
    /// Present only if #BAS_ENERGY_STATUS_FLAGS_AVAILABLE_ENERGY_LAST_CHARGE_PRESENT_BIT set in Flags
    BAS_ENERGY_STATUS_SIZE_AVAILABLE_ENERGY_LAST_CHARGE = 2u,

    /// Maximum size
    BAS_ENERGY_STATUS_SIZE_MAX = 13u,
};

/// Flags bit field meaning for Battery Time Status characteristic
enum bas_time_status_flags_bf
{
    /// Time until discharged on standby present - Position
    BAS_TIME_STATUS_FLAGS_TIME_UNTIL_DISCHARGED_STANDBY_PRESENT_POS = 0u,
    /// Time until discharged on standby present - Bit
    BAS_TIME_STATUS_FLAGS_TIME_UNTIL_DISCHARGED_STANDBY_PRESENT_BIT
                                        = CO_BIT(BAS_TIME_STATUS_FLAGS_TIME_UNTIL_DISCHARGED_STANDBY_PRESENT_POS),

    /// Time until recharged present - Position
    BAS_TIME_STATUS_FLAGS_TIME_UNTIL_RECHARGED_PRESENT_POS = 1u,
    /// Time until recharged present - Bit
    BAS_TIME_STATUS_FLAGS_TIME_UNTIL_RECHARGED_PRESENT_BIT
                                        = CO_BIT(BAS_TIME_STATUS_FLAGS_TIME_UNTIL_RECHARGED_PRESENT_POS),
};

/// Position of fields for Battery Time Status characteristic value
enum bas_time_status_size
{
    /// Flags - 1 byte (see #bas_time_status_flags_bf enumeration)
    BAS_TIME_STATUS_SIZE_FLAGS = 1u,
    /// Time until discharged (in minutes) - 3 bytes\n
    BAS_TIME_STATUS_TIME_UNTIL_DISCHARGED = 3u,
    /// Time until discharged on standby (in minutes) - 3 bytes
    /// Present only if #BAS_TIME_STATUS_FLAGS_TIME_UNTIL_DISCHARGED_STANDBY_PRESENT_BIT set in Flags
    BAS_TIME_STATUS_TIME_UNTIL_DISCHARGED_STANDBY = 3u,
    /// Time until recharged (in minutes) - 3 bytes
    /// Present only if #BAS_TIME_STATUS_FLAGS_TIME_UNTIL_RECHARGED_PRESENT_BIT set in Flags
    BAS_TIME_STATUS_TIME_UNTIL_RECHARGED = 3u,

    /// Maximum size
    BAS_TIME_STATUS_SIZE_MAX = 10u,
};

/// Flags bit field meaning for Battery Health Status characteristic
enum bas_health_status_flags_bf
{
    /// Battery health summary present - Position
    BAS_HEALTH_STATUS_FLAGS_SUMMARY_PRESENT_POS = 0u,
    /// Battery health summary present - Bit
    BAS_HEALTH_STATUS_FLAGS_SUMMARY_PRESENT_BIT = CO_BIT(BAS_HEALTH_STATUS_FLAGS_SUMMARY_PRESENT_POS),

    /// Cycle count present - Position
    BAS_HEALTH_STATUS_FLAGS_CYCLE_COUNT_PRESENT_POS = 1u,
    /// Cycle count present - Bit
    BAS_HEALTH_STATUS_FLAGS_CYCLE_COUNT_PRESENT_BIT = CO_BIT(BAS_HEALTH_STATUS_FLAGS_CYCLE_COUNT_PRESENT_POS),

    /// Current temperature present - Position
    BAS_HEALTH_STATUS_FLAGS_CURRENT_TEMPERATURE_PRESENT_POS = 2u,
    /// Current temperature present - Bit
    BAS_HEALTH_STATUS_FLAGS_CURRENT_TEMPERATURE_PRESENT_BIT
                                            = CO_BIT(BAS_HEALTH_STATUS_FLAGS_CURRENT_TEMPERATURE_PRESENT_POS),

    /// Deep discharge count present - Position
    BAS_HEALTH_STATUS_FLAGS_DEEP_DISCHARGE_COUNT_PRESENT_POS = 3u,
    /// Deep discharge count present - Bit
    BAS_HEALTH_STATUS_FLAGS_DEEP_DISCHARGE_COUNT_PRESENT_BIT
                                = CO_BIT(BAS_HEALTH_STATUS_FLAGS_DEEP_DISCHARGE_COUNT_PRESENT_POS),
};

/// Position of fields for Battery Health Status characteristic value
enum bas_health_status_size
{
    /// Flags - 1 byte (see #bas_health_status_flags_bf enumeration)
    BAS_HEALTH_STATUS_SIZE_FLAGS = 1u,
    /// Battery health summary - 1 byte\n
    /// Present only if #BAS_HEALTH_STATUS_FLAGS_SUMMARY_PRESENT_BIT set in Flags
    BAS_HEALTH_STATUS_SIZE_SUMMARY = 1u,
    /// Cycle count - 2 bytes\n
    /// Present only if #BAS_HEALTH_STATUS_FLAGS_CYCLE_COUNT_PRESENT_BIT set in Flags
    BAS_HEALTH_STATUS_SIZE_CYCLE_COUNT = 2u,
    /// Current temperature (in celcius degrees) - 1 byte\n
    /// Present only if #BAS_HEALTH_STATUS_FLAGS_CURRENT_TEMPERATURE_PRESENT_BIT set in Flags
    BAS_HEALTH_STATUS_SIZE_CURRENT_TEMPERATURE = 1u,
    /// Deep discharge count - 2 bytes\n
    /// Present only if #BAS_HEALTH_STATUS_FLAGS_DEEP_DISCHARGE_COUNT_PRESENT_BIT set in Flags
    BAS_HEALTH_STATUS_SIZE_DEEP_DISCHARGE_COUNT = 2u,

    /// Maximum size
    BAS_HEALTH_STATUS_SIZE_MAX = 7u,
};

/// Flags bit field meaning for Battery Health Information characteristic
enum bas_health_info_flags_bf
{
    /// Cycle count designed lifetime present - Position
    BAS_HEALTH_INFO_FLAGS_CYCLE_COUNT_DESIGNED_LIFETIME_PRESENT_POS = 0u,
    /// Cycle count designed lifetime present - Bit
    BAS_HEALTH_INFO_FLAGS_CYCLE_COUNT_DESIGNED_LIFETIME_PRESENT_BIT
                                    = CO_BIT(BAS_HEALTH_INFO_FLAGS_CYCLE_COUNT_DESIGNED_LIFETIME_PRESENT_POS),

    /// Minimum and maximum designed operating temperature present - Position
    BAS_HEALTH_INFO_FLAGS_MIN_MAX_TEMPERATURE_PRESENT_POS = 1u,
    /// Minimum and maximum designed operating temperature present - Bit
    BAS_HEALTH_INFO_FLAGS_MIN_MAX_TEMPERATURE_PRESENT_BIT
                                        = CO_BIT(BAS_HEALTH_INFO_FLAGS_MIN_MAX_TEMPERATURE_PRESENT_POS),
};

/// Position of fields for Battery Health Information characteristic value
enum bas_health_info_size
{
    /// Flags - 1 byte (see #bas_health_info_flags_bf enumeration)
    BAS_HEALTH_INFO_SIZE_FLAGS = 1u,
    /// Cycle count designed lifetime - 2 bytes\n
    /// Present only if #BAS_HEALTH_INFO_FLAGS_CYCLE_COUNT_DESIGNED_LIFETIME_PRESENT_BIT bit set in Flags
    BAS_HEALTH_INFO_SIZE_CYCLE_COUNT_DESIGNED_LIFETIME = 2u,
    /// Minimum designed operating temperature (in celcius) - 1 byte\n
    /// Present only if #BAS_HEALTH_INFO_FLAGS_MIN_MAX_TEMPERATURE_PRESENT_BIT bit set in Flags
    BAS_HEALTH_INFO_SIZE_MIN_TEMPERATURE = 1u,
    /// Maximum designed operating temperature (in celcius) - 1 byte\n
    /// Present only if #BAS_HEALTH_INFO_FLAGS_MIN_MAX_TEMPERATURE_PRESENT_BIT bit set in Flags
    BAS_HEALTH_INFO_SIZE_MAX_TEMPERATURE = 1u,

    /// Maximum size
    BAS_HEALTH_INFO_SIZE_MAX = 5u,
};

/// Battery features bit field meaning
enum bas_features_bf
{
    /// Battery replaceable - Position
    BAS_FEATURES_REPLACEABLE_POS = 0u,
    /// Battery replaceable - Bit
    BAS_FEATURES_REPLACEABLE_BIT = CO_BIT(BAS_FEATURES_REPLACEABLE_POS),

    /// Battery rechargeable - Position
    BAS_FEATURES_RECHARGEABLE_POS = 1u,
    /// Battery rechargeable - Bit
    BAS_FEATURES_RECHARGEABLE_BIT = CO_BIT(BAS_FEATURES_RECHARGEABLE_POS),
};

/// Battery chemistry values
enum bas_chemistry
{
    /// Unknown
    BAS_CHEMISTRY_UNKNOWN = 0u,
    /// Alkaline (zinc-manganese dioxide)
    BAS_CHEMISTRY_ALKALINE,
    /// Lead acid
    BAS_CHEMISTRY_LEAD_ACID,
    /// Lithium (lithium-iron disulfide)
    BAS_CHEMISTRY_LITHIUM_IRON_DISULFIDE,
    /// Lithium (lithium-manganese dioxide)
    BAS_CHEMISTRY_LITHIUM_MANGANESE_DIOXIDE,
    /// Lithium Ion Li
    BAS_CHEMISTRY_LITHIUM_ION_LI,
    /// Lithium polymer
    BAS_CHEMISTRY_LITHIUM_POLYMER,
    /// Nickel oxyhydroxide
    BAS_CHEMISTRY_NICKEL_OXYHYDROXIDE,
    /// Nickel cadmium
    BAS_CHEMISTRY_NICKEL_CADMIUM,
    /// Nickel-metal hydride
    BAS_CHEMISTRY_NICKEL_METAL_HYBRIDE,
    /// Silver oxide
    BAS_CHEMISTRY_NICKEL_SILVER_OXIDE,
    /// Zinc chloride
    BAS_CHEMISTRY_NICKEL_ZINC_CHLORIDE,
    /// Zinc air
    BAS_CHEMISTRY_NICKEL_ZINC_AIR,
    /// Zinc carbon
    BAS_CHEMISTRY_NICKEL_ZINC_CARBON,
};

/// Flags bit field meaning for Battery Information characteristic
enum bas_info_flags_bf
{
    /// Battery manufacture date present - Position
    BAS_INFO_FLAGS_MANUFACTURE_DATE_PRESENT_POS = 0u,
    /// Battery manufacture date present - Bit
    BAS_INFO_FLAGS_MANUFACTURE_DATE_PRESENT_BIT = CO_BIT(BAS_INFO_FLAGS_MANUFACTURE_DATE_PRESENT_POS),

    /// Battery expiration date present - Position
    BAS_INFO_FLAGS_EXPIRATION_DATE_PRESENT_POS = 1u,
    /// Battery expiration date present - Bit
    BAS_INFO_FLAGS_EXPIRATION_DATE_PRESENT_BIT = CO_BIT(BAS_INFO_FLAGS_EXPIRATION_DATE_PRESENT_POS),

    /// Battery designed capacity present - Position
    BAS_INFO_FLAGS_DESIGNED_CAPACITY_PRESENT_POS = 2u,
    /// Battery designed capacity present - Bit
    BAS_INFO_FLAGS_DESIGNED_CAPACITY_PRESENT_BIT = CO_BIT(BAS_INFO_FLAGS_DESIGNED_CAPACITY_PRESENT_POS),

    /// Battery low energy present - Position
    BAS_INFO_FLAGS_LOW_ENERGY_PRESENT_POS = 3u,
    /// Battery low energy present - Bit
    BAS_INFO_FLAGS_LOW_ENERGY_PRESENT_BIT = CO_BIT(BAS_INFO_FLAGS_LOW_ENERGY_PRESENT_POS),

    /// Battery critical energy present - Position
    BAS_INFO_FLAGS_CRITICAL_ENERGY_PRESENT_POS = 4u,
    /// Battery critical energy present - Bit
    BAS_INFO_FLAGS_CRITICAL_ENERGY_PRESENT_BIT = CO_BIT(BAS_INFO_FLAGS_CRITICAL_ENERGY_PRESENT_POS),

    /// Battery chemistry present - Position
    BAS_INFO_FLAGS_CHEMISTRY_PRESENT_POS = 5u,
    /// Battery chemistry present - Bit
    BAS_INFO_FLAGS_CHEMISTRY_PRESENT_BIT = CO_BIT(BAS_INFO_FLAGS_CHEMISTRY_PRESENT_POS),

    /// Nominal voltage present - Position
    BAS_INFO_FLAGS_NOMINAL_VOLTAGE_PRESENT_POS = 6u,
    /// Nominal voltage present - Bit
    BAS_INFO_FLAGS_NOMINAL_VOLTAGE_PRESENT_BIT = CO_BIT(BAS_INFO_FLAGS_NOMINAL_VOLTAGE_PRESENT_POS),

    /// Battery aggregation group present - Position
    BAS_INFO_FLAGS_AGGREGATION_GROUP_PRESENT_POS = 7u,
    /// Battery aggregation group present - Bit
    BAS_INFO_FLAGS_AGGREGATION_GROUP_PRESENT_BIT = CO_BIT(BAS_INFO_FLAGS_AGGREGATION_GROUP_PRESENT_POS),
};

/// Position of fields for Battery Information characteristic value
enum bas_info_size
{
    /// Flags - 2 bytes (see #bas_info_flags_bf enumeration)
    BAS_INFO_SIZE_FLAGS = 2u,
    /// Battery features - 1 byte (see #bas_features_bf enumeration)
    BAS_INFO_SIZE_FEATURES = 1u,
    /// Battery manufacture date - 3 bytes\n
    /// Present only if #BAS_INFO_FLAGS_MANUFACTURE_DATE_PRESENT_BIT bit set in Flags
    BAS_INFO_SIZE_MANUFACTURE_DATE = 3u,
    /// Battery expiration date - 3 bytes\n
    /// Present only if #BAS_INFO_FLAGS_EXPIRATION_DATE_PRESENT_BIT bit set in Flags
    BAS_INFO_SIZE_EXPIRATION_DATE = 3u,
    /// Battery designed capacity - 2 bytes\n
    /// Present only if #BAS_INFO_FLAGS_DESIGNED_CAPACITY_PRESENT_BIT bit set in Flags
    BAS_INFO_SIZE_DESIGNED_CAPACITY = 2u,
    /// Battery low energy - 2 bytes\n
    /// Present only if #BAS_INFO_FLAGS_LOW_ENERGY_PRESENT_BIT bit set in Flags
    BAS_INFO_SIZE_LOW_ENERGY = 2u,
    /// Battery critical energy - 2 bytes\n
    /// Present only if #BAS_INFO_FLAGS_CRITICAL_ENERGY_PRESENT_BIT bit set in Flags
    BAS_INFO_SIZE_CRITICAL_ENERGY = 2u,
    /// Battery chemistry - 1 byte (see #bas_chemistry enumeration)\n
    /// Present only if #BAS_INFO_FLAGS_CHEMISTRY_PRESENT_BIT bit set in Flags
    BAS_INFO_SIZE_CHEMISTRY = 1u,
    /// Nominal voltage - 2 bytes\n
    /// Present only if #BAS_INFO_FLAGS_NOMINAL_VOLTAGE_PRESENT_BIT bit set in Flags
    BAS_INFO_SIZE_NOMINAL_VOLTAGE = 2u,
    /// Battery aggregation group - 1 byte\n
    /// Present only if #BAS_INFO_FLAGS_AGGREGATION_GROUP_PRESENT_BIT bit set in Flags
    BAS_INFO_SIZE_AGGREGATION_GROUP = 1u,

    /// Maximum size
    BAS_INFO_SIZE_MAX = 19u,
};

/// Position of fields for Estimated Service Date characteristic value
enum bas_service_date_size
{
    /// Service date - 3 bytes
    BAS_SERVICE_DATE_SIZE_DATE = 3u,

    /// Maximum size
    BAS_SERVICE_DATE_SIZE_MAX = 3u,
};

/// @} BAS_API_COMMON

#endif // BAS_H_
