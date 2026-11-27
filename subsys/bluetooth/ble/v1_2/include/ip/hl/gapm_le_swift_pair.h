/**
 ****************************************************************************************
 *
 * @file gapm_le_swift_pair.h
 *
 * @brief Generic Access Profile Manager - Microsoft Swift Pair
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPM_LE_SWIFT_PAIR_H_
#define GAPM_LE_SWIFT_PAIR_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup GAPM_LE_SWIFT_PAIR Microsoft Swift Pair
 *
 * @ingroup GAPM_LE_ADV_ACTV_API
 *
 * @brief Definitions allowing to add advertising data specific for Microsoft Swift Pair feature support.
 * Depending on the scenario, application shall contains one of following payloads:
 *
 * - Pairing over Bluetooth LE only
 * \image html gapm_le_swift_pair_scenario_0.png "" width=50%
 * \n
 *
 * - Pairing over BR/EDR only, using Bluetooth LE for discovery
 * \image html gapm_le_swift_pair_scenario_1.png "" width=80%
 * \n
 *
 * - Pairing over Bluetooth LE and BR/EDR with Secure Connection
 * \image html gapm_le_swift_pair_scenario_2.png "" width=50%
 *
 * More details on https://learn.microsoft.com/en-us/windows-hardware/design/component-guidelines/bluetooth-swift-pair
 *
 * @{
 ****************************************************************************************
 */


/*
 * DEFINES
 ****************************************************************************************
 */

/// Minimal length of advertising payload for Pairing over Bluetooth LE only case
#define GAPM_LE_SWIFT_PAIR_MIN_LENGTH_LE_ONLY (7u)
/// Minimal length of advertising payload for Pairing over Bluetooth LE and BR/EDR with Secure Connections case
#define GAPM_LE_SWIFT_PAIR_MIN_LENGTH_DM (10u)
/// Minimal length of advertising payload for Pairing over BR/EDR only, using Bluetooth LE for discovery
#define GAPM_LE_SWIFT_PAIR_MIN_LENGTH_BT_ONLY (16u)

/// Microsoft Beacon ID
#define GAPM_LE_SWIFT_PAIR_BEACON_ID (0x03u)
/// Reserved RSSI Byte value
#define GAPM_LE_SWIFT_PAIR_RESERVED_RSSI (0x80u)

/// Microsoft Vendor ID - Byte 0
#define GAPM_LE_SWIFT_PAIR_VENDOR_ID_LOW (0x06u)
/// Microsoft Vendor ID - Byte 1
#define GAPM_LE_SWIFT_PAIR_VENDOR_ID_HIGH (0x00u)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Microsoft Beacon Sub Scenario value
enum gapm_le_swift_pair_sub_scenario
{
    /// Pairing over Bluetooth LE only case
    GAPM_LE_SWIFT_PAIR_SCENARIO_LE_ONLY = 0u,
    /// Pairing over BR/EDR only, using Bluetooth LE for discovery
    GAPM_LE_SWIFT_PAIR_SCENARIO_BT_ONLY,
    /// Pairing over Bluetooth LE and BR/EDR with Secure Connections
    GAPM_LE_SWIFT_PAIR_SCENARIO_DM
};

/// @} GAPM_LE_SWIFT_PAIR

#endif // GAPM_LE_SWIFT_PAIR_H_
