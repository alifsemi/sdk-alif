/**
 ****************************************************************************************
 *
 * @file rwip_config.h
 *
 * @brief Configuration of the RW IP SW
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef RWIP_CONFIG_H_
#define RWIP_CONFIG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup ROOT
 * @{
 *
 *  Information about RW SW IP options and flags
 *
 *        BT_DUAL_MODE             BT/BLE Dual Mode
 *        BT_STD_MODE              BT Only
 *        BLE_STD_MODE             BLE Only
 *
 *        BT_EMB_PRESENT           BT controller exists
 *        BLE_EMB_PRESENT          BLE controller exists
 *        BLE_HOST_PRESENT         BLE host exists
 *
 *        MAC154_EMB_PRESENT       MAC154 controller exists
 *        MAC154_HOST_PRESENT      ....
 *
 * @name RW Stack Configuration
 * @{
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
/******************************************************************************************/
/* -------------------------   EAVESDROPPING FLAG      -----------------------------------*/
/******************************************************************************************/
#if defined(CFG_ED)
    #define EAVESDROPPING_SUPPORT           1
#else // defined(CFG_ED)
    #define EAVESDROPPING_SUPPORT           0
#endif // defined(CFG_ED)


/******************************************************************************************/
/* -------------------------   STACK PARTITIONING      -----------------------------------*/
/******************************************************************************************/
#if defined(CFG_BT_EMB)
#define BT_EMB_PRESENT              1
#else // !defined(CFG_BT_EMB)
#define BT_EMB_PRESENT              0
#endif // defined(CFG_BT_EMB)

#if defined(CFG_BLE_EMB)
#define BLE_EMB_PRESENT             1
#else // !defined(CFG_BLE_EMB)
#define BLE_EMB_PRESENT             0
#endif // defined(CFG_BLE_EMB)

#if defined(CFG_MAC154_EMB)
#define MAC154_EMB_PRESENT             1
#define MAC154_APP_PRESENT             1
#else // !defined(CFG_MAC154_EMB)
#define MAC154_EMB_PRESENT             0
#define MAC154_APP_PRESENT             0
#endif // defined(CFG_MAC154_EMB)

#if defined(CFG_MM_EMB)
#define MM_EMB_PRESENT             1
#else // !defined(CFG_MM_EMB)
#define MM_EMB_PRESENT             0
#endif // defined(CFG_MM_EMB)

#define EMB_PRESENT                 (BT_EMB_PRESENT || BLE_EMB_PRESENT || MAC154_EMB_PRESENT)

#if (defined(CFG_BLE_HOST))
#define BLE_HOST_PRESENT            (1)
#else // !(defined(CFG_BLE_HOST))
#define BLE_HOST_PRESENT            (0)
#endif // (defined(CFG_BLE_HOST))


#if (defined(CFG_BT_HOST))
#define BT_HOST_PRESENT             (1)
#else // !(defined(CFG_BT_HOST))
#define BT_HOST_PRESENT             (0)
#endif // (defined(CFG_BT_HOST))

#define HOST_PRESENT                (BLE_HOST_PRESENT || BT_HOST_PRESENT)

#if (defined(CFG_APP) && HOST_PRESENT)
#define APP_PRESENT                 1
#if (!BT_HOST_PRESENT)
#define BLE_APP_PRESENT             1
#else // BT_HOST_PRESENT
#define BLE_APP_PRESENT             0
#endif // (!BT_HOST_PRESENT)
#else // !(defined(CFG_APP) && HOST_PRESENT)
#define APP_PRESENT                 0
#define BLE_APP_PRESENT             0
#endif // (defined(CFG_APP) && HOST_PRESENT)

#define BT_DUAL_MODE                (BT_EMB_PRESENT && BLE_EMB_PRESENT)
#define MM_BLE_MAC154               (BLE_EMB_PRESENT && MAC154_EMB_PRESENT && !BT_DUAL_MODE)
#define HCI_PRESENT                 1

/// Flag indicating that Mesh is supported
#if defined(CFG_BLE_MESH)
#define BLE_MESH                    BLE_HOST_PRESENT
#else  // !defined(CFG_BLE_MESH)
#define BLE_MESH                    0
#endif // defined(CFG_BLE_MESH)

/// Flag indicating that Mesh Profile of version 1.1 is supported
#if (defined(CFG_BLE_MESH_11))
#define BLE_MESH_11                          (1)
#else
#define BLE_MESH_11                          (0)
#endif //(defined(CFG_BLE_MESH_11))

/// Memory leak detection - platform specific debug
#if (defined(CFG_MEM_LEAK_DETECT))
#define PLF_MEM_LEAK_DETECT 1
#else
#define PLF_MEM_LEAK_DETECT 0
#endif // (defined(CFG_MEM_LEAK_DETECT))

/******************************************************************************************/
/* --------------------------      BT VERSION       --------------------------------------*/
/******************************************************************************************/

/// Flag indicating BT 5.3 support
#if defined(CFG_BT_53)
#define BT_53                       (1)
#else // !defined(CFG_BT_53)
#define BT_53                       (0)
#endif // defined(CFG_BT_53)

/// Flag indicating BT 5.4 support
#if defined(CFG_BT_54)
#define BT_54                       (1)
#else // !defined(CFG_BT_54)
#define BT_54                       (0)
#endif // defined(CFG_BT_54)

/// Flag indicating BT 6.0 support
#if defined(CFG_BT_60)
#define BT_60                       (1)
#else // !defined(CFG_BT_60)
#define BT_60                       (0)
#endif // defined(CFG_BT_60)

/// Flag indicating BT 6.1 support
#if defined(CFG_BT_61)
#define BT_61                       (1)
#else // !defined(CFG_BT_61)
#define BT_61                       (0)
#endif // defined(CFG_BT_61)

/******************************************************************************************/
/* -------------------------   INTERFACES DEFINITIONS      -------------------------------*/
/******************************************************************************************/

/// Application Host Interface - requires host message API
#if defined(CFG_AHITL) && defined(CFG_HL_MSG_API)
#define AHI_TL_SUPPORT       1
#elif MAC154_EMB_PRESENT && !BLE_HOST_PRESENT
#define AHI_TL_SUPPORT       1
#else // defined(CFG_AHITL) && defined(CFG_HL_MSG_API)
#define AHI_TL_SUPPORT       0
#endif // defined(CFG_AHITL) && defined(CFG_HL_MSG_API)

/// Control Application Interface
#if defined(CFG_APP_CTRL)
#define CTRLI_TL_SUPPORT     1
#else // defined(CFG_APP_CTRL)
#define CTRLI_TL_SUPPORT     0
#endif // defined(CFG_APP_CTRL)

/// Host Controller Interface Support (defines if HCI parser is present or not)
#if (defined(CFG_HCITL) || BT_HOST_PRESENT)
#define HCI_TL_SUPPORT      1
#else //defined(CFG_HCITL)
#define HCI_TL_SUPPORT      0
#endif //defined(CFG_HCITL)

#if HOST_PRESENT
#define RWTL_SUPPORT      (AHI_TL_SUPPORT || CTRLI_TL_SUPPORT || (HCI_TL_SUPPORT))
#else // !HOST_PRESENT
#define RWTL_SUPPORT      (HCI_TL_SUPPORT)
#endif // HOST_PRESENT

#if (defined (CFG_H4TL) && RWTL_SUPPORT)
#define H4TL_SUPPORT     1
#else
#define H4TL_SUPPORT     0
#endif // (defined (CFG_H4TL) && RWTL_SUPPORT)

#if (defined (CFG_MBTL) && RWTL_SUPPORT)
#define MBTL_SUPPORT     1
#else
#define MBTL_SUPPORT     0
#endif // (defined (CFG_MBTL) && RWTL_SUPPORT)

/// Number of HCI commands the stack can handle simultaneously
#define HCI_NB_CMD_PKTS   (5 * HCI_TL_SUPPORT)

/******************************************************************************************/
/* --------------------------   BT COMMON DEFINITIONS      ------------------------------*/
/******************************************************************************************/

#if defined (CFG_CON_SCO)
/// Maximum number of Synchronous connections (0 to 2)
#define MAX_NB_SYNC                      (CFG_CON_SCO)
#else //  defined (CFG_CON_SCO)
#define MAX_NB_SYNC                      (0)
#endif // defined (CFG_CON_SCO)

/// Flag indicating if debug mode is activated or not
#if defined (CFG_VOHCI) && (MAX_NB_SYNC > 0)
    #define VOICE_OVER_HCI                   1
#else
    #define VOICE_OVER_HCI                   0
#endif // defined (CFG_VOHCI) && (MAX_NB_SYNC > 0)


/// Support of Connectionless Slave Broadcast
#if defined (CFG_CSB)
    #define CSB_SUPPORT  1
#else
    #define CSB_SUPPORT  0
#endif // defined (CFG_CSB)

/// Support of Piconet Clock Adjust
#if defined (CFG_PCA)
    #define PCA_SUPPORT  1
#else
    #define PCA_SUPPORT  0
#endif // defined (CFG_PCA)

// Mutually exclusive features - Support of Broadcast Encryption only if not PCA Supported
#define BCAST_ENC_SUPPORT (!PCA_SUPPORT)


/******************************************************************************************/
/* --------------------------   BLE COMMON DEFINITIONS      ------------------------------*/
/******************************************************************************************/
/// Kernel Heap memory sized reserved for allocate dynamically connection environment
#define KE_HEAP_MEM_RESERVED        (4)


/// Broadcaster
#if (defined(CFG_BROADCASTER) || defined(CFG_PERIPHERAL) || (defined(CFG_PAWR) && defined(CFG_CENTRAL)))
#define BLE_BROADCASTER      1
#else
#define BLE_BROADCASTER      0
#endif // (defined(CFG_BROADCASTER) || defined(CFG_PERIPHERAL) || (defined(CFG_PAWR) && defined(CFG_CENTRAL)))

/// Observer
#if (defined(CFG_OBSERVER) || defined(CFG_CENTRAL) || (defined(CFG_PAWR) && defined(CFG_PERIPHERAL)))
#define BLE_OBSERVER      1
#else
#define BLE_OBSERVER      0
#endif // (defined(CFG_OBSERVER) || defined(CFG_CENTRAL) || (defined(CFG_PAWR) && defined(CFG_PERIPHERAL)))

/// Central
#if defined(CFG_CENTRAL)
#define BLE_CENTRAL      1
#else
#define BLE_CENTRAL      0
#endif // defined(CFG_CENTRAL)

/// Peripheral
#if defined(CFG_PERIPHERAL)
#define BLE_PERIPHERAL      1
#else
#define BLE_PERIPHERAL      0
#endif // defined(CFG_PERIPHERAL)

#if (HOST_PRESENT)
#define HOST_ACTIVITY_MAX            (CFG_HL_ACT)
#if (BLE_CENTRAL || BLE_PERIPHERAL)
#if (BT_HOST_PRESENT)
    #define HOST_CONNECTION_MAX      (CFG_CON + CFG_CON_ACL)
#else // !(BT_HOST_PRESENT)
    #define HOST_CONNECTION_MAX      (CFG_CON)
#endif // !(BT_HOST_PRESENT)
#else // !(BLE_CENTRAL || BLE_PERIPHERAL)
#if (BT_HOST_PRESENT)
    #define HOST_CONNECTION_MAX      (CFG_CON_ACL)
#else // !(BT_HOST_PRESENT)
    #define HOST_CONNECTION_MAX      (0)
#endif // !(BT_HOST_PRESENT)
#endif // !(BLE_CENTRAL || BLE_PERIPHERAL)
#endif // (HOST_PRESENT)

#if (BLE_EMB_PRESENT || BLE_HOST_PRESENT)
#if ((BLE_BROADCASTER+BLE_OBSERVER+BLE_PERIPHERAL+BLE_CENTRAL) == 0)
    #error "No application role defined"
#endif // ((BLE_BROADCASTER+BLE_OBSERVER+BLE_PERIPHERAL+BLE_CENTRAL) == 0)

#if (BLE_EMB_PRESENT || BLE_HOST_PRESENT)
/// Maximum number of simultaneous BLE activities (scan, connection, advertising, initiating)
#define BLE_ACTIVITY_MAX          ((uint8_t)CFG_ACT)
#endif // (BLE_EMB_PRESENT || BLE_HOST_PRESENT)

/// Maximum number of devices in RAL
#define BLE_RAL_MAX          (CFG_RAL)

/// Max advertising reports before sending the info to the host
#define BLE_ADV_REPORTS_MAX              (1)

#if (BLE_EMB_PRESENT)
/// Maximum number of ADV reports in the HCI queue to Host
#define BLE_MAX_NB_ADV_REP_FRAG       (4 * HCI_TL_SUPPORT)
/// Maximum number of IQ reports in the HCI queue to Host
#define BLE_MAX_NB_IQ_REP             (4 * HCI_TL_SUPPORT)
#endif // (BLE_EMB_PRESENT)
#endif // (BLE_EMB_PRESENT || BLE_HOST_PRESENT)

/******************************************************************************************/
/* --------------------------   MAC154 COMMON DEFINITIONS      ------------------------------*/
/******************************************************************************************/
#if defined(CFG_MAC154_EMB) && CFG_MAC154_EMB
    #define MAC154_ACTIVITY_MAX          (CFG_MAC154_ACT)
    #define MAC154_RX_DESC_NB            (4)
    #define MAC154_TX_DESC_NB            (2)
    #define MAC154_ED_DESC_NB            (1)
    #define MAC154_RX_BUFFER_DATA_NB     (4)
    #define MAC154_TX_BUFFER_DATA_NB     (2)
    #define MAC154_ED_BUFFER_DATA_NB     (1)
    #define MAC154_NB_CHANNEL_MAX        (16)

#else //defined(CFG_MAC154_EMB) && CFG_MAC154_EMB
    #define MAC154_ACTIVITY_MAX          (0)
#endif //defined(CFG_MAC154_EMB) && CFG_MAC154_EMB


/******************************************************************************************/
/* --------------------------      Power Control                 -------------------------*/
/******************************************************************************************/

/// BT Classic Power Control
#if defined(CFG_BT_PWR_CTRL)
    #define BT_PWR_CTRL                 (1)
#else // !defined(CFG_BT_PWR_CTRL)
    #define BT_PWR_CTRL                 (0)
#endif // defined(CFG_BT_PWR_CTRL)

/// BLE Power Control
#if defined(CFG_LE_PWR_CTRL)
    #define BLE_PWR_CTRL                (1)
#else // !defined(CFG_LE_PWR_CTRL)
    #define BLE_PWR_CTRL                (0)
#endif // defined(CFG_LE_PWR_CTRL)

/******************************************************************************************/
/* ----------------------- Periodic Advertising Sync Transfer (PAST)   -------------------*/
/******************************************************************************************/

/// Periodic Advertising Sync Transfer
#if defined(CFG_PAST) && defined(CFG_PER_ADV) && (BLE_CENTRAL | BLE_PERIPHERAL)
    #define BLE_PAST                        (1)
#else  // ! (defined(CFG_PAST) && defined(CFG_PER_ADV))
    #define BLE_PAST                        (0)
#endif // defined(CFG_PAST) && defined(CFG_PER_ADV)

/******************************************************************************************/
/* -------------------------------- Periodic Advertising  --------------------------------*/
/******************************************************************************************/

/// Periodic Advertising
#if defined(CFG_PER_ADV)
    #define BLE_PER_ADV                        (1)
#else  // !defined(CFG_PER_ADV)
    #define BLE_PER_ADV                        (0)
#endif // defined(CFG_PER_ADV)

/******************************************************************************************/
/* ------------------------- Periodic Advertising with Responses--------------------------*/
/******************************************************************************************/

/// Periodic Advertising with Responses
#if defined(CFG_PAWR) && defined(CFG_PER_ADV) && (BLE_PAST) && BT_54
    #define BLE_PAWR                           (1)
#else // !(defined (CFG_PAWR) && defined(CFG_PER_ADV) && (BLE_PAST))
    #define BLE_PAWR                           (0)
#endif // defined (CFG_PAWR) && defined(CFG_PER_ADV) && (BLE_PAST)

/******************************************************************************************/
/* ----------------------- Advertising Channel Index                   -------------------*/
/******************************************************************************************/

/// Advertising Channel Index
#if defined(CFG_ADV_CH_IDX)
    #define BLE_ADV_CH_IDX                  (1)
#else  // !defined(CFG_ADV_CH_IDX)
    #define BLE_ADV_CH_IDX                  (0)
#endif // defined(CFG_ADV_CH_IDX)

/******************************************************************************************/
/* -----------------------             LE Subrating                    -------------------*/
/******************************************************************************************/

/// LE Subrating
#if defined(CFG_LE_SUBRATING) && BT_53
    #define BLE_SUBRATING                      (1)
#else // !(defined(CFG_LE_SUBRATING) && BT_53)
    #define BLE_SUBRATING                      (0)
#endif // (defined(CFG_LE_SUBRATING) && BT_53)

/******************************************************************************************/
/* ----------------------- Coding Scheme Selection on Advertising      -------------------*/
/******************************************************************************************/

/// Coding Scheme Selection on Advertising
#if defined (CFG_CSSA) && BT_54
    #define BLE_CSSA                           (1)
#else // !(defined (CFG_CSSA))
    #define BLE_CSSA                           (0)
#endif // !(defined (CFG_CSSA))

/******************************************************************************************/
/* ------------------------- Channel Sounding --------------------------------------------*/
/******************************************************************************************/

/// Channel Sounding
#if (defined(CFG_CHSD) && (BLE_CENTRAL || BLE_PERIPHERAL))
    #define BLE_CHSD                         (1)
    /// CHannel Sounding test mode
    #if defined (CFG_CHSD_TEST)
        #define BLE_CHSD_TEST                (1)
    #else // defined !(CFG_CHSD_TEST)
        #define BLE_CHSD_TEST                (0)
    #endif // defined (CFG_CHSD_TEST)
#else // !(defined(CFG_CHSD) && (BLE_CENTRAL || BLE_PERIPHERAL))
    #define BLE_CHSD                         (0)
    #define BLE_CHSD_TEST                    (0)
#endif // (defined(CFG_CHSD) && (BLE_CENTRAL || BLE_PERIPHERAL))

/******************************************************************************************/
/* ----------------------- ISOAL Unsegmented Framed Mode               -------------------*/
/******************************************************************************************/

/// ISOAL Unsegmented Framed Mode
#if defined (CFG_EISOAL)
    #define BLE_EISOAL                       (1)
#else // !(defined (CFG_EISOAL))
    #define BLE_EISOAL                       (0)
#endif // !(defined (CFG_EISOAL))

/******************************************************************************************/
/* ----------------------- Frame Space Update                          -------------------*/
/******************************************************************************************/

/// Frame Space Update
#if defined (CFG_FSU)
    #define BLE_FSU                          (1)
#else // !(defined (CFG_FSU))
    #define BLE_FSU                          (0)
#endif // !(defined (CFG_FSU))

/******************************************************************************************/
/* ----------------------- Monitored Advertisers List                  -------------------*/
/******************************************************************************************/

/// Monitored Advertisers List
#if (BLE_OBSERVER && defined(CFG_MONITOR_ADV))
    #define BLE_MONITOR_ADV                          (1)
#else // !(BLE_OBSERVER && defined(CFG_MONITOR_ADV))
    #define BLE_MONITOR_ADV                          (0)
#endif // !(BLE_OBSERVER && defined(CFG_MONITOR_ADV))

/******************************************************************************************/
/* ----------------------- Higher Data Throughput                      -------------------*/
/******************************************************************************************/

/// Higher Data Throughput
#if defined (CFG_HDT)
    #define BLE_HDT                          (1)
#else // !(defined (CFG_HDT))
    #define BLE_HDT                          (0)
#endif // !(defined (CFG_HDT))

/******************************************************************************************/
/* ----------------------- Test Mode                                   -------------------*/
/******************************************************************************************/

/// Enhanced Test Mode
#if defined (CFG_BLE_ENH_TEST_MODE)
    #define BLE_ENH_TEST_MODE                   (1)
#else // defined !(CFG_BLE_ENH_TEST_MODE)
    #define BLE_ENH_TEST_MODE                   (0)
#endif // defined (CFG_BLE_ENH_TEST_MODE)

/// Upper Tester for BLE Enhanced Test Mode
#if defined (CFG_BLE_UPPER_TESTER)
    #define BLE_UPPER_TESTER                    (1)
#else // defined !(CFG_BLE_UPPER_TESTER)
    #define BLE_UPPER_TESTER                    (0)
#endif // defined (CFG_BLE_UPPER_TESTER)

/// Lower Tester for BLE Enhanced Test Mode
#if defined (CFG_BLE_LOWER_TESTER)
    #define BLE_LOWER_TESTER                    (1)
#else // defined !(CFG_BLE_LOWER_TESTER)
    #define BLE_LOWER_TESTER                    (0)
#endif // defined (CFG_BLE_LOWER_TESTER)

#if (BLE_LOWER_TESTER) && !(BLE_UPPER_TESTER)
#error "BLE Lower Tester requires BLE Upper Tester"
#endif // (BLE_LOWER_TESTER) && !(BLE_UPPER_TESTER)

/******************************************************************************************/
/* ----------------------- MISRA-C Checkers                            -------------------*/
/******************************************************************************************/
/// MISRA-C Sanity checkers
#if defined (CFG_MISRAC_CHECKER)
    #define MISRAC_CHECKER                    (1)
#else // defined !(CFG_MISRAC_CHECKER)
    #define MISRAC_CHECKER                    (0)
#endif // defined (CFG_MISRAC_CHECKER)

/******************************************************************************************/
/* --------------------------   Direction finding (AoA/AoD)      -------------------------*/
/******************************************************************************************/

#if defined(CFG_CON_CTE_REQ)
    #define BLE_CON_CTE_REQ                (1)
#else // defined(CFG_CON_CTE_REQ)
    #define BLE_CON_CTE_REQ                (0)
#endif // defined(CFG_CON_CTE_REQ)

#if defined(CFG_CON_CTE_RSP)
    #define BLE_CON_CTE_RSP                (1)
#else // defined(CFG_CON_CTE_RSP)
    #define BLE_CON_CTE_RSP                (0)
#endif // defined(CFG_CON_CTE_RSP)

#if defined(CFG_CONLESS_CTE_TX) && defined(CFG_PER_ADV)
    #define BLE_CONLESS_CTE_TX             (1)
#else // defined(CFG_CONLESS_CTE_TX)
    #define BLE_CONLESS_CTE_TX             (0)
#endif // defined(CFG_CONLESS_CTE_TX)

#if defined(CFG_CONLESS_CTE_RX) && defined(CFG_PER_ADV)
    #define BLE_CONLESS_CTE_RX             (1)
#else // defined(CFG_CONLESS_CTE_RX)
    #define BLE_CONLESS_CTE_RX             (0)
#endif // defined(CFG_CONLESS_CTE_RX)

#if defined(CFG_AOD)
    #define BLE_AOD                        (1)
#else // defined(CFG_AOD)
    #define BLE_AOD                        (0)
#endif // defined(CFG_AOD)

#if defined(CFG_AOA)
    #define BLE_AOA                        (1)
#else // defined(CFG_AOA)
    #define BLE_AOA                        (0)
#endif // defined(CFG_AOA)

/******************************************************************************************/
/* --------------------------   ISOCHRONOUS CONFIGURATION        -------------------------*/
/******************************************************************************************/

// check if isochronous is enabled or not
#if  (defined(CFG_ISO_CON) && (CFG_ISO_CON > 0) && (defined(CFG_ISO_MODE_0) || defined(CFG_CIS) || (defined(CFG_BIS) && defined(CFG_PER_ADV))))
    #define BLE_ISO_PRESENT                (1)
#else // !(defined(CFG_ISO_CON) && (CFG_ISO_CON > 0))
    #define BLE_ISO_PRESENT                (0)
#endif // (defined(CFG_ISO_CON) && (CFG_ISO_CON > 0))

//  *** Definition of supported isochronous mode ***

// Isochronous Mode 0 - Proprietary mode
#if  (BLE_ISO_PRESENT && defined(CFG_ISO_MODE_0))
    #define BLE_ISO_MODE_0                (BLE_CENTRAL | BLE_PERIPHERAL)
    #define BLE_ISO_MODE_0_PROFILE        (BLE_ISO_MODE_0 & BLE_HOST_PRESENT)
    #if defined(CFG_ISO_MODE_0_RSA)
    #define BLE_ISO_MODE_0_RSA            (BLE_ISO_MODE_0_PROFILE)
    #else
    #define BLE_ISO_MODE_0_RSA            (0)
    #endif // defined(CFG_ISO_MODE_0_RSA)
    #if defined(CFG_ISO_MODE_0_DEVICE)
    #define BLE_ISO_MODE_0_DEVICE         (BLE_ISO_MODE_0_PROFILE)
    #else
    #define BLE_ISO_MODE_0_DEVICE         (0)
    #endif // defined(CFG_ISO_MODE_0_RSA)
#else
    #define BLE_ISO_MODE_0                (0)
    #define BLE_ISO_MODE_0_PROFILE        (0)
    #define BLE_ISO_MODE_0_RSA            (0)
    #define BLE_ISO_MODE_0_DEVICE         (0)
#endif // (BLE_ISO_PRESENT && defined(CFG_AUDIO))

// Connected Isochronous Stream
#if  (BLE_ISO_PRESENT && defined(CFG_CIS))
#define BLE_CIS                           (BLE_CENTRAL | BLE_PERIPHERAL)
#else
#define BLE_CIS                           (0)
#endif // (BLE_ISO_PRESENT && defined(CFG_CIS))

// Broadcast Isochronous Stream
#if  (BLE_ISO_PRESENT && defined(CFG_BIS) && defined(CFG_PER_ADV) && (BLE_BROADCASTER || BLE_OBSERVER))
#define BLE_BIS                           (1)
#else
#define BLE_BIS                           (0)
#endif // (BLE_ISO_PRESENT && defined(CFG_BIS) && defined(CFG_PER_ADV))

// sanity check for ISO presence
#if defined(BLE_ISO_PRESENT)
#if (BLE_ISO_PRESENT) && !(BLE_ISO_MODE_0 | BLE_BIS | BLE_CIS)
#error "Error in sanity check for ISO presence"
#endif // (BLE_ISO_PERSENT) && !(BLE_ISO_MODE_0 | BLE_BIS | BLE_CIS)
#endif // defined(BLE_ISO_PRESENT)

#if (BLE_ISO_PRESENT)
    /// Maximum number of ISO streams/groups
    #define BLE_ISO_STREAM_MAX       ((uint8_t)CFG_ISO_CON)
    #define BLE_ISO_GROUP_MAX        ((uint8_t)CFG_ISO_CON)

    /// Maximum number of octets that can be received/transmitted over Isochronous channels
    #define BLE_MAX_ISO_OCTETS  (251) // number of octets

    /// Define number of ISO TX/RX buffers per isochronous channel
    #define BLE_NB_ISO_BUFF_PER_STREAM          (10u)

    /// Define number of ISO descriptors per isochronous channel
    /// Must be equal to max(BLE_NB_ISODESC_PER_BIS, BLE_NB_RX_ISODESC_PER_CIS + BLE_NB_TX_ISODESC_PER_CIS)
    #define BLE_NB_ISODESC_PER_STREAM           (4u)

    /// Number of ISO Descriptors - one descriptor required for update subevent: 1 per stream
    #define BLE_ISO_DESC_NB                     ((uint16_t)(((uint16_t)BLE_ISO_STREAM_MAX * (uint16_t)BLE_NB_ISODESC_PER_STREAM) + (uint16_t)BLE_ISO_GROUP_MAX))
    /// Number of ISO buffers
    #define BLE_ISO_BUF_NB                         ((uint16_t)((uint16_t)BLE_ISO_STREAM_MAX * (uint16_t)BLE_NB_ISO_BUFF_PER_STREAM))

    #if (BLE_CIS)
    /// Define number of ISO TX/RX descriptors and buffers per CIS
    #define BLE_NB_RXTX_ISO_DESC_BUF_PER_CIS       (2)
    #endif // (BLE_CIS)

    #if (BLE_BIS)
    /// Define number of ISO RX or TX descriptors per BIS
    #define BLE_NB_ISODESC_PER_BIS                 (4)
    #endif // (BLE_BIS)

    #if (BLE_CIS | BLE_BIS)
        /// Size of hopping sequence for ISO streams
        #define BLE_ISO_HOP_SEQ_SIZE       (0x20u) // Depends on max number of subevent supported
        /// Number of hopping sequence for ISO streams
        #define BLE_ISO_HOP_SEQ_NB         ((uint8_t)(BLE_ISO_STREAM_MAX+1u))
    #endif // (BLE_CIS | BLE_BIS)
#endif // (BLE_ISO_PRESENT)

/// Flag indicating that Generic Audio Framework is supported
#if defined(CFG_BLE_GAF)
#define BLE_GAF_PRESENT             BLE_HOST_PRESENT & BLE_ISO_PRESENT
#else //defined(CFG_BLE_GAF)
#define BLE_GAF_PRESENT             0
#endif //defined(CFG_BLE_GAF)

/******************************************************************************************/
/* --------------------------   CHANNEL ASSESSMENT CONFIGURATION -------------------------*/
/******************************************************************************************/

#if defined(CFG_BLE_CH_SCAN)
    #define BLE_CH_SCAN_SUPPORT (1)
#else
    #define BLE_CH_SCAN_SUPPORT (0)
#endif

#define RWIP_RSSI_INTERF_THR     (-70)

/// Maximum duration without receiving a packet before considering a channel quality unknown (in slots) (10 secs)
#define RWIP_ASSESS_VALID_TO       0x3E00

/// Maximum counter value for channel assessment
#define RWIP_ASSESS_COUNT_MAX      4
/// Counter threshold to consider a channel good
#define RWIP_ASSESS_COUNT_THR_GOOD 2
/// Counter threshold to consider a channel bad
#define RWIP_ASSESS_COUNT_THR_BAD  -2
/// Minimum counter value for channel assessment
#define RWIP_ASSESS_COUNT_MIN      -4

// Access address for the channel scan
#define RWIP_CH_SCAN_ACCESS_ADDR 0x555555555


/******************************************************************************************/
/* ------------------   ISOCHRONOUS DATA PATH DRIVERS       ------------------------------*/
/******************************************************************************************/

/// ISO over HCI
#if defined(CFG_ISOOHCI)
    #define BLE_ISOOHCI                  (BLE_ISO_PRESENT)
#else
    #define BLE_ISOOHCI                  (0)
#endif

/// Internal ISO generator for validation purpose
#if defined(CFG_ISOGEN)
    #define BLE_ISOGEN                   (BLE_ISO_PRESENT)
#else
    #define BLE_ISOGEN                   (0)
#endif

/// ISO over shared memory
#if defined(CFG_ISOOSHM)
    #define BLE_ISOOSHM                  (BLE_ISO_PRESENT)
#else
    #define BLE_ISOOSHM                  (0)
#endif

/******************************************************************************************/
/* -------------------   TWS LE FORWARDING EXTENSION       -------------------------------*/
/******************************************************************************************/

#if defined(CFG_TWS_LE_FWD_PRIMARY)
    #define TWS_LE_FWD_PRIMARY                   (BLE_ISO_PRESENT)
#else
    #define TWS_LE_FWD_PRIMARY                   (0)
#endif // defined(CFG_TWS_LE_FWD_PRIMARY)

#if defined(CFG_TWS_LE_FWD_SECONDARY)
    #define TWS_LE_FWD_SECONDARY                 (BLE_ISO_PRESENT)
#else
    #define TWS_LE_FWD_SECONDARY                 (0)
#endif // defined(CFG_TWS_LE_FWD_SECONDARY)


/******************************************************************************************/
/* -------------------   TIME TACKING - ACTIVITY SHIFT     -------------------------------*/
/******************************************************************************************/
#if (TWS_LE_FWD_PRIMARY)
#define SCH_TIME                                 (1)
#else
#define SCH_TIME                                 (0)
#endif // (TWS_LE_FWD_PRIMARY)

/******************************************************************************************/
/* --------------------------   BUFFER SETUP       --------------------------------------*/
/******************************************************************************************/

/// Buffer management - used only by host for the moment
#if (HOST_PRESENT)
#define CO_BUF_PRESENT         1

/// Size and number of big buffers available by default
#define CO_BUF_BIG_SIZE        (GAP_LE_MTU_MAX)
#define CO_BUF_BIG_NB          (2)
#define CO_BUF_BIG_POOL_SIZE   ((CO_ALIGN4_HI(sizeof(co_buf_t) + CO_BUF_BIG_SIZE)) * CO_BUF_BIG_NB)


/// Size and number of small buffers available by default
#if ((!EMB_PRESENT) &&(BLE_ISOOHCI))
#define CO_BUF_SMALL_SIZE      (256)
#else
#define CO_BUF_SMALL_SIZE      (128)
#endif //((!EMB_PRESENT) &&(BLE_ISOOHCI))
#if(BLE_EMB_PRESENT)
#define CO_BUF_SMALL_NB        (BLE_ACL_BUF_NB_TX)
#else // !(BLE_EMB_PRESENT)
#define CO_BUF_SMALL_NB        (12)
#endif //(BLE_EMB_PRESENT)
#define CO_BUF_SMALL_POOL_SIZE ((CO_ALIGN4_HI(sizeof(co_buf_t) + CO_BUF_SMALL_SIZE)) * CO_BUF_SMALL_NB)
#else
#define CO_BUF_PRESENT         0
#endif //HOST_PRESENT

/******************************************************************************************/
/* --------------------------   FLOW CONTROL       ---------------------------------------*/
/******************************************************************************************/

/// Controller to Host flow control enable/disable
#if defined(CFG_HCI_C2H_FLOW_CTRL)
#define HCI_CTRL_TO_HOST_FLOW_CTRL      1
#else
#define HCI_CTRL_TO_HOST_FLOW_CTRL      0
#endif //CFG_HCI_C2H_FLOW_CTRL

/******************************************************************************************/
/* --------------------------   DISPLAY SETUP        -------------------------------------*/
/******************************************************************************************/

/// Display controller enable/disable
#if defined(CFG_DISPLAY)
#define DISPLAY_SUPPORT      1
#else
#define DISPLAY_SUPPORT      0
#endif //CFG_DISPLAY

/******************************************************************************************/
/* --------------------------   GPIO SETUP        -------------------------------------*/
/******************************************************************************************/

/// Display controller enable/disable
#if defined(CFG_GPIO)
#define GPIO_SUPPORT      1
#else
#define GPIO_SUPPORT      0
#endif //CFG_GPIO

/******************************************************************************************/
/* --------------------------      RTC SETUP         -------------------------------------*/
/******************************************************************************************/

/// RTC enable/disable
#if defined(CFG_RTC)
#define RTC_SUPPORT      1
#else
#define RTC_SUPPORT      0
#endif //CFG_RTC

/******************************************************************************************/
/* --------------------------      PS2 SETUP         -------------------------------------*/
/******************************************************************************************/

/// PS2 enable/disable
#if defined(CFG_PS2)
#define PS2_SUPPORT      1
#else
#define PS2_SUPPORT      0
#endif //CFG_PS2

/******************************************************************************************/
/* --------------------------      BHSU SETUP         -------------------------------------*/
/******************************************************************************************/

/// Bluetooth Hardware Security Unit (BHSU) enable/disable
#if defined(CFG_BHSU_SUPPORT)
#define BHSU_SUPPORT           (1)
#else
#define BHSU_SUPPORT           (0)
#endif //(CFG_BHSU_SUPPORT)

/******************************************************************************************/
/* --------------------------   AUDIO SYNC SETUP        ----------------------------------*/
/******************************************************************************************/

/// Audio Sync enable/disable
#if (defined(CFG_AUDIO_SYNC) && EMB_PRESENT)
#define AUDIO_SYNC_SUPPORT      (1)
#else
#define AUDIO_SYNC_SUPPORT      (0)
#endif // (defined(CFG_AUDIO_SYNC) && EMB_PRESENT)

/******************************************************************************************/
/* --------------------------      GAIA SETUP        -------------------------------------*/
/******************************************************************************************/

/// GAIA enable/disable
#if defined(CFG_GAIA)
#define GAIA_SUPPORT      1
#else
#define GAIA_SUPPORT      0
#endif //CFG_GAIA

/******************************************************************************************/
/* ------------------------      RTOS I/F SUPPORT    -------------------------------------*/
/******************************************************************************************/

/// RTOS interface enable/disable
#if defined(CFG_RTOS)
#define RTOS_SUPPORT      1
#else
#define RTOS_SUPPORT      0
#endif //CFG_RTOS

/******************************************************************************************/
/* --------------------------      TRACER SETUP      -------------------------------------*/
/******************************************************************************************/

/// tracer enable/disable
#if defined(CFG_TRC_EN)
    #define TRACER_PRESENT                   (RWTL_SUPPORT)
    #include "dbg_trc_config.h"
#else
    #define TRACER_PRESENT                   0
#endif // CFG_TRC_EN

/******************************************************************************************/
/* -------------------------   DEEP SLEEP SETUP      -------------------------------------*/
/******************************************************************************************/

/// Low Power clock: 32kHz or 32.768kHz
#define HZ32000                                     (GAIA_SUPPORT)

/// Time to wake-up Radio Module (in us)
#define SLEEP_RM_WAKEUP_DELAY                       625
/// Time for stabilization of the high frequency oscillator following a sleep-timer expiry (in us)
#define SLEEP_OSC_NORMAL_WAKEUP_DELAY               5000
/// Time for stabilization of the high frequency oscillator following an external wake-up request (in us)
#define SLEEP_OSC_EXT_WAKEUP_DELAY                  5000
/// Time for warm boot delay (in us)
#define WARMBOOT_EXT_WAKEUP_DELAY                   4000

#if (HZ32000)
/// Sleep Duration Value in periodic wake-up mode (in LP clock cycles)
#define MAX_SLEEP_DURATION_PERIODIC_WAKEUP      16000   // 0.5s
/// Sleep Duration Value in external wake-up mode (in LP clock cycles)
#define MAX_SLEEP_DURATION_EXTERNAL_WAKEUP      320000  // 10s
#else // (HZ32000)
/// Sleep Duration Value in periodic wake-up mode (in LP clock cycles)
#define MAX_SLEEP_DURATION_PERIODIC_WAKEUP      16384   // 0.5s
/// Sleep Duration Value in external wake-up mode (in LP clock cycles)
#define MAX_SLEEP_DURATION_EXTERNAL_WAKEUP      327680  // 10s
#endif // (HZ32000)

/**
 * Margins for entering sleep and waking-up (in LP clock cycles)
 *  - 50 us + 2 LP cycles for sleep entering (UART check, system enters sleep)
 *  - 70 us for SW processing at wake-up
 *  - 312.5 us for clock correction at wake-up
 *  - Total ~ 500 us
 *  Register suspend + power off + register resume
 *  =========================
 *  - Same as normal sleep + extsys0 startup time with storing/restoring BLE, MAC154 & registers
 */
#define SLEEP_ENTER_FUNC_MARGIN         13      // 375 us    at 32kHz, 366 us  at 32.768kHz
#define SLEEP_WAKEUP_FUNC_MARGIN        4       // 125 us    at 32kHz, 122 us  at 32.768kHz
#define SLEEP_WAKEUP_CLOCK_CORR_MARGIN  10      // 312.5 us  at 32kHz, 305 us  at 32.768kHz
#define DEEP_SLEEP_ENTER_FUNC_MARGIN    13      // 375 us    at 32kHz, 366 us  at 32.768kHz
#define DEEP_SLEEP_WAKEUP_FUNC_MARGIN   8       // 250 us    at 32kHz, 244 us  at 32.768kHz

/// Margin for SW to program a frame at wake-up (in half-us)
#define SLEEP_PROG_MARGIN   700      // 350 us


/******************************************************************************************/
/* --------------------------   BASEBAND SETUP       -------------------------------------*/
/******************************************************************************************/

#if (EMB_PRESENT)
#include "rf.h"     // rf configurations, for BB & radio setup
#include "arch.h"   // platform configurations, for BB setup
#endif // (EMB_PRESENT)

/// Default programming delay, margin for programming the baseband in advance of each activity (in half-slots)
#define IP_PROG_DELAY_DFT  (3)

/**
 * Prefetch time (in us)
 *  - Radio power up: radio defined (worst case)
 *  - EM fetch & HW logic: platform defined (worst case)
 *
 * Prefetch time = power up time + EM fetch & HW logic time
 */
#define IP_PREFETCH_TIME_US         (RF_POWERUP_TIME_US + PLF_EM_FETCH_TIME_US)

/**
 * Prefetch Abort time (in us)
 *
 * - EM update & HW logic: platform defined (worst case)
 * - Radio power down: radio defined (worst case)
 *
 * Prefetch abort time = prefetch time + + EM update & HW logic time + power down time
 */
#define IP_PREFETCHABORT_TIME_US  (IP_PREFETCH_TIME_US + PLF_EM_UPDATE_TIME_US + RF_POWERDOWN_TIME_US)


/******************************************************************************************/
/* --------------------------   RADIO SETUP       ----------------------------------------*/
/******************************************************************************************/

/// Power control features
#define RF_TXPWR                            1
/// Class of device
#define RF_CLASS1                           0

/******************************************************************************************/
/* -------------------------   SUPPORTED RADIO PHY    ------------------------------------*/
/******************************************************************************************/

#define BLE_PHY_1MBPS_SUPPORT                       (RF_BLE_PHY_1MBPS_SUPPORT)
#define BLE_PHY_2MBPS_SUPPORT                       (RF_BLE_PHY_2MBPS_SUPPORT)
#define BLE_PHY_CODED_SUPPORT                       (RF_BLE_PHY_CODED_SUPPORT)
#define BLE_STABLE_MOD_IDX_TX_SUPPORT               (RF_STABLE_MOD_IDX_TX_SUPPORT)
#define BLE_STABLE_MOD_IDX_RX_SUPPORT               (RF_STABLE_MOD_IDX_RX_SUPPORT)
#define BLE_PWR_CLASS_1_SUPPORT                     (RF_PWR_CLASS_1_SUPPORT)

/******************************************************************************************/
/* -------------------------   COEXISTENCE SETUP      ------------------------------------*/
/******************************************************************************************/

/// WLAN Coexistence
#if defined(CFG_WLAN_COEX)
    #define RW_WLAN_COEX                 1
    #define RW_WLAN_COEX_TEST            (defined(CFG_WLAN_COEX_TEST))
#else
    #define RW_WLAN_COEX                 0
    #define RW_WLAN_COEX_TEST            0
#endif // defined(CFG_WLAN_COEX)

/// MWS Coexistence
#if defined(CFG_MWS_COEX)
    #define RW_MWS_COEX                 1
    #define RW_MWS_COEX_TEST            (defined(CFG_MWS_COEX_TEST))
#else
    #define RW_MWS_COEX                 0
    #define RW_MWS_COEX_TEST            0
#endif // defined(CFG_MWS_COEX)

/// To let the HW use the default values set in the registers
#if (0 || BT_60)
#define RW_PTI_PRIO_AUTO    0x00
/// To let the SW force the override values set with PTI_PRIO[3:0]
#define RW_PTI_PRIO_FORCED  0x10
#else // !(EAVESDROPPING_SUPPORT || BT_60)
#define RW_PTI_PRIO_AUTO    0x1F
#endif // !(EAVESDROPPING_SUPPORT || BT_60)

/******************************************************************************************/
/* ------------------------   RSSI & POWER CONTROL   -------------------------------------*/
/******************************************************************************************/

#if BLE_PWR_CTRL

/// RSSI averaging weight (2^N)
/*
 * Calculate an RSSI average based on a weighted history of RSSI values (RW proprietary method):
 * A weight of 2 (2^2 = 4) provides a weighted averaging at 25%, 18.5%, 14%, 10.5%, 8%, 6%, 4.5%, 3.3%, 2.5%...
 * A weight of 3 (2^3 = 8) provides a weighted averaging at 12.5%, 10.9%, 9.6%, 8.3%, 7.3%, 6.4%, 5.6%, 4.9%, 4.3%...
 */
#define RW_RSSI_AV_WEIGHT 2

/// APR target offset within local RSSI golden receive window
#define RW_RSSI_APR_TARGET           (5) // 5 dBm above low threshold

#endif // BLE_PWR_CTRL

#if (BLE_SUBRATING)
/******************************************************************************************/
/* ------------------------   AUTO-SUBRATE THRESHOLD   -----------------------------------*/
/******************************************************************************************/

// Subrate factor at which subrating is auto-disabled for procedures with instants
#define RW_AUTO_SUBRATE_THRESHOLD   (4) // subrate threshold, in units of connection event

#endif // (BLE_SUBRATING)

/******************************************************************************************/
/* -----------------------   SLOT AVAILABILITY MASKS   -----------------------------------*/
/******************************************************************************************/

/// Maximum support peer SAM map size
#define RW_MAX_PEER_SAM_MAP_SLOTS      (256u)
#define RW_PEER_SAM_MAP_MAX_LEN      (RW_MAX_PEER_SAM_MAP_SLOTS/4u) // 2-bit field per slot

/******************************************************************************************/
/* -----------------------   SECURITY MODULE SETUP  --------------------------------------*/
/******************************************************************************************/

/// Use Elliptic Curve P-256 (Secure Connection)
#if defined(CFG_ECC_P256_SUPPORT)
    #define ECC_P256_SUPPORT          (1)
    /// 16 or 32-bit computation method
    #if defined(CFG_ECC_16_BITS_ALGO)
    #define ECC_MULT_ALGO_TYPE        (16)
    #else // !defined(CFG_ECC_16_BITS_ALGO)
    #define ECC_MULT_ALGO_TYPE        (32)
    #endif // defined(CFG_ECC_16_BITS_ALGO)
#else // !defined(CFG_ECC_P256_SUPPORT)
    #define ECC_P256_SUPPORT          (0)
#endif // defined(CFG_ECC_P256_SUPPORT)

/// Use RSA Algorithm
#if defined(CFG_RSA_SUPPORT)
#define RSA_SUPPORT           (1)
#else
#define RSA_SUPPORT           (0)
#endif // defined(CFG_RSA_SUPPORT)

/// SW AES used for cipher instead of HW Accelerator
#if defined(CFG_SW_AES_CIPHER)
#define SW_AES_CIPHER         (1)
#else
#define SW_AES_CIPHER         (0)
#endif // defined(CFG_SW_AES_CIPHER)

/// SW AES used for decipher - Required by Google Fast Pair if HW Accelerator not instantiated
#if defined(CFG_SW_AES_DECIPHER)
#define SW_AES_DECIPHER         (1)
#else
#define SW_AES_DECIPHER         (0)
#endif // defined(CFG_SW_AES_DECIPHER)

#if defined(CFG_CRYPTO_UT)
#define CRYPTO_UT                 (1)
#else //defined(CFG_CRYPTO_UT)
#define CRYPTO_UT                 (0)
#endif //defined(CFG_CRYPTO_UT)


/******************************************************************************************/
/* --------------------------   DEBUG SETUP       ----------------------------------------*/
/******************************************************************************************/

/// Flag indicating if debug mode is activated or not
#if defined(CFG_DBG)
    #define RW_DEBUG                        (EMB_PRESENT || HOST_PRESENT)
#if (EMB_PRESENT)
    #define RW_SWDIAG                       1
#else
    #define RW_SWDIAG                       0
#endif
    #define KE_PROFILING                    1
#else
    #define RW_DEBUG                        0
    #define RW_SWDIAG                       0
    #define KE_PROFILING                    0
#endif /* CFG_DBG */

#if defined(CFG_ENABLE_KE_PROFILING)
    #undef  KE_PROFILING
    #define KE_PROFILING                    1
#endif

/// Flag indicating if Read/Write memory commands are supported or not
#if defined(CFG_DBG_MEM)
    #define RW_DEBUG_MEM               1
#else //CFG_DBG_MEM
    #define RW_DEBUG_MEM               0
#endif //CFG_DBG_MEM

/// Flag indicating if Flash debug commands are supported or not
#if defined(CFG_DBG_FLASH)
    #define RW_DEBUG_FLASH                  1
#else //CFG_DBG_FLASH
    #define RW_DEBUG_FLASH                  0
#endif //CFG_DBG_FLASH

/// Flag indicating if CPU stack profiling commands are supported or not
#if defined(CFG_DBG_STACK_PROF)
    #define RW_DEBUG_STACK_PROF             1
#else
    #define RW_DEBUG_STACK_PROF             0
#endif // defined (CFG_DBG_STACK_PROF)

/// Flag indicating if Security Test API is supported
#if defined(CFG_SEC_TEST_API)
    #define SEC_TEST_API                    1
#else
    #define SEC_TEST_API                    0
#endif // defined (CFG_SEC_TEST_API)

/// Scheduling Planner unit test (HCI debug commands to test scheduling planner functions)
#define SCH_PLAN_UT                 (1)

/// BLE I&Q sample Generator control interface
#define BLE_IQ_GEN                  (RW_DEBUG && (BLE_CON_CTE_REQ | BLE_CONLESS_CTE_RX))


/******************************************************************************************/
/* --------------------------      VS SETUP         --------------------------------------*/
/******************************************************************************************/

/// Read piconnect clock
#if defined (CFG_BT_READ_PICONET_CLOCK)
#define BT_READ_PICONET_CLOCK   1
#else
#define BT_READ_PICONET_CLOCK   0
#endif //defined (CFG_BT_READ_PICONET_CLOCK)

#define BT_HCI_TEST_MODE        1

/// I2C over HCI bridge
#if defined (CFG_BT_I2C_OVER_HCI)
#define BT_I2C_OVER_HCI       1
#else
#define BT_I2C_OVER_HCI       0
#endif // defined (CFG_BT_I2C_OVER_HCI)


/******************************************************************************************/
/* --------------------------      NVDS SETUP       --------------------------------------*/
/******************************************************************************************/

/// Flag indicating if NVDS feature is supported or not
#if defined(CFG_NVDS)
    #define NVDS_SUPPORT                    1
#else //CFG_DBG_NVDS
    #define NVDS_SUPPORT                    0
#endif //CFG_DBG_NVDS


/******************************************************************************************/
/* --------------------------     MEMORY SETUP      --------------------------------------*/
/******************************************************************************************/
/// Amount of memory that must be reserved to system allocation in case of memory overflow.
#if defined(CFG_SYSTEM_MEM_SAFETY_MARGIN_SIZE)
    #define SYSTEM_MEM_SAFETY_MARGIN_SIZE               (CFG_SYSTEM_MEM_SAFETY_MARGIN_SIZE)
#else // CFG_SYSTEM_MEM_SAFETY_MARGIN_SIZE
    #define SYSTEM_MEM_SAFETY_MARGIN_SIZE               (0)
#endif // CFG_SYSTEM_MEM_SAFETY_MARGIN_SIZE

/******************************************************************************************/
/* --------------------------      MISC SETUP       --------------------------------------*/
/******************************************************************************************/
/// Manufacturer: Alif Semiconductor Inc
#define RW_COMP_ID                           0x0CDD


/******************************************************************************************/
/* -------------------------   BT / BLE / BLE HL CONFIG    -------------------------------*/
/******************************************************************************************/

#if (BT_EMB_PRESENT)
#include "rwbt_config.h"    // BT stack configuration
#endif //BT_EMB_PRESENT

#if (BLE_EMB_PRESENT)
#include "rwble_config.h"   // BLE stack configuration
#endif //BLE_EMB_PRESENT

#if (HOST_PRESENT)
#include "rwhl_config.h"    // BLE Host stack configuration
#endif //HOST_PRESENT

/******************************************************************************************/
/* -------------------------   KERNEL SETUP          -------------------------------------*/
/******************************************************************************************/

/// Force zero initialization of Kernel messages
/// (burden allocation but ensure that memory properly overwritten)
#if(defined(CFG_KE_MSG_CALLOC))
#define KE_MSG_CALLOC                 (1)
#else
#define KE_MSG_CALLOC                 (0)
#endif // (defined(CFG_KE_MSG_CALLOC))

/// Event types definition (ordered in reversed priority order)
/*@TRACE*/
enum KE_EVENT_TYPE
{
    #if(APP_PRESENT)
    APP_EVENT,
    #endif // (APP_PRESENT)

    #if DISPLAY_SUPPORT
    KE_EVENT_DISPLAY,
    #endif //DISPLAY_SUPPORT

    #if RTC_SUPPORT
    KE_EVENT_RTC_1S_TICK,
    #endif //RTC_SUPPORT

    #if RSA_SUPPORT
    KE_EVENT_RSA_SIGN,
    #endif // RSA_SUPPORT

    /// ECDH background event
    KE_EVENT_ECC_P256_MULTIPLICATION,

    #if (BT_EMB_PRESENT)
    KE_EVENT_ECC_P192_MULTIPLICATION,
    #endif // (BT_EMB_PRESENT)

    #if (TRACER_PRESENT)
    KE_EVENT_TRC,
    #endif /*(TRACER_PRESENT)*/

    /// Delayed job handler - Low Priority
    KE_EVENT_DJOB_LP,

    /// Message handling
    KE_EVENT_KE_MESSAGE,

    /// Delayed job handler - High Priority
    KE_EVENT_DJOB_HP,

    #if (BLE_EMB_PRESENT || BT_EMB_PRESENT)
    /// Defer AES processing
    KE_EVENT_AES_DEFER,
    #endif // (BLE_EMB_PRESENT || BT_EMB_PRESENT)

    /// Timer handler
    KE_EVENT_TIMER,

    #if (BLE_ISOOHCI)
    KE_EVENT_ISOOHCI_IN_DEFER,
    KE_EVENT_ISOOHCI_OUT_DEFER,
    #endif //(BLE_ISOOHCI)

    /// Delayed job handler - ISR Defer
    KE_EVENT_DJOB_ISR,

    KE_EVENT_MAX,
};

/// Tasks types definition
/*@TRACE*/
enum KE_TASK_TYPE
{
    #if (BT_EMB_PRESENT)
    // BT Controller Tasks
    TASK_LM,
    TASK_LC,
    TASK_LB,
    #endif // (BT_EMB_PRESENT)

    #if (BLE_EMB_PRESENT)
    // Link Layer Tasks
    TASK_LLM,
    TASK_LLC,
    #if (BLE_ISO_PRESENT)
    TASK_LLI,
    #endif // (BLE_ISO_PRESENT)
    #endif // (BLE_EMB_PRESENT)

    #if (MAC154_EMB_PRESENT)
    TASK_MAC154C,    // MAC154 Controller Tasks
    TASK_MAC154D,    // MAC154 Driver Tasks
    #endif // (MAC154_EMB_PRESENT)

#if (EMB_PRESENT)
    TASK_DBG,
#endif // (EMB_PRESENT)

#if (DISPLAY_SUPPORT)
    TASK_DISPLAY,
#endif // (DISPLAY_SUPPORT)

    #if (HOST_PRESENT && HOST_MSG_API)
    #if (APP_PRESENT)
    TASK_APP,
    #endif // (APP_PRESENT)
    #endif // ((HOST_PRESENT && HOST_MSG_API))

    #if (MAC154_EMB_PRESENT) || (defined(CFG_WVT_TEST) && HOST_PRESENT)
    TASK_MAC154APP,
    #endif // (MAC154_EMB_PRESENT) || (defined(CFG_WVT_TEST) && HOST_PRESENT)

    #if (HOST_PRESENT && HOST_MSG_API)
    #if (BLE_CENTRAL || BLE_PERIPHERAL || BT_HOST_PRESENT)
    TASK_GAPC,    // Generic Access Profile Controller
    #endif // (BLE_CENTRAL || BLE_PERIPHERAL || BT_HOST_PRESENT)
    TASK_GAPM,    // Generic Access Profile Manager
    TASK_L2CAP,   // L2CAP Task
    TASK_GATT,    // Generic Attribute Profile
    TASK_PRF_MIN,
    // allocate a certain number of profiles task
    TASK_PRF_MAX = (TASK_PRF_MIN + (HOST_NB_PROFILES - 1)),

    #if (BLE_GAF_PRESENT)
    TASK_GAF,
    #endif //(BLE_GAF_PRESENT)
    #if (BLE_ISO_PRESENT)
    TASK_GAPI,
    #endif //(BLE_ISO_PRESENT)
    #endif //(HOST_PRESENT && HOST_MSG_API)

    #if (AHI_TL_SUPPORT)
    TASK_AHI,
    #endif // (AHI_TL_SUPPORT)

    /// Maximum number of tasks
    TASK_MAX,

    TASK_NONE = 0xFF,
};

/// Kernel memory heaps types.
/*@TRACE*/
enum KE_MEM_HEAP
{
    /* For SPARK_EXTSYS0 platform, re-order the heap memory identifiers. The non-retention heap is
       not protected by the firewall, so should only be used for data intended to be shared.
       Stack's heap allocation algorithm will allocate memory in a heap with a higher index if the
       desired heap is full, so placing the non-retention heap at index 0 prevents any "overflow"
       of data from the other heaps into this heap. */
    #if defined(CFG_SPARK_EXTSYS0)
    /// Non Retention memory block
    KE_MEM_NON_RETENTION,
    #endif

    /// Memory allocated for environment variables
    KE_MEM_ENV,

    #if (HOST_PRESENT)
    /// Memory allocated for profiles
    KE_MEM_PROFILE,
    #endif // (HOST_PRESENT)

    /// Memory allocated for kernel messages
    KE_MEM_KE_MSG,

    #if !defined(CFG_SPARK_EXTSYS0)
    KE_MEM_NON_RETENTION,
    #endif

    KE_MEM_BLOCK_MAX,
};



#if (BT_EMB_PRESENT)
#define BT_HEAP_MSG_SZ       (BT_HEAP_MSG_SIZE)
#define BT_HEAP_ENV_SZ       (BT_HEAP_ENV_SIZE)
#else
#define BT_HEAP_MSG_SZ       (0)
#define BT_HEAP_ENV_SZ       (0)
#endif //BT_EMB_PRESENT

#if (BLE_EMB_PRESENT)
#define BLE_HEAP_MSG_SZ      (BLE_HEAP_MSG_SIZE)
#define BLE_HEAP_ENV_SZ      (BLE_HEAP_ENV_SIZE)
#else
#define BLE_HEAP_MSG_SZ      (0)
#define BLE_HEAP_ENV_SZ      (0)
#endif //BLE_EMB_PRESENT

#if (MAC154_EMB_PRESENT)
#define MAC154_HEAP_MSG_SZ      (MAC154_HEAP_MSG_SIZE)
#define MAC154_HEAP_ENV_SZ      (MAC154_HEAP_ENV_SIZE)
#else // MAC154_EMB_PRESENT
#define MAC154_HEAP_MSG_SZ      (0)
#define MAC154_HEAP_ENV_SZ      (0)
#endif //MAC154_EMB_PRESENT

#if (HOST_PRESENT)
#define HOST_HEAP_MSG_SZ     (HOST_HEAP_MSG_SIZE)
#define HOST_HEAP_ENV_SZ     (HOST_HEAP_ENV_SIZE)
#define HOST_HEAP_PROFILE_SZ (HOST_HEAP_PROFILE_SIZE)
#else
#define HOST_HEAP_MSG_SZ     (0)
#define HOST_HEAP_ENV_SZ     (0)
#define HOST_HEAP_PROFILE_SZ (0)
#endif // (HOST_PRESENT)

#if (TWS_LE_FWD_PRIMARY)
#define TWS_HEAP_ENV_SIZE_      (10000)
#else //(TWS_LE_FWD_PRIMARY)
#define TWS_HEAP_ENV_SIZE_      (0)
#endif //(TWS_LE_FWD_PRIMARY)

#if (BT_EMB_PRESENT || BLE_EMB_PRESENT)
#define ECC_HEAP_NON_RET_SIZE_   (328*2) // Could only have 2 ECC computations simultaneously
#else // (BT_EMB_PRESENT || BLE_EMB_PRESENT)
#define ECC_HEAP_NON_RET_SIZE_   (0)
#endif // (BT_EMB_PRESENT || BLE_EMB_PRESENT)

/// Kernel Message Heap
#define RWIP_HEAP_MSG_SIZE         (  BT_HEAP_MSG_SZ      + \
                                      BLE_HEAP_MSG_SZ     + \
                                      MAC154_HEAP_MSG_SZ  + \
                                      HOST_HEAP_MSG_SZ      )

/// Size of Environment heap
#if defined(CFG_SPARK_EXTSYS0)
// For SPARK_EXTSYS0 platform, ECC memory should be allocated in ENV heap
#define RWIP_HEAP_ENV_SIZE         ( BT_HEAP_ENV_SZ       + \
                                     BLE_HEAP_ENV_SZ      + \
                                     MAC154_HEAP_ENV_SZ   + \
                                     HOST_HEAP_ENV_SZ     + \
                                     ECC_HEAP_NON_RET_SIZE_ + \
                                     TWS_HEAP_ENV_SIZE_        )
#else
#define RWIP_HEAP_ENV_SIZE         ( BT_HEAP_ENV_SZ       + \
                                     BLE_HEAP_ENV_SZ      + \
                                     MAC154_HEAP_ENV_SZ   + \
                                     HOST_HEAP_ENV_SZ     + \
                                     TWS_HEAP_ENV_SIZE_        )
#endif


/// Size of Attribute database heap
#define RWIP_HEAP_PROFILE_SIZE     (  HOST_HEAP_PROFILE_SZ  )

/**
 * Size of non-retention heap
 *
 * This heap can be used to split the RAM into 2 parts:
 *    - an always-on part that can handle a certain number of links
 *    - a secondary memory that could be powered-off when not used, and retained only when used
 *
 * With such mechanism, the previous heaps need to be reduced so that they can contain all required data
 * in a light scenario (few connections, few profiles). Then the non-retention heap is sized in order to
 * cover the worst case scenario (max connections, max profiles, etc ...)
 *
 * The current size show what is already known as not needing to be retained during deep sleep.
 */
#define RWIP_HEAP_NON_RET_SIZE    ( ECC_HEAP_NON_RET_SIZE_ )

/******************************************************************************************/
/* -------------------------     CONFIGURABLE PARAMETERS     -----------------------------*/
/******************************************************************************************/

/// List of parameters identifiers
enum PARAM_ID
{
    /// Definition of the tag associated to each parameters
    /// Local Bd Address
    PARAM_ID_BD_ADDRESS                 = 0x01,
    /// Device Name
    PARAM_ID_DEVICE_NAME                = 0x02,
    /// Low Power Clock Drift
    PARAM_ID_LPCLK_DRIFT                = 0x07,
    /// Active Clock Drift
    PARAM_ID_ACTCLK_DRIFT               = 0x09,
    /// External wake-up time
    PARAM_ID_EXT_WAKEUP_TIME            = 0x0D,
    /// Oscillator wake-up time
    PARAM_ID_OSC_WAKEUP_TIME            = 0x0E,
    /// Radio wake-up time
    PARAM_ID_RM_WAKEUP_TIME             = 0x0F,
    /// UART baudrate
    PARAM_ID_UART_BAUDRATE              = 0x10,
    /// Enable sleep mode
    PARAM_ID_SLEEP_ENABLE               = 0x11,
    /// Enable External Wakeup
    PARAM_ID_EXT_WAKEUP_ENABLE          = 0x12,

    /// Activity Move Configuration (enables/disables activity move for BLE connections and BT (e)SCO links)
    PARAM_ID_ACTIVITY_MOVE_CONFIG       = 0x15,

    /// Enable/disable scanning for extended advertising PDUs
    PARAM_ID_SCAN_EXT_ADV               = 0x16,

    /// Duration of the schedule reservation for long activities such as scan, inquiry, page, HDC advertising
    PARAM_ID_SCHED_SCAN_DUR             = 0x17,

    /// Programming delay, margin for programming the baseband in advance of each activity (in half-slots)
    PARAM_ID_PROG_DELAY                 = 0x18,

    /// Enable/disable channel assessment for BT and/or BLE
    PARAM_ID_CH_ASS_EN                  = 0x19,

    /// Default MD bit used by slave when sending a data packet on a BLE connection
    PARAM_ID_DFT_SLAVE_MD               = 0x20,

    /// Synchronous links configuration
    PARAM_ID_SYNC_CONFIG                = 0x2C,
    /// Tracer configuration
    PARAM_ID_TRACER_CONFIG              = 0x2F,

    /// Diagport configuration
    PARAM_ID_DIAG_BT_HW                 = 0x30,
    PARAM_ID_DIAG_BLE_HW                = 0x31,
    PARAM_ID_DIAG_SW                    = 0x32,
    PARAM_ID_DIAG_DM_HW                 = 0x33,
    PARAM_ID_DIAG_PLF                   = 0x34,
    PARAM_ID_DIAG_CHSD_HW               = 0x35,
	PARAM_ID_DIAG_MAC154_HW             = 0x39,

    /// Channel classification reporting interval (in ms) (0 if not used) (default value: 0ms)
    PARAM_ID_CH_CLASS_REP_INTV           = 0x36,

    /// IDC selection (for audio demo)
    PARAM_ID_IDCSEL_PLF                 = 0x37,

    /// RSSI threshold tags
    PARAM_ID_RSSI_HIGH_THR              = 0x3A,
    PARAM_ID_RSSI_LOW_THR               = 0x3B,
    PARAM_ID_RSSI_INTERF_THR            = 0x3C,

    /// RF identifier
    PARAM_ID_RF_ID                      = 0x3D,

    /// RF BTIPT
    PARAM_ID_RF_BTIPT_VERSION          = 0x3E,
    PARAM_ID_RF_BTIPT_XO_SETTING       = 0x3F,
    PARAM_ID_RF_BTIPT_GAIN_SETTING     = 0x40,

    /// Link keys
    PARAM_ID_BT_LINK_KEY_NB             = 0x60,
    PARAM_ID_BT_LINK_KEY_FIRST          = 0x61,
    PARAM_ID_BLE_LINK_KEY_FIRST         = 0x70,
    PARAM_ID_BLE_LINK_KEY_LAST          = 0x7F,

    /// SC Private Key (Low Energy)
    PARAM_ID_LE_PRIVATE_KEY_P256        = 0x80,
    /// SC Debug: Used Fixed Private Key from NVDS (Low Energy)
    PARAM_ID_LE_DBG_FIXED_P256_KEY      = 0x82,

    #if (!BLE_CSSA)
    /// LE Coded PHY 500 Kbps selection
    PARAM_ID_LE_CODED_PHY_500           = 0x85,
    #endif // (!BLE_CSSA)

    /// Application specific
    PARAM_ID_APP_SPECIFIC_FIRST         = 0x90,
    PARAM_ID_APP_SPECIFIC_LAST          = 0xAF,

    /// Platform specific
    /// UART input clock frequency(Internal)
    PARAM_ID_UART_INPUT_CLK_FREQ        = 0xC0,

    /// Alif
    PARAM_ID_EXT_WARMBOOT_WAKEUP_TIME   = 0xD0,
    /// 32 bits of configurations for SPARK platform.
    PARAM_ID_CONFIGURATION              = 0xD1,
};

/// List of parameters lengths
enum PARAM_LEN
{
    // Definition of length associated to each parameters
    /// Local Bd Address
    PARAM_LEN_BD_ADDRESS                 = 6,
    /// Device Name
    PARAM_LEN_DEVICE_NAME                = 248,
    /// Low power clock drift
    PARAM_LEN_LPCLK_DRIFT                = 2,
    /// Active clock drift
    PARAM_LEN_ACTCLK_DRIFT               = 1,
    /// External wake-up time
    PARAM_LEN_EXT_WAKEUP_TIME            = 2,
    /// Oscillator wake-up time
    PARAM_LEN_OSC_WAKEUP_TIME            = 2,
    /// Radio wake-up time
    PARAM_LEN_RM_WAKEUP_TIME             = 2,
    /// UART baudrate
    PARAM_LEN_UART_BAUDRATE              = 4,
    /// Enable sleep mode
    PARAM_LEN_SLEEP_ENABLE               = 1,
    /// Enable External Wakeup
    PARAM_LEN_EXT_WAKEUP_ENABLE          = 1,

    /// Activity Move Configuration
    PARAM_LEN_ACTIVITY_MOVE_CONFIG       = 1,

    /// Enable/disable scanning for extended advertising PDUs
    PARAM_LEN_SCAN_EXT_ADV               = 1,

    /// Duration of the schedule reservation for long activities such as scan, inquiry, page, HDC advertising
    PARAM_LEN_SCHED_SCAN_DUR             = 2,

    /// Programming delay, margin for programming the baseband in advance of each activity (in half-slots)
    PARAM_LEN_PROG_DELAY                 = 1,

    /// Enable/disable channel assessment for BT and/or BLE
    PARAM_LEN_CH_ASS_EN                  = 1,

    /// Default MD bit used by slave when sending a data packet on a BLE connection
    PARAM_LEN_DFT_SLAVE_MD               = 1,

    /// Synchronous links configuration
    PARAM_LEN_SYNC_CONFIG                = 2,
    /// Tracer configuration
    PARAM_LEN_TRACER_CONFIG              = 4,

    /// Diagport configuration
    PARAM_LEN_DIAG_BT_HW                 = 4,
    PARAM_LEN_DIAG_BLE_HW                = 4,
    PARAM_LEN_DIAG_SW                    = 4,
    PARAM_LEN_DIAG_DM_HW                 = 4,
    PARAM_LEN_DIAG_PLF                   = 4,
    PARAM_LEN_DIAG_CHSD_HW               = 4,
    PARAM_LEN_DIAG_MAC154_HW             = 4,

    /// Channel classification reporting interval (in ms) (0 if not used) (default value: 0ms)
    PARAM_LEN_CH_CLASS_REP_INTV          = 2,

    /// IDC selection (for audio demo)
    PARAM_LEN_IDCSEL_PLF                 = 4,

    /// RSSI thresholds
    PARAM_LEN_RSSI_THR                   = 1,

    /// RF identifier
    PARAM_LEN_RF_ID                     = 4,

    /// RF BTIPT
    PARAM_LEN_RF_BTIPT_VERSION          = 1,
    PARAM_LEN_RF_BTIPT_XO_SETTING       = 1,
    PARAM_LEN_RF_BTIPT_GAIN_SETTING     = 2,

    /// Link keys
    PARAM_LEN_BT_LINK_KEY_NB             = 1,
    PARAM_LEN_BT_LINK_KEY                = 22,
    PARAM_LEN_BLE_LINK_KEY               = 48,

    /// P256
    PARAM_LEN_LE_PRIVATE_KEY_P256        = 32,
    PARAM_LEN_LE_DBG_FIXED_P256_KEY      = 1,

    #if (!BLE_CSSA)
    /// LE Coded PHY 500 Kbps selection
    PARAM_LEN_LE_CODED_PHY_500           = 1,
    #endif // (!(BLE_CSSA))

    /// Platform specific
    /// UART input clock frequency parameter length(Internal)
    PARAM_LEN_UART_INPUT_CLK_FREQ        = 4,

    /// Alif
    PARAM_LEN_EXT_WARMBOOT_WAKEUP_TIME   = 2,
    PARAM_LEN_CONFIGURATION              = 4,
};

/******************************************************************************************/
/* -------------------------        BT-BLE COEX        -----------------------------------*/
/******************************************************************************************/

#if (BLE_EMB_PRESENT || BT_EMB_PRESENT)
/// Enable and diable definition for the PTI
///Enable TX busy signal
#define RWIP_PTI_TXEN           1
///Disable TX busy signal
#define RWIP_PTI_TXDIS          0
/// Tx busy position
#define RWIP_TXBSY_POS          0

///Enable RX busy signal
#define RWIP_PTI_RXEN           1
///Disable RX busy signal
#define RWIP_PTI_RXDIS          0
/// Rx busy position
#define RWIP_RXBSY_POS          1

///Enable do not abort TX
#define RWIP_PTI_DNABORTEN      1
///Disable do not abort TX
#define RWIP_PTI_DNABORTDIS     0
/// Do not abort busy position
#define RWIP_DNABORT_POS        2

/// SAM disabled
#define RWIP_SAM_DIS            0
/// SAM enabled
#define RWIP_SAM_EN             1
/// SAM enable position
#define RWIP_SAMEN_POS          3

/// Bit masking
#define RWIP_COEX_BIT_MASK      1

/// Coex configuration index
enum rwip_coex_config_idx
{
    #if (BT_EMB_PRESENT)
    RWIP_COEX_MSSWITCH_IDX,
    RWIP_COEX_SNIFFATT_IDX,
    RWIP_COEX_PAGE_IDX,
    RWIP_COEX_PSCAN_IDX,
    RWIP_COEX_INQ_IDX,
    RWIP_COEX_INQRES_IDX,
    RWIP_COEX_SCORSVD_IDX,
    RWIP_COEX_BCAST_IDX,
    RWIP_COEX_CONNECT_IDX,
    #endif //#if (BT_EMB_PRESENT)
    #if (BLE_EMB_PRESENT)
    RWIP_COEX_CON_IDX,
    RWIP_COEX_CON_DATA_IDX,
    RWIP_COEX_ADV_IDX,
    RWIP_COEX_SCAN_IDX,
    RWIP_COEX_INIT_IDX,
    RWIP_COEX_CH_SCAN_IDX,
    #endif // (BLE_EMB_PRESENT)
    #if (MAC154_EMB_PRESENT)
    RWIP_COEX_MAC154_RX_IDX,
    RWIP_COEX_MAC154_TX_IDX,
    RWIP_COEX_MAC154_ED_IDX,
    #endif // (MAC154_EMB_PRESENT)
    /// Max configuration index
    RWIP_COEX_CFG_MAX,
};
#endif //(BLE_EMB_PRESENT || BT_EMB_PRESENT)

/******************************************************************************************/
/* -------------------------     BT-BLE PRIORITIES     -----------------------------------*/
/******************************************************************************************/
#if (BLE_EMB_PRESENT || BT_EMB_PRESENT || MAC154_EMB_PRESENT)
/// Priority index definition
enum rwip_prio_idx
{
    #if (BT_EMB_PRESENT)
    /// ACL event default priority
    RWIP_PRIO_ACL_DFT_IDX,
    /// ACL event priority with activity
    RWIP_PRIO_ACL_ACT_IDX,
    /// ACL Role Switch event default priority
    RWIP_PRIO_ACL_RSW_IDX,
    /// ACL sniff event default priority
    RWIP_PRIO_ACL_SNIFF_DFT_IDX,
    /// ACL sniff transition event default priority
    RWIP_PRIO_ACL_SNIFF_TRANS_IDX,
    #if (MAX_NB_SYNC > 0)
    /// SCO event default priority
    RWIP_PRIO_SCO_DFT_IDX,
    #endif //MAX_NB_SYNC
    /// Broadcast ACL event default priority
    RWIP_PRIO_BCST_DFT_IDX,
    /// Broadcast ACL event with LMP activity priority
    RWIP_PRIO_BCST_ACT_IDX,
    /// CSB RX event default priority
    RWIP_PRIO_CSB_RX_DFT_IDX,
    /// CSB TX event default priority
    RWIP_PRIO_CSB_TX_DFT_IDX,
    /// Inquiry event default priority
    RWIP_PRIO_INQ_DFT_IDX,
    /// Inquiry Scan event default priority
    RWIP_PRIO_ISCAN_DFT_IDX,
    /// Page event default priority
    RWIP_PRIO_PAGE_DFT_IDX,
    /// Page event default priority
    RWIP_PRIO_PAGE_1ST_PKT_IDX,
    /// Page first packet event default priority
    RWIP_PRIO_PCA_DFT_IDX,
    /// Page scan event default priority
    RWIP_PRIO_PSCAN_DFT_IDX,
    /// Page scan event priority increment when canceled
    RWIP_PRIO_PSCAN_1ST_PKT_IDX,
    /// Synchronization Scan event default priority
    RWIP_PRIO_SSCAN_DFT_IDX,
    /// Synchronization Train event default priority
    RWIP_PRIO_STRAIN_DFT_IDX,
    #endif //#if (BT_EMB_PRESENT)
    #if (BLE_EMB_PRESENT)
    /// Default priority for scanning events
    RWIP_PRIO_SCAN_IDX,
    /// Default priority for auxillary scan/init (no_asap) rx events
    RWIP_PRIO_AUX_RX_IDX,
    /// Default priority for periodic adv rx events
    RWIP_PRIO_PER_ADV_RX_DFT_IDX,
    /// Priority for periodic adv rx events before sync is established
    RWIP_PRIO_PER_ADV_RX_ESTAB_IDX,
    /// Default priority for initiating events
    RWIP_PRIO_INIT_IDX,
    /// LE connection events default priority
    RWIP_PRIO_CONNECT_DFT_IDX,
    /// LE connection events priority with activity
    RWIP_PRIO_CONNECT_ACT_IDX,
    /// LE connection events priority before the connection is established
    RWIP_PRIO_CONNECT_ESTAB_IDX,
    /// LE connection events priority before instant
    RWIP_PRIO_CONNECT_INSTANT_IDX,
    /// Default priority for advertising events
    RWIP_PRIO_ADV_IDX,
    /// Default priority for advertising high duty cycle events
    RWIP_PRIO_ADV_HDC_IDX,
    /// Default priority for aux advertising events
    RWIP_PRIO_ADV_AUX_IDX,
    /// Default priority for periodic advertising events
    RWIP_PRIO_PER_ADV_IDX,
    /// Default priority for resolvable private addresses renewal event
    RWIP_PRIO_RPA_RENEW_IDX,
    #if (BLE_CIS)
    /// Default priority for master CIS connect events
    RWIP_PRIO_M_CIS_IDX,
    /// Default priority for slave CIS connect events
    RWIP_PRIO_S_CIS_IDX,
    #endif // (BLE_CIS)
    #if (BLE_BIS)
    /// Default priority for master BIS events
    RWIP_PRIO_M_BIS_IDX,
    /// Default priority for slave BIS events
    RWIP_PRIO_S_BIS_IDX,
    /// Default priority for BIS control subevent
    RWIP_PRIO_BIS_CTRL_IDX,
    #endif // (BLE_BIS)
    #if (BLE_CH_SCAN_SUPPORT)
    /// Priority for Channel Scan activity
    RWIP_PRIO_CH_SCAN_IDX,
    #endif //(BLE_CH_SCAN_SUPPORT)
	#if (BLE_TEST_MODE_SUPPORT)
	/// Default priority for test mode events
	RWIP_PRIO_TEST_MODE_IDX,
	#endif // (BLE_TEST_MODE_SUPPORT)
    #if (BLE_CHSD)
    /// Priority for Channel Sounding activity
    RWIP_PRIO_CHSD_IDX,
    #endif // (BLE_CHSD)
    #endif // (BLE_EMB_PRESENT)

    #if (MAC154_EMB_PRESENT)
    /// Default priority for MAC154 RX activity
    RWIP_PRIO_MAC154_RX_IDX,
    /// Default priority for MAC154 TX activity
    RWIP_PRIO_MAC154_TX_IDX,
    /// Default priority for MAC154 ED activity
    RWIP_PRIO_MAC154_ED_IDX,
    #endif // (MAC154_EMB_PRESENT)

    RWIP_PRIO_IDX_MAX
};
/// Default priority value definition
enum rwip_prio_dft
{
    #if (BT_EMB_PRESENT)
    /// ACL event default priority
    RWIP_PRIO_ACL_DFT               = 40,
    /// ACL event priority with activity
    RWIP_PRIO_ACL_ACT               = 80,
    /// ACL Role Switch event default priority
    RWIP_PRIO_ACL_RSW               = 160,
    /// ACL sniff event default priority
    RWIP_PRIO_ACL_SNIFF_DFT         = 120,
    /// ACL sniff transition event default priority
    RWIP_PRIO_ACL_SNIFF_TRANS       = 80,
    #if (MAX_NB_SYNC > 0)
    /// SCO event default priority
    RWIP_PRIO_SCO_DFT               = 136,
    #endif //MAX_NB_SYNC
    /// Broadcast ACL event default priority
    RWIP_PRIO_BCST_DFT              = 40,
    /// Broadcast ACL event with LMP activity priority
    RWIP_PRIO_BCST_ACT              = 80,
    /// CSB RX event default priority
    RWIP_PRIO_CSB_RX_DFT            = 80,
    /// CSB TX event default priority
    RWIP_PRIO_CSB_TX_DFT            = 80,
    /// Inquiry event default priority
    RWIP_PRIO_INQ_DFT               = 40,
    /// Inquiry Scan event default priority
    RWIP_PRIO_ISCAN_DFT             = 40,
    /// Page event default priority
    RWIP_PRIO_PAGE_DFT              = 64,
    /// Page first packet event default priority
    RWIP_PRIO_PAGE_1ST_PKT          = 160,
    /// PCA event default priority
    RWIP_PRIO_PCA_DFT               = 160,
    /// Page scan event default priority
    RWIP_PRIO_PSCAN_DFT             = 64,
    /// Page scan event priority increment when canceled
    RWIP_PRIO_PSCAN_1ST_PKT         = 160,
    /// Synchronization Scan event default priority
    RWIP_PRIO_SSCAN_DFT             = 80,
    /// Synchronization Train event default priority
    RWIP_PRIO_STRAIN_DFT            = 40,
    #endif //#if (BT_EMB_PRESENT)
    #if (BLE_EMB_PRESENT)
    /// Default priority for scanning events
    RWIP_PRIO_SCAN_DFT              = 40,
    /// Default priority for auxillary scan/init (no_asap) rx events
    RWIP_PRIO_AUX_RX_DFT            = 96,
    /// Default priority for periodic adv rx events
    RWIP_PRIO_PER_ADV_RX_DFT        = 100, /* Must be higher than AUX_RX_DFT (LL/DDI/SCN/BV-47-C) */
    /// Priority for periodic adv rx events before sync is established
    RWIP_PRIO_PER_ADV_RX_ESTAB      = 160,
    /// Default priority for initiating events
    RWIP_PRIO_INIT_DFT              = 80,
    /// LE connection events default priority
    RWIP_PRIO_CONNECT_DFT           = 112,
    /// LE connection events priority with activity
    RWIP_PRIO_CONNECT_ACT           = 128,
    /// LE connection events priority before the connection is established
    RWIP_PRIO_CONNECT_ESTAB         = 160,
    /// LE connection events priority before instant
    RWIP_PRIO_CONNECT_INSTANT       = 160,
    /// Default priority for advertising events
    RWIP_PRIO_ADV_DFT               = 40,
    /// Default priority for advertising high duty cycle events
    RWIP_PRIO_ADV_HDC_DFT           = 80,
    /// Default priority for aux advertising events
    RWIP_PRIO_ADV_AUX_DFT           = 96,
    /// Default priority for periodic advertising events
    RWIP_PRIO_PER_ADV_DFT           = 80,
    /// Default priority for resolvable private addresses renewal event
    RWIP_PRIO_RPA_RENEW_DFT         = 80,
    #if (BLE_CIS)
    /// Default priority for Master CIS Connect
    RWIP_PRIO_M_CIS_DFT             = 100,
    /// Default priority for Slave CIS Connect
    RWIP_PRIO_S_CIS_DFT             = 100,
    #endif // (BLE_CIS)
    #if (BLE_BIS)
    /// Default priority for Master BIS
    RWIP_PRIO_M_BIS_DFT             = 100,
    /// Default priority for Slave BIS
    RWIP_PRIO_S_BIS_DFT             = 100,
    /// Default priority for BIS control subevent
    RWIP_PRIO_BIS_CTRL_DFT          = 160,
    #endif // (BLE_BIS)
    #if (BLE_CH_SCAN_SUPPORT)
    /// Default priority for Channel Scan activity
    RWIP_PRIO_CH_SCAN_DFT           = 40,
    #endif // (BLE_CH_SCAN_SUPPORT)
    #if (BLE_TEST_MODE_SUPPORT)
    /// Default priority for test mode events
    RWIP_PRIO_TEST_MODE_DFT         = 40,
    #endif // (BLE_TEST_MODE_SUPPORT)
    #if (BLE_CHSD)
    /// Default priority for Channel Sounding activity
    RWIP_PRIO_CHSD_DFT              = 100,
    #endif // (BLE_CHSD)
    #endif // #if (BLE_EMB_PRESENT)
    #if (MAC154_EMB_PRESENT)
    /// Default priority for RX events
    RWIP_PRIO_MAC154_RX_DFT            = 20,
    /// Default priority for TX events
    RWIP_PRIO_MAC154_TX_DFT            = 60,
    /// Default priority for ED events
    RWIP_PRIO_MAC154_ED_DFT            = 20,
    #endif // (MAC154_EMB_PRESENT)

    /// Max priority
    RWIP_PRIO_MAX                   = 255,
};
/// Default increment value definition
enum rwip_incr_dft
{
    #if (BT_EMB_PRESENT)
    /// ACL event default increment
    RWIP_INCR_ACL_DFT               = 8,
    /// ACL event increment with activity
    RWIP_INCR_ACL_ACT               = 8,
    /// ACL Role Switch event default increment
    RWIP_INCR_ACL_RSW               = 8,
    /// ACL sniff event default increment
    RWIP_INCR_ACL_SNIFF_DFT         = 8,
    /// ACL sniff transition event default increment
    RWIP_INCR_ACL_SNIFF_TRANS       = 8,
    #if (MAX_NB_SYNC > 0)
    /// SCO event default increment
    RWIP_INCR_SCO_DFT               = 8,
    #endif //MAX_NB_SYNC
    /// Broadcast ACL event default increment
    RWIP_INCR_BCST_DFT              = 8,
    /// Broadcast ACL event with LMP activity increment
    RWIP_INCR_BCST_ACT              = 8,
    /// CSB RX event default increment
    RWIP_INCR_CSB_RX_DFT            = 8,
    /// CSB TX event default increment
    RWIP_INCR_CSB_TX_DFT            = 8,
    /// Inquiry event default increment
    RWIP_INCR_INQ_DFT               = 8,
    /// Inquiry Scan event default increment
    RWIP_INCR_ISCAN_DFT             = 8,
    /// Page event default increment
    RWIP_INCR_PAGE_DFT              = 8,
    /// Page event default increment
    RWIP_INCR_PAGE_1ST_PKT          = 12,
    /// Page first packet event default increment
    RWIP_INCR_PCA_DFT               = 8,
    /// Page scan event default increment
    RWIP_INCR_PSCAN_DFT             = 8,
    /// Page scan event increment increment when canceled
    RWIP_INCR_PSCAN_1ST_PKT         = 8,
    /// Synchronization Scan event default increment
    RWIP_INCR_SSCAN_DFT             = 8,
    /// Synchronization Train event default increment
    RWIP_INCR_STRAIN_DFT            = 8,
    #endif //#if (BT_EMB_PRESENT)
    #if (BLE_EMB_PRESENT)
    /// Default increment for scanning events
    RWIP_INCR_SCAN_DFT              = 8,
    /// Default increment for auxillary scan/init (no_asap) rx events
    RWIP_INCR_AUX_RX_DFT            = 8,
    /// Default increment for periodic adv rx events
    RWIP_INCR_PER_ADV_RX_DFT        = 8,
    /// Increment for periodic adv rx events before sync is established
    RWIP_INCR_PER_ADV_RX_ESTAB      = 8,
    /// Default increment for initiating events
    RWIP_INCR_INIT_DFT              = 8,
    /// LE connection events default increment
    RWIP_INCR_CONNECT_DFT           = 8,
    /// LE connection events increment with activity
    RWIP_INCR_CONNECT_ACT           = 8,
    /// LE connection events increment before the connection is established
    RWIP_INCR_CONNECT_ESTAB         = 8,
    /// LE connection events increment before instant
    RWIP_INCR_CONNECT_INSTANT       = 8,
    /// Default increment for advertising events
    RWIP_INCR_ADV_DFT               = 8,
    /// Default increment for advertising high duty cycle events
    RWIP_INCR_ADV_HDC_PRIO_DFT      = 8,
    /// Default increment for aux advertising events
    RWIP_INCR_ADV_AUX_DFT           = 8,
    /// Default increment for periodic advertising events
    RWIP_INCR_PER_ADV_DFT           = 8,
    /// Default increment for resolvable private addresses renewal event
    RWIP_INCR_RPA_RENEW_DFT         = 8,
    #if (BLE_CIS)
    /// Default priority for Master CIS Connect
    RWIP_INCR_M_CIS_DFT             = 8,
    /// Default priority for Slave CIS Connect
    RWIP_INCR_S_CIS_DFT             = 8,
    #endif // (BLE_CIS)
    #if (BLE_BIS)
    /// Default priority increment for Master BIS
    RWIP_INCR_M_BIS_DFT             = 8,
    /// Default priority increment for Slave BIS
    RWIP_INCR_S_BIS_DFT             = 8,
    /// Default priority increment for BIS control subevent
    RWIP_INCR_BIS_CTRL_DFT          = 8,
    #endif // (BLE_BIS)
    /// Default priority for Channel Scan activity
    RWIP_INCR_CH_SCAN_DFT           = 0,
    #if (BLE_TEST_MODE_SUPPORT)
    /// Default priority increment for test mode events
    RWIP_INCR_TEST_MODE_DFT         = 0,
    #endif // (BLE_TEST_MODE_SUPPORT)
    #if (BLE_CHSD)
    /// Default increment for Channel Sounding activity
    RWIP_INCR_CHSD_DFT              = 8,
    #endif // (BLE_CHSD)
    #endif // #if (BLE_EMB_PRESENT)
    #if (MAC154_EMB_PRESENT)
    /// Default increment for MAC154 RX events
    RWIP_INCR_MAC154_RX_DFT            = 8,
    /// Default increment for MAC154 TX events
    RWIP_INCR_MAC154_TX_DFT            = 8,
    /// Default increment for MAC154 ED events
    RWIP_INCR_MAC154_ED_DFT            = 8,
    #endif // (MAC154_EMB_PRESENT)
};
#endif //#if (BLE_EMB_PRESENT || BT_EMB_PRESENT || MAC154_EMB_PRESENT)
/// @} BT Stack Configuration
/// @} ROOT
#if (MAC154_EMB_PRESENT)
    #define RESET_SPLIT  (1)
    #define UART_SPLIT   (1)
#else
    #define RESET_SPLIT  (0)
    #define UART_SPLIT   (0)
#endif

#if (defined(PLF_SPARK_EXTSYS0) && PLF_SPARK_EXTSYS0)
/// @brief  Configuration bits for spark board. This configuration is received from NVDS

extern uint32_t spark_configuration;

/// bit0        HPA/LPA if bit is set the HPA is used
#define SPARK_CONFIG_HPA_ENABLED ((spark_configuration & 1) == 1)
#endif

#endif //RWIP_CONFIG_H_
