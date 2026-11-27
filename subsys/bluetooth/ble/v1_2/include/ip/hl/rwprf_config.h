/**
 ****************************************************************************************
 *
 * @file rwprf_config.h
 *
 * @brief Header file - Profile Configuration
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef RWPRF_CONFIG_H_
#define RWPRF_CONFIG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup PRF_CONFIG_API
 * @ingroup PROFILE_API
 * @brief Profile configuration
 *
 * @{
 ****************************************************************************************
 */
// ----------------------------  BT Dual Mode Profiles -------------------------------
#define GFP_PROVIDER         0

// ----------------------------  BT Low Energy Profiles -------------------------------

//ATT DB,Testing and Qualification related flags
#if (HL_LE_CENTRAL || HL_LE_PERIPHERAL)
/// Proximity Profile Monitor Role
#if defined(CFG_PRF_PXPM)
#define BLE_PROX_MONITOR        1
#else
#define BLE_PROX_MONITOR        0
#endif // defined(CFG_PRF_PXPM)

/// Proximity Profile Reporter Role
#if defined(CFG_PRF_PXPR)
#define BLE_PROX_REPORTER       1
#else
#define BLE_PROX_REPORTER       0
#endif // defined(CFG_PRF_PXPR)

///Find Me Profile Locator role
#if defined(CFG_PRF_FMPL)
#define BLE_FINDME_LOCATOR      1
#else
#define BLE_FINDME_LOCATOR      0
#endif // defined(CFG_PRF_FMPL)

///Find Me Profile Target role
#if defined(CFG_PRF_FMPT)
#define BLE_FINDME_TARGET       1
#else
#define BLE_FINDME_TARGET       0
#endif // defined(CFG_PRF_FMPT)

///Health Thermometer Profile Collector Role
#if defined(CFG_PRF_HTPC)
#define BLE_HT_COLLECTOR        1
#else
#define BLE_HT_COLLECTOR        0
#endif // defined(CFG_PRF_HTPC)

///Health Thermometer Profile Thermometer Role
#if defined(CFG_PRF_HTPT)
#define BLE_HT_THERMOM          1
#else
#define BLE_HT_THERMOM          0
#endif // defined(CFG_PRF_HTPT)

///Device Information Service Client Role
#if defined(CFG_PRF_DISC)
#define BLE_DIS_CLIENT          1
#else
#define BLE_DIS_CLIENT          0
#endif // defined(CFG_PRF_DISC)

///Device Information Service Server Role
#if defined(CFG_PRF_DISS)
#define BLE_DIS_SERVER          1
#else
#define BLE_DIS_SERVER          0
#endif // defined(CFG_PRF_DISS)

///Blood Pressure Profile Collector Role
#if defined(CFG_PRF_BLPC)
#define BLE_BP_COLLECTOR        1
#else
#define BLE_BP_COLLECTOR        0
#endif // defined(CFG_PRF_BLPC)

///Blood Pressure Profile Sensor Role
#if defined(CFG_PRF_BLPS)
#define BLE_BP_SENSOR           1
#else
#define BLE_BP_SENSOR           0
#endif // defined(CFG_PRF_BLPS)

///Time Profile Client Role
#if defined(CFG_PRF_TIPC)
#define BLE_TIP_CLIENT          1
#else
#define BLE_TIP_CLIENT          0
#endif // defined(CFG_PRF_TIPC)

///Time Profile Server Role
#if defined(CFG_PRF_TIPS)
#define BLE_TIP_SERVER          1
#else
#define BLE_TIP_SERVER          0
#endif // defined(CFG_PRF_TIPS)

///Heart Rate Profile Collector Role
#if defined(CFG_PRF_HRPC)
#define BLE_HR_COLLECTOR        1
#else
#define BLE_HR_COLLECTOR        0
#endif // defined(CFG_PRF_HRPC)

///Heart Rate Profile Sensor Role
#if defined(CFG_PRF_HRPS)
#define BLE_HR_SENSOR           1
#else
#define BLE_HR_SENSOR           0
#endif // defined(CFG_PRF_HRPS)

///Scan Parameter Profile Client Role
#if defined(CFG_PRF_SCPPC)
#define BLE_SP_CLIENT           1
#else
#define BLE_SP_CLIENT           0
#endif // defined(CFG_PRF_SCPPC)

///Scan Parameter Profile Server Role
#if defined(CFG_PRF_SCPPS)
#define BLE_SP_SERVER           1
#else
#define BLE_SP_SERVER           0
#endif // defined(CFG_PRF_SCPPS)

///Battery Service Client Role
#if defined(CFG_PRF_BASC)
#define BLE_BATT_CLIENT         1
#define BASC_NB_BAS_INSTANCES_MAX (2) // Maximum number of Battery Service instances we can handle
#else
#define BLE_BATT_CLIENT         0
#endif // defined(CFG_PRF_BASC)

///Battery Service Server Role
#if defined(CFG_PRF_BASS)
#define BLE_BATT_SERVER         1
#define BASS_NB_BAS_INSTANCES_MAX (2) // Maximal number of BAS that can be added in the DB
#else
#define BLE_BATT_SERVER         0
#endif // defined(CFG_PRF_BASS)

///HID Device Role
#if defined(CFG_PRF_HOGPD)
#define BLE_HID_DEVICE          1
#else
#define BLE_HID_DEVICE          0
#endif // defined(CFG_PRF_HOGPD)

///HID Boot Host Role
#if defined(CFG_PRF_HOGPBH)
#define BLE_HID_BOOT_HOST       1
#else
#define BLE_HID_BOOT_HOST       0
#endif // defined(CFG_PRF_HOGPBH)

///HID Report Host Role
#if defined(CFG_PRF_HOGPRH)
#define BLE_HID_REPORT_HOST     1
#else
#define BLE_HID_REPORT_HOST     0
#endif // defined(CFG_PRF_HOGPRH)

/// Glucose Profile Collector Role
#if defined(CFG_PRF_GLPC)
#define BLE_GL_COLLECTOR        1
#else
#define BLE_GL_COLLECTOR        0
#endif // defined(CFG_PRF_GLPC)

/// Glucose Profile Sensor Role
#if defined(CFG_PRF_GLPS)
#define BLE_GL_SENSOR           1
#else
#define BLE_GL_SENSOR           0
#endif // defined(CFG_PRF_GLPS)

/// Running Speed and Cadence Profile Collector Role
#if defined(CFG_PRF_RSCPC)
#define BLE_RSC_COLLECTOR       1
#else
#define BLE_RSC_COLLECTOR       0
#endif // defined(CFG_PRF_RSCPC)

/// Running Speed and Cadence Profile Server Role
#if defined(CFG_PRF_RSCPS)
#define BLE_RSC_SENSOR          1
#else
#define BLE_RSC_SENSOR          0
#endif // defined(CFG_PRF_RSCPS)

/// Cycling Speed and Cadence Profile Collector Role
#if defined(CFG_PRF_CSCPC)
#define BLE_CSC_COLLECTOR       1
#else
#define BLE_CSC_COLLECTOR       0
#endif // defined(CFG_PRF_CSCPC)

/// Cycling Speed and Cadence Profile Server Role
#if defined(CFG_PRF_CSCPS)
#define BLE_CSC_SENSOR          1
#else
#define BLE_CSC_SENSOR          0
#endif // defined(CFG_PRF_CSCPS)

/// Cycling Power Profile Collector Role
#if defined(CFG_PRF_CPPC)
#define BLE_CP_COLLECTOR        1
#else
#define BLE_CP_COLLECTOR        0
#endif // defined (CFG_PRF_CPPC)

/// Cycling Power Profile Server Role
#if defined(CFG_PRF_CPPS)
#define BLE_CP_SENSOR            1
#else
#define BLE_CP_SENSOR            0
#endif // defined (CFG_PRF_CPPS)

/// Location and Navigation Profile Collector Role
#if defined(CFG_PRF_LANC)
#define BLE_LN_COLLECTOR        1
#else
#define BLE_LN_COLLECTOR        0
#endif // defined (CFG_PRF_LANC)

/// Location and Navigation Profile Server Role
#if defined(CFG_PRF_LANS)
#define BLE_LN_SENSOR            1
#else
#define BLE_LN_SENSOR            0
#endif // defined (CFG_PRF_LANS)

/// Alert Notification Profile Client Role
#if defined(CFG_PRF_ANPC)
#define BLE_AN_CLIENT           1
#else
#define BLE_AN_CLIENT           0
#endif // defined(CFG_PRF_ANPC)

/// Alert Notification Profile Server Role
#if defined(CFG_PRF_ANPS)
#define BLE_AN_SERVER           1
#else
#define BLE_AN_SERVER           0
#endif // defined(CFG_PRF_ANPS)

/// Phone Alert Status Profile Client Role
#if defined(CFG_PRF_PASPC)
#define BLE_PAS_CLIENT          1
#else
#define BLE_PAS_CLIENT          0
#endif // defined(CFG_PRF_PASPC)

/// Phone Alert Status Profile Server Role
#if defined(CFG_PRF_PASPS)
#define BLE_PAS_SERVER          1
#else
#define BLE_PAS_SERVER          0
#endif // defined(CFG_PRF_PASPS)

/// Internet Protocol Support Profile Server Role
#if defined(CFG_PRF_IPSS)
#define BLE_IPS_SERVER          1
#else
#define BLE_IPS_SERVER          0
#endif // defined(CFG_PRF_IPSS)

/// Internet Protocol Support Profile Client Role
#if defined(CFG_PRF_IPSC)
#define BLE_IPS_CLIENT          1
#else
#define BLE_IPS_CLIENT          0
#endif // defined(CFG_PRF_IPSC)

/// Environmental Sensing Profile Server Role
#if defined(CFG_PRF_ENVS)
#define BLE_ENV_SERVER          1
#else
#define BLE_ENV_SERVER          0
#endif // defined(CFG_PRF_ENVS)

/// Environmental Sensing Profile Client Role
#if defined(CFG_PRF_ENVC)
#define BLE_ENV_CLIENT          1
#else
#define BLE_ENV_CLIENT          0
#endif // defined(CFG_PRF_ENVC)

/// Weight Scale Profile Server Role
#if defined(CFG_PRF_WSCS)
#define BLE_WSC_SERVER          1
#else
#define BLE_WSC_SERVER          0
#endif // defined(CFG_PRF_WSCS)

/// Weight Scale Profile Client Role
#if defined(CFG_PRF_WSCC)
#define BLE_WSC_CLIENT          1
#else
#define BLE_WSC_CLIENT          0
#endif // defined(CFG_PRF_WSCC)

/// Body Composition Server Role
#if defined(CFG_PRF_BCSS)
#define BLE_BCS_SERVER          1
#else
#define BLE_BCS_SERVER          0
#endif // defined(CFG_PRF_BCSS)

/// Body Composition Client Role
#if defined(CFG_PRF_BCSC)
#define BLE_BCS_CLIENT          1
#else
#define BLE_BCS_CLIENT          0
#endif // defined(CFG_PRF_BCSC)

/// User Data Service Server Role
#if defined(CFG_PRF_UDSS)
#define BLE_UDS_SERVER          1
#else
#define BLE_UDS_SERVER          0
#endif // defined(CFG_PRF_UDSS)

/// User Data Service Client Role
#if defined(CFG_PRF_UDSC)
#define BLE_UDS_CLIENT          1
#else
#define BLE_UDS_CLIENT          0
#endif // defined(CFG_PRF_UDSC)

/// Wireless Power Transfer Profile Server Role
#if defined(CFG_PRF_WPTS)
#define BLE_WPT_SERVER          1
#else
#define BLE_WPT_SERVER          0
#endif // defined(CFG_PRF_WPTS)

/// Wireless Power Transfer Profile Client Role
#if defined(CFG_PRF_WPTC)
#define BLE_WPT_CLIENT          1
#else
#define BLE_WPT_CLIENT          0
#endif // defined(CFG_PRF_WPTC)

/// Pulse Oximeter Profile Server Role
#if defined(CFG_PRF_PLXS)
#define BLE_PLX_SERVER          1
#else
#define BLE_PLX_SERVER          0
#endif // defined(CFG_PRF_PLXS)

/// Pulse Oximete Profile Client Role
#if defined(CFG_PRF_PLXC)
#define BLE_PLX_CLIENT          1
#else
#define BLE_PLX_CLIENT          0
#endif // defined(CFG_PRF_PLXC)

/// Continuous Glucose Monitoring Profile Server Role
#if (defined(CFG_PRF_CGMSS) || defined(CFG_PRF_CGMS))
#define BLE_CGMS_SERVER         1
#else
#define BLE_CGMS_SERVER         0
#endif // (defined(CFG_PRF_CGMSS) || defined(CFG_PRF_CGMS))

/// Continuous Glucose Monitoring Profile Client Role
#if (defined(CFG_PRF_CGMSC) || defined(CFG_PRF_CGMC))
#define BLE_CGMS_CLIENT         1
#else
#define BLE_CGMS_CLIENT         0
#endif // (defined(CFG_PRF_CGMSC) || defined(CFG_PRF_CGMC))

/// Coordinated Set Identification Profile Set Member Role
#if defined(CFG_PRF_CSISM)
#define BLE_CSIS_MEMBER         1
#if defined(CFG_HL_MSG_API) && (!defined(CFG_ATC_CSISM) || defined(CFG_GAF_MSG_DBG))
#define BLE_CSISM_MSG_API       1
#else //defined(CFG_HL_MSG_API) && (!defined(CFG_ATC_CSISM) || defined(CFG_GAF_MSG_DBG))
#define BLE_CSISM_MSG_API       0
#endif //defined(CFG_HL_MSG_API) && (!defined(CFG_ATC_CSISM) || defined(CFG_GAF_MSG_DBG))
#else
#define BLE_CSIS_MEMBER         0
#define BLE_CSISM_MSG_API       0
#endif // defined(CFG_PRF_CSISM)

#if defined(CFG_PRF_CSISC)
#define BLE_CSIS_COORD          1
#if defined(CFG_HL_MSG_API) && (!defined(CFG_ATC_CSISC) || defined(CFG_GAF_MSG_DBG))
#define BLE_CSISC_MSG_API       1
#else //defined(CFG_HL_MSG_API) && (!defined(CFG_ATC_CSISC) || defined(CFG_GAF_MSG_DBG))
#define BLE_CSISC_MSG_API       0
#endif //defined(CFG_HL_MSG_API) && (!defined(CFG_ATC_CSISC) || defined(CFG_GAF_MSG_DBG))
#else
#define BLE_CSIS_COORD          0
#define BLE_CSISC_MSG_API       0
#endif // defined(CFG_PRF_CSISC)

/// Object Transfer Profile Server Role
#if defined(CFG_PRF_OTS)
#define BLE_OT_SERVER           1
#if defined(CFG_HL_MSG_API) && (!defined(CFG_ACC_OTS) || defined(CFG_GAF_MSG_DBG))
#define BLE_OTS_MSG_API         1
#else //defined(CFG_HL_MSG_API) && (!defined(CFG_ACC_OTS) || defined(CFG_GAF_MSG_DBG))
#define BLE_OTS_MSG_API         0
#endif //defined(CFG_HL_MSG_API) && (!defined(CFG_ACC_OTS) || defined(CFG_GAF_MSG_DBG))
#else
#define BLE_OT_SERVER           0
#define BLE_OTS_MSG_API         0
#endif // defined(CFG_PRF_OTS)

/// Object Transfer Profile Client Role
#if defined(CFG_PRF_OTC)
#define BLE_OT_CLIENT           1
#if defined(CFG_HL_MSG_API) && (!defined(CFG_ACC_OTC) || defined(CFG_GAF_MSG_DBG))
#define BLE_OTC_MSG_API         1
#else //defined(CFG_HL_MSG_API) && (!defined(CFG_ACC_OTC) || defined(CFG_GAF_MSG_DBG))
#define BLE_OTC_MSG_API         0
#endif //defined(CFG_HL_MSG_API) && (!defined(CFG_ACC_OTC) || defined(CFG_GAF_MSG_DBG))
#else
#define BLE_OT_CLIENT           0
#define BLE_OTC_MSG_API         0
#endif // defined(CFG_PRF_OTC)

/// Bond Management Service Server Role
#if defined(CFG_PRF_BMSS)
#define BLE_BMS_SERVER          1
#else
#define BLE_BMS_SERVER          0
#endif // defined(CFG_PRF_BMSS)

/// Bond Management Service Client Role
#if defined(CFG_PRF_BMSC)
#define BLE_BMS_CLIENT          1
#else
#define BLE_BMS_CLIENT          0
#endif // defined(CFG_PRF_BMSC)

/// Throughput tester profile for debug usage
#if defined(CFG_PRF_THPP)
#define BLE_DBG_THPP            1
#else
#define BLE_DBG_THPP            0
#endif // defined(CFG_PRF_DISS)

/// ESL Service Access point role
#define BLE_ESL_AP          0

/// ESL Service TAG role
#define BLE_ESL_TAG          0

/// Ranging Service Server role
#if defined(CFG_PRF_RASS) // && (0)
#define BLE_RAS_SERVER          1
#else
#define BLE_RAS_SERVER          0
#endif // defined(CFG_PRF_RASS) && (0)

/// Ranging Service Client role
#if defined(CFG_PRF_RASC) // && (0)
#define BLE_RAS_CLIENT          1
#else
#define BLE_RAS_CLIENT          0
#endif // defined(CFG_PRF_RASS) && (0)

/// Immediate Alert Service Server role
#if defined(CFG_PRF_IASS)
#define BLE_IAS_SERVER (1)
#else // defined(CFG_PRF_IASS)
#define BLE_IAS_SERVER (0)
#endif // defined(CFG_PRF_IASS)

/// Immediate Alert Service Client role
#if defined(CFG_PRF_IASC)
#define BLE_IAS_CLIENT (1)
#else // defined(CFG_PRF_IASC)
#define BLE_IAS_CLIENT (0)
#endif // defined(CFG_PRF_IASC)

/// Link Loss Service Server role
#if defined(CFG_PRF_LLSS)
#define BLE_LLS_SERVER (1)
#else // defined(CFG_PRF_LLSS)
#define BLE_LLS_SERVER (0)
#endif // defined(CFG_PRF_LLSS)

/// Link Loss Service Client role
#if defined(CFG_PRF_LLSC)
#define BLE_LLS_CLIENT (1)
#else // defined(CFG_PRF_LLSC)
#define BLE_LLS_CLIENT (0)
#endif // defined(CFG_PRF_LLSC)

/// Tx Power Service Server role
#if defined(CFG_PRF_TPSS)
#define BLE_TPS_SERVER (1)
#else // defined(CFG_PRF_TPSS)
#define BLE_TPS_SERVER (0)
#endif // defined(CFG_PRF_TPSS)

/// Tx Power Service Client role
#if defined(CFG_PRF_TPSC)
#define BLE_TPS_CLIENT (1)
#else // defined(CFG_PRF_TPSC)
#define BLE_TPS_CLIENT (0)
#endif // defined(CFG_PRF_TPSC)

/// BLE_CLIENT_PRF indicates if at least one client profile is present
#if (BLE_PROX_MONITOR || BLE_FINDME_LOCATOR || BLE_HT_COLLECTOR || BLE_BP_COLLECTOR \
        || BLE_HR_COLLECTOR || BLE_DIS_CLIENT || BLE_TIP_CLIENT || BLE_SP_CLIENT \
        || BLE_BATT_CLIENT || BLE_GL_COLLECTOR || BLE_HID_BOOT_HOST || BLE_HID_REPORT_HOST \
        || BLE_RSC_COLLECTOR || BLE_CSC_COLLECTOR || BLE_CP_COLLECTOR || BLE_LN_COLLECTOR || BLE_AN_CLIENT \
        || BLE_PAS_CLIENT || BLE_IPS_CLIENT || BLE_ENV_CLIENT || BLE_WSC_CLIENT \
        || BLE_UDS_CLIENT || BLE_BCS_CLIENT || BLE_WPT_CLIENT || BLE_PLX_CLIENT \
        || BLE_CGMS_CLIENT || BLE_CSIS_COORD || BLE_OT_CLIENT || BLE_DBG_THPP || BLE_MESH || BLE_GAF_PRESENT \
        || BLE_BMS_CLIENT || BLE_ESL_AP || BLE_RAS_CLIENT || BLE_IAS_CLIENT || BLE_LLS_CLIENT || BLE_TPS_CLIENT)
#define BLE_CLIENT_PRF          1
#else
#define BLE_CLIENT_PRF          0
#endif //(BLE_PROX_MONITOR || BLE_FINDME_LOCATOR ...)

/// BLE_SERVER_PRF indicates if at least one server profile is present
#if (BLE_PROX_REPORTER || BLE_FINDME_TARGET || BLE_HT_THERMOM || BLE_BP_SENSOR \
        || BLE_TIP_SERVER || BLE_HR_SENSOR || BLE_DIS_SERVER || BLE_SP_SERVER \
        || BLE_BATT_SERVER || BLE_HID_DEVICE || BLE_GL_SENSOR || BLE_RSC_SENSOR  \
        || BLE_CSC_SENSOR || BLE_CP_SENSOR || BLE_LN_SENSOR || BLE_AN_SERVER \
        || BLE_PAS_SERVER || BLE_IPS_SERVER || BLE_ENV_SERVER || BLE_WSC_SERVER \
        || BLE_UDS_SERVER || BLE_BCS_SERVER || BLE_WPT_SERVER || BLE_PLX_SERVER \
        || BLE_CGMS_SERVER || BLE_CSIS_MEMBER || BLE_OT_SERVER || BLE_DBG_THPP || BLE_MESH || BLE_GAF_PRESENT \
        || GFP_PROVIDER || (0) || BLE_BMS_SERVER || BLE_ESL_TAG || BLE_RAS_SERVER \
        || BLE_IAS_SERVER || BLE_LLS_SERVER || BLE_TPS_SERVER)
#define BLE_SERVER_PRF          1
#else
#define BLE_SERVER_PRF          0
#endif //(BLE_PROX_REPORTER || BLE_FINDME_TARGET ...)

#if ((0) || BLE_WPT_SERVER || BLE_WPT_CLIENT)
#define BLE_PRF_PROPRIETARY_SVC_SUPPORT     (1)
#else // !((0) || BLE_WPT_SERVER || BLE_WPT_CLIENT)
#define BLE_PRF_PROPRIETARY_SVC_SUPPORT     (0)
#endif // ((0) || BLE_WPT_SERVER || BLE_WPT_CLIENT)

//Force ATT parts depending on profile roles or compile options
/// Attribute Client
#if (BLE_CLIENT_PRF)
#define BLE_ATTC                    1
#endif //(BLE_CLIENT_PRF)

/// Attribute Server
#if (BLE_SERVER_PRF)
#define BLE_ATTS                    1
#endif //(BLE_SERVER_PRF)

#else
#define BLE_PROX_MONITOR        0
#define BLE_PROX_REPORTER       0
#define BLE_FINDME_LOCATOR      0
#define BLE_FINDME_TARGET       0
#define BLE_HT_COLLECTOR        0
#define BLE_HT_THERMOM          0
#define BLE_DIS_CLIENT          0
#define BLE_DIS_SERVER          0
#define BLE_BP_COLLECTOR        0
#define BLE_BP_SENSOR           0
#define BLE_TIP_CLIENT          0
#define BLE_TIP_SERVER          0
#define BLE_HR_COLLECTOR        0
#define BLE_HR_SENSOR           0
#define BLE_SP_CLIENT           0
#define BLE_SP_SERVER           0
#define BLE_BATT_CLIENT         0
#define BLE_BATT_SERVER         0
#define BLE_HID_DEVICE          0
#define BLE_HID_BOOT_HOST       0
#define BLE_HID_REPORT_HOST     0
#define BLE_GL_COLLECTOR        0
#define BLE_GL_SENSOR           0
#define BLE_RSC_COLLECTOR       0
#define BLE_RSC_SENSOR          0
#define BLE_CSC_COLLECTOR       0
#define BLE_CSC_SENSOR          0
#define BLE_CP_COLLECTOR        0
#define BLE_CP_SENSOR            0
#define BLE_LN_COLLECTOR        0
#define BLE_LN_SENSOR            0
#define BLE_AN_CLIENT           0
#define BLE_AN_SERVER           0
#define BLE_PAS_CLIENT          0
#define BLE_PAS_SERVER          0
#define BLE_IPS_SERVER          0
#define BLE_IPS_CLIENT          0
#define BLE_ENV_SERVER          0
#define BLE_ENV_CLIENT          0
#define BLE_WSC_SERVER          0
#define BLE_WSC_CLIENT          0
#define BLE_BCS_SERVER          0
#define BLE_BCS_CLIENT          0
#define BLE_UDS_SERVER          0
#define BLE_UDS_CLIENT          0
#define BLE_WPT_SERVER          0
#define BLE_WPT_CLIENT          0
#define BLE_PLX_SERVER          0
#define BLE_PLX_CLIENT          0
#define BLE_CGMS_SERVER         0
#define BLE_CGMS_CLIENT         0
#define BLE_CSIS_MEMBER         0
#define BLE_CSISM_MSG_API       0
#define BLE_CSIS_COORD          0
#define BLE_CSISC_MSG_API       0
#define BLE_OT_SERVER           0
#define BLE_OTS_MSG_API         0
#define BLE_OT_CLIENT           0
#define BLE_OTC_MSG_API         0
#define BLE_BMS_SERVER          0
#define BLE_BMS_CLIENT          0
#define BLE_DBG_THPP            0
#define BLE_PRF_PROPRIETARY_SVC_SUPPORT 0
#define BLE_EXT_ATTS_DB         0
#define BLE_SERVER_PRF          0
#define BLE_CLIENT_PRF          0
#define BLE_ESL_AP              0
#define BLE_ESL_TAG             0
#define BLE_RAS_SERVER          0
#define BLE_RAS_CLIENT          0
#define BLE_IAS_SERVER (0)
#define BLE_IAS_CLIENT (0)
#define BLE_LLS_SERVER (0)
#define BLE_LLS_CLIENT (0)
#define BLE_TPS_SERVER (0)
#define BLE_TPS_CLIENT (0)
#endif //(HL_LE_CENTRAL || HL_LE_PERIPHERAL)

// ----------------------------  BT Classic Profiles -------------------------------
#define BT_A2DP_SINK          0
#define BT_A2DP_SOURCE        0
#define BT_AVRCP              0
#define BT_HFP_AG             0
#define BT_HFP_HF             0
#define BT_SPP                0
#define BT_PBAP_CLIENT        0
#define MFI_IAP             0

/// Profile identifiers
enum rwprf_id
{
    #if (HL_LE_CENTRAL || HL_LE_PERIPHERAL)
    #if (BLE_HT_THERMOM)
    PRF_ID_HTPT,
    #endif // (BLE_HT_THERMOM)

    #if (BLE_HT_COLLECTOR)
    PRF_ID_HTPC,
    #endif // (BLE_HT_COLLECTOR)

    #if (BLE_DIS_SERVER)
    PRF_ID_DISS,
    #endif // (BLE_DIS_SERVER)

    #if (BLE_DIS_CLIENT)
    PRF_ID_DISC,
    #endif // (BLE_DIS_CLIENT)

    #if (BLE_BP_SENSOR)
    PRF_ID_BLPS,
    #endif // (BLE_BP_SENSOR)

    #if (BLE_BP_COLLECTOR)
    PRF_ID_BLPC,
    #endif // (BLE_BP_COLLECTOR)

    #if (BLE_TIP_SERVER)
    PRF_ID_TIPS,
    #endif // (BLE_TIP_SERVER)

    #if (BLE_TIP_CLIENT)
    PRF_ID_TIPC,
    #endif // (BLE_TIP_CLIENT)

    #if (BLE_HR_SENSOR)
    PRF_ID_HRPS,
    #endif // (BLE_HR_SENSOR)

    #if (BLE_HR_COLLECTOR)
    PRF_ID_HRPC,
    #endif // (BLE_HR_COLLECTOR)

    #if (BLE_FINDME_LOCATOR)
    PRF_ID_FINDL,
    #endif // (BLE_FINDME_LOCATOR)

    #if (BLE_FINDME_TARGET)
    PRF_ID_FINDT,
    #endif // (BLE_FINDME_TARGET)

    #if (BLE_PROX_MONITOR)
    PRF_ID_PROXM,
    #endif // (BLE_PROX_MONITOR)

    #if (BLE_PROX_REPORTER)
    PRF_ID_PROXR,
    #endif // (BLE_PROX_REPORTER)

    #if (BLE_SP_SERVER)
    PRF_ID_SCPPS,
    #endif // (BLE_SP_SERVER)

    #if (BLE_SP_CLIENT)
    PRF_ID_SCPPC,
    #endif // (BLE_SP_CLIENT)

    #if (BLE_BATT_SERVER)
    PRF_ID_BASS,
    #endif // (BLE_BATT_SERVER)

    #if (BLE_BATT_CLIENT)
    PRF_ID_BASC,
    #endif // (BLE_BATT_CLIENT)

    #if (BLE_HID_DEVICE)
    PRF_ID_HOGPD,
    #endif // (BLE_HID_DEVICE)

    #if (BLE_HID_BOOT_HOST)
    PRF_ID_HOGPBH,
    #endif // (BLE_HID_BOOT_HOST)

    #if (BLE_HID_REPORT_HOST)
    PRF_ID_HOGPRH,
    #endif // (BLE_HID_REPORT_HOST)

    #if (BLE_GL_COLLECTOR)
    PRF_ID_GLPC,
    #endif // (BLE_GL_COLLECTOR)

    #if (BLE_GL_SENSOR)
    PRF_ID_GLPS,
    #endif // (BLE_GL_SENSOR)

    #if (BLE_RSC_COLLECTOR)
    PRF_ID_RSCPC,
    #endif // (BLE_RSC_COLLECTOR)

    #if (BLE_RSC_SENSOR)
    PRF_ID_RSCPS,
    #endif // (BLE_RSC_SENSOR)

    #if (BLE_CSC_COLLECTOR)
    PRF_ID_CSCPC,
    #endif // (BLE_CSC_COLLECTOR)

    #if (BLE_CSC_SENSOR)
    PRF_ID_CSCPS,
    #endif // (BLE_CSC_SENSOR)

    #if (BLE_CP_COLLECTOR)
    PRF_ID_CPPC,
    #endif // (BLE_CP_COLLECTOR)

    #if (BLE_CP_SENSOR)
    PRF_ID_CPPS,
    #endif // (BLE_CP_SENSOR)

    #if (BLE_LN_COLLECTOR)
    PRF_ID_LANC,
    #endif // (BLE_LN_COLLECTOR)

    #if (BLE_LN_SENSOR)
    PRF_ID_LANS,
    #endif // (BLE_LN_SENSOR)

    #if (BLE_AN_CLIENT)
    PRF_ID_ANPC,
    #endif // (BLE_AN_CLIENT)

    #if (BLE_AN_SERVER)
    PRF_ID_ANPS,
    #endif // (BLE_AN_SERVER)

    #if (BLE_PAS_CLIENT)
    PRF_ID_PASPC,
    #endif // (BLE_PAS_CLIENT)

    #if (BLE_PAS_SERVER)
    PRF_ID_PASPS,
    #endif // (BLE_PAS_SERVER)


    #if (BLE_IPS_SERVER)
    PRF_ID_IPSS,
    #endif //(BLE_IPS_SERVER)

    #if (BLE_IPS_CLIENT)
    PRF_ID_IPSC,
    #endif //(BLE_IPS_CLIENT)

    #if (BLE_ENV_SERVER)
    PRF_ID_ENVS,
    #endif //(BLE_ENV_SERVER)

    #if (BLE_ENV_CLIENT)
    PRF_ID_ENVC,
    #endif //(BLE_ENV_CLIENT)

    #if (BLE_WSC_SERVER)
    PRF_ID_WSCS,
    #endif //(BLE_WSC_SERVER)

    #if (BLE_WSC_CLIENT)
    PRF_ID_WSCC,
    #endif //(BLE_WSC_CLIENT)

    #if (BLE_BCS_SERVER)
    PRF_ID_BCSS,
    #endif //(BLE_BCS_SERVER)

    #if (BLE_BCS_CLIENT)
    PRF_ID_BCSC,
    #endif //(BLE_BCS_CLIENT)

    #if (BLE_UDS_SERVER)
    PRF_ID_UDSS,
    #endif //(BLE_UDS_SERVER)

    #if (BLE_UDS_CLIENT)
    PRF_ID_UDSC,
    #endif //(BLE_UDS_CLIENT)

    #if (BLE_WPT_SERVER)
    PRF_ID_WPTS,
    #endif //(BLE_WPT_SERVER)

    #if (BLE_WPT_CLIENT)
    PRF_ID_WPTC,
    #endif //(BLE_WPT_CLIENT)

    #if (BLE_PLX_SERVER)
    PRF_ID_PLXS,
    #endif //(BLE_PLX_SERVER)

    #if (BLE_PLX_CLIENT)
    PRF_ID_PLXC,
    #endif //(BLE_PLX_CLIENT)

    #if (BLE_CGMS_SERVER)
    PRF_ID_CGMSS,
    PRF_ID_CGMS = PRF_ID_CGMSS, // Line deprecated
    #endif //(BLE_CGMS_SERVER)

    #if (BLE_CGMS_CLIENT)
    PRF_ID_CGMSC,
    PRF_ID_CGMC = PRF_ID_CGMSC, // Line deprecated
    #endif //(BLE_CGMS_CLIENT)

    #if (BLE_DBG_THPP)
    PRF_ID_THPP,
    #endif //(BLE_DBG_THPP)

    #if (BLE_CSIS_MEMBER)
    PRF_ID_CSISM,
    #endif //(BLE_CSIS_MEMBER)

    #if (BLE_CSIS_COORD)
    PRF_ID_CSISC,
    #endif //(BLE_CSIS_COORD)

    #if (BLE_OT_SERVER)
    PRF_ID_OTS,
    #endif //(BLE_OT_SERVER)

    #if (BLE_OT_CLIENT)
    PRF_ID_OTC,
    #endif //(BLE_OT_CLIENT)

    #if (BLE_BMS_SERVER)
    PRF_ID_BMSS,
    #endif //(BLE_BMS_SERVER)

    #if (BLE_BMS_CLIENT)
    PRF_ID_BMSC,
    #endif //(BLE_BMS_CLIENT)
    #endif // (HL_LE_CENTRAL || HL_LE_PERIPHERAL)

    #if (BLE_MESH)
    PRF_ID_MESH,
    #endif //(BLE_MESH)

    #if (BT_A2DP_SINK)
    PRF_ID_A2DP_SINK,
    #endif //(BT_A2DP_SINK)

    #if (BT_A2DP_SOURCE)
    PRF_ID_A2DP_SOURCE,
    #endif //(BT_A2DP_SOURCE)

    #if (BT_AVRCP)
    PRF_ID_AVRCP,
    #endif //(BT_AVRCP)

    #if (BT_HFP_HF)
    PRF_ID_HFP_HF,
    #endif //(BT_HFP_HF)

    #if (BT_HFP_AG)
    PRF_ID_HFP_AG,
    #endif //(BT_HFP_AG)

    #if (BT_SPP)
    PRF_ID_SPP,
    #endif //(BT_SPP)

    #if (BT_PBAP_CLIENT)
    PRF_ID_PBAP_CLIENT,
    #endif //(BT_PBAP_CLIENT)

    #if (MFI_IAP)
    PRF_ID_MFI_IAP,
    #endif //(MFI_IAP)

    #if (GFP_PROVIDER)
    PRF_ID_GFP_PROVIDER,
    #endif //(GFP_PROVIDER)

    #if (BLE_ESL_AP)
    PRF_ID_ESL_AP,
    #endif //(BLE_ESL_AP)

    #if (BLE_ESL_TAG)
    PRF_ID_ESL_TAG,
    #endif //(BLE_ESL_TAG)

    #if (BLE_RAS_SERVER)
    PRF_ID_RASS,
    #endif //(BLE_RAS_SERVER)

    #if (BLE_RAS_CLIENT)
    PRF_ID_RASC,
    #endif //(BLE_RAS_CLIENT)

    #if (BLE_IAS_SERVER)
    PRF_ID_IASS,
    #endif //(BLE_IAS_SERVER)

    #if (BLE_IAS_CLIENT)
    PRF_ID_IASC,
    #endif //(BLE_IAS_CLIENT)

    #if (BLE_LLS_SERVER)
    PRF_ID_LLSS,
    #endif //(BLE_LLS_SERVER)

    #if (BLE_LLS_CLIENT)
    PRF_ID_LLSC,
    #endif //(BLE_LLS_CLIENT)

    #if (BLE_TPS_SERVER)
    PRF_ID_TPSS,
    #endif //(BLE_TPS_SERVER)

    #if (BLE_TPS_CLIENT)
    PRF_ID_TPSC,
    #endif //(BLE_TPS_CLIENT)

    PRF_ID_MAX,

    /// Invalid profile identifier
    PRF_ID_INVALID = 0xFF,
};

/// @} PRF_CONFIG_API

#endif /* RWPRF_CONFIG_H_ */
