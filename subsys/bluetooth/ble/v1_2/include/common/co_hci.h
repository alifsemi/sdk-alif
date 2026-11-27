/**
 ****************************************************************************************
 *
 * @file co_hci.h
 *
 * @brief This file contains the HCI Bluetooth defines, enumerations and structures
 *        definitions for use by all modules in RW stack.
 *
 * Copyright (C) RivieraWaves 2009-2019
 *
 ****************************************************************************************
 */

#ifndef CO_HCI_H_
#define CO_HCI_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup COMMON Common SW Block
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdbool.h>       // standard boolean definitions
#include <stddef.h>        // standard definitions
#include <stdint.h>        // standard integer definitions
#include "rwip_config.h"   // IP configuration
#include "co_bt_defines.h" // Common definitions
#include "compiler.h"      // compiler definitions

#if (BLE_CHSD && RW_DEBUG)
#include "aes_drbg.h"      // AES DRBG defintions
#endif // (BLE_CHSD && RW_DEBUG)

/*
 * DEFINES
 ****************************************************************************************
 */

// Maximum number of connection info in number of completed packets
#if (EMB_PRESENT)
#if(BT_EMB_PRESENT)
#define MAX_NB_CMP_PKTS_CON  (MAX_NB_ACTIVE_ACL)
#elif BLE_EMB_PRESENT
#define MAX_NB_CMP_PKTS_CON  (BLE_ACTIVITY_MAX+1) // ensure that at least 1 element is present
#elif  MAC154_EMB_PRESENT
#define MAX_NB_CMP_PKTS_CON  (MAC154_ACTIVITY_MAX)
#else // !(BT_EMB_PRESENT || BLE_EMB_PRESENT || MAC154_EMB_PRESENT)
#error non defined protocol
#endif // (EMB_PRESENT)

#else // (HOST_PRESENT)
#define MAX_NB_CMP_PKTS_CON  (HOST_ACTIVITY_MAX)
#endif // (EMB_PRESENT)

/******************************************************************************************/
/* -----------------   UART TRANSPORT LAYER DEFINITIONS Part IV.A    ---------------------*/
/******************************************************************************************/

///HCI Transport Header length - change if different transport
#define HCI_TRANSPORT_HDR_LEN                       0x01

/// H4 Message logical channels
enum h4_msg_lc
{
    ///UART header: command message type
    H4_MSG_LC_HCI_CMD                        = 0x01,
    ///UART header: ACL data message type
    H4_MSG_LC_HCI_ACL                        = 0x02,
    ///UART header: Synchronous data message type
    H4_MSG_LC_HCI_SYNC                       = 0x03,
    ///UART header: event message type
    H4_MSG_LC_HCI_EVT                        = 0x04,
    ///UART header: ISO data message type
    H4_MSG_LC_HCI_ISO                        = 0x05,
    #if (TRACER_PRESENT)
    /// Debug Tracer message type
    H4_MSG_LC_DBG_TRC                        = 0x06,
    #endif // (TRACER_PRESENT)
    /// AHI kernel message type
    #if (AHI_TL_SUPPORT)
    H4_MSG_LC_AHI                            = 0x10,
    #endif // (AHI_TL_SUPPORT)
    #if (CTRLI_TL_SUPPORT)
    /// Control to application logical channel
    H4_MSG_LC_CTRLI                          = 0x11u,
    #endif //(CTRLI_TL_SUPPORT)
    H4_MSG_LC_NB,
};

/******************************************************************************************/
/* -------------------------   HCI DEFINITIONS Part 4.E     ------------------------------*/
/******************************************************************************************/

///HCI Command Opcode byte length
#define HCI_CMD_OPCODE_LEN         (0x02)

///HCI Event code byte length
#define HCI_EVT_CODE_LEN           (0x01)

///HCI Command/Event parameter length field byte length
#define HCI_CMDEVT_PARLEN_LEN      (0x01)

///HCI Command header length
#define HCI_CMD_HDR_LEN            (HCI_CMD_OPCODE_LEN + HCI_CMDEVT_PARLEN_LEN)

///HCI Event header length
#define HCI_EVT_HDR_LEN            (HCI_EVT_CODE_LEN + HCI_CMDEVT_PARLEN_LEN)

/// HCI ACL header: handle and flags decoding
enum  hci_acl_hdr_fields
{
    /// bits[00:11]: Connection handle
    HCI_ACL_HDR_HDL_LSB        = (0),
    HCI_ACL_HDR_HDL_MASK       = (0x0FFF),
    /// bits[12:13]: Packet boundary flag
    HCI_ACL_HDR_PB_FLAG_LSB    = (12),
    HCI_ACL_HDR_PB_FLAG_MASK   = (0x3000),
    /// bits[14:15]: Broadcast flag
    HCI_ACL_HDR_BC_FLAG_LSB    = (14),
    HCI_ACL_HDR_BC_FLAG_MASK   = (0xC000),
    /// Packet boundary and Broadcast flags
    HCI_ACL_HDR_DATA_FLAGS_LSB  = (12),
    HCI_ACL_HDR_DATA_FLAGS_MASK = (0xF000),
};

#define HCI_ACL_HDR_HDL_FLAGS_POS  (0)
#define HCI_ACL_HDR_HDL_FLAGS_LEN  (2)
/// HCI ACL header: data length field length
#define HCI_ACL_HDR_DATA_LEN_POS   (HCI_ACL_HDR_HDL_FLAGS_LEN)
#define HCI_ACL_HDR_DATA_LEN_LEN   (2)

///HCI ACL data packet header length
#define HCI_ACL_HDR_LEN            (HCI_ACL_HDR_HDL_FLAGS_LEN + HCI_ACL_HDR_DATA_LEN_LEN)

/// HCI Synchronous header: handle and flags decoding
enum hci_syn_hdr_fields
{
    /// bits[00:11]: Connection handle
    HCI_SYNC_HDR_HDL_LSB   = (0),
    HCI_SYNC_HDR_HDL_MASK  = (0x0FFF),
    /// bits[12:13]: Packet status flag
    HCI_SYNC_HDR_PSF_LSB   = (12),
    HCI_SYNC_HDR_PSF_MASK  = (0x3000),
    /// bits[14:15]: RFU
    HCI_SYNC_HDR_RFU_LSB   = (14),
    HCI_SYNC_HDR_RFU_MASK  = (0xC000),
};

#define HCI_SYNC_HDR_HDL_FLAGS_POS  (0)
#define HCI_SYNC_HDR_HDL_FLAGS_LEN  (2)


/// HCI Synchronous header: data length field length
#define HCI_SYNC_HDR_DATA_LEN_POS   (HCI_SYNC_HDR_HDL_FLAGS_LEN)
#define HCI_SYNC_HDR_DATA_LEN_LEN   (1)
#define HCI_SYNC_MAX_DATA_SIZE      (255)

///HCI sync data packet header length
#define HCI_SYNC_HDR_LEN           (HCI_SYNC_HDR_HDL_FLAGS_LEN + HCI_SYNC_HDR_DATA_LEN_LEN)

///HCI Command Complete Event minimum parameter length: 1(nb_pk)+2(opcode)
#define HCI_CCEVT_HDR_PARLEN       (0x03)

///HCI Command Complete Event header length:1(code)+1(len)+1(pk)+2(opcode)
#define HCI_CCEVT_HDR_LEN          (HCI_EVT_HDR_LEN + HCI_CCEVT_HDR_PARLEN)

///HCI Basic Command Complete Event packet length
#define HCI_CCEVT_BASIC_LEN        (HCI_CCEVT_HDR_LEN + 1)

///HCI Command Status Event parameter length - constant
#define HCI_CSEVT_PARLEN           (0x04)

///HCI Command Status Event length:1(code)+1(len)+1(st)+1(pk)+2(opcode)
#define HCI_CSEVT_LEN              (HCI_EVT_HDR_LEN + HCI_CSEVT_PARLEN)

///HCI Reset Command parameter length
#define HCI_RESET_CMD_PARLEN       0

/// Default return parameter length for HCI Command Complete Event
#define HCI_CCEVT_BASIC_RETPAR_LEN 1

/// Max HCI commands param size
#define HCI_MAX_CMD_PARAM_SIZE    255

/// Max HCI event param size
#define HCI_MAX_EVT_PARAM_SIZE    255

/// Macro to extract OCF from OPCODE
#define HCI_OP2OCF(opcode)        ((opcode) & 0x03FF)

/// Macro to extract OGF from OPCODE
#define HCI_OP2OGF(opcode)        ((opcode) >> 10 & 0x003F)

/// Macro to create OPCODE from OGF and OCF
#define HCI_GET_OPCODE(ocf, ogf)      (((ogf) << 10) | ocf)

/// Maximum length of HCI advertising data fragments
#define HCI_ADV_DATA_FRAG_MAX_LEN        251

/// Maximum length of HCI periodic advertising data fragments
#define HCI_PER_ADV_DATA_FRAG_MAX_LEN    252

#if (BLE_CHSD)
/// Maximum length of HCI setp data in Subevent Results event
#define HCI_LE_CS_SUBEVENT_RESULT_MAX_STEP_DATA (HCI_MAX_EVT_PARAM_SIZE - 16)

/// Maximum length of HCI setp data in Subevent Continue Results event
#define HCI_LE_CS_SUBEVENT_RESULT_CONT_MAX_STEP_DATA (HCI_MAX_EVT_PARAM_SIZE - 9)

/// CS Test connection handle
#define HCI_CONHDL_CS_TEST        0x0FFF
#endif // (BLE_CHSD)

/// HCI ISO header: handle and flags decoding
enum  hci_iso_hdr_fields
{
    /// Connection handle (12 bits)
    HCI_ISO_HDR_HDL_LSB        = (0),
    HCI_ISO_HDR_HDL_MASK       = (0x0FFF),
    /// Packet boundary flag (2 bits)
    HCI_ISO_HDR_PB_FLAG_LSB    = (12),
    HCI_ISO_HDR_PB_FLAG_MASK   = (0x3000),
    /// Time_Stamp flag (1 bit)
    HCI_ISO_HDR_TS_FLAG_POS    = (14),
    HCI_ISO_HDR_TS_FLAG_BIT    = (0x4000),
    /// RFU (1 bit)
    HCI_ISO_HDR_RFU_FLAG_POS    = (0),
    HCI_ISO_HDR_RFU_FLAG_BIT    = (0x8000),
    /// ISO_Data_Load_Length (14 bits)
    HCI_ISO_HDR_ISO_DATA_LOAD_LEN_LSB  = (0),
    HCI_ISO_HDR_ISO_DATA_LOAD_LEN_MASK = (0x3FFF),
    /// RFU2 (2 bits)
    HCI_ISO_HDR_ISO_RFU2_LSB    = (14),
    HCI_ISO_HDR_ISO_RFU2_MASK   = (0xC000),
};

#define HCI_ISO_HDR_HDL_FLAGS_POS  (0)
#define HCI_ISO_HDR_HDL_FLAGS_LEN  (2)
/// HCI ISO header: ISO_Data_Load field length
#define HCI_ISO_HDR_ISO_DATA_LOAD_LEN_POS   (HCI_ISO_HDR_HDL_FLAGS_LEN)
#define HCI_ISO_HDR_ISO_DATA_LOAD_LEN_LEN   (2)

///HCI ACL data packet header length
#define HCI_ISO_HDR_LEN            (HCI_ISO_HDR_HDL_FLAGS_LEN + HCI_ISO_HDR_ISO_DATA_LOAD_LEN_LEN)

/// Packet Boundary Flag   HCI:5.4.5
#define PB_FLAG_1ST_FRAG          0x00
#define PB_FLAG_CONT_FRAG         0x01
#define PB_FLAG_CMP_FRAG          0x02
#define PB_FLAG_LAST_FRAG         0x03

/// HCI ISO header: handle and flags decoding
/// Time_Stamp (32 bits)
#define HCI_ISO_DATA_LOAD_TIME_STAMP_POS      (0)
#define HCI_ISO_DATA_LOAD_TIME_STAMP_LSB      (0)
#define HCI_ISO_DATA_LOAD_TIME_STAMP_MASK     (0xFFFFFFFFu)
/// Packet_Sequence_Number (16 bits)
#define HCI_ISO_DATA_LOAD_PKT_SEQ_NB_POS      (4)
#define HCI_ISO_DATA_LOAD_PKT_SEQ_NB_LSB      (0)
#define HCI_ISO_DATA_LOAD_PKT_SEQ_NB_MASK     (0xFFFF)
/// ISO_SDU_Length (12 bits)
#define HCI_ISO_DATA_LOAD_ISO_SDU_LEN_POS     (6)
#define HCI_ISO_DATA_LOAD_ISO_SDU_LEN_LSB     (0)
#define HCI_ISO_DATA_LOAD_ISO_SDU_LEN_MASK    (0x0FFF)
/// RFU (2 bits)
#define HCI_ISO_DATA_LOAD_RFU_POS             (6)
#define HCI_ISO_DATA_LOAD_RFU_LSB             (12)
#define HCI_ISO_DATA_LOAD_RFU_MASK            (0x3000)
/// Packet_Status_Flag (2 bits)
#define HCI_ISO_DATA_LOAD_PKT_STAT_FLAG_LSB   (14)
#define HCI_ISO_DATA_LOAD_PKT_STAT_FLAG_MASK  (0xC000)

/// HCI ISO_Data_Load - Length of Time_Stamp field
#define HCI_ISO_DATA_LOAD_TIME_STAMP_LEN    (4)

/// HCI ISO_Data_Load - Length of Packet Sequence Number field
#define HCI_ISO_DATA_LOAD_PKT_SEQ_NB_LEN    (2)

/// HCI ISO_Data_Load - Length of ISO SDU Length and packet status flags field
#define HCI_ISO_DATA_LOAD_ISO_SDU_LEN_LEN   (2)

/// HCI ISO_Data_Load - maximum header length
#define HCI_ISO_DATA_LOAD_HDR_LEN_MAX    (HCI_ISO_DATA_LOAD_TIME_STAMP_LEN + HCI_ISO_DATA_LOAD_PKT_SEQ_NB_LEN + HCI_ISO_DATA_LOAD_ISO_SDU_LEN_LEN)

/// HCI ISO_Data_Load - Packet Status Flag
enum  hci_iso_pkt_stat_flag
{
    /// Valid data. The complete ISO_SDU was received correctly
    HCI_ISO_PKT_STAT_FLAG_VALID   = (0),
    /// Possibly invalid data. The contents of the ISO_SDU may contain errors or part of the ISO_SDU may
    /// be missing. This is reported as "data with possible errors".
    HCI_ISO_PKT_STAT_FLAG_INVALID = (1),
    /// Part(s) of the ISO_SDU were not received correctly. This is reported as "lost data".
    HCI_ISO_PKT_STAT_FLAG_LOST    = (2),
};

/// HCI Connection event number of events tracing maximum
#define HCI_CON_MAX_TRACE_NB_EVENT       (10)

///HCI Command header components structure
typedef struct hci_cmd_hdr
{
    /// Opcode field
    uint16_t opcode;
    ///Parameter length - the number of bytes of the command parameters
    uint8_t parlen;
} hci_cmd_hdr_t;

///HCI ACL data packets header structure
struct hci_acl_hdr
{
    ///Connection handle & Data Flags
    uint16_t hdl_flags;
    ///Data length in number of bytes
    uint16_t datalen;
};

///HCI synchronous data packets header structure
struct hci_sync_hdr
{
    /// Connection handle & Data Flags
    uint16_t conhdl_flags;
    /// Data total length in number of bytes
    uint8_t data_total_len;
};

///HCI Event header components structure - contains all details possible in an event
typedef struct hci_evt_hdr
{
    ///Event code
    uint8_t code;
    ///Event parameters length
    uint8_t parlen;
} hci_evt_hdr_t;

///HCI ISO data packets header structure
typedef struct
{
    /// Connection handle & Data Flags
    uint16_t conhdl_flags;
    /// ISO Data load length in number of bytes
    uint16_t iso_data_load_len;
} hci_iso_hdr_t;

#if (BLE_PAWR)
/// HCI BLE PAWR subevent index value for no subevent case
#define HCI_LE_PAWR_NO_SUBEVT_VALUE   (0xFF)
/// HCI BLE PAWR subevent index maximum valid value
#define HCI_LE_PAWR_SUBEVT_VALUE_MAX  (0x7F)
#endif // (BLE_PAWR)

/**************************************************************************************
 **************                       HCI COMMANDS                     ****************
 **************************************************************************************/

///HCI enumeration of possible Command OGF values.
enum
{
    ///HCI Link Control Commands Group OGF code
    LK_CNTL_OGF = 0x01,
    ///HCI Link Policy Commands Group OGF code
    LK_POL_OGF,
    ///HCI Controller and Baseband Commands Group OGF code
    CNTLR_BB_OGF,
    ///HCI Information Parameters Commands Group OGF code
    INFO_PAR_OGF,
    ///HCI Status Commands Group OGF code
    STAT_PAR_OGF,
    ///HCI Test Commands Group OGF code
    TEST_OGF,
    ///HCI Low Energy Commands Group OGF code
    LE_CNTLR_OGF=0x08,
    ///HCI Vendor Specific Group OGF code
    VS_OGF = 0x3F,
    MAX_OGF
};

///Commands Opcodes: OGF(6b) | OCF(10b)
/* Some Abbreviation used in names:
 *  - LK   = Link Key
 *  - RD   = Read
 *  - WR   = Write
 *  - REM  = Remote
 *  - STG  = Settings
 *  - CON  = Connection
 *  - CHG  = Change
 *  - DFT  = Default
 *  - PER  = Periodic
 */

///HCI enumeration of possible Command OP Codes.
/*@TRACE*/
enum hci_opcode
{
    HCI_NO_OPERATION_CMD_OPCODE               = 0x0000,

    //Link Control Commands
    HCI_INQ_CMD_OPCODE                        = 0x0401,
    HCI_INQ_CANCEL_CMD_OPCODE                 = 0x0402,
    HCI_PER_INQ_MODE_CMD_OPCODE               = 0x0403,
    HCI_EXIT_PER_INQ_MODE_CMD_OPCODE          = 0x0404,
    HCI_CREATE_CON_CMD_OPCODE                 = 0x0405,
    HCI_DISCONNECT_CMD_OPCODE                 = 0x0406,
    HCI_CREATE_CON_CANCEL_CMD_OPCODE          = 0x0408,
    HCI_ACCEPT_CON_REQ_CMD_OPCODE             = 0x0409,
    HCI_REJECT_CON_REQ_CMD_OPCODE             = 0x040A,
    HCI_LK_REQ_REPLY_CMD_OPCODE               = 0x040B,
    HCI_LK_REQ_NEG_REPLY_CMD_OPCODE           = 0x040C,
    HCI_PIN_CODE_REQ_REPLY_CMD_OPCODE         = 0x040D,
    HCI_PIN_CODE_REQ_NEG_REPLY_CMD_OPCODE     = 0x040E,
    HCI_CHG_CON_PKT_TYPE_CMD_OPCODE           = 0x040F,
    HCI_AUTH_REQ_CMD_OPCODE                   = 0x0411,
    HCI_SET_CON_ENC_CMD_OPCODE                = 0x0413,
    HCI_CHG_CON_LK_CMD_OPCODE                 = 0x0415,
    HCI_MASTER_LK_CMD_OPCODE                  = 0x0417,
    HCI_REM_NAME_REQ_CMD_OPCODE               = 0x0419,
    HCI_REM_NAME_REQ_CANCEL_CMD_OPCODE        = 0x041A,
    HCI_RD_REM_SUPP_FEATS_CMD_OPCODE          = 0x041B,
    HCI_RD_REM_EXT_FEATS_CMD_OPCODE           = 0x041C,
    HCI_RD_REM_VER_INFO_CMD_OPCODE            = 0x041D,
    HCI_RD_CLK_OFF_CMD_OPCODE                 = 0x041F,
    HCI_RD_LMP_HDL_CMD_OPCODE                 = 0x0420,
    HCI_SETUP_SYNC_CON_CMD_OPCODE             = 0x0428,
    HCI_ACCEPT_SYNC_CON_REQ_CMD_OPCODE        = 0x0429,
    HCI_REJECT_SYNC_CON_REQ_CMD_OPCODE        = 0x042A,
    HCI_IO_CAP_REQ_REPLY_CMD_OPCODE           = 0x042B,
    HCI_USER_CFM_REQ_REPLY_CMD_OPCODE         = 0x042C,
    HCI_USER_CFM_REQ_NEG_REPLY_CMD_OPCODE     = 0x042D,
    HCI_USER_PASSKEY_REQ_REPLY_CMD_OPCODE     = 0x042E,
    HCI_USER_PASSKEY_REQ_NEG_REPLY_CMD_OPCODE = 0x042F,
    HCI_REM_OOB_DATA_REQ_REPLY_CMD_OPCODE     = 0x0430,
    HCI_REM_OOB_DATA_REQ_NEG_REPLY_CMD_OPCODE = 0x0433,
    HCI_IO_CAP_REQ_NEG_REPLY_CMD_OPCODE       = 0x0434,
    HCI_ENH_SETUP_SYNC_CON_CMD_OPCODE         = 0x043D,
    HCI_ENH_ACCEPT_SYNC_CON_CMD_OPCODE        = 0x043E,
    HCI_TRUNC_PAGE_CMD_OPCODE                 = 0x043F,
    HCI_TRUNC_PAGE_CAN_CMD_OPCODE             = 0x0440,
    HCI_SET_CON_SLV_BCST_CMD_OPCODE           = 0x0441,
    HCI_SET_CON_SLV_BCST_REC_CMD_OPCODE       = 0x0442,
    HCI_START_SYNC_TRAIN_CMD_OPCODE           = 0x0443,
    HCI_REC_SYNC_TRAIN_CMD_OPCODE             = 0x0444,
    HCI_REM_OOB_EXT_DATA_REQ_REPLY_CMD_OPCODE = 0x0445,

    //Link Policy Commands
    HCI_HOLD_MODE_CMD_OPCODE                  = 0x0801,
    HCI_SNIFF_MODE_CMD_OPCODE                 = 0x0803,
    HCI_EXIT_SNIFF_MODE_CMD_OPCODE            = 0x0804,
    HCI_PARK_STATE_CMD_OPCODE                 = 0x0805,
    HCI_EXIT_PARK_STATE_CMD_OPCODE            = 0x0806,
    HCI_QOS_SETUP_CMD_OPCODE                  = 0x0807,
    HCI_ROLE_DISCOVERY_CMD_OPCODE             = 0x0809,
    HCI_SWITCH_ROLE_CMD_OPCODE                = 0x080B,
    HCI_RD_LINK_POL_STG_CMD_OPCODE            = 0x080C,
    HCI_WR_LINK_POL_STG_CMD_OPCODE            = 0x080D,
    HCI_RD_DFT_LINK_POL_STG_CMD_OPCODE        = 0x080E,
    HCI_WR_DFT_LINK_POL_STG_CMD_OPCODE        = 0x080F,
    HCI_FLOW_SPEC_CMD_OPCODE                  = 0x0810,
    HCI_SNIFF_SUB_CMD_OPCODE                  = 0x0811,

    //Controller and Baseband Commands
    HCI_SET_EVT_MASK_CMD_OPCODE               = 0x0C01,
    HCI_RESET_CMD_OPCODE                      = 0x0C03,
    HCI_SET_EVT_FILTER_CMD_OPCODE             = 0x0C05,
    HCI_FLUSH_CMD_OPCODE                      = 0x0C08,
    HCI_RD_PIN_TYPE_CMD_OPCODE                = 0x0C09,
    HCI_WR_PIN_TYPE_CMD_OPCODE                = 0x0C0A,
    HCI_CREATE_NEW_UNIT_KEY_CMD_OPCODE        = 0x0C0B,
    HCI_RD_STORED_LK_CMD_OPCODE               = 0x0C0D,
    HCI_WR_STORED_LK_CMD_OPCODE               = 0x0C11,
    HCI_DEL_STORED_LK_CMD_OPCODE              = 0x0C12,
    HCI_WR_LOCAL_NAME_CMD_OPCODE              = 0x0C13,
    HCI_RD_LOCAL_NAME_CMD_OPCODE              = 0x0C14,
    HCI_RD_CON_ACCEPT_TO_CMD_OPCODE           = 0x0C15,
    HCI_WR_CON_ACCEPT_TO_CMD_OPCODE           = 0x0C16,
    HCI_RD_PAGE_TO_CMD_OPCODE                 = 0x0C17,
    HCI_WR_PAGE_TO_CMD_OPCODE                 = 0x0C18,
    HCI_RD_SCAN_EN_CMD_OPCODE                 = 0x0C19,
    HCI_WR_SCAN_EN_CMD_OPCODE                 = 0x0C1A,
    HCI_RD_PAGE_SCAN_ACT_CMD_OPCODE           = 0x0C1B,
    HCI_WR_PAGE_SCAN_ACT_CMD_OPCODE           = 0x0C1C,
    HCI_RD_INQ_SCAN_ACT_CMD_OPCODE            = 0x0C1D,
    HCI_WR_INQ_SCAN_ACT_CMD_OPCODE            = 0x0C1E,
    HCI_RD_AUTH_EN_CMD_OPCODE                 = 0x0C1F,
    HCI_WR_AUTH_EN_CMD_OPCODE                 = 0x0C20,
    HCI_RD_CLASS_OF_DEV_CMD_OPCODE            = 0x0C23,
    HCI_WR_CLASS_OF_DEV_CMD_OPCODE            = 0x0C24,
    HCI_RD_VOICE_STG_CMD_OPCODE               = 0x0C25,
    HCI_WR_VOICE_STG_CMD_OPCODE               = 0x0C26,
    HCI_RD_AUTO_FLUSH_TO_CMD_OPCODE           = 0x0C27,
    HCI_WR_AUTO_FLUSH_TO_CMD_OPCODE           = 0x0C28,
    HCI_RD_NB_BDCST_RETX_CMD_OPCODE           = 0x0C29,
    HCI_WR_NB_BDCST_RETX_CMD_OPCODE           = 0x0C2A,
    HCI_RD_HOLD_MODE_ACTIVITY_CMD_OPCODE      = 0x0C2B,
    HCI_WR_HOLD_MODE_ACTIVITY_CMD_OPCODE      = 0x0C2C,
    HCI_RD_TX_PWR_LVL_CMD_OPCODE              = 0x0C2D,
    HCI_RD_SYNC_FLOW_CTRL_EN_CMD_OPCODE       = 0x0C2E,
    HCI_WR_SYNC_FLOW_CTRL_EN_CMD_OPCODE       = 0x0C2F,
    HCI_SET_CTRL_TO_HOST_FLOW_CTRL_CMD_OPCODE = 0x0C31,
    HCI_HOST_BUF_SIZE_CMD_OPCODE              = 0x0C33,
    HCI_HOST_NB_CMP_PKTS_CMD_OPCODE           = 0x0C35,
    HCI_RD_LINK_SUPV_TO_CMD_OPCODE            = 0x0C36,
    HCI_WR_LINK_SUPV_TO_CMD_OPCODE            = 0x0C37,
    HCI_RD_NB_SUPP_IAC_CMD_OPCODE             = 0x0C38,
    HCI_RD_CURR_IAC_LAP_CMD_OPCODE            = 0x0C39,
    HCI_WR_CURR_IAC_LAP_CMD_OPCODE            = 0x0C3A,
    HCI_SET_AFH_HOST_CH_CLASS_CMD_OPCODE      = 0x0C3F,
    HCI_RD_INQ_SCAN_TYPE_CMD_OPCODE           = 0x0C42,
    HCI_WR_INQ_SCAN_TYPE_CMD_OPCODE           = 0x0C43,
    HCI_RD_INQ_MODE_CMD_OPCODE                = 0x0C44,
    HCI_WR_INQ_MODE_CMD_OPCODE                = 0x0C45,
    HCI_RD_PAGE_SCAN_TYPE_CMD_OPCODE          = 0x0C46,
    HCI_WR_PAGE_SCAN_TYPE_CMD_OPCODE          = 0x0C47,
    HCI_RD_AFH_CH_ASSESS_MODE_CMD_OPCODE      = 0x0C48,
    HCI_WR_AFH_CH_ASSESS_MODE_CMD_OPCODE      = 0x0C49,
    HCI_RD_EXT_INQ_RSP_CMD_OPCODE             = 0x0C51,
    HCI_WR_EXT_INQ_RSP_CMD_OPCODE             = 0x0C52,
    HCI_REFRESH_ENC_KEY_CMD_OPCODE            = 0x0C53,
    HCI_RD_SP_MODE_CMD_OPCODE                 = 0x0C55,
    HCI_WR_SP_MODE_CMD_OPCODE                 = 0x0C56,
    HCI_RD_LOC_OOB_DATA_CMD_OPCODE            = 0x0C57,
    HCI_RD_INQ_RSP_TX_PWR_LVL_CMD_OPCODE      = 0x0C58,
    HCI_WR_INQ_TX_PWR_LVL_CMD_OPCODE          = 0x0C59,
    #if (VOICE_OVER_HCI)
    HCI_RD_DFT_ERR_DATA_REP_CMD_OPCODE        = 0x0C5A,
    HCI_WR_DFT_ERR_DATA_REP_CMD_OPCODE        = 0x0C5B,
    #endif // (VOICE_OVER_HCI)
    HCI_ENH_FLUSH_CMD_OPCODE                  = 0x0C5F,
    HCI_SEND_KEYPRESS_NOTIF_CMD_OPCODE        = 0x0C60,
    HCI_SET_EVT_MASK_PAGE_2_CMD_OPCODE        = 0x0C63,
    HCI_RD_FLOW_CNTL_MODE_CMD_OPCODE          = 0x0C66,
    HCI_WR_FLOW_CNTL_MODE_CMD_OPCODE          = 0x0C67,
    HCI_RD_ENH_TX_PWR_LVL_CMD_OPCODE          = 0x0C68,
    HCI_RD_LE_HOST_SUPP_CMD_OPCODE            = 0x0C6C,
    HCI_WR_LE_HOST_SUPP_CMD_OPCODE            = 0x0C6D,
    HCI_SET_MWS_CHANNEL_PARAMS_CMD_OPCODE     = 0x0C6E,
    HCI_SET_EXTERNAL_FRAME_CONFIG_CMD_OPCODE  = 0x0C6F,
    HCI_SET_MWS_SIGNALING_CMD_OPCODE          = 0x0C70,
    HCI_SET_MWS_TRANSPORT_LAYER_CMD_OPCODE    = 0x0C71,
    HCI_SET_MWS_SCAN_FREQ_TABLE_CMD_OPCODE    = 0x0C72,
    HCI_SET_MWS_PATTERN_CONFIG_CMD_OPCODE     = 0x0C73,
    HCI_SET_RES_LT_ADDR_CMD_OPCODE            = 0x0C74,
    HCI_DEL_RES_LT_ADDR_CMD_OPCODE            = 0x0C75,
    HCI_SET_CON_SLV_BCST_DATA_CMD_OPCODE      = 0x0C76,
    HCI_RD_SYNC_TRAIN_PARAM_CMD_OPCODE        = 0x0C77,
    HCI_WR_SYNC_TRAIN_PARAM_CMD_OPCODE        = 0x0C78,
    HCI_RD_SEC_CON_HOST_SUPP_CMD_OPCODE       = 0x0C79,
    HCI_WR_SEC_CON_HOST_SUPP_CMD_OPCODE       = 0x0C7A,
    HCI_RD_AUTH_PAYL_TO_CMD_OPCODE            = 0x0C7B,
    HCI_WR_AUTH_PAYL_TO_CMD_OPCODE            = 0x0C7C,
    HCI_RD_LOC_OOB_EXT_DATA_CMD_OPCODE        = 0x0C7D,
    HCI_RD_EXT_PAGE_TO_CMD_OPCODE             = 0x0C7E,
    HCI_WR_EXT_PAGE_TO_CMD_OPCODE             = 0x0C7F,
    HCI_RD_EXT_INQ_LEN_CMD_OPCODE             = 0x0C80,
    HCI_WR_EXT_INQ_LEN_CMD_OPCODE             = 0x0C81,
    HCI_SET_ECO_BASE_INTV_CMD_OPCODE          = 0x0C82,
    HCI_CONFIG_DATA_PATH_CMD_OPCODE           = 0x0C83,
    #if (BT_53)
    HCI_SET_MIN_ENC_KEY_SIZE_CMD_OPCODE       = 0x0C84,
    #endif //(BT_53)

    //Info Params
    HCI_RD_LOCAL_VER_INFO_CMD_OPCODE               = 0x1001,
    HCI_RD_LOCAL_SUPP_CMDS_CMD_OPCODE              = 0x1002,
    HCI_RD_LOCAL_SUPP_FEATS_CMD_OPCODE             = 0x1003,
    HCI_RD_LOCAL_EXT_FEATS_CMD_OPCODE              = 0x1004,
    HCI_RD_BUF_SIZE_CMD_OPCODE                     = 0x1005,
    HCI_RD_BD_ADDR_CMD_OPCODE                      = 0x1009,
    HCI_RD_DATA_BLOCK_SIZE_CMD_OPCODE              = 0x100A,
    HCI_RD_LOCAL_SUPP_CODECS_CMD_OPCODE            = 0x100B,
    HCI_RD_LOCAL_SP_OPT_CMD_OPCODE                 = 0x100C,
    HCI_RD_LOCAL_SUPP_CODECS_V2_CMD_OPCODE         = 0x100D,
    HCI_RD_LOCAL_SUPP_CODEC_CAP_CMD_OPCODE         = 0x100E,
    HCI_RD_LOCAL_SUPP_CTRL_DELAY_CMD_OPCODE        = 0x100F,

    //Status Params
    HCI_RD_FAIL_CONTACT_CNT_CMD_OPCODE             = 0x1401,
    HCI_RST_FAIL_CONTACT_CNT_CMD_OPCODE            = 0x1402,
    HCI_RD_LINK_QUAL_CMD_OPCODE                    = 0x1403,
    HCI_RD_RSSI_CMD_OPCODE                         = 0x1405,
    HCI_RD_AFH_CH_MAP_CMD_OPCODE                   = 0x1406,
    HCI_RD_CLK_CMD_OPCODE                          = 0x1407,
    HCI_RD_ENC_KEY_SIZE_CMD_OPCODE                 = 0x1408,
    HCI_GET_MWS_TRANSPORT_LAYER_CONFIG_CMD_OPCODE  = 0x140C,

    //Testing Commands
    HCI_RD_LOOPBACK_MODE_CMD_OPCODE                = 0x1801,
    HCI_WR_LOOPBACK_MODE_CMD_OPCODE                = 0x1802,
    HCI_EN_DUT_MODE_CMD_OPCODE                     = 0x1803,
    HCI_WR_SP_DBG_MODE_CMD_OPCODE                  = 0x1804,
    HCI_WR_SEC_CON_TEST_MODE_CMD_OPCODE            = 0x180A,

    /// LE Commands Opcodes
    HCI_LE_SET_EVT_MASK_CMD_OPCODE                      = 0x2001,
    HCI_LE_RD_BUF_SIZE_CMD_OPCODE                       = 0x2002,
    HCI_LE_RD_LOCAL_SUPP_FEATS_CMD_OPCODE               = 0x2003,
    HCI_LE_SET_RAND_ADDR_CMD_OPCODE                     = 0x2005,
    HCI_LE_SET_ADV_PARAM_CMD_OPCODE                     = 0x2006,
    HCI_LE_RD_ADV_CHNL_TX_PW_CMD_OPCODE                 = 0x2007,
    HCI_LE_SET_ADV_DATA_CMD_OPCODE                      = 0x2008,
    HCI_LE_SET_SCAN_RSP_DATA_CMD_OPCODE                 = 0x2009,
    HCI_LE_SET_ADV_EN_CMD_OPCODE                        = 0x200A,
    HCI_LE_SET_SCAN_PARAM_CMD_OPCODE                    = 0x200B,
    HCI_LE_SET_SCAN_EN_CMD_OPCODE                       = 0x200C,
    HCI_LE_CREATE_CON_CMD_OPCODE                        = 0x200D,
    HCI_LE_CREATE_CON_CANCEL_CMD_OPCODE                 = 0x200E,
    HCI_LE_RD_WLST_SIZE_CMD_OPCODE                      = 0x200F,
    HCI_LE_CLEAR_WLST_CMD_OPCODE                        = 0x2010,
    HCI_LE_ADD_DEV_TO_WLST_CMD_OPCODE                   = 0x2011,
    HCI_LE_RMV_DEV_FROM_WLST_CMD_OPCODE                 = 0x2012,
    HCI_LE_CON_UPDATE_CMD_OPCODE                        = 0x2013,
    HCI_LE_SET_HOST_CH_CLASS_CMD_OPCODE                 = 0x2014,
    HCI_LE_RD_CH_MAP_CMD_OPCODE                         = 0x2015,
    HCI_LE_RD_REM_FEATS_CMD_OPCODE                      = 0x2016,
    HCI_LE_ENC_CMD_OPCODE                               = 0x2017,
    HCI_LE_RAND_CMD_OPCODE                              = 0x2018,
    HCI_LE_EN_ENC_CMD_OPCODE                            = 0x2019,
    HCI_LE_LTK_REQ_REPLY_CMD_OPCODE                     = 0x201A,
    HCI_LE_LTK_REQ_NEG_REPLY_CMD_OPCODE                 = 0x201B,
    HCI_LE_RD_SUPP_STATES_CMD_OPCODE                    = 0x201C,
    HCI_LE_RX_TEST_V1_CMD_OPCODE                        = 0x201D,
    HCI_LE_TX_TEST_V1_CMD_OPCODE                        = 0x201E,
    HCI_LE_TEST_END_CMD_OPCODE                          = 0x201F,
    HCI_LE_REM_CON_PARAM_REQ_REPLY_CMD_OPCODE           = 0x2020,
    HCI_LE_REM_CON_PARAM_REQ_NEG_REPLY_CMD_OPCODE       = 0x2021,
    HCI_LE_SET_DATA_LEN_CMD_OPCODE                      = 0x2022,
    HCI_LE_RD_SUGGTED_DFT_DATA_LEN_CMD_OPCODE           = 0x2023,
    HCI_LE_WR_SUGGTED_DFT_DATA_LEN_CMD_OPCODE           = 0x2024,
    HCI_LE_RD_LOC_P256_PUB_KEY_CMD_OPCODE               = 0x2025,
    HCI_LE_GEN_DHKEY_V1_CMD_OPCODE                      = 0x2026,
    HCI_LE_ADD_DEV_TO_RSLV_LIST_CMD_OPCODE              = 0x2027,
    HCI_LE_RMV_DEV_FROM_RSLV_LIST_CMD_OPCODE            = 0x2028,
    HCI_LE_CLEAR_RSLV_LIST_CMD_OPCODE                   = 0x2029,
    HCI_LE_RD_RSLV_LIST_SIZE_CMD_OPCODE                 = 0x202A,
    HCI_LE_RD_PEER_RSLV_ADDR_CMD_OPCODE                 = 0x202B,
    HCI_LE_RD_LOC_RSLV_ADDR_CMD_OPCODE                  = 0x202C,
    HCI_LE_SET_ADDR_RESOL_EN_CMD_OPCODE                 = 0x202D,
    HCI_LE_SET_RSLV_PRIV_ADDR_TO_CMD_OPCODE             = 0x202E,
    HCI_LE_RD_MAX_DATA_LEN_CMD_OPCODE                   = 0x202F,
    HCI_LE_RD_PHY_CMD_OPCODE                            = 0x2030,
    HCI_LE_SET_DFT_PHY_CMD_OPCODE                       = 0x2031,
    HCI_LE_SET_PHY_CMD_OPCODE                           = 0x2032,
    HCI_LE_RX_TEST_V2_CMD_OPCODE                        = 0x2033,
    HCI_LE_TX_TEST_V2_CMD_OPCODE                        = 0x2034,
    HCI_LE_SET_ADV_SET_RAND_ADDR_CMD_OPCODE             = 0x2035,
    HCI_LE_SET_EXT_ADV_PARAM_V1_CMD_OPCODE              = 0x2036,
    HCI_LE_SET_EXT_ADV_DATA_CMD_OPCODE                  = 0x2037,
    HCI_LE_SET_EXT_SCAN_RSP_DATA_CMD_OPCODE             = 0x2038,
    HCI_LE_SET_EXT_ADV_EN_CMD_OPCODE                    = 0x2039,
    HCI_LE_RD_MAX_ADV_DATA_LEN_CMD_OPCODE               = 0x203A,
    HCI_LE_RD_NB_SUPP_ADV_SETS_CMD_OPCODE               = 0x203B,
    HCI_LE_RMV_ADV_SET_CMD_OPCODE                       = 0x203C,
    HCI_LE_CLEAR_ADV_SETS_CMD_OPCODE                    = 0x203D,
    #if (BLE_PER_ADV)
    HCI_LE_SET_PER_ADV_PARAM_V1_CMD_OPCODE              = 0x203E,
    HCI_LE_SET_PER_ADV_DATA_CMD_OPCODE                  = 0x203F,
    HCI_LE_SET_PER_ADV_EN_CMD_OPCODE                    = 0x2040,
    #endif // (BLE_PER_ADV)
    HCI_LE_SET_EXT_SCAN_PARAM_CMD_OPCODE                = 0x2041,
    HCI_LE_SET_EXT_SCAN_EN_CMD_OPCODE                   = 0x2042,
    HCI_LE_EXT_CREATE_CON_V1_CMD_OPCODE                 = 0x2043,
    #if (BLE_PER_ADV)
    HCI_LE_PER_ADV_CREATE_SYNC_CMD_OPCODE               = 0x2044,
    HCI_LE_PER_ADV_CREATE_SYNC_CANCEL_CMD_OPCODE        = 0x2045,
    HCI_LE_PER_ADV_TERM_SYNC_CMD_OPCODE                 = 0x2046,
    HCI_LE_ADD_DEV_TO_PER_ADV_LIST_CMD_OPCODE           = 0x2047,
    HCI_LE_RMV_DEV_FROM_PER_ADV_LIST_CMD_OPCODE         = 0x2048,
    HCI_LE_CLEAR_PER_ADV_LIST_CMD_OPCODE                = 0x2049,
    HCI_LE_RD_PER_ADV_LIST_SIZE_CMD_OPCODE              = 0x204A,
    #endif // (BLE_PER_ADV)
    HCI_LE_RD_TX_PWR_CMD_OPCODE                         = 0x204B,
    HCI_LE_RD_RF_PATH_COMP_CMD_OPCODE                   = 0x204C,
    HCI_LE_WR_RF_PATH_COMP_CMD_OPCODE                   = 0x204D,
    HCI_LE_SET_PRIV_MODE_CMD_OPCODE                     = 0x204E,
    HCI_LE_RX_TEST_V3_CMD_OPCODE                        = 0x204F,
    HCI_LE_TX_TEST_V3_CMD_OPCODE                        = 0x2050,
    #if (BLE_PER_ADV)
    #if (BLE_CONLESS_CTE_TX)
    HCI_LE_SET_CONLESS_CTE_TX_PARAM_CMD_OPCODE          = 0x2051,
    HCI_LE_SET_CONLESS_CTE_TX_EN_CMD_OPCODE             = 0x2052,
    #endif // (BLE_CONLESS_CTE_TX)
    #if (BLE_CONLESS_CTE_RX)
    HCI_LE_SET_CONLESS_IQ_SAMPL_EN_CMD_OPCODE           = 0x2053,
    #endif // (BLE_CONLESS_CTE_RX)
    #endif // (BLE_PER_ADV)
    #if (BLE_CON_CTE_REQ)
    HCI_LE_SET_CON_CTE_RX_PARAM_CMD_OPCODE              = 0x2054,
    #endif // (BLE_CON_CTE_REQ)
    #if (BLE_CON_CTE_RSP)
    HCI_LE_SET_CON_CTE_TX_PARAM_CMD_OPCODE              = 0x2055,
    #endif // (BLE_CON_CTE_RSP)
    #if (BLE_CON_CTE_REQ)
    HCI_LE_CON_CTE_REQ_EN_CMD_OPCODE                    = 0x2056,
    #endif // (BLE_CON_CTE_REQ)
    #if (BLE_CON_CTE_RSP)
    HCI_LE_CON_CTE_RSP_EN_CMD_OPCODE                    = 0x2057,
    #endif // (BLE_CON_CTE_RSP)
    #if (BLE_AOA || BLE_AOD)
    HCI_LE_RD_ANTENNA_INF_CMD_OPCODE                    = 0x2058,
    #endif // (BLE_AOA || BLE_AOD)
    #if (BLE_PER_ADV)
    HCI_LE_SET_PER_ADV_REC_EN_CMD_OPCODE                = 0x2059,
    #endif // (BLE_PER_ADV)
    #if (BLE_PAST)
    HCI_LE_PER_ADV_SYNC_TRANSF_CMD_OPCODE               = 0x205A,
    HCI_LE_PER_ADV_SET_INFO_TRANSF_CMD_OPCODE           = 0x205B,
    HCI_LE_SET_PER_ADV_SYNC_TRANSF_PARAM_CMD_OPCODE     = 0x205C,
    HCI_LE_SET_DFT_PER_ADV_SYNC_TRANSF_PARAM_CMD_OPCODE = 0x205D,
    #endif // (BLE_PAST)
    HCI_LE_GEN_DHKEY_V2_CMD_OPCODE                      = 0x205E,
    HCI_LE_MOD_SLEEP_CLK_ACC_CMD_OPCODE                 = 0x205F,
    HCI_LE_RD_BUF_SIZE_V2_CMD_OPCODE                    = 0x2060,
    #if (BLE_CIS || BLE_BIS)
    HCI_LE_RD_ISO_TX_SYNC_CMD_OPCODE                    = 0x2061,
    #endif // (BLE_CIS || BLE_BIS)
    #if (BLE_CIS)
    HCI_LE_SET_CIG_PARAMS_CMD_OPCODE                    = 0x2062,
    HCI_LE_SET_CIG_PARAMS_TEST_CMD_OPCODE               = 0x2063,
    HCI_LE_CREATE_CIS_CMD_OPCODE                        = 0x2064,
    HCI_LE_REMOVE_CIG_CMD_OPCODE                        = 0x2065,
    HCI_LE_ACCEPT_CIS_REQ_CMD_OPCODE                    = 0x2066,
    HCI_LE_REJECT_CIS_REQ_CMD_OPCODE                    = 0x2067,
    #endif // (BLE_CIS)
    #if (BLE_BIS)
    HCI_LE_CREATE_BIG_CMD_OPCODE                        = 0x2068,
    HCI_LE_CREATE_BIG_TEST_CMD_OPCODE                   = 0x2069,
    HCI_LE_TERMINATE_BIG_CMD_OPCODE                     = 0x206A,
    HCI_LE_BIG_CREATE_SYNC_CMD_OPCODE                   = 0x206B,
    HCI_LE_BIG_TERMINATE_SYNC_CMD_OPCODE                = 0x206C,
    #endif // (BLE_BIS)
    HCI_LE_REQ_PEER_SCA_CMD_OPCODE                      = 0x206D,
    #if (BLE_CIS || BLE_BIS)
    HCI_LE_SETUP_ISO_DATA_PATH_CMD_OPCODE               = 0x206E,
    HCI_LE_REMOVE_ISO_DATA_PATH_CMD_OPCODE              = 0x206F,
    HCI_LE_ISO_TX_TEST_CMD_OPCODE                       = 0x2070,
    HCI_LE_ISO_RX_TEST_CMD_OPCODE                       = 0x2071,
    HCI_LE_ISO_READ_TEST_COUNTERS_CMD_OPCODE            = 0x2072,
    HCI_LE_ISO_TEST_END_CMD_OPCODE                      = 0x2073,
    #endif // (BLE_CIS || BLE_BIS)
    HCI_LE_SET_HOST_FEATURE_CMD_OPCODE                  = 0x2074,
    #if (BLE_CIS || BLE_BIS)
    HCI_LE_RD_ISO_LINK_QUALITY_CMD_OPCODE               = 0x2075,
    #endif // (BLE_CIS || BLE_BIS)
    HCI_LE_ENH_RD_TX_PWR_LVL_CMD_OPCODE                 = 0x2076,
    HCI_LE_RD_REMOTE_TX_PWR_LVL_CMD_OPCODE              = 0x2077,
    HCI_LE_SET_PATH_LOSS_REP_PARAM_CMD_OPCODE           = 0x2078,
    HCI_LE_SET_PATH_LOSS_REP_EN_CMD_OPCODE              = 0x2079,
    HCI_LE_SET_TX_POWER_REP_EN_CMD_OPCODE               = 0x207A,
    HCI_LE_TX_TEST_V4_CMD_OPCODE                        = 0x207B,
    #if (BT_53)
    HCI_LE_SET_DATA_RELATED_ADDR_CHG_CMD_OPCODE         = 0x207C,
    #endif // (BT_53)
    #if (BLE_SUBRATING)
    HCI_LE_SET_DFT_SUBRATE_CMD_OPCODE                   = 0x207D,
    HCI_LE_SUBRATE_REQ_CMD_OPCODE                       = 0x207E,
    #endif // (BLE_SUBRATING)

    #if (BLE_CSSA)
    HCI_LE_SET_EXT_ADV_PARAM_V2_CMD_OPCODE              = 0x207F,
    #endif // (BLE_CSSA)

    #if (BLE_PAWR)
    HCI_LE_SET_PER_ADV_SUBEVT_DATA_CMD_OPCODE           = 0x2082,
    HCI_LE_SET_PER_ADV_RSP_DATA_CMD_OPCODE              = 0x2083,
    HCI_LE_SET_PER_SYNC_SUBEVT_CMD_OPCODE               = 0x2084,
    HCI_LE_EXT_CREATE_CON_V2_CMD_OPCODE                 = 0x2085,
    HCI_LE_SET_PER_ADV_PARAM_V2_CMD_OPCODE              = 0x2086,
    #endif //(BLE_PAWR)

    #if (BT_60)
    HCI_LE_RD_ALL_LOC_SUPP_FEATS_CMD_OPCODE             = 0x2087,
    HCI_LE_RD_ALL_REM_FEATS_CMD_OPCODE                  = 0x2088,
    #endif // (BT_60)

    #if (BLE_CHSD)
    HCI_LE_CS_RD_LOC_SUPP_CAP_CMD_OPCODE                = 0x2089,
    HCI_LE_CS_RD_REM_SUPP_CAP_CMD_OPCODE                = 0x208A,
    HCI_LE_CS_WR_CACHED_REM_SUPP_CAP_CMD_OPCODE         = 0x208B,
    HCI_LE_CS_SEC_EN_CMD_OPCODE                         = 0x208C,
    HCI_LE_CS_SET_DFT_SETTINGS_CMD_OPCODE               = 0x208D,
    HCI_LE_CS_RD_REM_FAE_TABLE_CMD_OPCODE               = 0x208E,
    HCI_LE_CS_WR_CACHED_REM_FAE_TABLE_CMD_OPCODE        = 0x208F,
    HCI_LE_CS_CREATE_CONFIG_CMD_OPCODE                  = 0x2090,
    HCI_LE_CS_REMOVE_CONFIG_CMD_OPCODE                  = 0x2091,
    HCI_LE_CS_SET_CH_CLASS_CMD_OPCODE                   = 0x2092,
    HCI_LE_CS_SET_PROC_PARAMS_CMD_OPCODE                = 0x2093,
    HCI_LE_CS_PROC_EN_CMD_OPCODE                        = 0x2094,
    HCI_LE_CS_TEST_CMD_OPCODE                           = 0x2095,
    HCI_LE_CS_TEST_END_CMD_OPCODE                       = 0x2096,
    #endif //(BLE_CHSD)

    #if (BLE_HOST_PRESENT && BLE_MONITOR_ADV)
    HCI_LE_MAL_ADD_DEVICE_CMD_OPCODE                    = 0x2098,
    HCI_LE_MAL_REMOVE_DEVICE_CMD_OPCODE                 = 0x2099,
    HCI_LE_MAL_CLEAR_CMD_OPCODE                         = 0x209A,
    HCI_LE_MAL_READ_SIZE_CMD_OPCODE                     = 0x209B,
    HCI_LE_MAL_ENABLE_CMD_OPCODE                        = 0x209C,
    #endif //(BLE_HOST_PRESENT && BLE_MONITOR_ADV)

    #if (BLE_FSU)
    HCI_LE_FRAME_SPACE_UPD_CMD_OPCODE                   = 0x209D,
    #endif //(BLE_FSU)

    #if (BLE_ENH_TEST_MODE)
    HCI_LE_UTP_SEND_CMD_OPCODE                          = 0x03E8, // TBD
    HCI_LE_EN_OTA_UTP_MODE_CMD_OPCODE                   = 0x03E9, // TBD
    #endif // (BLE_ENH_TEST_MODE)


    ///Debug commands - OGF = 0x3F (spec)
    HCI_DBG_RD_MEM_CMD_OPCODE                      = 0xFC01,
    HCI_DBG_WR_MEM_CMD_OPCODE                      = 0xFC02,
    HCI_DBG_DEL_PAR_CMD_OPCODE                     = 0xFC03,
    HCI_DBG_ID_FLASH_CMD_OPCODE                    = 0xFC05,
    HCI_DBG_ER_FLASH_CMD_OPCODE                    = 0xFC06,
    HCI_DBG_WR_FLASH_CMD_OPCODE                    = 0xFC07,
    HCI_DBG_RD_FLASH_CMD_OPCODE                    = 0xFC08,
    HCI_DBG_RD_PAR_CMD_OPCODE                      = 0xFC09,
    HCI_DBG_WR_PAR_CMD_OPCODE                      = 0xFC0A,
    HCI_DBG_WLAN_COEX_CMD_OPCODE                   = 0xFC0B,
    HCI_DBG_WLAN_COEXTST_SCEN_CMD_OPCODE           = 0xFC0D,
    HCI_DBG_BT_SEND_LMP_CMD_OPCODE                 = 0xFC0E,
    HCI_DBG_SET_LOCAL_CLOCK_CMD_OPCODE             = 0xFC0F,
    HCI_DBG_RD_KE_STATS_CMD_OPCODE                 = 0xFC10,
    HCI_DBG_PLF_RESET_CMD_OPCODE                   = 0xFC11,
    HCI_DBG_RD_MEM_INFO_CMD_OPCODE                 = 0xFC12,
    HCI_VS_SET_PREF_SLAVE_LATENCY_CMD_OPCODE       = 0xFC13,
    HCI_VS_SET_PREF_SLAVE_EVT_DUR_CMD_OPCODE       = 0xFC14,
    HCI_VS_SET_MAX_RX_SIZE_AND_TIME_CMD_OPCODE     = 0xFC15,
    HCI_VS_SET_PRIO_ELEVATION_CMD_OPCODE           = 0xFC16,
    #if (RW_DEBUG)
    HCI_DBG_LE_CON_EVT_TRACE_CMD_OPCODE            = 0xFC17,
    #endif // (RW_DEBUG)
    #if (BLE_CH_SCAN_SUPPORT)
    HCI_VS_LE_CH_SCAN_CMD_OPCODE                   = 0xFC18,
    HCI_VS_LE_CH_SCAN_END_CMD_OPCODE               = 0xFC19,
    #endif // (BLE_CH_SCAN_SUPPORT)
    #if (BLE_PWR_CTRL)
    HCI_VS_LE_SET_TX_PWR_CMD_OPCODE                = 0xFC1A,
    #endif // (BLE_PWR_CTRL)
    #if (MAX_NB_SYNC || TWS_LE_FWD_PRIMARY)
    HCI_VS_BT_SETUP_AUDIO_DATA_PATH_CMD_OPCODE     = 0xFC1B,
    HCI_VS_BT_REMOVE_AUDIO_DATA_PATH_CMD_OPCODE    = 0xFC1C,
    #endif // (MAX_NB_SYNC || TWS_LE_FWD_PRIMARY)
    #if (TWS_LE_FWD_PRIMARY)
    HCI_VS_BT_SET_AUDIO_ROUTE_CMD_OPCODE           = 0xFC1D,
    HCI_VS_BT_REMOVE_AUDIO_ROUTE_CMD_OPCODE        = 0xFC1E,
    #if (BLE_CENTRAL)
    HCI_VS_LE_START_FOLLOW_ESCO_CMD_OPCODE         = 0xFC1F,
    HCI_VS_LE_STOP_FOLLOW_ESCO_CMD_OPCODE          = 0xFC20,
    #endif // (BLE_CENTRAL)
    #endif // (TWS_LE_FWD_PRIMARY)
    #if (BLE_UPPER_TESTER)
    HCI_VS_LE_TEST_MODE_CMD_OPCODE                 = 0xFC21,
    #endif // (BLE_UPPER_TESTER)
    HCI_DBG_LE_SET_PER_ADV_SUBEVT_DATA_CMD_OPCODE  = 0xFC22,
    HCI_DBG_LE_SET_PER_ADV_RSP_DATA_CMD_OPCODE     = 0xFC23,
    HCI_DBG_BLE_REG_RD_CMD_OPCODE                  = 0xFC30,
    HCI_DBG_BLE_REG_WR_CMD_OPCODE                  = 0xFC31,
    HCI_DBG_SEND_LLCP_CMD_OPCODE                   = 0xFC35,
    HCI_DBG_LLCP_DISCARD_CMD_OPCODE                = 0xFC36,
    HCI_DBG_RF_REG_RD_CMD_OPCODE                   = 0xFC39,
    HCI_DBG_RF_REG_WR_CMD_OPCODE                   = 0xFC3A,
    HCI_DBG_RF_SWITCH_CLK_CMD_OPCODE               = 0xFC3C,
    HCI_DBG_RF_WR_DATA_TX_CMD_OPCODE               = 0xFC3D,
    HCI_DBG_RF_RD_DATA_RX_CMD_OPCODE               = 0xFC3E,
    HCI_DBG_RF_CNTL_TX_CMD_OPCODE                  = 0xFC3F,
    HCI_DBG_RF_SYNC_P_CNTL_CMD_OPCODE              = 0xFC40,

    #if (RW_DEBUG && BT_EMB_PRESENT)
    HCI_DBG_BT_DISCARD_LMP_EN_CMD_OPCODE           = 0xFC44,
    #endif //RW_DEBUG && BT_EMB_PRESENT

    HCI_DBG_MWS_COEX_CMD_OPCODE                    = 0xFC45,
    HCI_DBG_MWS_COEXTST_SCEN_CMD_OPCODE            = 0xFC46,

    HCI_VS_I2C_READ_CMD_OPCODE                     = 0xFC47,
    HCI_VS_I2C_WRITE_CMD_OPCODE                    = 0xFC48,

    #if (BT_READ_PICONET_CLOCK)
    HCI_VS_RD_PICONET_CLOCK_CMD_OPCODE             = 0xFC50,
    #endif // (BT_READ_PICONET_CLOCK)

    #if (BLE_ISO_MODE_0)
    /// Vendor Specific commands for ISO Mode 0
    HCI_VS_MIC_LESS_SET_CMD_OPCODE                   = 0xFC51,
    HCI_VS_SETUP_AM0_DATA_PATH_CMD_OPCODE            = 0xFC52,
    HCI_VS_REMOVE_AM0_DATA_PATH_CMD_OPCODE           = 0xFC53,
    HCI_VS_SETUP_AM0_STREAM_CMD_OPCODE               = 0xFC54,
    HCI_VS_REMOVE_AM0_STREAM_CMD_OPCODE              = 0xFC55,
    #endif // (BLE_ISO_MODE_0)

    #if (BLE_CIS || BLE_BIS)
    HCI_VS_ISO_SUBEVT_MARGIN_SET_CMD_OPCODE        = 0xFC57,
    #endif // (BLE_CIS || BLE_BIS)

    #if (BLE_CHSD && RW_DEBUG)
    HCI_DBG_TEST_AES_DRBG_AA_FUNC_CMD_OPCODE       = 0xFC58,
    HCI_DBG_TEST_AES_DRBG_CR1_FUNC_CMD_OPCODE      = 0xFC59,
    HCI_DBG_TEST_AES_DRBG_H9_FUNC_CMD_OPCODE       = 0xFC5A,
    #endif //(BLE_CHSD && RW_DEBUG)

    #if CRYPTO_UT
    HCI_DBG_TEST_CRYPTO_FUNC_CMD_OPCODE            = 0xFC60,
    #endif //CRYPTO_UT

    #if RW_DEBUG
    HCI_DBG_TEST_SCH_PLAN_SET_CMD_OPCODE           = 0xFC61,
    HCI_DBG_TEST_SCH_PLAN_REM_CMD_OPCODE           = 0xFC62,
    HCI_DBG_TEST_SCH_PLAN_CHK_CMD_OPCODE           = 0xFC63,
    HCI_DBG_TEST_SCH_PLAN_REQ_CMD_OPCODE           = 0xFC64,
    #endif //RW_DEBUG

    #if BLE_IQ_GEN
    HCI_DBG_IQGEN_CFG_CMD_OPCODE                   = 0xFC65,
    #endif //BLE_IQ_GEN

    #if (AUDIO_SYNC_SUPPORT)
    HCI_VS_AUSY_CON_EVT_CNT_GET_CMD_OPCODE        = 0xFC66,
    HCI_VS_AUSY_EVT_TX_TIME_GET_CMD_OPCODE        = 0xFC67,
    HCI_VS_AUSY_LAST_RX_TIME_GET_CMD_OPCODE       = 0xFC68,
    HCI_VS_AUSY_CLOCK_SAMPLE_GET_CMD_OPCODE       = 0xFC69,
    HCI_VS_AUSY_CLOCK_CONVERT_CMD_OPCODE          = 0xFC6A,
    HCI_VS_AUSY_CIS_EVT_CTRL_CMD_OPCODE           = 0xFC6B,
    HCI_VS_AUSY_BIS_EVT_CTRL_CMD_OPCODE           = 0xFC6C,
    #endif // (AUDIO_SYNC_SUPPORT)

    #if (BT_HCI_TEST_MODE)
    HCI_VS_RX_TEST_CMD_OPCODE                     = 0xFC70,
    HCI_VS_TX_TEST_CMD_OPCODE                     = 0xFC71,
    HCI_VS_TEST_END_CMD_OPCODE                    = 0xFC72,
    #endif // (BT_HCI_TEST_MODE)

    #if (BT_53)
    HCI_DBG_SET_MAX_ENC_KEY_SIZE_CMD_OPCODE       = 0xFC73,
    #endif // (BT_53)

    #if ((BLE_EMB_PRESENT && !HOST_PRESENT) || (!EMB_PRESENT && BLE_HOST_PRESENT))
    HCI_VS_LE_DECRYPT_CMD_OPCODE                  = 0xFC74,
    #endif // ((BLE_EMB_PRESENT && !HOST_PRESENT) || (!EMB_PRESENT && BLE_HOST_PRESENT))

    HCI_VS_BT_SET_RX_MAX_SLOT_CMD_OPCODE            = 0xFC75,
    #if (RW_DEBUG && (BT_EMB_PRESENT || BLE_EMB_PRESENT))
    HCI_VS_SET_SCHED_PRIO_CMD_OPCODE                = 0xFC76,
    #endif // (RW_DEBUG && (BT_EMB_PRESENT || BLE_EMB_PRESENT))
    #if (BLE_EMB_PRESENT && BLE_TEST_MODE_SUPPORT)
    HCI_VS_LE_SET_INFINITE_TEST_CMD_OPCODE          = 0xFC77,
    #endif // (BLE_EMB_PRESENT && BLE_TEST_MODE_SUPPORT)

    #if (BLE_PWR_CTRL && RW_DEBUG)
    HCI_DBG_LE_SET_RSP_TX_PWR_CMD_OPCODE            = 0xFC78,
    #endif // (BLE_PWR_CTRL && RW_DEBUG)

    #if (BLE_HDT && RW_DEBUG)
    HCI_VS_LE_HDT_EN_CMD_OPCODE                     = 0xFC79,
    #endif // (BLE_HDT && RW_DEBUG)
    /*
     * Ellisys defined HCI VS Commands for EBQ control, from default IXIT Ellisys HCI VCS Offset 0xE0.
     * 0xE0-0xE7 reserved for future extension.
     */
    #if (RW_DEBUG) && (RW_MWS_COEX)
    HCI_VS_EBQ_INIT_SAM_NEGO_CMD_OPCODE             = 0xFCE0,
    #endif // (RW_DEBUG) && (RW_MWS_COEX)
    HCI_VS_SET_MDM_SWITCH_PARAM_CMD_OPCODE            = 0xFCF0,
};

/**************************************************************************************
 **************                        HCI EVENTS                      ****************
 **************************************************************************************/

///Event Codes
/*@TRACE*/
enum hci_evt_code
{
    HCI_INQ_CMP_EVT_CODE                       = 0x01,
    HCI_INQ_RES_EVT_CODE                       = 0x02,
    HCI_CON_CMP_EVT_CODE                       = 0x03,
    HCI_CON_REQ_EVT_CODE                       = 0x04,
    HCI_DISC_CMP_EVT_CODE                      = 0x05,
    HCI_AUTH_CMP_EVT_CODE                      = 0x06,
    HCI_REM_NAME_REQ_CMP_EVT_CODE              = 0x07,
    HCI_ENC_CHG_EVT_CODE                       = 0x08,
    HCI_CHG_CON_LK_CMP_EVT_CODE                = 0x09,
    HCI_MASTER_LK_CMP_EVT_CODE                 = 0x0A,
    HCI_RD_REM_SUPP_FEATS_CMP_EVT_CODE         = 0x0B,
    HCI_RD_REM_VER_INFO_CMP_EVT_CODE           = 0x0C,
    HCI_QOS_SETUP_CMP_EVT_CODE                 = 0x0D,
    HCI_CMD_CMP_EVT_CODE                       = 0x0E,
    HCI_CMD_STATUS_EVT_CODE                    = 0x0F,
    HCI_HW_ERR_EVT_CODE                        = 0x10,
    HCI_FLUSH_OCCURRED_EVT_CODE                = 0x11,
    HCI_ROLE_CHG_EVT_CODE                      = 0x12,
    HCI_NB_CMP_PKTS_EVT_CODE                   = 0x13,
    HCI_MODE_CHG_EVT_CODE                      = 0x14,
    HCI_RETURN_LINK_KEYS_EVT_CODE              = 0x15,
    HCI_PIN_CODE_REQ_EVT_CODE                  = 0x16,
    HCI_LK_REQ_EVT_CODE                        = 0x17,
    HCI_LK_NOTIF_EVT_CODE                      = 0x18,
    HCI_DATA_BUF_OVFLW_EVT_CODE                = 0x1A,
    HCI_MAX_SLOT_CHG_EVT_CODE                  = 0x1B,
    HCI_RD_CLK_OFF_CMP_EVT_CODE                = 0x1C,
    HCI_CON_PKT_TYPE_CHG_EVT_CODE              = 0x1D,
    HCI_QOS_VIOL_EVT_CODE                      = 0x1E,
    HCI_PAGE_SCAN_REPET_MODE_CHG_EVT_CODE      = 0x20,
    HCI_FLOW_SPEC_CMP_EVT_CODE                 = 0x21,
    HCI_INQ_RES_WITH_RSSI_EVT_CODE             = 0x22,
    HCI_RD_REM_EXT_FEATS_CMP_EVT_CODE          = 0x23,
    HCI_SYNC_CON_CMP_EVT_CODE                  = 0x2C,
    HCI_SYNC_CON_CHG_EVT_CODE                  = 0x2D,
    HCI_SNIFF_SUB_EVT_CODE                     = 0x2E,
    HCI_EXT_INQ_RES_EVT_CODE                   = 0x2F,
    HCI_ENC_KEY_REFRESH_CMP_EVT_CODE           = 0x30,
    HCI_IO_CAP_REQ_EVT_CODE                    = 0x31,
    HCI_IO_CAP_RSP_EVT_CODE                    = 0x32,
    HCI_USER_CFM_REQ_EVT_CODE                  = 0x33,
    HCI_USER_PASSKEY_REQ_EVT_CODE              = 0x34,
    HCI_REM_OOB_DATA_REQ_EVT_CODE              = 0x35,
    HCI_SP_CMP_EVT_CODE                        = 0x36,
    HCI_LINK_SUPV_TO_CHG_EVT_CODE              = 0x38,
    HCI_ENH_FLUSH_CMP_EVT_CODE                 = 0x39,
    HCI_USER_PASSKEY_NOTIF_EVT_CODE            = 0x3B,
    HCI_KEYPRESS_NOTIF_EVT_CODE                = 0x3C,
    HCI_REM_HOST_SUPP_FEATS_NOTIF_EVT_CODE     = 0x3D,
    HCI_LE_META_EVT_CODE                       = 0x3E,
    HCI_MAX_EVT_MSK_PAGE_1_CODE                = 0x40,
    HCI_TRIGGERED_CLOCK_CAPTURE_CODE           = 0x4E,
    HCI_SYNC_TRAIN_CMP_EVT_CODE                = 0x4F,
    HCI_SYNC_TRAIN_REC_EVT_CODE                = 0x50,
    HCI_CON_SLV_BCST_REC_EVT_CODE              = 0x51,
    HCI_CON_SLV_BCST_TO_EVT_CODE               = 0x52,
    HCI_TRUNC_PAGE_CMP_EVT_CODE                = 0x53,
    HCI_SLV_PAGE_RSP_TO_EVT_CODE               = 0x54,
    HCI_CON_SLV_BCST_CH_MAP_CHG_EVT_CODE       = 0x55,
    HCI_AUTH_PAYL_TO_EXP_EVT_CODE              = 0x57,
    HCI_SAM_STATUS_CHANGE_EVT_CODE             = 0x58,
    HCI_ENC_CHG_V2_EVT_CODE                    = 0x59,
    HCI_MAX_EVT_MSK_PAGE_2_CODE,
    HCI_VS_META_EVT_CODE                       = 0xFF,
};

/*@TRACE*/
enum hci_le_evt_subcode
{
    /// LE Events Subcodes
    HCI_LE_CON_CMP_EVT_SUBCODE                 = 0x01,
    HCI_LE_ADV_REPORT_EVT_SUBCODE              = 0x02,
    HCI_LE_CON_UPDATE_CMP_EVT_SUBCODE          = 0x03,
    HCI_LE_RD_REM_FEATS_CMP_EVT_SUBCODE        = 0x04,
    HCI_LE_LTK_REQUEST_EVT_SUBCODE             = 0x05,
    HCI_LE_REM_CON_PARAM_REQ_EVT_SUBCODE       = 0x06,
    HCI_LE_DATA_LEN_CHG_EVT_SUBCODE            = 0x07,
    HCI_LE_RD_LOC_P256_PUB_KEY_CMP_EVT_SUBCODE = 0x08,
    HCI_LE_GEN_DHKEY_CMP_EVT_SUBCODE           = 0x09,
    HCI_LE_ENH_CON_CMP_V1_EVT_SUBCODE          = 0x0A,
    HCI_LE_DIR_ADV_REP_EVT_SUBCODE             = 0x0B,
    HCI_LE_PHY_UPD_CMP_EVT_SUBCODE             = 0x0C,
    HCI_LE_EXT_ADV_REPORT_EVT_SUBCODE          = 0x0D,
    #if (BLE_PER_ADV)
    HCI_LE_PER_ADV_SYNC_EST_V1_EVT_SUBCODE     = 0x0E,
    HCI_LE_PER_ADV_REPORT_V1_EVT_SUBCODE       = 0x0F,
    HCI_LE_PER_ADV_SYNC_LOST_EVT_SUBCODE       = 0x10,
    #endif // (BLE_PER_ADV)
    HCI_LE_SCAN_TIMEOUT_EVT_SUBCODE            = 0x11,
    HCI_LE_ADV_SET_TERMINATED_EVT_SUBCODE      = 0x12,
    HCI_LE_SCAN_REQ_RCVD_EVT_SUBCODE           = 0x13,
    HCI_LE_CH_SEL_ALGO_EVT_SUBCODE             = 0x14,
    #if (BLE_CONLESS_CTE_RX)
    HCI_LE_CONLESS_IQ_REPORT_EVT_SUBCODE       = 0x15,
    #endif // (BLE_CONLESS_CTE_RX)
    #if (BLE_CON_CTE_REQ)
    HCI_LE_CON_IQ_REPORT_EVT_SUBCODE           = 0x16,
    HCI_LE_CTE_REQ_FAILED_EVT_SUBCODE          = 0x17,
    #endif // (BLE_CON_CTE_REQ)
    #if (BLE_PAST)
    HCI_LE_PER_ADV_SYNC_TRANSF_REC_V1_EVT_SUBCODE = 0x18,
    #endif // (BLE_PAST)
    #if (BLE_CIS)
    HCI_LE_CIS_ESTABLISHED_V1_EVT_SUBCODE      = 0x19,
    HCI_LE_CIS_REQUEST_EVT_SUBCODE             = 0x1A,
    #endif // (BLE_CIS)
    #if (BLE_BIS)
    HCI_LE_CREATE_BIG_CMP_EVT_SUBCODE          = 0x1B,
    HCI_LE_TERMINATE_BIG_CMP_EVT_SUBCODE       = 0x1C,
    HCI_LE_BIG_SYNC_ESTABLISHED_EVT_SUBCODE    = 0x1D,
    HCI_LE_BIG_SYNC_LOST_EVT_SUBCODE           = 0x1E,
    #endif // (BLE_BIS)
    HCI_LE_REQ_PEER_SCA_CMP_EVT_SUBCODE        = 0x1F,
    HCI_LE_PATH_LOSS_THRESHOLD_EVT_SUBCODE     = 0x20,
    HCI_LE_TX_POWER_REPORTING_EVT_SUBCODE      = 0x21,
    #if (BLE_BIS)
    HCI_LE_BIG_INFO_ADV_REPORT_EVT_SUBCODE     = 0x22,
    #endif // (BLE_BIS)
    #if (BLE_SUBRATING)
    HCI_LE_SUBRATE_CHG_EVT_SUBCODE             = 0x23,
    #endif // (BLE_SUBRATING)
    #if (BLE_PAWR)
    HCI_LE_PER_ADV_SYNC_EST_V2_EVT_SUBCODE     = 0x24,
    HCI_LE_PER_ADV_REPORT_V2_EVT_SUBCODE       = 0x25,
    #if (BLE_PAST)
    HCI_LE_PER_ADV_SYNC_TRANSF_REC_V2_EVT_SUBCODE = 0x26,
    #endif // (BLE_PAST)
    HCI_LE_PER_ADV_SUBEVT_DATA_REQ_EVT_SUBCODE = 0x27,
    HCI_LE_PER_ADV_RSP_REPORT_EVT_SUBCODE      = 0x28,
    HCI_LE_ENH_CON_CMP_V2_EVT_SUBCODE          = 0x29,
    #endif // (BLE_PAWR)
    #if (BLE_CIS)
    HCI_LE_CIS_ESTABLISHED_V2_EVT_SUBCODE      = 0x2A,
    #endif // (BLE_CIS)
    #if (BT_60)
    HCI_LE_CS_RD_ALL_REM_FEATS_CMP_EVT_SUBCODE = 0x2B,
    #endif // (BT_60)
    #if (BLE_CHSD)
    HCI_LE_CS_RD_REM_SUPP_CAP_CMP_EVT_SUBCODE  = 0x2C,
    HCI_LE_CS_RD_REM_FAE_TABLE_CMP_EVT_SUBCODE = 0x2D,
    HCI_LE_CS_SEC_EN_CMP_EVT_SUBCODE           = 0x2E,
    HCI_LE_CS_CONFIG_CMP_EVT_SUBCODE           = 0x2F,
    HCI_LE_CS_PROC_EN_CMP_EVT_SUBCODE          = 0x30,
    HCI_LE_CS_SUBEVT_RESULT_EVT_SUBCODE        = 0x31,
    HCI_LE_CS_SUBEVT_RESULT_CONT_EVT_SUBCODE   = 0x32,
    HCI_LE_CS_TEST_END_CMP_EVT_SUBCODE         = 0x33,
    #endif // (BLE_CHSD)
    #if (BLE_HOST_PRESENT && BLE_MONITOR_ADV)
    HCI_LE_MAL_REPORT_EVT_SUBCODE              = 0x34,
    #endif // (BLE_HOST_PRESENT && BLE_MONITOR_ADV)
    #if (BLE_FSU)
    HCI_LE_FRAME_SPACE_UPD_CMP_EVT_SUBCODE     = 0x35,
    #endif // (BLE_FSU)
    #if (BLE_ENH_TEST_MODE)
    HCI_LE_UTP_RECEIVE_EVT_SUBCODE             = 0x40, // TBD
    #endif // (BLE_ENH_TEST_MODE)
};

/*@TRACE*/
enum hci_vs_evt_subcode
{
    #if (RW_DEBUG)
    /// DBG Events Subcodes
    HCI_DBG_ASSERT_EVT_SUBCODE              = 0x02,
    HCI_DBG_LE_CON_EVT_TRACE_EVT_SUBCODE    = 0x03,
    #endif //(RW_DEBUG)

    #if (AUDIO_SYNC_SUPPORT && BLE_CIS)
    HCI_VS_AUSY_CIS_ESTAB_PARAM_EVT_SUBCODE = 0x04,
    #endif // (AUDIO_SYNC_SUPPORT && BLE_CIS)
    #if (AUDIO_SYNC_SUPPORT && BLE_BIS)
    HCI_VS_AUSY_BIS_ESTAB_PARAM_EVT_SUBCODE = 0x05,
    #endif // (AUDIO_SYNC_SUPPORT && BLE_BIS)

    #if (BLE_UPPER_TESTER)
    HCI_VS_LE_TEST_MODE_EVT_SUBCODE         = 0x06,
    #endif // (BLE_UPPER_TESTER)

    HCI_VS_INVALID_EVT_SUBCODE              = 0xFF,
};

/// Action of HCI Dbg LE Set Periodic Subevent/Response Data commands
enum hci_dbg_per_adv_auto_response
{
    PAWR_AUTO_RSP_DISABLE  = 0,
    PAWR_AUTO_RSP_ENABLE   = 1,
    PAWR_AUTO_RSP_ONE_SHOT = 2,
};

/// Event mask page enum
enum hci_evt_mask_page
{
    /// page 0
    HCI_PAGE_0,
    /// page 1
    HCI_PAGE_1,
    /// page 2
    HCI_PAGE_2,
    /// Default
    HCI_PAGE_DFT,
    /// LE event
    HCI_PAGE_LE,
};

#if (BLE_ISO_PRESENT)
#if (BLE_ISO_MODE_0)
/// Current audio mode
enum iso_am0_ctrl
{
    // Stop Audio Mode 0 Stream
    ISO_AM0_CRL_DISABLE,
    // Start Audio Mode 0 Stream
    ISO_AM0_CRL_ENABLE,
};
#endif // (BLE_ISO_MODE_0)
#endif // (BLE_ISO_PRESENT)



/**************************************************************************************
 **************                 HCI MESSAGE STRUCTURES                 ****************
 **************************************************************************************/

/// HCI ACL data packet structure
/*@TRACE*/
typedef struct hci_acl_data
{
    /// bits[00:11]: Connection handle
    /// bits[12:13]: Packet boundary flag
    /// bits[14:15]: Broadcast flag
    uint16_t conhdl_pb_bc_flag;
    /// length of the data
    uint16_t length;
    /// Memory Pointer address
    uint32_t buf_ptr;
} hci_acl_data_t;

/// HCI Synchronous data packet structure
/*@TRACE*/
typedef struct hci_sync_data
{
    /// bits[00:11]: Connection handle
    /// bits[12:13]: Packet status flag
    uint16_t conhdl_psf;
    /// length of the data
    uint8_t length;
    /// EM buffer pointer
    uint16_t buf_ptr;
} hci_sync_data_t;

#if BLE_ISO_PRESENT
/// ISO_Data_load structure
/*@TRACE*/
typedef struct iso_data_load
{
    /// Time_Stamp
    uint32_t time_stamp;
    /// Packet Sequence Number
    uint16_t pkt_seq_nb;
    /// length of the ISO SDU
    /// bits[00:11]: ISO SDU length
    /// bits[12:13]: RFU
    /// bits[14:15]: Packet_Status_Flag
    uint16_t  iso_sdu_length_psf;
    /// Pointer to the SDU
    uint8_t* iso_sdu_ptr;
} iso_data_load_t;

/// HCI ISO data packet structure
/*@TRACE*/
typedef struct hci_iso_data
{
    /// bits[00:11]: Connection handle
    /// bits[12:13]: Packet boundary flag
    /// bit[14]: Time_Stamp flag
    /// bit[15]: RFU
    uint16_t conhdl_pbf_tsf;
    /// ISO_Data_Load_Length and 2 RFU bits
    uint16_t iso_data_load_len;
    /// ISO_Data_Load
    struct iso_data_load iso_data_load;
} hci_iso_data_t;
#endif //BLE_ISO_PRESENT

/*
 * HCI COMMANDS PARAMETERS (to classify)
 ****************************************************************************************
 */

/// HCI basic command structure with connection handle
/*@TRACE
 * hci_rd_rssi_cmd = hci_basic_conhdl_cmd
 * hci_le_rd_ch_map_cmd = hci_basic_conhdl_cmd
 * hci_le_ltk_req_neg_reply_cmd = hci_basic_conhdl_cmd*/
typedef struct hci_basic_conhdl_cmd
{
    /// connection handle
    uint16_t conhdl;
} hci_basic_conhdl_cmd_t;

/// HCI basic command structure with BD address
typedef struct hci_basic_bd_addr_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
} hci_basic_bd_addr_cmd_t;

/// HCI Accept connection request command structure
/*@TRACE*/
typedef struct hci_accept_con_req_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///Page Scan Repetition Mode
    uint8_t role;
} hci_accept_con_req_cmd_t;

/// HCI Accept synchronous connection request command structure
/*@TRACE*/
typedef struct hci_accept_sync_con_req_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///Transmit bandwidth
    uint32_t tx_bw;
    ///Receive bandwidth
    uint32_t rx_bw;
    ///Max latency
    uint16_t max_lat;
    ///Voice settings
    uint16_t vx_set;
    ///Retransmission effort
    uint8_t retx_eff;
    ///Packet type
    uint16_t pkt_type  ;
} hci_accept_sync_con_req_cmd_t;

/// HCI Enhanced Accept synchronous connection request command structure
/*@TRACE*/
typedef struct hci_enh_accept_sync_con_cmd
{

    struct bd_addr bd_addr; // BD address
    uint32_t tx_bw; // Transmit Bandwidth (in B/sec)
    uint32_t rx_bw; // Receive Bandwidth (in B/sec)
    uint8_t tx_cod_fmt[5]; // Transmit Coding Format
    uint8_t rx_cod_fmt[5]; // Receive Coding Format
    uint16_t tx_cod_fr_sz; // Transmit Codec Frame Size (in B)
    uint16_t rx_cod_fr_sz; // Receive Codec Frame Size (in B)
    uint32_t in_bw; // Input Bandwidth (in B/sec)
    uint32_t out_bw; // Output Bandwidth (in B/sec)
    uint8_t in_cod_fmt[5]; // Input Coding Format
    uint8_t out_cod_fmt[5]; // Output Coding Format
    uint16_t in_cod_data_sz; // Input Coded Data Size (in bits)
    uint16_t out_cod_data_sz; // Output Coded Data Size (in bits)
    uint8_t in_data_fmt; // Input PCM Data Format
    uint8_t out_data_fmt; // Output PCM Data Format
    uint8_t in_msb_pos; // Input PCM Sample Payload MSB Position (in bits)
    uint8_t out_msb_pos; // Output PCM Sample Payload MSB Position (in bits)
    uint8_t in_data_path; // Input Data Path
    uint8_t out_data_path; // Output Data Path
    uint8_t in_tr_unit_sz; // Input Transport Unit Size (in bits)
    uint8_t out_tr_unit_sz; // Output Transport Unit Size (in bits)
    uint16_t max_lat; // Max Latency (in ms)
    uint16_t packet_type; // Packet Type
    uint8_t retx_eff; // Retransmission Effort
} hci_enh_accept_sync_con_cmd_t;

/// HCI reject connection request command structure
/*@TRACE*/
typedef struct hci_reject_con_req_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///Reason
    uint8_t reason;
} hci_reject_con_req_cmd_t;

/// HCI reject synchronous connection request command structure
/*@TRACE*/
typedef struct hci_reject_sync_con_req_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///Reason
    uint8_t reason;
} hci_reject_sync_con_req_cmd_t;

/// HCI link key request reply command structure
/*@TRACE*/
typedef struct hci_lk_req_reply_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///Key
    struct ltk key;
} hci_lk_req_reply_cmd_t;

/// HCI link key request reply command structure
/*@TRACE*/
typedef struct hci_pin_code_req_reply_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///Pin code length
    uint8_t pin_len;
    ///Key
    struct pin_code pin;
} hci_pin_code_req_reply_cmd_t;

/// HCI switch role command structure
/*@TRACE*/
typedef struct hci_switch_role_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///Read all flag
    uint8_t role;
} hci_switch_role_cmd_t;

/// HCI flow specification command parameters structure
/*@TRACE*/
typedef struct hci_flow_spec_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Flags
    uint8_t flags;
    ///Flow direction
    uint8_t flow_dir;
    ///Service type
    uint8_t serv_type;
    ///Token rate
    uint32_t tk_rate;
    ///Token buffer size
    uint32_t tk_buf_sz;
    ///Peak bandwidth
    uint32_t pk_bw;
    ///Access latency
    uint32_t acc_lat;
} hci_flow_spec_cmd_t;

/// HCI enhanced flush command parameters structure
/*@TRACE*/
typedef struct hci_enh_flush_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Packet Type
    uint8_t pkt_type;
} hci_enh_flush_cmd_t;

/// HCI command complete event structure for the read auto flush TO command
typedef struct hci_rd_auto_flush_to_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Flush timeout
    uint16_t flush_to;
} hci_rd_auto_flush_to_cmd_cmp_evt_t;

/// HCI write flush timeout command parameters structure
/*@TRACE*/
typedef struct hci_wr_auto_flush_to_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Flush timeout
    uint16_t flush_to;
} hci_wr_auto_flush_to_cmd_t;

/// HCI change connection packet type command parameters structure
/*@TRACE*/
typedef struct hci_chg_con_pkt_type_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Packet type
    uint16_t pkt_type;
} hci_chg_con_pkt_type_cmd_t;

/// HCI read link policy settings command parameters structure
/*@TRACE*/
typedef struct hci_rd_link_pol_stg_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Link policy
    uint16_t lnk_policy;
} hci_rd_link_pol_stg_cmd_cmp_evt_t;

/// HCI read link policy settings command parameters structure
/*@TRACE*/
typedef struct hci_wr_link_pol_stg_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Link policy
    uint16_t lnk_policy;
} hci_wr_link_pol_stg_cmd_t;

/// HCI sniff mode request command parameters structure
/*@TRACE*/
typedef struct hci_sniff_mode_cmd
{
    ///Connection handle
    uint16_t conhdl;
    /// Maximum interval (in slots)
    uint16_t max_int;
    /// Minimum interval (in slots)
    uint16_t min_int;
    /// Attempts (number of receive slots) (in slots)
    uint16_t attempt;
    /// Timeout (number of receive slots) (in slots)
    uint16_t timeout;
} hci_sniff_mode_cmd_t;

/// HCI sniff subrating mode request command parameters structure
/*@TRACE*/
typedef struct hci_sniff_sub_cmd
{
    ///Connection handle
    uint16_t conhdl;
    /// Maximum latency used to calculate the maximum sniff subrate that the remote device may use (in slots)
    uint16_t max_lat;
    /// Minimum base sniff subrate timeout that the remote device may use (in slots)
    uint16_t min_rem_to;
    /// Minimum base sniff subrate timeout that the local device may use (in slots)
    uint16_t min_loc_to;
} hci_sniff_sub_cmd_t;

/// HCI role discovery complete event parameters structure
/*@TRACE*/
typedef struct hci_role_discovery_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Role
    uint8_t role;

} hci_role_discovery_cmd_cmp_evt_t;

/// HCI read failed contact counter command parameters structure
/*@TRACE*/
typedef struct hci_rd_fail_contact_cnt_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Fail contact counter
    uint16_t fail_cnt;
} hci_rd_fail_contact_cnt_cmd_cmp_evt_t;

/// HCI read link quality complete event parameters structure
/*@TRACE*/
typedef struct hci_rd_link_qual_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Quality type
    uint8_t quality;
} hci_rd_link_qual_cmd_cmp_evt_t;

/// HCI read afh channel map complete event parameters structure
/*@TRACE*/
typedef struct hci_rd_afh_ch_map_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    /// AFH mode
    uint8_t afh_mode;
    /// AFH channel map
    struct bt_ch_map afh_map;
} hci_rd_afh_ch_map_cmd_cmp_evt_t;

/// HCI read lmp handle complete event parameters structure
/*@TRACE*/
typedef struct hci_rd_lmp_hdl_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///lmp handle
    uint8_t lmp_hdl;
    ///rsvd
    uint32_t rsvd;
} hci_rd_lmp_hdl_cmd_cmp_evt_t;

/// HCI read remote extended features command parameters structure
/*@TRACE*/
typedef struct hci_rd_rem_ext_feats_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///page number
    uint8_t pg_nb;
} hci_rd_rem_ext_feats_cmd_t;

/// HCI read encryption key size complete event parameters structure
/*@TRACE*/
typedef struct hci_rd_enc_key_size_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Key size
    uint8_t key_sz;
} hci_rd_enc_key_size_cmd_cmp_evt_t;

/// HCI read enhanced transmit power command parameters structure
/*@TRACE*/
typedef struct hci_rd_enh_tx_pwr_lvl_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Type
    uint8_t type;
} hci_rd_enh_tx_pwr_lvl_cmd_t;

/// HCI read enhanced transmit power complete event parameters structure
/*@TRACE*/
typedef struct hci_rd_enh_tx_pwr_lvl_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Transmit power GFSK
    int8_t pw_gfsk;
    ///Transmit power DQPSK
    int8_t pw_dqpsk;
    ///Transmit power 8DPSK
    int8_t pw_8dpsk;
} hci_rd_enh_tx_pwr_lvl_cmd_cmp_evt_t;


/*
 * HCI LINK CONTROL COMMANDS PARAMETERS
 ****************************************************************************************
 */

/// Format of the message of the Group: LINK_CONTROL_COMMANDS
/// HCI Inquiry command parameters structure
/*@TRACE*/
typedef struct hci_inq_cmd
{
    ///Lap
    struct lap lap;
    ///Inquiry Length in units of 1.28 s
    uint8_t inq_len;
    ///Number of response
    uint8_t nb_rsp;
} hci_inq_cmd_t;

/*@TRACE*/
typedef struct hci_per_inq_mode_cmd
{
    ///Maximum period length
    uint16_t max_per_len;
    ///Minimum period length
    uint16_t min_per_len;
    ///lap
    struct lap lap;
    ///Inquiry length in units of 1.28 s
    uint8_t inq_len;
    ///Number of response
    uint8_t nb_rsp;
} hci_per_inq_mode_cmd_t;

/*@TRACE*/
typedef struct hci_create_con_cmd
{
    /// BdAddr
    struct bd_addr bd_addr;
    /// Packet Type
    uint16_t pkt_type;
    /// Page Scan Repetition Mode
    uint8_t page_scan_rep_mode;
    /// Reserved
    uint8_t rsvd;
    /**
     * Clock Offset
     *
     * Bits 14-0 : Bits 16-2 of CLKNslave-CLK
     * Bit 15 : Clock_Offset_Valid_Flag
     *   Invalid Clock Offset = 0
     *   Valid Clock Offset = 1
     */
    uint16_t clk_off;
    /// Allow Switch
    uint8_t switch_en;
} hci_create_con_cmd_t;

/// HCI disconnect command structure
/*@TRACE*/
typedef struct hci_disconnect_cmd
{
    /// connection handle
    uint16_t conhdl;
    /// reason
    uint8_t reason;
} hci_disconnect_cmd_t;

/// HCI master link key command structure
/*@TRACE*/
typedef struct hci_master_lk_cmd
{
    ///Key flag
    uint8_t key_flag;
} hci_master_lk_cmd_t;

/// HCI authentication request command parameters structure
/*@TRACE*/
typedef struct hci_set_con_enc_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Encryption mode
    uint8_t enc_en;
} hci_set_con_enc_cmd_t;

/*@TRACE*/
typedef struct hci_rem_name_req_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///Page Scan Repetition Mode
    uint8_t page_scan_rep_mode;
    ///Reserved
    uint8_t rsvd;
    /**
     * Clock Offset
     *
     * Bits 14-0 : Bits 16-2 of CLKNslave-CLK
     * Bit 15 : Clock_Offset_Valid_Flag
     *   Invalid Clock Offset = 0
     *   Valid Clock Offset = 1
     */
    uint16_t clk_off;
} hci_rem_name_req_cmd_t;

/// HCI remote name request complete event structure
/*@TRACE*/
typedef struct hci_rem_name_req_cmp_evt
{
    /// Status
    uint8_t status;
    /// BD Addr
    struct bd_addr bd_addr;
    /// Name
    struct device_name name;
} hci_rem_name_req_cmp_evt_t;

/// HCI setup synchronous connection command structure
/*@TRACE*/
typedef struct hci_setup_sync_con_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Transmit bandwidth
    uint32_t tx_bw;
    ///Receive bandwidth
    uint32_t rx_bw;
    ///Max latency
    uint16_t max_lat;
    ///Voice setting
    uint16_t vx_set;
    ///Retransmission effort
    uint8_t retx_eff;
    ///Packet type
    uint16_t pkt_type;
} hci_setup_sync_con_cmd_t;

/// HCI setup synchronous connection command structure
/*@TRACE*/
typedef struct hci_enh_setup_sync_con_cmd
{
    uint16_t conhdl;               // Connection Handle
    uint32_t tx_bw;                // Transmit Bandwidth (in B/sec)
    uint32_t rx_bw;                // Receive Bandwidth (in B/sec)
    uint8_t tx_cod_fmt[5];        // Transmit Coding Format
    uint8_t rx_cod_fmt[5];        // Receive Coding Format
    uint16_t tx_cod_fr_sz;         // Transmit Codec Frame Size (in B)
    uint16_t rx_cod_fr_sz;         // Receive Codec Frame Size (in B)
    uint32_t in_bw;                // Input Bandwidth (in B/sec)
    uint32_t out_bw;               // Output Bandwidth (in B/sec)
    uint8_t in_cod_fmt[5];        // Input Coding Format
    uint8_t out_cod_fmt[5];       // Output Coding Format
    uint16_t in_cod_data_sz;       // Input Coded Data Size (in bits)
    uint16_t out_cod_data_sz;      // Output Coded Data Size (in bits)
    uint8_t in_data_fmt;          // Input PCM Data Format
    uint8_t out_data_fmt;         // Output PCM Data Format
    uint8_t in_msb_pos;           // Input PCM Sample Payload MSB Position (in bits)
    uint8_t out_msb_pos;          // Output PCM Sample Payload MSB Position (in bits)
    uint8_t in_data_path;         // Input Data Path
    uint8_t out_data_path;        // Output Data Path
    uint8_t in_tr_unit_sz;        // Input Transport Unit Size (in bits)
    uint8_t out_tr_unit_sz;       // Output Transport Unit Size (in bits)
    uint16_t max_lat;              // Max Latency (in ms)
    uint16_t packet_type;          // Packet Type
    uint8_t retx_eff;             // Retransmission Effort
} hci_enh_setup_sync_con_cmd_t;

/// HCI io capability request reply command structure
/*@TRACE*/
typedef struct hci_io_cap_req_reply_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///IO capability
    uint8_t io_capa;
    ///OOB data present
    uint8_t oob_data_pres;
    ///Authentication requirements
    uint8_t auth_req;

} hci_io_cap_req_reply_cmd_t;

/// HCI io capability request negative reply command structure
/*@TRACE*/
typedef struct hci_io_cap_req_neg_reply_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///Reason
    uint8_t reason;
} hci_io_cap_req_neg_reply_cmd_t;

/// HCI user pass key request reply command structure
/*@TRACE*/
typedef struct hci_user_passkey_req_reply_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///Numeric value
    uint32_t num_val;
} hci_user_passkey_req_reply_cmd_t;

/// HCI remote oob data request reply command structure
/*@TRACE*/
typedef struct hci_rem_oob_data_req_reply_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///hash part
    struct hash oob_c;
    ///random part
    struct randomizer oob_r;
} hci_rem_oob_data_req_reply_cmd_t;

/// HCI send key press notification command structure
/*@TRACE*/
typedef struct hci_send_keypress_notif_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///Notification type
    uint8_t notif_type;
} hci_send_keypress_notif_cmd_t;

/// HCI truncated page command structure
/*@TRACE*/
typedef struct hci_trunc_page_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
    /// Page Scan Repetition Mode
    uint8_t page_scan_rep_mode;
    /**
     * Clock Offset
     *
     * Bits 14-0 : Bits 16-2 of CLKNslave-CLK
     * Bit 15 : Clock_Offset_Valid_Flag
     *   Invalid Clock Offset = 0
     *   Valid Clock Offset = 1
     */
    uint16_t clk_off;
} hci_trunc_page_cmd_t;

/// HCI truncated page cancel command structure
/*@TRACE*/
typedef struct hci_trunc_page_can_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
} hci_trunc_page_can_cmd_t;

/// HCI set connectionless slave broadcast command structure
/*@TRACE*/
typedef struct hci_set_con_slv_bcst_cmd
{
    /// Enable
    uint8_t enable;
    /// LT_ADDR
    uint8_t lt_addr;
    /// LPO_Allowed
    uint8_t lpo_allowed;
    /// Packet_Type
    uint16_t packet_type;
    /// Interval_Min (in slots)
    uint16_t interval_min;
    /// Interval_Max (in slots)
    uint16_t interval_max;
    /// CSB_supervisionTO (in slots)
    uint16_t csb_supv_to;
} hci_set_con_slv_bcst_cmd_t;

/// HCI set connectionless slave broadcast command complete event structure
/*@TRACE*/
typedef struct hci_set_con_slv_bcst_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// LT_ADDR
    uint8_t lt_addr;
    /// Interval (in slots)
    uint16_t interval;
} hci_set_con_slv_bcst_cmd_cmp_evt_t;

/// HCI set connectionless slave broadcast receive command structure
/*@TRACE*/
typedef struct hci_set_con_slv_bcst_rec_cmd
{
    /// Enable
    uint8_t enable;
    /// BD_ADDR
    struct bd_addr  bd_addr;
    /// LT_ADDR
    uint8_t lt_addr;
    /// Interval (in slots)
    uint16_t interval;
    /// Clock_Offset (28 bits) - (CLKNslave - CLK) modulo 2^28
    uint32_t clock_offset;
    /// Next_Connectionless_Slave_Broadcast_Clock (28 bits)
    uint32_t next_csb_clock;
    /// CSB_supervisionTO (in slots)
    uint16_t csb_supv_to;
    /// Remote_Timing_Accuracy (in ppm)
    uint8_t remote_timing_accuracy;
    /// Skip
    uint8_t skip;
    /// Packet_Type
    uint16_t packet_type;
    /// AFH_Channel_Map
    struct bt_ch_map afh_ch_map;
} hci_set_con_slv_bcst_rec_cmd_t;

/// HCI set connectionless slave broadcast receive command complete event structure
/*@TRACE*/
typedef struct hci_set_con_slv_bcst_rec_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// BD_ADDR
    struct bd_addr  bd_addr;
    /// LT_ADDR
    uint8_t lt_addr;
} hci_set_con_slv_bcst_rec_cmd_cmp_evt_t;

/// HCI Receive Synchronization Train command structure
/*@TRACE*/
typedef struct hci_rec_sync_train_cmd
{
    /// BD_ADDR
    struct bd_addr  bd_addr;
    /// Synchronization_scanTO (in slots)
    uint16_t sync_scan_to;
    /// Sync_Scan_Window (in slots)
     uint16_t sync_scan_win;
    /// Sync_Scan_Interval (in slots)
     uint16_t sync_scan_int;
} hci_rec_sync_train_cmd_t;

/// HCI remote oob extended data request reply command structure
/*@TRACE*/
typedef struct hci_rem_oob_ext_data_req_reply_cmd
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///hash part
    struct hash oob_c_192;
    ///random part
    struct randomizer oob_r_192;
    ///hash part
    struct hash oob_c_256;
    ///random part
    struct randomizer oob_r_256;
} hci_rem_oob_ext_data_req_reply_cmd_t;

/*@TRACE*/
typedef struct hci_le_gen_dhkey_v1_cmd
{
    /// Remote P-256 public key
    uint8_t public_key[64];
} hci_le_gen_dhkey_v1_cmd_t;

/*@TRACE*/
typedef struct hci_le_gen_dhkey_v2_cmd
{
    /// Remote P-256 public key
    uint8_t public_key[64];

    /// Private key type (@see enum priv_key_type)
    uint8_t key_type;
} hci_le_gen_dhkey_v2_cmd_t;

/*@TRACE*/
typedef struct hci_le_mod_sleep_clk_acc_cmd
{
    /// Switch to more or less accurate clock (@see enum clk_acc_action)
    uint8_t action;
} hci_le_mod_sleep_clk_acc_cmd_t;


/*
 * HCI LINK POLICY COMMANDS PARAMETERS
 ****************************************************************************************
 */

/// HCI setup quality of service command structure
/*@TRACE*/
typedef struct hci_qos_setup_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Flags
    uint8_t flags;
    ///Service type
    uint8_t serv_type;
    ///Token rate
    uint32_t tok_rate;
    ///Peak bandwidth
    uint32_t pk_bw;
    ///Latency
    uint32_t lat;
    ///Delay variation
    uint32_t del_var;
} hci_qos_setup_cmd_t;

/// HCI command complete event structure for read default link policy command structure
/*@TRACE*/
typedef struct hci_rd_dft_link_pol_stg_cmd_cmp_evt
{
    ///Status of the command reception
    uint8_t status;
    ///Link policy
    uint16_t link_pol_stg;
} hci_rd_dft_link_pol_stg_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_dft_link_pol_stg_cmd
{
    ///Link policy
    uint16_t link_pol_stg;
} hci_wr_dft_link_pol_stg_cmd_t;

/*
 * HCI CONTROL & BASEBAND COMMANDS PARAMETERS
 ****************************************************************************************
 */

/// HCI set event mask command structure
/*@TRACE*/
typedef struct hci_set_evt_mask_cmd
{
    ///Event Mask
    struct evt_mask event_mask;
} hci_set_evt_mask_cmd_t;

/// HCI set event filter command structure
/*@TRACE*/
typedef struct hci_set_evt_filter_cmd
{
    /// Filter type
    uint8_t filter_type;

    /// Filters
    union hci_filter
    {
        uint8_t clear_all_filter_reserved;

        /// Inquiry Result Filter
        struct inq_res_filter
        {
            /// Filter Condition type
            uint8_t cond_type;

            /// Filter conditions
            union hci_inq_filter_cond
            {
                /// Reserved value (Inquiry Result Filter - condition type 0x00 has no condition)
                uint8_t cond_0_reserved;

                /// Inquiry Result Filter Condition - condition type 0x01
                struct inq_res_filter_cond_1
                {
                    /// Class_of_Device
                    struct devclass class_of_dev;
                    /// Class_of_Device_Mask
                    struct devclass class_of_dev_msk;
                } cond_1;

                /// Inquiry Result Filter Condition - condition type 0x02
                struct inq_res_filter_cond_2
                {
                    /// BD Address
                    struct bd_addr bd_addr;
                } cond_2;
            } cond;
        } inq_res;

        /// Connection Setup Filter
        struct con_set_filter
        {
            /// Filter Condition type
            uint8_t cond_type;

            /// Filter conditions
            union hci_con_filter_cond
            {
                /// Connection Setup Filter Condition - condition type 0x00
                struct con_set_filter_cond_0
                {
                    /// Auto_Accept_Flag
                    uint8_t auto_accept;
                } cond_0;

                /// Connection Setup Filter Condition - condition type 0x01
                struct con_set_filter_cond_1
                {
                    /// Class_of_Device
                    struct devclass class_of_dev;
                    /// Class_of_Device_Mask
                    struct devclass class_of_dev_msk;
                    /// Auto_Accept_Flag
                    uint8_t auto_accept;
                } cond_1;

                /// Connection Setup Filter Condition - condition type 0x02
                struct con_set_filter_cond_2
                {
                    /// BD Address
                    struct bd_addr bd_addr;
                    /// Auto_Accept_Flag
                    uint8_t auto_accept;
                } cond_2;
            } cond;

        } con_set;

    } filter;
} hci_set_evt_filter_cmd_t;

/// HCI command completed event structure for the flush command
/*@TRACE*/
typedef struct hci_flush_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
} hci_flush_cmd_cmp_evt_t;

/// HCI command complete event structure for the Read pin type command
/*@TRACE*/
typedef struct hci_rd_pin_type_cmd_cmp_evt
{
    ///Status of the command
    uint8_t status;
    ///PIN type
    uint8_t pin_type;
} hci_rd_pin_type_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_pin_type_cmd
{
    ///PIN type
    uint8_t pin_type;
} hci_wr_pin_type_cmd_t;

/*@TRACE*/
typedef struct hci_rd_stored_lk_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///Read all flag
    uint8_t rd_all_flag;
} hci_rd_stored_lk_cmd_t;

/// HCI command complete event structure for read stored link key command
/*@TRACE*/
typedef struct hci_rd_stored_lk_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Maximum number of key
    uint16_t num_key_max;
    ///Read number of key
    uint16_t num_key_rd;
} hci_rd_stored_lk_cmd_cmp_evt_t;

#if BT_EMB_PRESENT
/*@TRACE*/
typedef struct hci_wr_stored_lk_cmd
{
    /// Number of key to write
    uint8_t num_key_wr;

    /// BD Address + Key table
    struct bd_addr_plus_key link_keys[HCI_MAX_CMD_PARAM_SIZE / sizeof(struct bd_addr_plus_key)];
} hci_wr_stored_lk_cmd_t;
#endif //BT_EMB_PRESENT

/// HCI command complete event structure for write stored link key command
/*@TRACE*/
typedef struct hci_wr_stored_lk_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///number of key written
    uint8_t num_key_wr;
} hci_wr_stored_lk_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_del_stored_lk_cmd
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///Delete all flag
    uint8_t del_all_flag;
} hci_del_stored_lk_cmd_t;

/// HCI command complete event structure for delete stored link key command
/*@TRACE*/
typedef struct hci_del_stored_lk_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Read number of key
    uint16_t num_key_del;
} hci_del_stored_lk_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_local_name_cmd
{
    ///Name
    struct device_name name;
} hci_wr_local_name_cmd_t;

/// HCI command complete event structure for the read local name command
/*@TRACE*/
typedef struct hci_rd_local_name_cmd_cmp_evt
{
    ///Status of the command
    uint8_t status;
    ///Name
    uint8_t name[BD_NAME_SIZE];
} hci_rd_local_name_cmd_cmp_evt_t;

/// HCI command complete event structure for the Read connection accept to command
/*@TRACE*/
typedef struct hci_rd_con_accept_to_cmd_cmp_evt
{
    ///Status of the command
    uint8_t status;
    /// Connection accept timeout (in slots)
    uint16_t con_acc_to;
} hci_rd_con_accept_to_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_con_accept_to_cmd
{
    /// Connection accept timeout (in slots)
    uint16_t con_acc_to;
} hci_wr_con_accept_to_cmd_t;

/// HCI command complete event structure for the Read page to command
/*@TRACE*/
typedef struct hci_rd_page_to_cmd_cmp_evt
{
    ///Status of the command
    uint8_t status;
    /// Page timeout (in slots)
    uint16_t page_to;
} hci_rd_page_to_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_page_to_cmd
{
    /// Page timeout (in slots)
    uint16_t page_to;
} hci_wr_page_to_cmd_t;

/// HCI command complete event structure for the Read scan enable command
/*@TRACE*/
typedef struct hci_rd_scan_en_cmd_cmp_evt
{
    ///Status of the command
    uint8_t status;
    ///Status of the scan enable
    uint8_t scan_en;
} hci_rd_scan_en_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_scan_en_cmd
{
    ///Status of the scan enable
    uint8_t scan_en;
} hci_wr_scan_en_cmd_t;

/// HCI command complete event structure for the Read scan activity command
/*@TRACE*/
typedef struct hci_rd_page_scan_act_cmd_cmp_evt
{
    ///Status of the command
    uint8_t status;
    /// Page scan interval (in slots)
    uint16_t page_scan_intv;
    /// Page scan window (in slots)
    uint16_t page_scan_win;
} hci_rd_page_scan_act_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_page_scan_act_cmd
{
    /// Page scan interval (in slots)
    uint16_t page_scan_intv;
    /// Page scan window (in slots)
    uint16_t page_scan_win;
} hci_wr_page_scan_act_cmd_t;

/// HCI command complete event structure for the Read inquiry scan activity command
/*@TRACE*/
typedef struct hci_rd_inq_scan_act_cmd_cmp_evt
{
    /// Status of the command
    uint8_t status;
    /// Inquiry scan interval (in slots)
    uint16_t inq_scan_intv;
    /// Inquiry scan window (in slots)
    uint16_t inq_scan_win;
} hci_rd_inq_scan_act_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_inq_scan_act_cmd
{
    /// Inquiry scan interval (in slots)
    uint16_t inq_scan_intv;
    /// Inquiry scan window (in slots)
    uint16_t inq_scan_win;
} hci_wr_inq_scan_act_cmd_t;

/// HCI command complete event structure for the Read authentication command
/*@TRACE*/
typedef struct hci_rd_auth_en_cmd_cmp_evt
{
    ///Status of the command
    uint8_t status;
    ///Value of the authentication
    uint8_t auth_en;
} hci_rd_auth_en_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_auth_en_cmd
{
    ///Value of the authentication
    uint8_t auth_en;
} hci_wr_auth_en_cmd_t;

/// HCI command complete event structure for the read class of device command
/*@TRACE*/
typedef struct hci_rd_class_of_dev_cmd_cmp_evt
{
    ///Status of the command
    uint8_t status;
    ///Class of device
    struct devclass class_of_dev;
} hci_rd_class_of_dev_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_class_of_dev_cmd
{
    ///Class of device
    struct devclass class_of_dev;
} hci_wr_class_of_dev_cmd_t;

/// HCI read voice settings complete event
/*@TRACE*/
typedef struct hci_rd_voice_stg_cmd_cmp_evt
{
    ///Status of the command reception
    uint8_t status;
    /// Voice setting
    uint16_t voice_stg;
} hci_rd_voice_stg_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_voice_stg_cmd
{
    /// voice setting
    uint16_t voice_stg;
} hci_wr_voice_stg_cmd_t;

/// HCI command complete event structure for read number of broadcast retrans command
/*@TRACE*/
typedef struct hci_rd_nb_bdcst_retx_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Read number of broadcast retransmission
    uint8_t num_bcst_ret;
} hci_rd_nb_bdcst_retx_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_nb_bdcst_retx_cmd
{
    ///Read number of broadcast retransmission
    uint8_t num_bcst_ret;
} hci_wr_nb_bdcst_retx_cmd_t;

/// HCI command complete event structure for the Read Synchronous Flow Control command
/*@TRACE*/
typedef struct hci_rd_sync_flow_ctrl_en_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Synchronous flow control enable
    uint8_t sync_flow_ctrl_en;
} hci_rd_sync_flow_ctrl_en_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_sync_flow_ctrl_en_cmd
{
    /// Synchronous Flow Control enable
    uint8_t sync_flow_ctrl_en;
} hci_wr_sync_flow_ctrl_en_cmd_t;

///HCI set controller to host flow control command
/*@TRACE*/
typedef struct hci_set_ctrl_to_host_flow_ctrl_cmd
{
    ///Flow control enable for controller
    uint8_t flow_cntl;
} hci_set_ctrl_to_host_flow_ctrl_cmd_t;

///HCI host buffer size command
/*@TRACE*/
typedef struct hci_host_buf_size_cmd
{
    ///Host ACL packet length
    uint16_t acl_pkt_len;
    ///Host synchronous packet length
    uint8_t sync_pkt_len;
    ///Host Total number of ACL packets allowed
    uint16_t nb_acl_pkts;
    ///Host total number of synchronous packets allowed
    uint16_t nb_sync_pkts;
} hci_host_buf_size_cmd_t;

/// Number of completed packet for a connection
/*@TRACE*/
typedef struct hci_nb_cmp_pkt
{
    /// connection handles
    uint16_t hdl;
    /// Number of completed packets values for connection handles.
    uint16_t nb_comp_pkt;
} hci_nb_cmp_pkt_t;

///HCI host number of completed packets command
/*@TRACE*/
typedef struct hci_host_nb_cmp_pkts_cmd
{
    ///Number of handles for which the completed packets number is given
    uint8_t nb_of_hdl;
    /// Array of number of completed packets per connection
    struct hci_nb_cmp_pkt con[MAX_NB_CMP_PKTS_CON];
} hci_host_nb_cmp_pkts_cmd_t;

/// HCI read link supervision timeout command parameters structure
typedef struct hci_rd_link_supv_to_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Link supervision timeout
    uint16_t lsto_val;
} hci_rd_link_supv_to_cmd_cmp_evt_t;

/// HCI write link supervision timeout command parameters structure
/*@TRACE*/
typedef struct hci_wr_link_supv_to_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Link supervision timeout
    uint16_t lsto_val;
} hci_wr_link_supv_to_cmd_t;

/// HCI command complete event structure for the nb of supported IAC command
/*@TRACE*/
typedef struct hci_rd_nb_supp_iac_cmd_cmp_evt
{
    ///Status of the command
    uint8_t status;
    ///nb_of iac
    uint8_t nb_iac;
} hci_rd_nb_supp_iac_cmd_cmp_evt_t;

/// HCI command complete event structure for read current IAC LAP command
/*@TRACE*/
typedef struct hci_rd_curr_iac_lap_cmd_cmp_evt
{
    ///Status of the command
    uint8_t status;
    ///nb of current iac
    uint8_t nb_curr_iac;
    ///lap
    struct lap iac_lap;
} hci_rd_curr_iac_lap_cmd_cmp_evt_t;

/// HCI write current IAC LAP command structure
/*@TRACE*/
typedef struct hci_wr_curr_iac_lap_cmd
{
    /// Number of current iac laps
    uint8_t nb_curr_iac;
    ///lap
    struct lap iac_lap[(HCI_MAX_CMD_PARAM_SIZE / BD_ADDR_LAP_LEN) - 1];
} hci_wr_curr_iac_lap_cmd_t;

/*@TRACE*/
typedef struct hci_set_afh_host_ch_class_cmd
{
    ///AFH channel map
    struct bt_ch_map afh_ch;
} hci_set_afh_host_ch_class_cmd_t;

/// HCI command complete event structure for write inquiry scan type command structure
/*@TRACE*/
typedef struct hci_rd_inq_scan_type_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    /// Inquiry scan type
    uint8_t inq_scan_type;
} hci_rd_inq_scan_type_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_inq_scan_type_cmd
{
    /// Inquiry scan type
    uint8_t inq_scan_type;
} hci_wr_inq_scan_type_cmd_t;

/// HCI command complete event structure for read inquiry mode command structure
/*@TRACE*/
typedef struct hci_rd_inq_mode_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    /// Inquiry mode
    uint8_t inq_mode;
} hci_rd_inq_mode_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_inq_mode_cmd
{
    /// Inquiry mode
    uint8_t inq_mode;
} hci_wr_inq_mode_cmd_t;

/// HCI command complete event structure for write page scan type command structure
/*@TRACE*/
typedef struct hci_rd_page_scan_type_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    /// Page scan type
    uint8_t page_scan_type;
} hci_rd_page_scan_type_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_page_scan_type_cmd
{
    /// Page scan type
    uint8_t page_scan_type;
} hci_wr_page_scan_type_cmd_t;

/// HCI command complete event structure for read assessment mode command structure
/*@TRACE*/
typedef struct hci_rd_afh_ch_assess_mode_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///AFH channel assessment mode
    uint8_t afh_ch_ass_mode;
} hci_rd_afh_ch_assess_mode_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_afh_ch_assess_mode_cmd
{
    ///AFH channel assessment mode
    uint8_t afh_ch_ass_mode;
} hci_wr_afh_ch_assess_mode_cmd_t;

/// HCI command complete event structure for remote name request cancel command
/*@TRACE*/
typedef struct hci_rd_ext_inq_rsp_cmd_cmp_evt
{
    ///status
    uint8_t status;
    ///FEC required
    uint8_t fec_req;
    ///Extended inquiry response
    struct eir eir;
} hci_rd_ext_inq_rsp_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_ext_inq_rsp_cmd
{
    ///FEC required
    uint8_t fec_req;
    ///Extended inquiry response
    struct eir eir;
} hci_wr_ext_inq_rsp_cmd_t;

/// HCI command complete event structure for remote name request cancel command
/*@TRACE*/
typedef struct hci_rd_sp_mode_cmd_cmp_evt
{
    ///status
    uint8_t status;
    ///Simple pairing mode
    uint8_t sp_mode;
} hci_rd_sp_mode_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_sp_mode_cmd
{
    ///Simple pairing mode
    uint8_t sp_mode;
} hci_wr_sp_mode_cmd_t;

/// HCI command complete event structure for read oob data command
/*@TRACE*/
typedef struct hci_rd_loc_oob_data_cmd_cmp_evt
{
    ///status
    uint8_t status;
    ///hash part
    struct hash oob_c;
    ///random part
    struct randomizer oob_r;
} hci_rd_loc_oob_data_cmd_cmp_evt_t;

/// HCI command complete event structure for read inquiry response transmit power command
/*@TRACE*/
typedef struct hci_rd_inq_rsp_tx_pwr_lvl_cmd_cmp_evt
{
    ///status
    uint8_t status;
    ///TX power
    uint8_t tx_pwr;
} hci_rd_inq_rsp_tx_pwr_lvl_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_inq_tx_pwr_lvl_cmd
{
    ///TX power
    int8_t tx_pwr;
} hci_wr_inq_tx_pwr_lvl_cmd_t;

#if (VOICE_OVER_HCI)
/// HCI command complete event structure for read erroneous data reporting command
/*@TRACE*/
typedef struct hci_rd_dft_err_data_rep_cmd_cmp_evt
{
    ///status
    uint8_t status;
    ///Erroneous data reporting
    uint8_t err_data_rep;
} hci_rd_dft_err_data_rep_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_dft_err_data_rep_cmd
{
    ///Erroneous data reporting
    uint8_t err_data_rep;
} hci_wr_dft_err_data_rep_cmd_t;
#endif // (VOICE_OVER_HCI)

/// HCI read LE Host Supported complete event
/*@TRACE*/
typedef struct hci_rd_le_host_supp_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    ///LE_Supported_Host
    uint8_t le_supported_host;
    ///Unused
    uint8_t unused;
} hci_rd_le_host_supp_cmd_cmp_evt_t;

/// HCI write LE Host Supported command
/*@TRACE*/
typedef struct hci_wr_le_host_supp_cmd
{
    ///LE_Supported_Host
    uint8_t le_supported_host;
    ///Unused
    uint8_t unused;
} hci_wr_le_host_supp_cmd_t;

/// HCI Set MWS Channel Parameters command
/*@TRACE*/
typedef struct hci_set_mws_channel_params_cmd
{
    ///MWS_Channel_Enable
    uint8_t mws_channel_enable;
    ///MWS_RX_Center_Frequency
    uint16_t mws_rx_center_frequency;
    ///MWS_TX_Center_Frequency
    uint16_t mws_tx_center_frequency;
    ///MWS_RX_Channel_Bandwidth
    uint16_t mws_rx_channel_bandwidth;
    ///MWS_TX_Channel_Bandwidth
    uint16_t mws_tx_channel_bandwidth;
    ///MWS_Channel_Type
    uint8_t mws_channel_type;
} hci_set_mws_channel_params_cmd_t;

/// HCI Set External Frame Configuration command
/*@TRACE*/
typedef struct hci_set_external_frame_config_cmd
{
    /// Ext_Frame_Duration
    uint16_t ext_fr_duration;
    /// Ext_Frame_Sync_Assert_Offset
    int16_t ext_fr_sync_assert_offset;
    /// Ext_Frame_Sync_Assert_Jitter
    uint16_t ext_fr_sync_assert_jitter;
    /// Ext_Frame_Num_Periods
    uint8_t ext_fr_num_periods;
    /// Period Durations & Types
    struct ext_fr_period period[1/*__ARRAY_EMPTY*/];
} hci_set_external_frame_config_cmd_t;

/// HCI Set MWS Signaling command
/*@TRACE*/
typedef struct hci_set_mws_signaling_cmd
{
    ///MWS_RX_Assert_Offset
    int16_t mws_rx_assert_offset;
    ///MWS_RX_Assert_Jitter
    uint16_t mws_rx_assert_jitter;
    ///MWS_RX_Deassert_Offset
    int16_t mws_rx_deassert_offset;
    ///MWS_RX_Deassert_Jitter
    uint16_t mws_rx_deassert_jitter;
    ///MWS_TX_Assert_Offset
    int16_t mws_tx_assert_offset;
    ///MWS_TX_Assert_Jitter
    uint16_t mws_tx_assert_jitter;
    ///MWS_TX_Deassert_Offset
    int16_t mws_tx_deassert_offset;
    ///MWS_TX_Deassert_Jitter
    uint16_t mws_tx_deassert_jitter;
    ///MWS_Pattern_Assert_Offset
    int16_t mws_pattern_assert_offset;
    ///MWS_Pattern_Assert_Jitter
    uint16_t mws_pattern_assert_jitter;
    ///MWS_Inactivity_Duration_Assert_Offset
    int16_t mws_inactivity_duration_assert_offset;
    ///MWS_Inactivity_Duration_Assert_Jitter
    uint16_t mws_inactivity_duration_assert_jitter;
    ///MWS_Scan_Frequency_Assert_Offset
    int16_t mws_scan_frequency_assert_offset;
    ///MWS_Scan_Frequency_Assert_Jitter
    uint16_t mws_scan_frequency_assert_jitter;
    ///MWS_Priority_Assert_Offset_Request
    uint16_t mws_priority_assert_offset_request;
} hci_set_mws_signaling_cmd_t;

/// HCI Set MWS Signaling command complete event
/*@TRACE*/
typedef struct hci_set_mws_signaling_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    ///Bluetooth_Rx_Priority_Assert_Offset
    int16_t bt_rx_prio_assert_offset;
    ///Bluetooth_Rx_Priority_Assert_Jitter
    uint16_t bt_rx_prio_assert_jitter;
    ///Bluetooth_Rx_Priority_Deassert_Offset
    int16_t bt_rx_prio_deassert_offset;
    ///Bluetooth_Rx_Priority_Deassert_Jitter
    uint16_t bt_rx_prio_deassert_jitter;
    ///802_Rx_Priority_Assert_Offset
    int16_t _802_rx_prio_assert_offset;
    ///802_Rx_Priority_Assert_Jitter
    uint16_t _802_rx_prio_assert_jitter;
    ///802_Rx_Priority_Deassert_Offset
    int16_t _802_rx_prio_deasssert_offset;
    ///802_Rx_Priority_Deassert_Jitter
    uint16_t _802_rx_prio_deassert_jitter;
    ///Bluetooth_Tx_On_Assert_Offset
    int16_t bt_tx_on_assert_offset;
    ///Bluetooth_Tx_On_Assert_Jitter
    uint16_t bt_tx_on_assert_jitter;
    ///Bluetooth_Tx_On_Deassert_Offset
    int16_t bt_tx_on_deassert_offset;
    ///Bluetooth_Tx_On_Deassert_Jitter
    uint16_t bt_tx_on_deassert_jitter;
    ///802_Tx_On_Assert_Offset
    int16_t _802_tx_on_assert_offset;
    ///802_Tx_On_Assert_Jitter
    uint16_t _802_tx_on_assert_jitter;
    ///802_Tx_On_Deassert_Offset
    int16_t _802_tx_on_deassert_offset;
    ///802_Tx_On_Deassert_Jitter
    uint16_t _802_tx_on_deassert_jitter;
} hci_set_mws_signaling_cmd_cmp_evt_t;

/// HCI Set MWS Transport Layer command
/*@TRACE*/
typedef struct hci_set_mws_transport_layer_cmd
{
    ///Transport_Layer
    uint8_t transport_layer;
    ///To_MWS_Baud_Rate
    uint32_t to_mws_baud_rate;
    ///From_MWS_Baud_Rate
    uint32_t from_mws_baud_rate;
} hci_set_mws_transport_layer_cmd_t;

/// HCI Set MWS Scan Frequency Table command
/*@TRACE*/
typedef struct hci_set_mws_scan_freq_table_cmd
{
    ///Num_Scan_Frequencies
    uint8_t num_scan_frequencies;
    ///Scan_Frequencys Low & High
    struct mws_scan_freq scan_freq[1/*__ARRAY_EMPTY*/];
} hci_set_mws_scan_freq_table_cmd_t;

/// HCI Set MWS Pattern Configuration command
/*@TRACE*/
typedef struct hci_set_mws_pattern_config_cmd
{
    ///MWS_PATTERN_Index
    uint8_t mws_pattern_index;
    ///MWS_PATTERN_NumIntervals
    uint8_t num_intervals;
    ///MWS_PATTERN_Interval Duration & Type
    struct mws_pattern_intv intv[1/*__ARRAY_EMPTY*/];
} hci_set_mws_pattern_config_cmd_t;

/// Hci Get MWS Transport Layer Configuration command complete event
/*@TRACE*/
typedef struct hci_get_mws_transport_layer_config_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    ///Num_Transports
    uint8_t num_transports;
    ///Transport_Layers
    struct mws_transport tran[1/*__ARRAY_EMPTY*/];
} hci_get_mws_transport_layer_config_cmd_cmp_evt_t;

/// HCI read Secure Connections Host Support complete event
/*@TRACE*/
typedef struct hci_rd_sec_con_host_supp_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    /// Secure Connections Host Support
    uint8_t sec_con_host_supp;
} hci_rd_sec_con_host_supp_cmd_cmp_evt_t;

/// HCI write Secure Connections Host Support command
/*@TRACE*/
typedef struct hci_wr_sec_con_host_supp_cmd
{
    /// Secure Connections Host Support
    uint8_t sec_con_host_supp;
} hci_wr_sec_con_host_supp_cmd_t;

/// HCI write Secure Connections Test Mode command
typedef struct hci_wr_sec_con_test_mode_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// DM1 ACL-U mode
    uint8_t dm1_acl_u_mode;
    /// eSCO loopback mode
    uint8_t esco_loopback_mode;
} hci_wr_sec_con_test_mode_cmd_t;

/// HCI write Secure Connections Test Mode complete event
typedef struct hci_wr_sec_con_test_mode_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    /// Connection handle
    uint16_t conhdl;
} hci_wr_sec_con_test_mode_cmd_cmp_evt_t;

/// HCI Set Reserved LT_ADDR command
/*@TRACE*/
typedef struct hci_set_res_lt_addr_cmd
{
    /// LT_ADDR
    uint8_t lt_addr;
} hci_set_res_lt_addr_cmd_t;

/// HCI Set Reserved LT_ADDR command complete event
/*@TRACE*/
typedef struct hci_set_res_lt_addr_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// LT_ADDR
    uint8_t lt_addr;
} hci_set_res_lt_addr_cmd_cmp_evt_t;

/// HCI Delete Reserved LT_ADDR command
/*@TRACE*/
typedef struct hci_del_res_lt_addr_cmd
{
    /// LT_ADDR
    uint8_t lt_addr;
} hci_del_res_lt_addr_cmd_t;

/// HCI Delete Reserved LT_ADDR command complete event
/*@TRACE*/
typedef struct hci_del_res_lt_addr_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// LT_ADDR
    uint8_t lt_addr;
} hci_del_res_lt_addr_cmd_cmp_evt_t;

/// HCI Set Connectionless Slave Broadcast Data command
/*@TRACE*/
typedef struct hci_set_con_slv_bcst_data_cmd
{
    /// LT_ADDR
    uint8_t lt_addr;
    /// Fragment
    uint8_t fragment;
    /// Data_Length (in bytes)
    uint8_t data_length;
    /// Data
    uint8_t data[__ARRAY_EMPTY];
} hci_set_con_slv_bcst_data_cmd_t;

/// HCI Set Connectionless Slave Broadcast Data command complete event
/*@TRACE*/
typedef struct hci_set_con_slv_bcst_data_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// LT_ADDR
    uint8_t lt_addr;
} hci_set_con_slv_bcst_data_cmd_cmp_evt_t;

/// HCI Read Synchronization Train Parameters command complete event
/*@TRACE*/
typedef struct hci_rd_sync_train_param_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Sync_Train_Interval (in slots)
    uint16_t sync_train_int;
    /// synchronization_trainTO (in slots)
    uint32_t sync_train_to;
    /// Service_Data
    uint8_t service_data;
} hci_rd_sync_train_param_cmd_cmp_evt_t;

/// HCI Write Synchronization Train Parameters command
/*@TRACE*/
typedef struct hci_wr_sync_train_param_cmd
{
    /// Interval_Min (in slots)
    uint16_t int_min;
    /// Interval_Max (in slots)
    uint16_t int_max;
    /// synchronization_trainTO (in slots)
    uint32_t sync_train_to;
    /// Service_Data
    uint8_t service_data;
} hci_wr_sync_train_param_cmd_t;

/// HCI Write Synchronization Train Parameters command complete event
/*@TRACE*/
typedef struct hci_wr_sync_train_param_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Sync_Train_Interval (in slots)
    uint16_t sync_train_int;
} hci_wr_sync_train_param_cmd_cmp_evt_t;

// HCI Synchronization Train Complete event
typedef struct hci_sync_train_cmp_evt
{
    /// Status
    uint8_t status;
} hci_sync_train_cmp_evt_t;

/// HCI read authenticated payload timeout command
/*@TRACE*/
typedef struct hci_rd_auth_payl_to_cmd
{
    ///Connection handle
    uint16_t conhdl;
} hci_rd_auth_payl_to_cmd_t;

/// HCI command complete event structure for the Read Authenticated Payload Timeout Command
/*@TRACE*/
typedef struct hci_rd_auth_payl_to_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Authenticated payload timeout
    uint16_t auth_payl_to;
} hci_rd_auth_payl_to_cmd_cmp_evt_t;

/// HCI command complete event structure for read oob extended data command
/*@TRACE*/
typedef struct hci_rd_loc_oob_ext_data_cmd_cmp_evt
{
    ///status
    uint8_t status;
    ///hash part
    struct hash oob_c_192;
    ///random part
    struct randomizer oob_r_192;
    ///hash part
    struct hash oob_c_256;
    ///random part
    struct randomizer oob_r_256;
} hci_rd_loc_oob_ext_data_cmd_cmp_evt_t;

/// HCI read Extended Page Timeout CC event
/*@TRACE*/
typedef struct hci_rd_ext_page_to_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /**
     * Extended Page Timeout measured in Number of Baseband slots. Interval Length = N * 0.625 msec (1 Baseband slot)
     * Range for N: 0x0000 (default) - 0xFFFF
     * Time Range: 0 - 40.9 Seconds
     */
    uint16_t ext_page_to;
} hci_rd_ext_page_to_cmd_cmp_evt_t;

/// HCI write Extended Page Timeout
/*@TRACE*/
typedef struct hci_wr_ext_page_to_cmd
{
    /**
     * Extended Page Timeout measured in Number of Baseband slots. Interval Length = N * 0.625 msec (1 Baseband slot)
     * Range for N: 0x0000 (default) - 0xFFFF
     * Time Range: 0 - 40.9 Seconds
     */
    uint16_t ext_page_to;
} hci_wr_ext_page_to_cmd_t;

/// HCI read Extended Inquiry Length CC event
/*@TRACE*/
typedef struct hci_rd_ext_inq_len_cmd_cmp_evt
{
    /// Status
    uint8_t  status;
    /// Extended Inquiry Length
    uint16_t ext_inq_len;
} hci_rd_ext_inq_len_cmd_cmp_evt_t;

/// HCI write Extended Inquiry Length
/*@TRACE*/
typedef struct hci_wr_ext_inq_len_cmd
{
    /// Extended Inquiry Length
    uint16_t ext_inq_len;
} hci_wr_ext_inq_len_cmd_t;

/// HCI Set Ecosystem Base Interval command
/*@TRACE*/
typedef struct hci_set_eco_base_intv_cmd
{
    /// Interval
    uint16_t interval;
} hci_set_eco_base_intv_cmd_t;

/// HCI Configure Data Path command
/*@TRACE*/
typedef struct hci_config_data_path_cmd
{
    /// Data Path Direction
    uint8_t data_path_direction;
    /// Data Path ID
    uint8_t data_path_id;
    /// Vendor Specific Config Length (in bytes)
    uint8_t vendor_specific_cfg_len;
    /// Vendor Specific Config
    uint8_t vendor_specific_cfg[__ARRAY_EMPTY];
} hci_config_data_path_cmd_t;

#if (BT_53)
/// HCI Set Min Encryption Key Size command
/*@TRACE*/
typedef struct hci_set_min_enc_key_size_cmd
{
    /// Min Encryption Key Size
    uint8_t min_enc_key_size;
} hci_set_min_enc_key_size_cmd_t;

/// HCI encryption change V2 event structure
/*@TRACE*/
typedef struct hci_enc_chg_v2_evt
{
    /// Status for command reception
    uint8_t status;
    /// Connection handle
    uint16_t conhdl;
    /// Encryption enabled information
    uint8_t enc_stat;
    /// Encryption key size
    uint8_t enc_key_size;
} hci_enc_chg_v2_evt_t;
#endif // (BT_53)

/*
 * HCI INFORMATIONAL PARAMETERS COMMANDS PARAMETERS
 ****************************************************************************************
 */

///HCI command complete event structure for read local version information
/*@TRACE*/
typedef struct hci_rd_local_ver_info_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///HCI version number
    uint8_t hci_ver;
    ///HCI revision number
    uint16_t hci_rev;
    ///LMP version
    uint8_t lmp_ver;
    ///manufacturer name
    uint16_t manuf_name;
    ///LMP Subversion
    uint16_t lmp_subver;
} hci_rd_local_ver_info_cmd_cmp_evt_t;

///HCI command complete event structure for read local supported commands
/*@TRACE*/
typedef struct hci_rd_local_supp_cmds_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Supported Commands structure
    struct supp_cmds local_cmds;
} hci_rd_local_supp_cmds_cmd_cmp_evt_t;

/// HCI command complete event structure for read local supported features command
/*@TRACE*/
typedef struct hci_rd_local_supp_feats_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Local  supported features
    struct features feats;
} hci_rd_local_supp_feats_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_rd_local_ext_feats_cmd
{
    ///Page number
    uint8_t page_nb;
} hci_rd_local_ext_feats_cmd_t;

/// HCI command complete event structure for read local extended features command
/*@TRACE*/
typedef struct hci_rd_local_ext_feats_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Page number
    uint8_t page_nb;
    ///Maximum page number
    uint8_t page_nb_max;
    ///Extended LMP features
    struct features ext_feats;
} hci_rd_local_ext_feats_cmd_cmp_evt_t;

///HCI command complete event structure for the Read Buffer Size Command
/*@TRACE*/
typedef struct hci_rd_buf_size_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///ACL data packet length controller can receive from host
    uint16_t hc_data_pk_len;
    ///Synchronous data packet length controller can receive from host
    uint8_t hc_sync_pk_len;
    ///Total number of ACL data packets controller can receive from host
    uint16_t hc_tot_nb_data_pkts;
    ///Total number of synchronous data packets controller can receive from host
    uint16_t hc_tot_nb_sync_pkts;
} hci_rd_buf_size_cmd_cmp_evt_t;

///HCI command complete event structure for read bd address
/*@TRACE*/
typedef struct hci_rd_bd_addr_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///BD address
    struct bd_addr local_addr;
} hci_rd_bd_addr_cmd_cmp_evt_t;

/// HCI command complete event structure for read local supported codecs
/*@TRACE*/
typedef struct hci_rd_local_supp_codecs_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    /// Array that contains list of codec
    /// uint8_t nb_supp_std_codecs
    /// uint8_t std_codec_id[nb_supp_codecs]
    /// uint8_t nb_supp_vs_codecs
    /// uint8_t vs_codec_id[4][nb_supp_vs_codecs]
    uint8_t codec_info[__ARRAY_EMPTY];
} hci_rd_local_supp_codecs_cmd_cmp_evt_t;

/// HCI command complete event structure for read local supported codecs v2
/*@TRACE*/
typedef struct hci_rd_local_supp_codecs_v2_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;

    /// Array that contains list of codec
    /// uint8_t nb_supp_std_codecs
    /// uint8_t std_codec_id[nb_supp_codecs]
    /// uint8_t std_codec_transport_id[nb_supp_codecs]
    /// uint8_t nb_supp_vs_codecs
    /// uint8_t vs_codec_id[4][nb_supp_vs_codecs]
    /// uint8_t vs_codec_transport_id[nb_supp_vs_codecs]
    uint8_t codec_info[__ARRAY_EMPTY];
} hci_rd_local_supp_codecs_v2_cmd_cmp_evt_t;

/// HCI command complete event structure for read local simple pairing options
typedef struct hci_rd_local_sp_opt_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    /// Simple Pairing options (bit 0: Remote public key validation)
    uint8_t sp_opt;
    /// Maximum Encryption Key Size (in octets)
    uint8_t max_enc_key_size;
} hci_rd_local_sp_opt_cmd_cmp_evt_t;

/// HCI Read Local Supported Codec Capabilities command
typedef struct hci_rd_local_supp_codec_cap_cmd
{
    /// Codec ID
    uint8_t codec_id[CODEC_ID_LEN];
    /// Logical Transport Type
    uint8_t logical_transport_type;
    /// Direction
    uint8_t direction;
} hci_rd_local_supp_codec_cap_cmd_t;

/// HCI command complete event structure for Read Local Supported Codec Capabilities command
typedef struct hci_rd_local_supp_codec_cap_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    /// Num Codec Capabilities
    uint8_t num_codec_capa;
    /// uint8_t codec_capa_len[i]
    /// uint8_t codec_capa[i][codec_capa_len[i]]
    uint8_t capabilities[MAX_CODEC_CAPA_LENGTH];
} hci_rd_local_supp_codec_cap_cmd_cmp_evt_t;

/// HCI Read Local Supported Controller Delay command
typedef struct hci_rd_local_supp_ctrl_delay_cmd
{
    /// Codec ID
    uint8_t codec_id[CODEC_ID_LEN];
    /// Logical Transport Type
    uint8_t logical_transport_type;
    /// Direction
    uint8_t direction;
    /// Codec Configuration Length
    uint8_t codec_cfg_len;
    /// Codec Configuration
    uint8_t codec_cfg[__ARRAY_EMPTY];
} hci_rd_local_supp_ctrl_delay_cmd_t;

/// HCI command complete event structure for Read Local Supported Controller Delay command
typedef struct hci_rd_local_supp_ctrl_delay_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    /// Min Controller Delay - 24-bit value in microseconds
    uint32_t min_ctrl_delay;
    /// Max Controller Delay - 24-bit value in microseconds
    uint32_t max_ctrl_delay;
} hci_rd_local_supp_ctrl_delay_cmd_cmp_evt_t;

/*
 * HCI STATUS PARAMETERS COMMANDS PARAMETERS
 ****************************************************************************************
 */

/// HCI command complete event structure for read rssi
/*@TRACE*/
typedef struct hci_rd_rssi_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///RSSI value
    int8_t rssi;
} hci_rd_rssi_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_rd_clk_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Which clock
    uint8_t clk_type;
} hci_rd_clk_cmd_t;

/// HCI read clock command structure
/*@TRACE*/
typedef struct hci_rd_clk_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///clock
    uint32_t clk;
    ///Accuracy
    uint16_t clk_acc;
} hci_rd_clk_cmd_cmp_evt_t;


/*
 * HCI TESTING COMMANDS PARAMETERS
 ****************************************************************************************
 */

/// HCI command complete event structure for read loop back mode command
/*@TRACE*/
typedef struct hci_rd_loopback_mode_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Local  supported features
    uint8_t lb_mode;
} hci_rd_loopback_mode_cmd_cmp_evt_t;

/*@TRACE*/
typedef struct hci_wr_loopback_mode_cmd
{
    ///Local  supported features
    uint8_t lb_mode;
} hci_wr_loopback_mode_cmd_t;

/*@TRACE*/
typedef struct hci_wr_sp_dbg_mode_cmd
{
    ///Simple pairing mode
    uint8_t sp_mode;
} hci_wr_sp_dbg_mode_cmd_t;


/*
 * HCI LE CONTROLLER COMMANDS PARAMETERS
 ****************************************************************************************
 */

///HCI LE Set Event Mask Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_evt_mask_cmd
{
    ///LE Event Mask
    struct evt_mask le_mask;
} hci_le_set_evt_mask_cmd_t;

///HCI LE Set Random Address Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_rand_addr_cmd
{
    ///Random address to set
    struct bd_addr rand_addr;
} hci_le_set_rand_addr_cmd_t;

///HCI LE Set Advertising Parameters Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_adv_param_cmd
{
    ///Minimum interval for advertising
    uint16_t adv_intv_min;
    ///Maximum interval for advertising
    uint16_t adv_intv_max;
    ///Advertising type (see #adv_type enumeration)
    uint8_t adv_type;
    ///Own address type:  public=0 / random=1 / rpa_or_pub=2 / rpa_or_rnd=3
    uint8_t own_addr_type;
    ///Peer address type: public=0 / random=1
    uint8_t peer_addr_type;
    ///Peer Bluetooth device address
    struct bd_addr peer_addr;
    ///Advertising channel map
    uint8_t adv_ch_map;
    ///Advertising filter policy
    uint8_t adv_filt_policy;
} hci_le_set_adv_param_cmd_t;

///HCI LE Set Advertising Data Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_adv_data_cmd
{
    ///Advertising data length
    uint8_t adv_data_len;
    ///Advertising data - maximum 31 bytes
    struct adv_data data;
} hci_le_set_adv_data_cmd_t;

///HCI LE Set Scan Response Data Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_scan_rsp_data_cmd
{
    ///Scan response data length
    uint8_t scan_rsp_data_len;
    ///Scan response data - maximum 31 bytes
    struct scan_rsp_data data;
} hci_le_set_scan_rsp_data_cmd_t;

///HCI LE Set Advertise Enable Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_adv_en_cmd
{
    ///Advertising enable - 0=disabled, 1=enabled
    uint8_t adv_en;
} hci_le_set_adv_en_cmd_t;

///HCI LE Set Scan Parameters Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_scan_param_cmd
{
    ///Scan type - 0=passive / 1=active
    uint8_t scan_type;
    ///Scan interval
    uint16_t scan_intv;
    ///Scan window size
    uint16_t scan_window;
    ///Own address type - public=0 / random=1 / rpa_or_pub=2 / rpa_or_rnd=3
    uint8_t own_addr_type;
    ///Scan filter policy
    uint8_t scan_filt_policy;
} hci_le_set_scan_param_cmd_t;

///HCI LE Set Scan Enable Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_scan_en_cmd
{
    ///Scan enable - 0=disabled, 1=enabled
    uint8_t scan_en;
    ///Enable for duplicates filtering - 0 =disabled/ 1=enabled
    uint8_t filter_duplic;
} hci_le_set_scan_en_cmd_t;

///HCI LE Create Connection Command parameters structure
/*@TRACE*/
typedef struct hci_le_create_con_cmd
{
    ///Scan interval (N * 0.625 ms)
    uint16_t scan_intv;
    ///Scan window size (N * 0.625 ms)
    uint16_t scan_window;
    ///Initiator filter policy
    uint8_t init_filt_policy;
    ///Peer address type - public=0 / random=1 / rpa_or_pub=2 / rpa_or_rnd=3
    uint8_t peer_addr_type;
    ///Peer BD address
    struct bd_addr peer_addr;
    ///Own address type - public=0 / random=1 / rpa_or_pub=2 / rpa_or_rnd=3
    uint8_t own_addr_type;
    ///Minimum of connection interval (N * 1.25 ms)
    uint16_t con_intv_min;
    ///Maximum of connection interval (N * 1.25 ms)
    uint16_t con_intv_max;
    ///Connection latency
    uint16_t con_latency;
    ///Link supervision timeout
    uint16_t superv_to;
    ///Minimum CE length (N * 0.625 ms)
    uint16_t ce_len_min;
    ///Maximum CE length (N * 0.625 ms)
    uint16_t ce_len_max;
} hci_le_create_con_cmd_t;

///HCI LE Add Device to White List Command parameters structure
/*@TRACE*/
typedef struct hci_le_add_dev_to_wlst_cmd
{
    ///Type of address of the device to be added to the White List - 0=public/1=random
    uint8_t dev_addr_type;
    ///Address of device to be added to White List
    struct bd_addr dev_addr;
} hci_le_add_dev_to_wlst_cmd_t;

///HCI LE Remove Device from White List Command parameters structure
/*@TRACE*/
typedef struct hci_le_rmv_dev_from_wlst_cmd
{
    ///Type of address of the device to be removed from the White List - 0=public/1=random
    uint8_t dev_addr_type;
    ///Address of device to be removed from White List
    struct bd_addr dev_addr;
} hci_le_rmv_dev_from_wlst_cmd_t;

///HCI LE Set Extended Scan Paramaters Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_ext_scan_param_cmd
{
    ///Own address type public/random/rpa
    uint8_t own_addr_type;
    ///Scanning filter policy
    uint8_t scan_filt_policy;
    ///Indicates the PHY(s) on which the advertising packets should be received (@see enum le_phy_mask)
    uint8_t scan_phys;
    ///Paramaters for PHY(s)
    struct scan_phy_param
    {
        ///Scaning Type: passive/active
        uint8_t scan_type;
        ///Scan interval (slots)
        uint16_t scan_intv;
        ///Scan window size (slots)
        uint16_t scan_window;
    } phy[MAX_SCAN_PHYS];
} hci_le_set_ext_scan_param_cmd_t;

///HCI LE Set Extended Scan Enable Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_ext_scan_en_cmd
{
    ///Scan enable - 0=disabled, 1=enabled
    uint8_t scan_en;
    ///Filter duplicates - 0=disabled, 1=enabled, 2=enabled & reset each scan period
    uint8_t filter_duplic;
    ///Scan duration (Time=N*10ms)  | 0x0000: Scan continuously until explicitly disable
    uint16_t duration;
    ///Scan period (Time=N*1.28sec) | 0x0000: Periodic scanning disabled
    uint16_t period;
} hci_le_set_ext_scan_en_cmd_t;

/*@TRACE*/
typedef struct init_phy_param
{
    ///Scan interval (N * 0.625 ms)
    uint16_t scan_interval;
    ///Scan window size (N * 0.625 ms)
    uint16_t scan_window;
    ///Minimum of connection interval (N * 1.25 ms)
    uint16_t con_intv_min;
    ///Maximum of connection interval (N * 1.25 ms)
    uint16_t con_intv_max;
    ///Connection latency
    uint16_t con_latency;
    ///Link supervision timeout
    uint16_t superv_to;
    ///Minimum CE length (N * 0.625 ms)
    uint16_t ce_len_min;
    ///Maximum CE length (N * 0.625 ms)
    uint16_t ce_len_max;
} init_phy_param_t;

///HCI LE Extended Create Connection V1 Command parameters structure
/*@TRACE*/
typedef struct hci_le_ext_create_con_v1_cmd
{
    ///Initiator filter policy
    uint8_t init_filter_policy;
    ///Own address type public/random/rpa
    uint8_t own_addr_type;
    ///Peer address type public/random/rpa
    uint8_t peer_addr_type;
    ///Peer address
    struct bd_addr peer_addr;
    ///Indicates the PHY(s) on which the advertising packets should be received
    uint8_t init_phys;
    ///Paramaters for PHY(s)
    struct init_phy_param phy[MAX_INIT_PHYS];
} hci_le_ext_create_con_v1_cmd_t;

#if (BLE_PAWR)
///HCI LE Extended Create Connection V2 Command parameters structure
/*@TRACE*/
typedef struct hci_le_ext_create_con_v2_cmd
{
    /// Advertising handle (if connecting between a per adv and a synced device)
    uint8_t adv_hdl;
    /// Subevent (if connecting between a per adv and a synced device)
    uint8_t subevt;
    ///Initiator filter policy
    uint8_t init_filter_policy;
    ///Own address type public/random/rpa
    uint8_t own_addr_type;
    ///Peer address type public/random/rpa
    uint8_t peer_addr_type;
    ///Peer address
    struct bd_addr peer_addr;
    ///Indicates the PHY(s) on which the advertising packets should be received
    uint8_t init_phys;
    ///Paramaters for PHY(s)
    struct init_phy_param phy[MAX_INIT_PHYS];
} hci_le_ext_create_con_v2_cmd_t;

/// Default HCI LE Extended Create Connection Command is V2
#define hci_le_ext_create_con_cmd hci_le_ext_create_con_v2_cmd
#else //!(BLE_PAWR)
/// Default HCI LE Extended Create Connection Command is V1
#define hci_le_ext_create_con_cmd hci_le_ext_create_con_v1_cmd
#endif //!(BLE_PAWR)

#if (BLE_PER_ADV)
///HCI LE Periodic Advertising Create Sync Command parameters structure
/*@TRACE*/
typedef struct hci_le_per_adv_create_sync_cmd
{
    /// Options (@see enum per_sync_opt)
    uint8_t options;
    ///Advertising SID
    uint8_t adv_sid;
    ///Advertising address type
    uint8_t adv_addr_type;
    ///Advertiser address
    struct bd_addr adv_addr;
    ///max Skip after receive
    uint16_t skip;
    ///Sync timeout (Time=N*10ms)
    uint16_t sync_to;
    /// Sync CTE type (@see enum sync_cte_type)
    uint8_t sync_cte_type;
} hci_le_per_adv_create_sync_cmd_t;

///HCI LE Periodic Advertising Terminate Sync Command parameters structure
/*@TRACE*/
typedef struct hci_le_per_adv_term_sync_cmd
{
    ///Sync handle
    uint16_t sync_hdl;
} hci_le_per_adv_term_sync_cmd_t;

///HCI LE Add Device to Periodic Advertiser List Command parameters structure
/*@TRACE*/
typedef struct hci_le_add_dev_to_per_adv_list_cmd
{
     ///Advertiser address type
    uint8_t adv_addr_type;
    ///Advertiser address
    struct bd_addr adv_addr;
    ///Advertising SID
    uint8_t adv_sid;
} hci_le_add_dev_to_per_adv_list_cmd_t;

///HCI LE Remove Device from Periodic Advertiser List Command parameters structure
/*@TRACE*/
typedef struct hci_le_rmv_dev_from_per_adv_list_cmd
{
     ///Advertiser address type
    uint8_t adv_addr_type;
    ///Advertiser address
    struct bd_addr adv_addr;
    ///Advertising SID
    uint8_t adv_sid;
} hci_le_rmv_dev_from_per_adv_list_cmd_t;
#endif // (BLE_PER_ADV)

///HCI LE Set Privacy Mode Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_priv_mode_cmd
{
    ///Peer identity address type
    uint8_t peer_addr_type;
    ///Peer identity address
    struct bd_addr  peer_addr;
    ///Privacy mode
    uint8_t priv_mode;
} hci_le_set_priv_mode_cmd_t;

///HCI LE Set Host Channel Classification Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_host_ch_class_cmd
{
    ///Channel map
    struct le_ch_map ch_map;
} hci_le_set_host_ch_class_cmd_t;


///HCI LE Receiver Test v1 Command parameters structure
/*@TRACE*/
typedef struct hci_le_rx_test_v1_cmd
{
    /// RX channel, range: 0x00 to 0x27
    uint8_t rx_channel;
} hci_le_rx_test_v1_cmd_t;

///HCI LE Transmitter Test v1 Command parameters structure
/*@TRACE*/
typedef struct hci_le_tx_test_v1_cmd
{
    /// TX channel, range: 0x00 to 0x27
    uint8_t tx_channel;
    /// Length of test data in bytes, range: 0x00 to 0xFF
    uint8_t test_data_len;
    /**
     * Packet payload
     * 0x00 PRBS9 sequence "11111111100000111101" (in transmission order) as described in [Vol 6] Part F, Section 4.1.5
     * 0x01 Repeated "11110000" (in transmission order) sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x02 Repeated "10101010" (in transmission order) sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x03 PRBS15 sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x04 Repeated "11111111" (in transmission order) sequence
     * 0x05 Repeated "00000000" (in transmission order) sequence
     * 0x06 Repeated "00001111" (in transmission order) sequence
     * 0x07 Repeated "01010101" (in transmission order) sequence
     * 0x08-0xFF Reserved for future use
     */
    uint8_t pkt_payl;
} hci_le_tx_test_v1_cmd_t;

///HCI LE Encrypt Command parameters structure
/*@TRACE*/
typedef struct hci_le_enc_cmd
{
    ///Long term key structure
    struct ltk key;
    ///Pointer to buffer with plain data to encrypt - 16 bytes
    uint8_t plain_data[16];
} hci_le_enc_cmd_t;

/// HCI LE Connection Update Command parameters structure
/*@TRACE*/
typedef struct hci_le_con_update_cmd
{
    ///Connection Handle
    uint16_t conhdl;
    ///Minimum of connection interval (units of 1.25 ms)
    uint16_t con_intv_min;
    ///Maximum of connection interval (units of 1.25 ms)
    uint16_t con_intv_max;
    ///Connection latency (units of connection event)
    uint16_t con_latency;
    ///Link supervision timeout (units of 10 ms)
    uint16_t superv_to;
    ///Minimum of CE length (units of 0.625 ms)
    uint16_t ce_len_min;
    ///Maximum of CE length (units of 0.625 ms)
    uint16_t ce_len_max;
} hci_le_con_update_cmd_t;

/// HCI LE Enable Encryption Command parameters structure
/*@TRACE*/
typedef struct hci_le_en_enc_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Random number - 8B
    struct rand_nb nb;
    ///Encryption Diversifier
    uint16_t enc_div;
    ///Long term key
    struct ltk ltk;
} hci_le_en_enc_cmd_t;

/// HCI long term key request reply command parameters structure
/*@TRACE*/
typedef struct hci_le_ltk_req_reply_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Long term key
    struct ltk ltk;
} hci_le_ltk_req_reply_cmd_t;

/// HCI long term key request negative reply command parameters structure
/*@TRACE*/
typedef struct hci_le_ltk_req_neg_reply_cmd
{
    ///Connection handle
    uint16_t conhdl;
} hci_le_ltk_req_neg_reply_cmd_t;

/// HCI LE remote connection parameter request reply command parameters structure
/*@TRACE*/
typedef struct hci_le_rem_con_param_req_reply_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Interval_Min
    uint16_t interval_min;
    ///Interval_Max
    uint16_t interval_max;
    ///Latency
    uint16_t latency;
    ///Timeout
    uint16_t timeout;
    ///Minimum_CE_Length
    uint16_t min_ce_len;
    ///Maximum_CE_Length
    uint16_t max_ce_len;
} hci_le_rem_con_param_req_reply_cmd_t;

/// HCI LE remote connection parameter request negative reply command parameters structure
/*@TRACE*/
typedef struct hci_le_rem_con_param_req_neg_reply_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Reason
    uint8_t reason;
} hci_le_rem_con_param_req_neg_reply_cmd_t;


/// HCI LE Set Data Length  Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_data_len_cmd
{
    ///Connection Handle
    uint16_t conhdl;
    ///Preferred maximum number of payload octets that the local Controller should include
    ///in a single Link Layer Data Channel PDU.
    uint16_t tx_octets;
    ///Preferred maximum number of microseconds that the local Controller should use to transmit
    ///a single Link Layer Data Channel PDU
    uint16_t tx_time;
} hci_le_set_data_len_cmd_t;

/// HCI LE Read Suggested Default Data Length Command
/*@TRACE*/
typedef struct hci_le_wr_suggted_dft_data_len_cmd
{
    ///Suggested value for the Controller's maximum transmitted number of payload octets to be used
    uint16_t suggted_max_tx_octets;
    ///Suggested value for the Controller's maximum packet transmission time to be used
    uint16_t suggted_max_tx_time;
} hci_le_wr_suggted_dft_data_len_cmd_t;

/// HCI LE Add Device to Resolving List Command
/*@TRACE*/
typedef struct hci_le_add_dev_to_rslv_list_cmd
{
    /// Peer Identity Address Type
    uint8_t peer_id_addr_type;
    /// Peer Identity Address
    struct bd_addr peer_id_addr;
    /// Peer IRK
    struct irk peer_irk;
    /// Local IRK
    struct irk local_irk;
} hci_le_add_dev_to_rslv_list_cmd_t;

/// HCI LE Remove Device From Resolving List Command
/*@TRACE*/
typedef struct hci_le_rmv_dev_from_rslv_list_cmd
{
    /// Peer Identity Address Type
    uint8_t peer_id_addr_type;
    /// Peer Identity Address
    struct bd_addr peer_id_addr;
} hci_le_rmv_dev_from_rslv_list_cmd_t;

/// HCI LE Read Peer Resolvable Address Command
/*@TRACE*/
typedef struct hci_le_rd_peer_rslv_addr_cmd
{
    /// Peer Identity Address Type
    uint8_t peer_id_addr_type;
    /// Peer Identity Address
    struct bd_addr peer_id_addr;
} hci_le_rd_peer_rslv_addr_cmd_t;

/// HCI LE Read Local Resolvable Address Command
/*@TRACE*/
typedef struct hci_le_rd_loc_rslv_addr_cmd
{
    /// Peer Identity Address Type
    uint8_t peer_id_addr_type;
    /// Peer Identity Address
    struct bd_addr peer_id_addr;
} hci_le_rd_loc_rslv_addr_cmd_t;

/// HCI LE Set Address Resolution Enable Command
/*@TRACE*/
typedef struct hci_le_set_addr_resol_en_cmd
{
    /// Address Resolution Enable
    uint8_t enable;
} hci_le_set_addr_resol_en_cmd_t;

/// HCI LE Set Resolvable Private Address Timeout Command
/*@TRACE*/
typedef struct hci_le_set_rslv_priv_addr_to_cmd
{
    /// RPA Timeout
    uint16_t rpa_timeout;
} hci_le_set_rslv_priv_addr_to_cmd_t;

/*
 * HCI EVENTS PARAMETERS
 ****************************************************************************************
 */

/// HCI inquiry complete event structure
/*@TRACE*/
typedef struct hci_inq_cmp_evt
{
    ///Status of the procedure
    uint8_t status;
} hci_inq_cmp_evt_t;


/// HCI Inquiry result event structure (with only 1 result)
/*@TRACE*/
typedef struct hci_inq_res_evt
{

    ///Number of response
    uint8_t nb_rsp;
    ///BdAddr
    struct bd_addr bd_addr;
    ///Page Scan Repetition Mode
    uint8_t page_scan_rep_mode;
    ///Reserved
    uint8_t reserved1;
    ///Reserved
    uint8_t reserved2;
    ///class of device
    struct devclass class_of_dev;
    ///Clock Offset
    uint16_t clk_off;

} hci_inq_res_evt_t;

/// HCI Inquiry result with rssi event structure (with only 1 result)
/*@TRACE*/
typedef struct hci_inq_res_with_rssi_evt
{
    ///Number of response
    uint8_t nb_rsp;
    ///BdAddr
    struct bd_addr bd_addr;
    ///Page Scan Repetition Mode
    uint8_t page_scan_rep_mode;
    ///Reserved
    uint8_t reserved1;
    ///class of device
    struct devclass class_of_dev;
    ///Clock Offset
    uint16_t clk_off;
    ///RSSI
    int8_t rssi;

} hci_inq_res_with_rssi_evt_t;

/// HCI Extended inquiry result indication structure (with only 1 result)
/*@TRACE*/
typedef struct hci_ext_inq_res_evt
{
    ///Number of response
    uint8_t nb_rsp;
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Page Scan Repetition Mode
    uint8_t page_scan_rep_mode;
    ///Reserved
    uint8_t reserved1;
    ///class of device
    struct devclass class_of_dev;
    ///Clock Offset
    uint16_t clk_off;
    ///RSSI
    int8_t rssi;
    ///Extended inquiry response data
    struct eir eir;
} hci_ext_inq_res_evt_t;

/// HCI disconnect complete event structure
/*@TRACE*/
typedef struct hci_disc_cmp_evt
{
    ///Status of received command
    uint8_t status;
    ///Connection Handle
    uint16_t conhdl;
    ///Reason for disconnection
    uint8_t reason;
} hci_disc_cmp_evt_t;

/// HCI basic command complete event structure
/*@TRACE
 * hci_dbg_wr_par_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_dbg_llcp_discard_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_reset_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_set_evt_mask_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_set_per_adv_en_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_create_con_cancel_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_set_evt_mask_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_set_host_ch_class_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_wr_suggted_dft_data_len_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_set_dft_phy_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_rmv_dev_from_wlst_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_set_adv_param_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_set_adv_data_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_set_scan_rsp_data_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_set_adv_en_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_set_scan_param_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_set_scan_en_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_set_rand_addr_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_set_addr_resol_en_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_set_rslv_priv_addr_to_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_rmv_dev_from_rslv_list_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_create_con_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_dbg_plf_reset_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * hci_le_wr_rf_path_comp_cmd_cmp_evt = hci_basic_cmd_cmp_evt
 * */
typedef struct hci_basic_cmd_cmp_evt
{
    ///Status of the command reception
    uint8_t status;
} hci_basic_cmd_cmp_evt_t;

/// HCI basic command complete event structure with connection handle
/*@TRACE*/
typedef struct hci_basic_conhdl_cmd_cmp_evt
{
    /// status
    uint8_t status;
    /// connection handle
    uint16_t conhdl;
} hci_basic_conhdl_cmd_cmp_evt_t;

/// HCI basic command complete event structure with connection handle
/*@TRACE*/
typedef struct hci_basic_conhdl_le_cmp_evt
{
    ///LE Subevent code
    uint8_t subcode;
    /// status
    uint8_t status;
    /// connection handle
    uint16_t conhdl;
} hci_basic_conhdl_le_cmp_evt_t;

/// HCI basic command complete event structure with advertising handle
/*@TRACE*/
typedef struct hci_basic_adv_hdl_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Advertising handle
    uint8_t adv_hdl;
} hci_basic_adv_hdl_cmd_cmp_evt_t;

/// HCI basic command complete event structure with sync handle
/*@TRACE*/
typedef struct hci_basic_sync_hdl_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Sync handle
    uint16_t sync_hdl;
} hci_basic_sync_hdl_cmd_cmp_evt_t;

/// HCI basic command complete event structure with BD address
typedef struct hci_basic_bd_addr_cmd_cmp_evt
{
    ///status
   uint8_t status;
   ///BdAddr
    struct bd_addr bd_addr;
} hci_basic_bd_addr_cmd_cmp_evt_t;

/// HCI basic event structure with status and BD address
typedef struct hci_basic_stat_bd_addr_evt
{
    ///status
   uint8_t status;
   ///BdAddr
    struct bd_addr bd_addr;
} hci_basic_stat_bd_addr_evt_t;

/// HCI basic event including a connection handle as parameter
typedef struct hci_basic_conhdl_evt
{
    ///Connection handle
    uint16_t conhdl;
} hci_basic_conhdl_evt_t;

/// HCI complete event with status only.
/*@TRACE*/
typedef struct hci_cmd_stat_event
{
    /// Status of the command reception
    uint8_t status;
} hci_cmd_stat_event_t;

/// HCI number of packet complete event structure
/*@TRACE*/
typedef struct hci_nb_cmp_pkts_evt
{
    /// number of handles
    uint8_t nb_of_hdl;

    /// Array of number of completed packets per connection
    struct hci_nb_cmp_pkt con[MAX_NB_CMP_PKTS_CON];
} hci_nb_cmp_pkts_evt_t;

/// HCI data buffer overflow event structure
/*@TRACE*/
typedef struct hci_data_buf_ovflw_evt
{
    ///Link type
    uint8_t link_type;
} hci_data_buf_ovflw_evt_t;

/// HCI Hardware Error Event parameters structure
/*@TRACE*/
typedef struct hci_hw_err_evt
{
    /// HW error code
    uint8_t hw_code;
} hci_hw_err_evt_t;

/// HCI encryption change event structure
typedef struct hci_enc_change_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Encryption enabled information
    uint8_t enc_stat;
} hci_enc_change_evt_t;

/// HCI encryption key refresh complete event structure
/*@TRACE
 * hci_enc_key_refresh_evt = hci_enc_key_ref_cmp_evt */
typedef struct hci_enc_key_ref_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
} hci_enc_key_ref_cmp_evt_t;

/// HCI Authenticated Payload Timeout Expired Event structure
/*@TRACE*/
typedef struct hci_auth_payl_to_exp_evt
{
    ///Connection handle
    uint16_t conhdl;
} hci_auth_payl_to_exp_evt_t;

/// HCI command complete event structure for create connection
/*@TRACE*/
typedef struct hci_con_cmp_evt
{
    /// Status
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Bluetooth Device address
    struct bd_addr bd_addr;
    ///Link type
    uint8_t link_type;
    ///Encryption state
    uint8_t enc_en;
} hci_con_cmp_evt_t;

/// HCI command complete event structure for qos setup
/*@TRACE*/
typedef struct hci_qos_setup_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Flags
    uint8_t flags;
    ///Service type
    uint8_t serv_type;
    ///Token rate
    uint32_t tok_rate;
    ///Peak bandwidth
    uint32_t pk_bw;
    ///Latency
    uint32_t lat;
    ///Delay variation
    uint32_t del_var;
} hci_qos_setup_cmp_evt_t;

/// HCI flow specification complete event parameters structure
/*@TRACE*/
typedef struct hci_flow_spec_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Flags
    uint8_t flags;
    ///Flow direction
    uint8_t flow_dir;
    ///Service type
    uint8_t serv_type;
    ///Token rate
    uint32_t tk_rate;
    ///Token buffer size
    uint32_t tk_buf_sz;
    ///Peak bandwidth
    uint32_t pk_bw;
    ///Access latency
    uint32_t acc_lat;
} hci_flow_spec_cmp_evt_t;

/// HCI role change event parameters structure
/*@TRACE*/
typedef struct hci_role_chg_evt
{
    ///Status
    uint8_t status;
    ///BD address
    struct bd_addr bd_addr;
    ///New role
    uint8_t new_role;
} hci_role_chg_evt_t;

/// HCI complete event structure for the read clock offset command
/*@TRACE*/
typedef struct hci_rd_clk_off_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Clock offset
    uint16_t clk_off_val;
} hci_rd_clk_off_cmp_evt_t;

/// HCI event structure for the flush occurred event
/*@TRACE*/
typedef struct hci_flush_occurred_evt
{
    ///Connection handle
    uint16_t conhdl;
} hci_flush_occurred_evt_t;

/// HCI max slot change event structure
/*@TRACE*/
typedef struct hci_max_slot_chg_evt
{
    ///Connection handle
    uint16_t conhdl;
    ///Max slot
    uint8_t max_slot;
} hci_max_slot_chg_evt_t;

/// HCI sniff subrating event parameters structure
/*@TRACE*/
typedef struct hci_sniff_sub_evt
{
    ///Status.
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Maximum transmit latency
    uint16_t max_lat_tx;
    ///Maximum receive latency
    uint16_t max_lat_rx;
    ///Minimum remote TO
    uint16_t min_rem_to;
    ///Minimum local TO
    uint16_t min_loc_to;
} hci_sniff_sub_evt_t;

/// HCI read remote extended features complete event parameters structure
/*@TRACE*/
typedef struct hci_rd_rem_ext_feats_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///page number
    uint8_t pg_nb;
    ///page number max
    uint8_t pg_nb_max;
    ///ext LMP features
    struct features ext_feats;
} hci_rd_rem_ext_feats_cmp_evt_t;

/// HCI read remote extended features complete event parameters structure
/*@TRACE*/
typedef struct hci_rem_host_supp_feats_notif_evt
{
    ///BD address
    struct bd_addr bd_addr;
    ///ext lmp features
    struct features ext_feats;
} hci_rem_host_supp_feats_notif_evt_t;

/// HCI command complete event structure for the read remote supported features command
/*@TRACE*/
typedef struct hci_rd_rem_supp_feats_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Remote features
    struct features rem_feats;
} hci_rd_rem_supp_feats_cmp_evt_t;

/// HCI command complete event structure for the read remote information version command
/*@TRACE*/
typedef struct hci_rd_rem_ver_info_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///LMP version
    uint8_t vers;
    ///Manufacturer name
    uint16_t compid;
    ///LMP subversion
    uint16_t subvers;
} hci_rd_rem_ver_info_cmp_evt_t;

/// HCI encryption change event structure
/*@TRACE*/
typedef struct hci_enc_chg_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Encryption enabled information
    uint8_t enc_stat;
} hci_enc_chg_evt_t;

/// HCI mode change event structure
/*@TRACE*/
typedef struct hci_mode_chg_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Current mode
    uint8_t cur_mode;
    /// Interval
    uint16_t interv;
} hci_mode_chg_evt_t;

/// HCI simple pairing complete event structure
/*@TRACE*/
typedef struct hci_sp_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Bluetooth Device address
    struct bd_addr bd_addr;
} hci_sp_cmp_evt_t;

/// HCI Authentication complete event structure
/*@TRACE*/
typedef struct hci_auth_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
} hci_auth_cmp_evt_t;

/// HCI change connection link key complete event structure
/*@TRACE*/
typedef struct hci_chg_con_lk_cmp_evt
{
    ///Status
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
} hci_chg_con_lk_cmp_evt_t;

/// HCI encryption key refresh complete event structure
/*@TRACE*/
typedef struct hci_enc_key_refresh_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
} hci_enc_key_refresh_cmp_evt_t;

/// HCI master link key complete event structure
/*@TRACE*/
typedef struct hci_master_lk_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Key flag
    uint8_t key_flag;
} hci_master_lk_cmp_evt_t;
/// HCI synchronous link connection complete event structure
/*@TRACE*/
typedef struct hci_sync_con_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///BD address
    struct bd_addr bd_addr;
    ///Link type
    uint8_t lk_type;
    ///Transmit interval
    uint8_t tx_int;
    ///Retransmission window
    uint8_t ret_win;
    ///rx packet length
    uint16_t rx_pkt_len;
    ///tx packet length
    uint16_t tx_pkt_len;
    ///Air mode
    uint8_t air_mode;

} hci_sync_con_cmp_evt_t;

/// HCI synchronous connection change event structure
/*@TRACE*/
typedef struct hci_sync_con_chg_evt
{
    ///Status for command reception
    uint8_t status;
    ///Synchronous Connection handle
    uint16_t sync_conhdl;
    ///Transmit interval
    uint8_t tx_int;
    ///Retransmission window
    uint8_t ret_win;
    ///rx packet length
    uint16_t rx_pkt_len;
    ///tx packet length
    uint16_t tx_pkt_len;
} hci_sync_con_chg_evt_t;

/// HCI connection packet type change event structure
/*@TRACE*/
typedef struct hci_con_pkt_type_chg_evt
{
    ///Status for command reception
    uint8_t status;
    ///Synchronous Connection handle
    uint16_t sync_conhdl;
    ///Synchronous packet type
    uint16_t pkt_type;
} hci_con_pkt_type_chg_evt_t;

/// HCI link supervision timeout change event structure
/*@TRACE*/
typedef struct hci_link_supv_to_chg_evt
{
    ///Connection handle
    uint16_t conhdl;
    ///Link supervision timeout
    uint16_t lsto_val;
} hci_link_supv_to_chg_evt_t;

/// HCI link key request event structure
/*@TRACE*/
typedef struct hci_lk_req_evt
{
    ///BD address
    struct bd_addr bd_addr;
} hci_lk_req_evt_t;

/// HCI encryption key refresh event structure
/*@TRACE*/
typedef struct hci_enc_key_refresh_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
} hci_enc_key_refresh_evt_t;

/// HCI connection request event structure
/*@TRACE*/
typedef struct hci_con_req_evt
{
    ///BD address
    struct bd_addr bd_addr;
    ///Class of device
    struct devclass classofdev;
    ///link type
    uint8_t lk_type;
} hci_con_req_evt_t;

/// HCI quality of service violation event structure
/*@TRACE*/
typedef struct hci_qos_viol_evt
{
    ///Connection handle
    uint16_t conhdl;
} hci_qos_viol_evt_t;

/// HCI io capability response event structure
/*@TRACE*/
typedef struct hci_io_cap_rsp_evt
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///IO capability
    uint8_t io_capa;
    ///OOB data present
    uint8_t oob_data_pres;
    ///Authentication requirements
    uint8_t auth_req;

} hci_io_cap_rsp_evt_t;

/// HCI IO capability response event structure
/*@TRACE*/
typedef struct hci_io_cap_req_evt
{
    ///BdAddr
    struct bd_addr bd_addr;
} hci_io_cap_req_evt_t;

/// HCI Return link keys event structure
/*@TRACE*/
typedef struct hci_return_link_keys_evt
{
    ///Number of Keys
    uint8_t num_keys;
    ///BdAddr
    struct bd_addr bd_addr;
    ///Key
    struct ltk key;
} hci_return_link_keys_evt_t;

/// HCI pin code request event structure
/*@TRACE*/
typedef struct hci_pin_code_req_evt
{
    ///BdAddr
    struct bd_addr bd_addr;
} hci_pin_code_req_evt_t;

/// HCI user passkey request event structure
/*@TRACE*/
typedef struct hci_user_passkey_req_evt
{
    ///BdAddr
    struct bd_addr bd_addr;
} hci_user_passkey_req_evt_t;

/// HCI user passkey notification event structure
/*@TRACE*/
typedef struct hci_user_passkey_notif_evt
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///Passkey
    uint32_t passkey;
} hci_user_passkey_notif_evt_t;

/// HCI remote OOB data request event structure
/*@TRACE*/
typedef struct hci_rem_oob_data_req_evt
{
    ///BdAddr
    struct bd_addr bd_addr;
} hci_rem_oob_data_req_evt_t;

/// HCI user confirmation request event structure
/*@TRACE*/
typedef struct hci_user_cfm_req_evt
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///Passkey
    uint32_t passkey;
} hci_user_cfm_req_evt_t;

/// HCI keypress notification event structure
/*@TRACE*/
typedef struct hci_keypress_notif_evt
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///type
    uint8_t type;
} hci_keypress_notif_evt_t;

/// HCI link key notification event structure
/*@TRACE*/
typedef struct hci_lk_notif_evt
{
    ///BdAddr
    struct bd_addr bd_addr;
    ///Key
    struct ltk key;
    ///type
    uint8_t key_type;
} hci_lk_notif_evt_t;

/// HCI SAM status change event strucutre
typedef struct hci_sam_status_change_evt
{
    ///Connection handle
    uint16_t conhdl;
    ///Local SAM index
    uint8_t loc_idx;
    ///Local SAM TX availability
    uint8_t loc_tx_av;
    ///Local SAM RX availability
    uint8_t loc_rx_av;
    ///Remote SAM index
    uint8_t rem_idx;
    ///Remote SAM TX availability
    uint8_t rem_tx_av;
    ///Remote SAM RX availability
    uint8_t rem_rx_av;
} hci_sam_status_change_evt_t;


/*
 * HCI LE META EVENTS PARAMETERS
 ****************************************************************************************
 */


// LE event structures

/// HCI command complete event structure for the Read Local Supported Features
/*@TRACE*/
typedef struct hci_le_rd_local_supp_feats_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Local LE supported features
    le_features_t feats;
} hci_le_rd_local_supp_feats_cmd_cmp_evt_t;

/// HCI command complete event structure for the Read Advertising Channel Tx Power Command
/*@TRACE
 * hci_le_rd_adv_chnl_tx_pw_cmd_cmp_evt = hci_rd_adv_chnl_tx_pw_cmd_cmp_evt*/
typedef struct hci_rd_adv_chnl_tx_pw_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Advertising channel Tx power level
    int8_t adv_tx_pw_lvl;
} hci_rd_adv_chnl_tx_pw_cmd_cmp_evt_t;

///HCI command complete event structure for the Read White List Size Command
/*@TRACE*/
typedef struct hci_le_rd_wlst_size_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///White List size
    uint8_t wlst_size;
} hci_le_rd_wlst_size_cmd_cmp_evt_t;

///HCI command complete event structure for the Read Buffer Size Command
/*@TRACE*/
typedef struct hci_le_rd_buf_size_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///ACL data packet length that can be sent from host to controller
    uint16_t hc_data_pk_len;
    ///Total number of ACL data packets that can be sent from host to controller.
    uint8_t hc_tot_nb_data_pkts;
} hci_le_rd_buf_size_cmd_cmp_evt_t;

///HCI command complete event structure for LE Rand Command
/*@TRACE*/
typedef struct hci_le_rand_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Random number
    struct rand_nb nb;
} hci_le_rand_cmd_cmp_evt_t;

///HCI command complete event structure for Read Supported States Command
/*@TRACE*/
typedef struct hci_le_rd_supp_states_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///LE supported states response
    struct le_states states;
} hci_le_rd_supp_states_cmd_cmp_evt_t;

///HCI command complete event structure for Test End
/*@TRACE
 *  * hci_le_test_end_cmd_cmp_evt = hci_test_end_cmd_cmp_evt*/
typedef struct hci_test_end_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Number of RX packets - null if TX test was the ended one
    uint16_t nb_packet_received;
} hci_test_end_cmd_cmp_evt_t;

///HCI LE Encrypt complete event structure
/*@TRACE*/
typedef struct hci_le_enc_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Encrypted data to return to command source.
    uint8_t encrypted_data[ENC_DATA_LEN];
} hci_le_enc_cmd_cmp_evt_t;

#if BLE_EMB_PRESENT || BLE_HOST_PRESENT
///HCI LE advertising report event structure
/*@TRACE*/
typedef struct hci_le_adv_report_evt
{
    ///LE Subevent code
    uint8_t subcode;
    ///Number of advertising reports in this event
    uint8_t nb_reports;
    ///Advertising reports structures array
    struct adv_report adv_rep[BLE_ADV_REPORTS_MAX];
} hci_le_adv_report_evt_t;

///HCI LE extended advertising report event structure
/*@TRACE*/
typedef struct hci_le_ext_adv_report_evt
{
    ///LE Subevent code
    uint8_t subcode;
    ///Number of advertising reports in this event
    uint8_t nb_reports;
    ///Paramaters for each report
    struct ext_adv_report adv_rep[BLE_ADV_REPORTS_MAX];
} hci_le_ext_adv_report_evt_t;

#if (BLE_PER_ADV)
///HCI LE periodic advertising sync established event structure
/*@TRACE*/
typedef struct hci_le_per_adv_sync_est_evt
{
    ///LE Subevent code
    uint8_t subcode;
    /// Status of the advertising sync
    uint8_t status;
    /// Sync Handle to be used
    uint16_t sync_hdl;
    /// Advertising SID
    uint8_t adv_sid;
    ///Advertising address type: public/random
    uint8_t adv_addr_type;
    ///Advertising address value
    struct bd_addr adv_addr;
    /// Advertiser PHY (@enum le_phy_value)
    uint8_t phy;
    /// Advertising interval (Time=N*1.25ms)
    uint16_t interval;
    /// Advertiser clock accuracy (@see enum SCA)
    uint8_t adv_ca;
    #if (BLE_PAWR)
    /// Num_Subevents
    uint8_t num_subevt;
    /// Subevent_Interval (units of 1.25ms)
    uint8_t subevt_intv;
    /// Response_Slot_Delay (in unit of 1.25ms)
    uint8_t rsp_slot_delay;
    /// Response_Slot_Spacing (in unit of 0.125ms)
    uint8_t rsp_slot_spacing;
    #endif // (BLE_PAWR)
} hci_le_per_adv_sync_est_evt_t;

///HCI LE periodic advertising report V1 event structure
/*@TRACE*/
typedef struct hci_le_per_adv_report_v1_evt
{
    ///LE Subevent code
    uint8_t subcode;
    /// Sync Handle to be used
    uint16_t sync_hdl;
    /// Tx Power
    int8_t tx_power;
    /// RSSI
    int8_t rssi;
    /// CTE type
    uint8_t cte_type;
    /// Data Status
    uint8_t status;
    ///Data length in advertising packet
    uint8_t data_len;
    ///Data of advertising packet
    uint8_t data[PER_ADV_DATA_MAX_LEN];
} hci_le_per_adv_report_v1_evt_t;

#if (BLE_PAWR)
///HCI LE periodic advertising report V2 event structure
/*@TRACE*/
typedef struct hci_le_per_adv_report_v2_evt
{
    ///LE Subevent code
    uint8_t subcode;
    /// Sync Handle to be used
    uint16_t sync_hdl;
    /// Tx Power
    int8_t tx_power;
    /// RSSI
    int8_t rssi;
    /// CTE type
    uint8_t cte_type;
    /// Periodic Event Counter
    uint16_t evt_cnt;
    /// Subevent
    uint8_t subevt;
    /// Data Status
    uint8_t status;
    ///Data length in advertising packet
    uint8_t data_len;
    ///Data of advertising packet
    uint8_t data[PER_ADV_DATA_MAX_LEN];
} hci_le_per_adv_report_v2_evt_t;
#endif // (BLE_PAWR)

///HCI LE periodic advertising sync lost event structure
/*@TRACE
 * hci_le_periodic_adv_sync_lost_evt = hci_le_per_adv_sync_lost_evt*/
typedef struct hci_le_per_adv_sync_lost_evt
{
    ///LE Subevent code
    uint8_t subcode;
    /// Sync Handle to be used
    uint16_t sync_hdl;
} hci_le_per_adv_sync_lost_evt_t;
#endif // (BLE_PER_ADV)

///HCI LE scan timeout event structure
/*@TRACE*/
typedef struct hci_le_scan_timeout_evt
{
    ///LE Subevent code
    uint8_t subcode;
} hci_le_scan_timeout_evt_t;

#endif //BLE_EMB_PRESENT || BLE_HOST_PRESENT

/// HCI command complete event structure for Read Channel Map Command
/*@TRACE*/
typedef struct hci_le_rd_ch_map_cmd_cmp_evt
{
    ///Status of command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Channel map
    struct le_ch_map ch_map;
} hci_le_rd_ch_map_cmd_cmp_evt_t;

/// HCI command complete event structure for Long Term Key Request Reply Command
/*@TRACE*/
typedef struct hci_le_ltk_req_reply_cmd_cmp_evt
{
    ///Status of command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
} hci_le_ltk_req_reply_cmd_cmp_evt_t;

/// HCI command complete event structure for Long Term Key Request Negative Reply Command
/*@TRACE*/
typedef struct hci_le_ltk_req_neg_reply_cmd_cmp_evt
{
    ///Status of command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
} hci_le_ltk_req_neg_reply_cmd_cmp_evt_t;

/// HCI command complete event structure for LE Read Suggested Default Data Length Command
/*@TRACE*/
typedef struct hci_le_rd_suggted_dft_data_len_cmd_cmp_evt
{
    ///Status of command reception
    uint8_t status;
    ///Host's suggested value for the Controller's maximum transmitted number of payload octets
    uint16_t suggted_max_tx_octets;
    ///Host's suggested value for the Controller's maximum packet transmission time
    uint16_t suggted_max_tx_time;
} hci_le_rd_suggted_dft_data_len_cmd_cmp_evt_t;

/// HCI command complete event structure for LE Read Maximum Data Length Command
/*@TRACE*/
typedef struct hci_le_rd_max_data_len_cmd_cmp_evt
{
    ///Status of command reception
    uint8_t status;
    ///Maximum number of payload octets that the local Controller supports for transmission
    uint16_t suppted_max_tx_octets;
    ///Maximum time, in microseconds, that the local Controller supports for transmission
    uint16_t suppted_max_tx_time;
    ///Maximum number of payload octets that the local Controller supports for reception
    uint16_t suppted_max_rx_octets;
    ///Maximum time, in microseconds, that the local Controller supports for reception
    uint16_t suppted_max_rx_time;
} hci_le_rd_max_data_len_cmd_cmp_evt_t;

/// HCI LE Read Peer Resolvable Address Command Complete Event
/*@TRACE*/
typedef struct hci_le_rd_peer_rslv_addr_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    /// Peer Resolvable Address
    struct bd_addr peer_rslv_addr;
} hci_le_rd_peer_rslv_addr_cmd_cmp_evt_t;

/// HCI LE Read Local Resolvable Address Command Complete Event
/*@TRACE*/
typedef struct hci_le_rd_loc_rslv_addr_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    /// Local Resolvable Address
    struct bd_addr loc_rslv_addr;
} hci_le_rd_loc_rslv_addr_cmd_cmp_evt_t;

/// HCI LE Read Resolving List Size Command Complete Event
/*@TRACE*/
typedef struct hci_le_rd_rslv_list_size_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    /// Resolving List Size
    uint8_t size;
} hci_le_rd_rslv_list_size_cmd_cmp_evt_t;


/// HCI write authenticated payload timeout command
/*@TRACE*/
typedef struct hci_wr_auth_payl_to_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Authenticated payload timeout (N*10ms)
    uint16_t auth_payl_to;
} hci_wr_auth_payl_to_cmd_t;

/// HCI command complete event structure for the Write Authenticated Payload Timeout Command
/*@TRACE*/
typedef struct hci_wr_auth_payl_to_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
} hci_wr_auth_payl_to_cmd_cmp_evt_t;

/// HCI command complete event structure for HCI LE Connection Update Command
/*@TRACE*/
typedef struct hci_le_con_update_cmp_evt
{
    ///LE Subevent code
    uint8_t subcode;
    ///Status of received command
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Connection interval value
    uint16_t con_interval;
    ///Connection latency value
    uint16_t con_latency;
    ///Supervision timeout
    uint16_t sup_to;
} hci_le_con_update_cmp_evt_t;

/// HCI command complete event structure for create connection
/*@TRACE*/
typedef struct hci_le_con_cmp_evt
{
    ///LE Subevent code
    uint8_t subcode;
    ///Status of received command
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Device role - 0=Master/ 1=Slave
    uint8_t role;
    ///Peer address type - 0=public/1=random
    uint8_t peer_addr_type;
    ///Peer address
    struct bd_addr peer_addr;
    ///Connection interval
    uint16_t con_interval;
    ///Connection latency
    uint16_t con_latency;
    ///Link supervision timeout
    uint16_t sup_to;
    ///Master clock accuracy
    uint8_t clk_accuracy;
} hci_le_con_cmp_evt_t;

/// HCI LE read remote used feature command parameters structure
/*@TRACE*/
typedef struct hci_le_rd_rem_feats_cmd
{
    ///Connection handle
    uint16_t conhdl;
} hci_le_rd_rem_feats_cmd_t;

/// HCI command complete event structure for HCI LE read remote feature Command
/*@TRACE*/
typedef struct hci_le_rd_rem_feats_cmd_cmp_evt
{
    ///LE Subevent code
    uint8_t subcode;
    ///Status of received command
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Le Features
    le_features_t le_feats;
} hci_le_rd_rem_feats_cmd_cmp_evt_t;

/// HCI command structure for the read transmit power level command
/*@TRACE*/
typedef struct hci_rd_tx_pwr_lvl_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Power Level type: current or maximum
    uint8_t type;
} hci_rd_tx_pwr_lvl_cmd_t;

/// HCI command complete event structure for the read transmit power level command
/*@TRACE*/
typedef struct hci_rd_tx_pwr_lvl_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Value of TX power level
    int8_t tx_pow_lvl;
} hci_rd_tx_pwr_lvl_cmd_cmp_evt_t;

/// HCI read remote information version command parameters structure
/*@TRACE*/
typedef struct hci_rd_rem_ver_info_cmd
{
    ///Connection handle
    uint16_t conhdl;
} hci_rd_rem_ver_info_cmd_t;

/// HCI LE remote connection parameter request event
/*@TRACE*/
typedef struct hci_le_rem_con_param_req_evt
{
    ///LE Subevent code
    uint8_t subcode;
    ///Connection handle
    uint16_t conhdl;
    ///Interval_Min
    uint16_t interval_min;
    ///Interval_Max
    uint16_t interval_max;
    ///Latency
    uint16_t latency;
    ///Timeout
    uint16_t timeout;
} hci_le_rem_con_param_req_evt_t;


/// HCI LE Enhance Connection Complete event strucutre
/*@TRACE*/
typedef struct hci_le_enh_con_cmp_evt
{
    ///LE Subevent code
    uint8_t subcode;
    ///Status of received command
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Device role - 0=Master/ 1=Slave
    uint8_t role;
    ///Peer address type - 0=public/1=random
    uint8_t peer_addr_type;
    ///Peer address
    struct bd_addr peer_addr;
    ///Local Resolvable Private Address
    struct bd_addr loc_rslv_priv_addr;
    ///Peer Resolvable Private Address
    struct bd_addr peer_rslv_priv_addr;
    ///Connection interval
    uint16_t con_interval;
    ///Connection latency
    uint16_t con_latency;
    ///Link supervision timeout
    uint16_t sup_to;
    ///Master clock accuracy
    uint8_t clk_accuracy;
    #if (BLE_PAWR)
    ///Advertising handle
    uint8_t adv_hdl;
    /// Sync handle
    uint16_t sync_hdl;
    #endif // (BLE_PAWR)
} hci_le_enh_con_cmp_evt_t;

typedef struct hci_le_generate_dhkey_cmp_evt
{
    ///LE Subevent code
    uint8_t subcode;
    ///Status of received command
    uint8_t status;
    /// The 32 byte Diffie Helman Key
    uint8_t dh_key[32];
} hci_le_generate_dhkey_cmp_evt_t;

typedef struct hci_le_rd_local_p256_public_key_cmp_evt
{
    ///LE Subevent code
    uint8_t subcode;
    ///Status of received command
    uint8_t status;
    /// The 32 byte Diffie Helman Key
    uint8_t public_key[64];

} hci_le_rd_local_p256_public_key_cmp_evt_t;

#if BLE_EMB_PRESENT || BLE_HOST_PRESENT
/// HCI LE Direct Advertising Report Event
/*@TRACE*/
typedef struct hci_le_dir_adv_rep_evt
{
    ///LE Subevent code
    uint8_t subcode;
    ///Number of reports
    uint8_t nb_reports;
    ///Direct Advertising reports structures array
    struct dir_adv_report   adv_rep[BLE_ADV_REPORTS_MAX];
} hci_le_dir_adv_rep_evt_t;
#endif //BLE_EMB_PRESENT || BLE_HOST_PRESENT

/// Connected LE event
typedef struct hci_le_con_evt
{
    ///LE Subevent code
    uint8_t subcode;
    ///Connection handle
    uint16_t conhdl;
} hci_le_con_evt_t;

/// HCI command complete event structure for HCI LE read remote used feature Command
/*@TRACE*/
typedef struct hci_le_ltk_request_evt
{
    ///LE Subevent code
    uint8_t subcode;
    ///Connection handle
    uint16_t conhdl;
    ///Random number
    struct rand_nb rand_val;
    ///Encryption diversifier
    uint16_t ediv;
} hci_le_ltk_request_evt_t;

/// HCI LE META event LE Data Length Change Event
/*@TRACE*/
typedef struct hci_le_data_len_chg_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Connection handle
    uint16_t conhdl;
    /// The maximum number of payload octets in TX
    uint16_t max_tx_octets;
    /// The maximum time that the local Controller will take to transmit (in us)
    uint16_t max_tx_time;
    /// The maximum number of payload octets in RX
    uint16_t max_rx_octets;
    /// The maximum time that the local Controller will take to receive (in us)
    uint16_t max_rx_time;
} hci_le_data_len_chg_evt_t;


/// HCI Synchronization Train Received Event
/*@TRACE*/
typedef struct hci_sync_train_rec_evt
{
    /// Status
    uint8_t status;
    /// BD_ADDR
    struct bd_addr bd_addr;
    /// Clock_Offset (28 bits) - (CLKNslave - CLK) modulo 2^28
    uint32_t clock_offset;
    /// AFH_Channel_Map
    struct bt_ch_map afh_ch_map;
    /// LT_ADDR
    uint8_t lt_addr;
    /// Next_Broadcast_Instant (28 bits)
    uint32_t next_bcst_instant;
    /// Connectionless_Slave_Broadcast_Interval (in slots)
    uint16_t csb_int;
    /// Service_Data
    uint8_t service_data;
} hci_sync_train_rec_evt_t;

/// HCI Connectionless Slave Broadcast Receive Event
/*@TRACE*/
typedef struct hci_con_slv_bcst_rec_evt
{
    /// BD_ADDR
    struct bd_addr bd_addr;
    /// LT_ADDR
    uint8_t lt_addr;
    /// CLK (28 bits)
    uint32_t clk;
    /// Offset (28 bits) - (CLKNslave - CLK) modulo 2^28
    uint32_t offset;
    /// Receive Status
    uint8_t receive_status;
    /// Fragment
    uint8_t fragment;
    /// Data_Length (in bytes)
    uint8_t data_length;
    /// Data
    uint8_t data[__ARRAY_EMPTY];
} hci_con_slv_bcst_rec_evt_t;

/// HCI Connectionless Slave Broadcast Timeout Event
/*@TRACE*/
typedef struct hci_con_slv_bcst_to_evt
{
    /// BD_ADDR
    struct bd_addr bd_addr;
    /// LT_ADDR
    uint8_t lt_addr;
} hci_con_slv_bcst_to_evt_t;

/// HCI Connectionless Slave Broadcast Channel Map Change Event
/*@TRACE*/
typedef struct hci_con_slv_bcst_ch_map_chg_evt
{
    /// Channel_Map
    struct bt_ch_map ch_map;
} hci_con_slv_bcst_ch_map_chg_evt_t;

/*@TRACE*/
typedef struct hci_le_gen_dhkey_cmp_evt
{
    ///LE Subevent code
    uint8_t subcode;
    uint8_t status;
    uint8_t dh_key[32];
} hci_le_gen_dhkey_cmp_evt_t;

/*@TRACE*/
typedef struct hci_le_rd_loc_p256_pub_key_cmp_evt
{
    ///LE Subevent code
    uint8_t subcode;
    uint8_t status;
    public_key_t public_key;
} hci_le_rd_loc_p256_pub_key_cmp_evt_t;


/// HCI command structure for the LE receiver test v3 command
/*@TRACE*/
typedef struct hci_le_rx_test_v3_cmd
{
    /// RX channel, range: 0x00 to 0x27
    uint8_t rx_channel;
    /// PHY (@enum le_phy_value)
    uint8_t phy;
    /// Modulation index (0: standard | 1: stable)
    uint8_t mod_idx;
    /// Expected CTE length in 8us units, range: 0x02 to 0x14
    uint8_t exp_cte_len;
    /// Expected CTE type (0: AOA | 1: AOD-1us | 2: AOD-2us)
    uint8_t exp_cte_type;
    /// Slot durations (1: 1 us | 2: 2 us)
    uint8_t slot_dur;
    /// Length of switching pattern (number of antenna IDs in the pattern), range: 0x02 to 0x4B
    uint8_t switching_pattern_len;
    /// Antenna IDs
    uint8_t antenna_id[MAX_SWITCHING_PATTERN_LEN];
} hci_le_rx_test_v3_cmd_t;

/// HCI command structure for the LE transmitter test v3 command
/*@TRACE*/
typedef struct hci_le_tx_test_v3_cmd
{
    /// TX channel, range: 0x00 to 0x27
    uint8_t tx_channel;
    /// Length of test data in bytes, range: 0x00 to 0xFF
    uint8_t test_data_len;
    /**
     * Packet payload
     * 0x00 PRBS9 sequence "11111111100000111101" (in transmission order) as described in [Vol 6] Part F, Section 4.1.5
     * 0x01 Repeated "11110000" (in transmission order) sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x02 Repeated "10101010" (in transmission order) sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x03 PRBS15 sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x04 Repeated "11111111" (in transmission order) sequence
     * 0x05 Repeated "00000000" (in transmission order) sequence
     * 0x06 Repeated "00001111" (in transmission order) sequence
     * 0x07 Repeated "01010101" (in transmission order) sequence
     * 0x08-0xFF Reserved for future use
     */
    uint8_t pkt_payl;
    /// PHY (@enum le_phy_value)
    uint8_t phy;
    /// CTE length (in 8us unit)
    uint8_t cte_len;
    /// CTE type (0: AOA | 1: AOD-1us | 2: AOD-2us)
    uint8_t cte_type;
    /// Length of switching pattern (number of antenna IDs in the pattern)
    uint8_t switching_pattern_len;
    /// Antenna IDs
    uint8_t antenna_id[MAX_SWITCHING_PATTERN_LEN];
} hci_le_tx_test_v3_cmd_t;

/// HCI command structure for the LE transmitter test v4 command
/*@TRACE*/
typedef struct hci_le_tx_test_v4_cmd
{
    /// TX channel, range: 0x00 to 0x27
    uint8_t tx_channel;
    /// Length of test data in bytes, range: 0x00 to 0xFF
    uint8_t test_data_len;
    /**
     * Packet payload
     * 0x00 PRBS9 sequence "11111111100000111101" (in transmission order) as described in [Vol 6] Part F, Section 4.1.5
     * 0x01 Repeated "11110000" (in transmission order) sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x02 Repeated "10101010" (in transmission order) sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x03 PRBS15 sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x04 Repeated "11111111" (in transmission order) sequence
     * 0x05 Repeated "00000000" (in transmission order) sequence
     * 0x06 Repeated "00001111" (in transmission order) sequence
     * 0x07 Repeated "01010101" (in transmission order) sequence
     * 0x08-0xFF Reserved for future use
     */
    uint8_t pkt_payl;
    /// PHY (@enum le_phy_value)
    uint8_t phy;
    /// CTE length (in 8us unit)
    uint8_t cte_len;
    /// CTE type (0: AOA | 1: AOD-1us | 2: AOD-2us)
    uint8_t cte_type;
    /// Length of switching pattern (number of antenna IDs in the pattern)
    uint8_t switching_pattern_len;
    /// Antenna IDs
    uint8_t antenna_id[MAX_SWITCHING_PATTERN_LEN];
    /// Transmit power level in dBm (0x7E: minimum | 0x7F: maximum | range: -127 to +20)
    int8_t tx_pwr_lvl;
} hci_le_tx_test_v4_cmd_t;

#if (BLE_CONLESS_CTE_TX)
/// HCI command structure for the LE set connectionless CTE transmit parameters command
/*@TRACE*/
typedef struct hci_le_set_conless_cte_tx_param_cmd
{
    /// Advertising handle
    uint8_t adv_hdl;
    /// CTE length (in 8us unit)
    uint8_t cte_len;
    /// CTE type (0: AOA | 1: AOD-1us | 2: AOD-2us)
    uint8_t cte_type;
    /// CTE count (number of CTEs to transmit in each periodic advertising interval, range 0x01 to 0x10)
    uint8_t cte_count;
    /// Length of switching pattern (number of antenna IDs in the pattern)
    uint8_t switching_pattern_len;
    /// Antenna IDs
    uint8_t antenna_id[MAX_SWITCHING_PATTERN_LEN];
} hci_le_set_conless_cte_tx_param_cmd_t;

/// HCI command structure for the LE set connectionless CTE transmit enable command
/*@TRACE*/
typedef struct hci_le_set_conless_cte_tx_en_cmd
{
    /// Advertising handle
    uint8_t adv_hdl;
    /// CTE enable
    uint8_t cte_en;
} hci_le_set_conless_cte_tx_en_cmd_t;
#endif // (BLE_CONLESS_CTE_TX)

#if (BLE_CONLESS_CTE_RX)
/// HCI command structure for LE set connectionless IQ sampling enable command
/*@TRACE*/
typedef struct hci_le_set_conless_iq_sampl_en_cmd
{
    /// Sync handle
    uint16_t sync_hdl;
    /// Sampling enable
    uint8_t sampl_en;
    /// Slot durations (1: 1us | 2: 2us)
    uint8_t slot_dur;
    /// Max sampled CTEs
    uint8_t max_sampl_cte;
    /// Length of switching pattern
    uint8_t switching_pattern_len;
    /// Antenna IDs
    uint8_t antenna_id[MAX_SWITCHING_PATTERN_LEN];
} hci_le_set_conless_iq_sampl_en_cmd_t;
#endif // (BLE_CONLESS_CTE_RX)

#if (BLE_CON_CTE_REQ)
/// HCI command structure for the LE set connection CTE receive parameters command
/*@TRACE*/
typedef struct hci_le_set_con_cte_rx_param_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Sampling enable
    uint8_t sampl_en;
    /// Slot durations (1: 1us | 2: 2us)
    uint8_t slot_dur;
    /// Length of switching pattern (number of antenna IDs in the pattern)
    uint8_t switching_pattern_len;
    /// Antenna IDs
    uint8_t antenna_id[MAX_SWITCHING_PATTERN_LEN];
} hci_le_set_con_cte_rx_param_cmd_t;
#endif // (BLE_CON_CTE_REQ)

#if (BLE_CON_CTE_RSP)
/// HCI command structure for the LE set connection CTE transmit parameters command
/*@TRACE*/
typedef struct hci_le_set_con_cte_tx_param_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// CTE types (bit0: AOA | bit1: AOD-1us | bit2: AOD-2us)
    uint8_t cte_types;
    /// Length of switching pattern (number of antenna IDs in the pattern)
    uint8_t switching_pattern_len;
    /// Antenna IDs
    uint8_t antenna_id[MAX_SWITCHING_PATTERN_LEN];
} hci_le_set_con_cte_tx_param_cmd_t;
#endif // (BLE_CON_CTE_RSP)

#if (BLE_CON_CTE_REQ)
/// HCI command structure for the LE connection CTE request enable command
/*@TRACE*/
typedef struct hci_le_con_cte_req_en_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Enable
    uint8_t en;
    /// CTE request interval (in number of connection events)
    uint16_t cte_req_intv;
    /// Requested CTE length (in 8us unit)
    uint8_t req_cte_len;
    /// Requested CTE type (0: AOA | 1: AOD-1us | 2: AOD-2us)
    uint8_t req_cte_type;
} hci_le_con_cte_req_en_cmd_t;
#endif // (BLE_CON_CTE_REQ)

#if (BLE_CON_CTE_RSP)
/// HCI command structure for the LE connection CTE response enable command
/*@TRACE*/
typedef struct hci_le_con_cte_rsp_en_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Enable
    uint8_t en;
} hci_le_con_cte_rsp_en_cmd_t;
#endif // (BLE_CON_CTE_RSP)

#if (BLE_AOA || BLE_AOD)
/// HCI command structure for LE read antenna information command complete event
/*@TRACE*/
typedef struct hci_le_rd_antenna_inf_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Supported switching sampling rates
    uint8_t supp_switching_sampl_rates;
    /// Number of antennae
    uint8_t antennae_num;
    /// Max length of switching pattern (number of antenna IDs in the pattern)
    uint8_t max_switching_pattern_len;
    /// Max CTE length
    uint8_t max_cte_len;
} hci_le_rd_antenna_inf_cmd_cmp_evt_t;
#endif // (BLE_AOA || BLE_AOD)

#if (BLE_PER_ADV)
/// HCI command structure for the LE set periodic advertising receive enable command
/*@TRACE*/
typedef struct hci_le_set_per_adv_rec_en_cmd
{
    /// Sync handle
    uint16_t sync_hdl;
    /// Enable (@see enum per_adv_rec_en)
    uint8_t en;
} hci_le_set_per_adv_rec_en_cmd_t;
#endif // (BLE_PER_ADV)

#if (BLE_PAST)
/// HCI command structure for the LE set periodic advertising receive enable command
/*@TRACE*/
typedef struct hci_le_per_adv_sync_transf_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Service data (value provided by the Host)
    uint16_t serv_data;
    /// Sync handle
    uint16_t sync_hdl;
} hci_le_per_adv_sync_transf_cmd_t;

/// HCI command structure for the LE Periodic Advertising Set Info Transfer command
/*@TRACE*/
typedef struct hci_le_per_adv_set_info_transf_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Service data (value provided by the Host)
    uint16_t serv_data;
    /// Advertising handle
    uint8_t adv_hdl;
} hci_le_per_adv_set_info_transf_cmd_t;

/// HCI command structure for the LE Set Periodic Advertising Sync Transfer Parameters command
/*@TRACE*/
typedef struct hci_le_set_per_adv_sync_transf_param_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Mode (@see enum per_adv_sync_info_rec_mode)
    uint8_t mode;
    /// The number of periodic advertising packets that can be skipped after a successful receive
    uint16_t skip;
    /// Sync timeout (Time=N*10ms)
    uint16_t sync_to;
    /// CTE type (@see enum sync_cte_type)
    uint8_t cte_type;
} hci_le_set_per_adv_sync_transf_param_cmd_t;

/// HCI command structure for the LE Set Default Periodic Advertising Sync Transfer Parameters command
/*@TRACE*/
typedef struct hci_le_set_dft_per_adv_sync_transf_param_cmd
{
    /// Mode (@see enum per_adv_sync_info_rec_mode)
    uint8_t mode;
    /// The number of periodic advertising packets that can be skipped after a successful receive
    uint16_t skip;
    /// Sync timeout (Time=N*10ms)
    uint16_t sync_to;
    /// CTE type (@see enum sync_cte_type)
    uint8_t cte_type;
} hci_le_set_dft_per_adv_sync_transf_param_cmd_t;

/// HCI LE Periodic Advertising Sync Transfer Received event strucutre
/*@TRACE*/
typedef struct hci_le_per_adv_sync_transf_rec_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Status
    uint8_t status;
    /// Connection handle
    uint16_t conhdl;
    /// Service data (value provided by the Host)
    uint16_t serv_data;
    /// Sync handle
    uint16_t sync_hdl;
    /// Advertising SID
    uint8_t adv_sid;
    ///Advertising address type: public/random
    uint8_t adv_addr_type;
    ///Advertising address value
    struct bd_addr adv_addr;
    /// Advertiser PHY (@enum le_phy_value)
    uint8_t phy;
    /// Advertising interval (Time=N*1.25ms)
    uint16_t interval;
    /// Advertiser clock accuracy (@see enum SCA)
    uint8_t adv_ca;
    #if (BLE_PAWR)
    /// Num_Subevents
    uint8_t num_subevt;
    /// Subevent_Interval (units of 1.25ms)
    uint8_t subevt_intv;
    /// Response_Slot_Delay (in unit of 1.25ms)
    uint8_t rsp_slot_delay;
    /// Response_Slot_Spacing (in unit of 0.125ms)
    uint8_t rsp_slot_spacing;
    #endif // (BLE_PAWR)
} hci_le_per_adv_sync_transf_rec_evt_t;
#endif // (BLE_PAST)

/// HCI command structure for the LE enhanced read transmit power level command
/*@TRACE*/
typedef struct hci_le_enh_rd_tx_pwr_lvl_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// PHY (@see enum le_phy_pwr_value)
    uint8_t phy;
} hci_le_enh_rd_tx_pwr_lvl_cmd_t;

/// HCI command structure for the LE enhanced read transmit power level command complete event
/*@TRACE*/
typedef struct hci_le_enh_rd_tx_pwr_lvl_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Connection handle
    uint16_t conhdl;
    /// PHY (@see enum le_phy_pwr_value)
    uint8_t phy;
    /// Current transmit power level (dBm)
    int8_t curr_tx_pwr_lvl;
    /// Max transmit power level (dBm)
    int8_t max_tx_pwr_lvl;
} hci_le_enh_rd_tx_pwr_lvl_cmd_cmp_evt_t;

/// HCI command structure for the LE read remote transmit power level command
/*@TRACE*/
typedef struct hci_le_rd_remote_tx_pwr_lvl_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// PHY (@see le_phy_pwr_value)
    uint8_t phy;
} hci_le_rd_remote_tx_pwr_lvl_cmd_t;

/// HCI command structure for the LE set path loss reporting parameters command
/*@TRACE*/
typedef struct hci_le_set_path_loss_rep_param_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// High threshold (dB)
    uint8_t hi_thr;
    /// High hysteresis (dB)
    uint8_t hi_hyst;
    /// Low threshold (dB)
    uint8_t lo_thr;
    /// Low hysteresis (dB)
    uint8_t lo_hyst;
    /// Min time spent (conn events)
    uint16_t min_time;
} hci_le_set_path_loss_rep_param_cmd_t;

/// HCI command structure for the LE set path loss reporting parameters command complete event
/*@TRACE*/
typedef struct hci_le_set_path_loss_rep_param_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Connection handle
    uint16_t conhdl;
} hci_le_set_path_loss_rep_param_cmd_cmp_evt_t;

/// HCI command structure for the LE set path loss reporting enable command
/*@TRACE*/
typedef struct hci_le_set_path_loss_rep_en_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Enable (@see enum pwr_report_en)
    uint8_t en;
} hci_le_set_path_loss_rep_en_cmd_t;

/// HCI command structure for the LE set path loss reporting enable command complete event
/*@TRACE*/
typedef struct hci_le_set_path_loss_rep_en_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Connection handle
    uint16_t conhdl;
} hci_le_set_path_loss_rep_en_cmd_cmp_evt_t;

/// HCI command structure for the LE set transmit power reporting enable command
/*@TRACE*/
typedef struct hci_le_set_tx_power_rep_en_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Local Enable (@see enum pwr_report_en)
    uint8_t local_en;
    /// Remote Enable (@see enum pwr_report_en)
    uint8_t remote_en;
} hci_le_set_tx_power_rep_en_cmd_t;

/// HCI command structure for the LE set transmit power reporting enable command complete event
/*@TRACE*/
typedef struct hci_le_set_tx_power_rep_en_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Connection handle
    uint16_t conhdl;
} hci_le_set_tx_power_rep_en_cmd_cmp_evt_t;

/// HCI command structure for the LE Path Loss Threshold event
/*@TRACE*/
typedef struct hci_le_path_loss_threshold_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Connection handle
    uint16_t conhdl;
    /// Current path loss (dB)
    uint8_t curr_path_loss;
    /// Zone entered (see enum #le_path_loss_zone) meaningful only with available current path loss
    uint8_t zone_entered;
} hci_le_path_loss_threshold_evt_t;

/// HCI command strucutre for the LE Transmit Power Reporting event
/*@TRACE*/
typedef struct hci_le_tx_power_rep_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Status
    uint8_t status;
    /// Connection handle
    uint16_t conhdl;
    /// Reason (@see enum pwr_report_reason)
    uint8_t reason;
    /// PHY (@see enum le_phy_pwr_value)
    uint8_t phy;
    /// Transmit Power level (dBm)
    int8_t tx_pwr;
    /// Transmit Power level flags (@see enum pwr_ctrl_flags)
    uint8_t flags;
    /// Delta (dB)
    int8_t delta;
} hci_le_tx_power_rep_evt_t;

#if (BLE_PWR_CTRL && RW_DEBUG)
/// HCI command strucutre for the LE Transmit Power Reporting event
/*@TRACE*/
typedef struct hci_dbg_le_set_rsp_tx_pwr_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// TX power
    int8_t tx_pwr;
} hci_dbg_le_set_rsp_tx_pwr_cmd_t;
#endif // (BLE_PWR_CTRL && RW_DEBUG)

/*
 * HCI VENDOR SPECIFIC COMMANDS PARAMETERS
 ****************************************************************************************
 */

/// Buffer structure
/*@TRACE*/
typedef struct buffer_tag
{
    /// length of buffer
    uint8_t length;
    /// data of 128 bytes length
    uint8_t data[128];
} buffer_tag_t;

/// Common structure for Command Complete Event of HCI Debug Read Memory/Flash/Param complete event parameters - vendor specific
/*@TRACE
 * hci_dbg_rd_mem_cmd_cmp_evt = hci_dbg_basic_rd_data_cmd_cmp_evt
 * hci_dbg_rd_flash_cmd_cmp_evt = hci_dbg_basic_rd_data_cmd_cmp_evt
 * hci_dbg_rd_par_cmd_cmp_evt = hci_dbg_basic_rd_data_cmd_cmp_evt
 * */
typedef struct hci_dbg_basic_rd_data_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    ///buffer structure to return
    struct buffer_tag buf;
} hci_dbg_basic_rd_data_cmd_cmp_evt_t;

///HCI Debug read memory variable command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_rd_mem_cmd
{
    ///Start address to read
    uint32_t start_addr;
    ///Access size
    uint8_t type;
    ///Length to read
    uint8_t length;
} hci_dbg_rd_mem_cmd_t;

///HCI Debug write memory variable command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_wr_mem_cmd
{
    ///Start address to read
    uint32_t start_addr;
    ///Access size
    uint8_t type;
    ///buffer structure to return
    struct buffer_tag buf;
} hci_dbg_wr_mem_cmd_t;

///HCI Debug delete parameter command parameters - vendor specific
/*@TRACE
 * hci_dbg_del_par_cmd = hci_dbg_del_param_cmd*/
typedef struct hci_dbg_del_param_cmd
{
    ///Parameter tag
    uint16_t param_tag;
} hci_dbg_del_param_cmd_t;

///HCI Debug erase flash command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_er_flash_cmd
{
    ///Flash type
    uint8_t flashtype;
    ///Start offset address
    uint32_t startoffset;
    ///Size to erase
    uint32_t size;
} hci_dbg_er_flash_cmd_t;

///HCI Debug write flash command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_wr_flash_cmd
{
    ///Flash type
    uint8_t flashtype;
    ///Start offset address
    uint32_t startoffset;
    ///buffer structure
    struct buffer_tag buf;
} hci_dbg_wr_flash_cmd_t;

///HCI Debug read flash command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_rd_flash_cmd
{
    ///Flash type
    uint8_t flashtype;
    ///Start offset address
    uint32_t startoffset;
    ///Size to read
    uint8_t size;
} hci_dbg_rd_flash_cmd_t;

///HCI Debug read parameter command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_rd_par_cmd
{
    ///Parameter tag
    uint16_t param_tag;
} hci_dbg_rd_par_cmd_t;

///HCI Debug read parameters command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_wr_par_cmd
{
    ///Parameter tag
    uint16_t param_tag;
    ///Structure buffer
    struct buffer_tag buf;
} hci_dbg_wr_par_cmd_t;

#if CRYPTO_UT
///HCI Debug Test cryptographic functions command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_test_crypto_func_cmd
{
    /// Id of Function to be tested
    uint8_t function;
    /// Structure buffer
    struct buffer_tag buf;
} hci_dbg_test_crypto_func_cmd_t;
#endif //CRYPTO_UT

#if (BT_READ_PICONET_CLOCK)
typedef struct hci_vs_rd_piconet_clock_cmd
{
    ///Connection handle of the link
    uint16_t conhdl;
    ///Pulse trigger management
    uint8_t trig_pulse;
    ///Clock type (0=native clock, 1=piconet clock of conhndl)
    uint8_t clk_type;
} hci_vs_rd_piconet_clock_cmd_t;

typedef struct hci_vs_rd_piconet_clock_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    ///Connection handle of the link
    uint16_t conhdl;
    /// Local timestamp(position in half slots)
    uint32_t loc_clk_hslt;
    /// Local timestamp fractional part (in half us)
    uint16_t loc_clk_hus;
    /// Piconet timestamp(position in half slots)
    uint32_t pic_clk_off_hslt;
    /// Piconet bit offset (in half us)
    uint16_t pic_bit_off_hus;
} hci_vs_rd_piconet_clock_cmd_cmp_evt_t;
#endif //(BT_READ_PICONET_CLOCK)

#if (BT_HCI_TEST_MODE)
/// HCI command structure for the LE receiver test v3 command
/*@TRACE*/
typedef struct hci_vs_rx_test_cmd
{
    /// RX channel, range: 0 to 79
    uint8_t rx_channel;
    /// Packet type
    uint8_t pkt_type;
    /// Infinite test indicator: false if 0, true if 1
    uint8_t infinite_test;
} hci_vs_rx_test_cmd_t;

/// HCI command structure for the LE transmitter test v4 command
/*@TRACE*/
typedef struct hci_vs_tx_test_cmd
{
    /// TX channel, range: 0 to 79
    uint8_t tx_channel;
    /// Length of test data in bytes, range: 0 to 1021
    uint16_t test_data_len;
    /**
     * Packet payload
     * 0x00 PRBS9 sequence "11111111100000111101" (in transmission order) as described in [Vol 6] Part F, Section 4.1.5
     * 0x01 Repeated "11110000" (in transmission order) sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x02 Repeated "10101010" (in transmission order) sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x03 PRBS15 sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x04 Repeated "11111111" (in transmission order) sequence
     * 0x05 Repeated "00000000" (in transmission order) sequence
     * 0x06 Repeated "00001111" (in transmission order) sequence
     * 0x07 Repeated "01010101" (in transmission order) sequence
     * 0x08-0xFF Reserved for future use
     */
    uint8_t pkt_payl;
    /// Packet_Type
    uint8_t pkt_type;
    /// Transmit power level in dBm (0x7E: minimum | 0x7F: maximum | range: -127 to +20)
    int8_t tx_pwr_lvl;
    /// Infinite test indicator: false if 0, true if 1
    uint8_t infinite_test;
} hci_vs_tx_test_cmd_t;

///HCI command complete event structure for Test End
/*@TRACE*/
typedef struct hci_vs_test_end_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Number of RX/TX packets
    uint16_t nb_packets;
} hci_vs_test_end_cmd_cmp_evt_t;
#endif //(BT_HCI_TEST_MODE)

#if RW_DEBUG
///HCI Debug Test scheduling planner set function command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_test_sch_plan_set_cmd
{
    /// Interval (in half-slots)
    uint32_t interval;
    /// Offset (in half-slots)
    uint32_t offset;
    /// Minimum duration (in half-slots)
    uint32_t duration_min;
    /// Maximum duration (in half-slots)
    uint32_t duration_max;
    /// Connection handle
    uint16_t conhdl;
    /// Reference connection handle, the primary activity on which the new activity refers to (e.g. ACL connection for a synchronous connection)
    uint16_t conhdl_ref;
    /// Margin to consider around the activity events, when not related to the same activity (in half-slots)
    uint16_t margin;
    /// Indicate activity is movable
    bool mobility_level;
} hci_dbg_test_sch_plan_set_cmd_t;

///HCI Debug Test scheduling planner set function command complete event parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_test_sch_plan_set_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Connection handle of the moved activity
    uint16_t moved_conhdl;
} hci_dbg_test_sch_plan_set_cmd_cmp_evt_t;

///HCI Debug Test scheduling planner remove function command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_test_sch_plan_rem_cmd
{
    /// Connection handle
    uint16_t conhdl;
} hci_dbg_test_sch_plan_rem_cmd_t;

///HCI Debug Test scheduling planner check function command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_test_sch_plan_chk_cmd
{
    /// Interval (in half-slots)
    uint32_t interval;
    /// Offset (in half-slots)
    uint32_t offset;
    /// Minimum duration (in half-slots)
    uint32_t duration_min;
    /// Connection handle
    uint16_t conhdl;
    /// Reference connection handle, the primary activity on which the new activity refers to (e.g. ACL connection for a synchronous connection)
    uint16_t conhdl_ref;
    /// Margin to consider around the activity events, when not related to the same activity (in half-slots)
    uint16_t margin;
} hci_dbg_test_sch_plan_chk_cmd_t;

///HCI Debug Test scheduling planner request function command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_test_sch_plan_req_cmd
{
    /// Minimum interval (in half-slots)
    uint32_t interval_min;
    /// Maximum interval (in half-slots)
    uint32_t interval_max;
    /// Minimum duration (in half-slots)
    uint32_t duration_min;
    /// Maximum duration (in half-slots)
    uint32_t duration_max;
    /// Connection handle
    uint16_t conhdl;
    /// Reference connection handle, the primary activity on which the new activity refers to (e.g. ACL connection for a synchronous connection)
    uint16_t conhdl_ref;
    /// Margin to consider around the activity events, when not related to the same activity (in half-slots)
    uint16_t margin;
    /// Period (in half-slots)
    uint8_t period;
} hci_dbg_test_sch_plan_req_cmd_t;

///HCI Debug Test scheduling planner request function command complete event parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_test_sch_plan_req_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Interval (in half-slots)
    uint32_t interval;
    /// Minimum offset (in half-slots)
    uint32_t offset_min;
    /// Maximum offset (in half-slots)
    uint32_t offset_max;
} hci_dbg_test_sch_plan_req_cmd_cmp_evt_t;
#endif //RW_DEBUG

///HCI Debug Read Kernel Statistics complete event parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_rd_ke_stats_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    ///Max message sent
    uint32_t max_msg_sent;
    ///Max message saved
    uint32_t max_msg_saved;
    ///Max timer used
    uint32_t max_timer_used;
    ///Max heap used
    uint32_t max_heap_used;
    ///Max stack used
    uint32_t max_stack_used;
} hci_dbg_rd_ke_stats_cmd_cmp_evt_t;


/// HCI Debug Read information about memory usage. - vendor specific
/*@TRACE*/
typedef struct hci_dbg_rd_mem_info_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    /// memory size currently used into each heaps.
    uint16_t mem_used[KE_MEM_BLOCK_MAX];
    /// peak of memory usage measured
    uint32_t max_mem_used;
} hci_dbg_rd_mem_info_cmd_cmp_evt_t;

///HCI Debug identify Flash command complete event parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_id_flash_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    ///Flash identity
    uint8_t flash_id;
} hci_dbg_id_flash_cmd_cmp_evt_t;

///HCI Debug RF Register read command
/*@TRACE*/
typedef struct hci_dbg_rf_reg_rd_cmd
{
    /// register address
    uint16_t addr;
} hci_dbg_rf_reg_rd_cmd_t;

///HCI Debug RF Register read command complete event
/*@TRACE*/
typedef struct hci_dbg_rf_reg_rd_cmd_cmp_evt
{
    /// status
    uint8_t status;
    /// register address
    uint16_t addr;
    /// register value
    uint32_t value;
} hci_dbg_rf_reg_rd_cmd_cmp_evt_t;

///HCI Debug RF Register write command
/*@TRACE*/
typedef struct hci_dbg_rf_reg_wr_cmd
{
    /// register address
    uint16_t addr;
    /// register value
    uint32_t value;
} hci_dbg_rf_reg_wr_cmd_t;

///HCI Debug RF Register write command complete event
/*@TRACE*/
typedef struct hci_dbg_rf_reg_wr_cmd_cmp_evt
{
    /// status
    uint8_t status;
    /// address
    uint16_t addr;
} hci_dbg_rf_reg_wr_cmd_cmp_evt_t;

///HCI Debug platform reset command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_plf_reset_cmd
{
    /// reason
    uint8_t reason;
} hci_dbg_plf_reset_cmd_t;

#if (RW_DEBUG && BT_EMB_PRESENT)
/// Send LMP Packets
/*@TRACE*/
typedef struct hci_dbg_bt_send_lmp_cmd
{
    /// Connection handle
    uint16_t conhdl;
    ///buffer structure to return
    struct buffer_tag buf;
} hci_dbg_bt_send_lmp_cmd_t;

/// Discard LMP Packets
/*@TRACE*/
typedef struct hci_dbg_bt_discard_lmp_en_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Enable/Disable LMP discard (0: disable / 1: enable)
    uint8_t enable;
} hci_dbg_bt_discard_lmp_en_cmd_t;

/// Set local clock
/*@TRACE*/
typedef struct hci_dbg_set_local_clock_cmd
{
    /// Clock (in half-slots)
    uint32_t clock;
} hci_dbg_set_local_clock_cmd_t;
#endif //(RW_DEBUG && BT_EMB_PRESENT)


#if(BT_I2C_OVER_HCI)
/// Read I2C Driver
/*@TRACE*/
typedef struct hci_vs_i2c_read_cmd
{
    /// I2C Slave Address
    uint8_t slave_address;
    /// I2C Register Address to read
    uint8_t reg_address;
    /// Data size to read
    uint8_t length;
} hci_vs_i2c_read_cmd_t;

/// Read I2C Driver
/*@TRACE*/
typedef struct hci_vs_i2c_read_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Data size
    uint8_t length;
    /// Data read
    uint8_t data[__ARRAY_EMPTY];
} hci_vs_i2c_read_cmd_cmp_evt_t;

/// Write I2C Driver
/*@TRACE*/
typedef struct hci_vs_i2c_write_cmd
{
    /// I2C Slave Address
    uint8_t slave_address;
    /// I2C Register Address to read
    uint8_t reg_address;
    /// Data size to write
    uint8_t length;
    /// Data to write
    uint8_t data[__ARRAY_EMPTY];
} hci_vs_i2c_write_cmd_t;
#endif // (BT_I2C_OVER_HCI)

#if (RW_WLAN_COEX)
///HCI Debug wlan coexistence command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_wlan_coex_cmd
{
    /// State
    uint8_t state;
} hci_dbg_wlan_coex_cmd_t;
#if (RW_WLAN_COEX_TEST)
///HCI Debug wlan coexistence test scenario command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_wlan_coextst_scen_cmd
{
    /// Scenario
    uint32_t scenario;
} hci_dbg_wlan_coextst_scen_cmd_t;
#endif //RW_WLAN_COEX_TEST
#endif //RW_WLAN_COEX

#if (RW_MWS_COEX)
///HCI Debug mws coexistence command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_mws_coex_cmd
{
    /// State
    uint8_t state;
} hci_dbg_mws_coex_cmd_t;
#if (RW_MWS_COEX_TEST)
///HCI Debug mws coexistence test scenario command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_mws_coextst_scen_cmd
{
    /// Scenario
    uint32_t scenario;
} hci_dbg_mws_coextst_scen_cmd_t;
#endif //RW_MWS_COEX_TEST
#if (RW_DEBUG)
///HCI init SAM Negotiations - EBQ vendor specific
/*@TRACE*/
typedef struct hci_vs_ebq_init_sam_nego_cmd
{
    /// SAM Index
    uint8_t index;
} hci_vs_ebq_init_sam_nego_cmd_t;
#endif // (RW_DEBUG)
#endif //RW_MWS_COEX

#if BLE_IQ_GEN
///HCI Debug I&Q samples generator command parameters - vendor specific
/// Maximum number of supported antenna patterns
#define DBG_IQGEN_MAX_ANTENNA (8)
/*@TRACE*/
typedef struct hci_dbg_iqgen_cfg_cmd
{
    /// Antenna switch/sample control
    // bit[0]: 0: up-sweep; 1: up-down sweep (internal switching mode)
    // bit[1]: 0: 1us intervals; 1: 2us intervals (internal switching mode)
    // bit[2]: 0: internal switching mode; 1: baseband switching mode
    uint8_t mode;
    /// Number of antenna patterns
    uint8_t nb_antenna;
    /// I/Q sample control
    struct
    {
        /// I-sample control
        /// 0: up-count from 0x01
        /// 1: down-count from 0xFF
        /// 2: Fixed value 0x00
        /// 3: PRBS pattern
        /// 4..0xFF: Fixed value
        uint8_t i;
        /// Q-sample control
        /// 0: up-count from 0x01
        /// 1: down-count from 0xFF
        /// 2: Fixed value 0x00
        /// 3: PRBS pattern
        /// 4..0xFF: Fixed value
        uint8_t q;
    } iq_ctrl[DBG_IQGEN_MAX_ANTENNA];
} hci_dbg_iqgen_cfg_cmd_t;
#endif //BLE_IQ_GEN

#if (BLE_ISO_MODE_0)
/// HCI VS Mic-Less Set command parameters
/*@TRACE*/
typedef struct hci_vs_mic_less_set_cmd
{
    /// Handle that identify an ACL link that is requesting MIC-less mode
    uint16_t conhdl;
} hci_vs_mic_less_set_cmd_t;

/// HCI VS Setup AM0 Data Path command parameters
typedef struct hci_vs_setup_am0_data_path_cmd
{
    /// Channel_Handle to be used to identify an Audio Mode 0 Channel (range 0x0000-0x0EFF)
    uint16_t am0_hdl;
    /// Data path direction
    /// 0x00       Input (Host to Controller)
    /// 0x01       Output (Controller to Host)
    /// Others     Reserved for future use
    uint8_t data_path_direction;
    /// Data path ID
    /// 0x00       HCI
    /// 0x01-0xFE  Logical_Channel_Number. The meaning of the logical channel is vendor specific.
    /// 0xFF       Test Mode
    uint8_t data_path_id;
    /// Codec Identifier
    uint8_t codec_id[CODEC_ID_LEN];
    /// Controller delay in ms (Range: 0x000000 to 0x3D0900)
    uint32_t ctrl_delay;
    /// Data path configuration length
    uint8_t cfg_len;
    /// Data path configuration
    uint8_t cfg[__ARRAY_EMPTY];
} hci_vs_setup_am0_data_path_cmd_t;

/// HCI VS Remove AM0 Data Path command parameters
typedef struct hci_vs_remove_am0_data_path_cmd
{
    /// Channel_Handle to be used to identify an Audio Mode 0 Channel (range 0x0000-0x0EFF)
    uint16_t am0_hdl;
    /// Bit[0]: Input data path, Bit[1]: Output data path
    uint8_t direction;
} hci_vs_remove_am0_data_path_cmd_t;

/// HCI VS Setup Audio Mode 0 channel command parameters
/*@TRACE*/
typedef struct hci_vs_setup_am0_stream_cmd
{
    /// Handle that identify an ACL link between a Master and a Slave device that is requesting
    /// an Audio Mode 0 Channel (range 0x0000-0x0EFF)
    uint16_t conhdl;
    /// The number of bits in each unit of data received from the Host over the data transport. (Range 0x01 - 0xFB)
    uint8_t tx_size;
    /// The number of bits in each unit of data sent to the Host over the data transport. (Range 0x01 - 0xFB)
    uint8_t rx_size;
} hci_vs_setup_am0_stream_cmd_t;

/// Used to read the maximum size of the data portion of isochronous packets
/// no parameter on command, only need to specify returned parameters
typedef struct hci_vs_setup_am0_stream_cmd_cmp_evt
{
    /// 0x00 - Request succeed ; 0x01-0xFF Failed reason
    uint8_t status;
    /// Handle that identify an ACL link between a Master and a Slave device that is requesting
    /// an Audio Mode 0 Channel (range 0x0000-0x0EFF)
    uint16_t conhdl;
    /// Channel_Handle to be used to identify an Audio Mode 0 Channel (range 0x0000-0x0EFF)
    uint16_t am0_hdl;
} hci_vs_setup_am0_stream_cmd_cmp_evt_t;

/// Removes existing Audio Mode 0 channel command parameters
/*@TRACE*/
typedef struct hci_vs_remove_am0_stream_cmd
{
    /// Channel_Handles used to identify Audio Mode 0 Channel (range 0x0000-0x0EFF)
    uint16_t am0_hdl;
} hci_vs_remove_am0_stream_cmd_t;

/// HCI basic command complete event structure with AM0 Channel handle
/*@TRACE*/
typedef struct hci_vs_basic_am0_cmd_cmp_evt
{
    /// status
    uint8_t status;
    /// Channel_Handles used to identify Audio Mode 0 Channel (range 0x0000-0x0EFF)
    uint16_t am0_hdl;
} hci_vs_basic_am0_cmd_cmp_evt_t;
#endif // (BLE_ISO_MODE_0)


///HCI Debug HW Register Read command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_ble_reg_rd_cmd
{
    /// register address
    uint16_t reg_addr;
} hci_dbg_ble_reg_rd_cmd_t;

///HCI Debug HW Register write command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_ble_reg_wr_cmd
{
    /// register address
    uint16_t reg_addr;
    /// extra parameter
    uint16_t reserved;
    /// register value
    uint32_t reg_value;
} hci_dbg_ble_reg_wr_cmd_t;

///HCI Debug HW Register Read Complete event parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_ble_reg_rd_cmd_cmp_evt
{
    /// status
    uint8_t  status;
    /// register address
    uint16_t reg_addr;
    /// register value
    uint32_t reg_value;
} hci_dbg_ble_reg_rd_cmd_cmp_evt_t;

///HCI Debug HW Register Write Complete event parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_ble_reg_wr_cmd_cmp_evt
{
    /// status
    uint8_t  status;
    /// register address
    uint16_t reg_addr;
} hci_dbg_ble_reg_wr_cmd_cmp_evt_t;

///HCI Vendor Specific Set Preferred Slave Latency command parameters
/*@TRACE*/
typedef struct hci_vs_set_pref_slave_latency_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Preferred latency (in number of connection events)
    uint16_t latency;
} hci_vs_set_pref_slave_latency_cmd_t;

///HCI Vendor Specific Set Preferred Slave event duration command parameters
/*@TRACE*/
typedef struct hci_vs_set_pref_slave_evt_dur_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Preferred event duration (N * 0.625 ms)
    uint16_t duration;
    /// Slave transmits a single packet per connection event (False/True)
    uint8_t single_tx;
} hci_vs_set_pref_slave_evt_dur_cmd_t;

///HCI Vendor Specific Set Maximum RX size and time command parameters
/*@TRACE*/
typedef struct hci_vs_set_max_rx_size_and_time_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Maximum RX size (in B)
    uint16_t rx_octets;
    /// Maximum RX time (in us)
    uint16_t rx_time;
} hci_vs_set_max_rx_size_and_time_cmd_t;

///HCI Vendor Specific Set Priority Elevation command parameters
/*@TRACE*/
typedef struct hci_vs_set_prio_elevation_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Arbitration priority elevation (in units of increment)
    uint8_t elevation;
} hci_vs_set_prio_elevation_cmd_t;

/// Send LLCP Packets
/*@TRACE*/
typedef struct hci_dbg_send_llcp_cmd
{
    /// Connection handle
    uint16_t conhdl;
    ///buffer structure to return
    struct buffer_tag buf;
} hci_dbg_send_llcp_cmd_t;

///HCI Debug LLC discard command parameters - vendor specific
/*@TRACE*/
typedef struct hci_dbg_llcp_discard_cmd
{
    /// Handle pointing to the connection for which LLCP commands have to be discarded
    uint16_t conhdl;
    /// Flag indicating if the discarding has to be enabled or disabled
    uint8_t enable;
} hci_dbg_llcp_discard_cmd_t;

#if (BT_53)
///HCI_DBG_Set_Max_Enc_Key_Size command parameters
/*@TRACE*/
typedef struct hci_dbg_set_max_enc_key_size_cmd
{
    /// Max Encryption Key Size
    uint8_t max_enc_key_size;
} hci_dbg_set_max_enc_key_size_cmd_t;
#endif // (BT_53)

///HCI_VS_BT_Set_Rx_Max_Slot command parameters
/*@TRACE*/
typedef struct hci_vs_bt_set_rx_max_slot_cmd
{
    /// Rx Max Slot (1/3/5 slot)
    uint8_t rx_max_slot;
} hci_vs_bt_set_rx_max_slot_cmd_t;

#if (RW_DEBUG && (BT_EMB_PRESENT || BLE_EMB_PRESENT))
///HCI_VS_Set_Sched_Prio command parameters
/*@TRACE*/
typedef struct hci_vs_set_sched_prio_cmd
{
    /// Number of priority/increment pairs
    uint8_t pair_count;
    /// Priority/increment pairs
    uint8_t prio_data[__ARRAY_EMPTY];
} hci_vs_set_sched_prio_cmd_t;
#endif // (RW_DEBUG && (BT_EMB_PRESENT || BLE_EMB_PRESENT))

#if (BLE_EMB_PRESENT && BLE_TEST_MODE_SUPPORT)
///HCI_VS_LE_Infinite_Test command parameters
/*@TRACE*/
typedef struct hci_vs_le_set_infinite_test_cmd
{
    /// Infinite test: false if 0, true if 1
    uint8_t infinite_test;
} hci_vs_le_set_infinite_test_cmd_t;
#endif // (BLE_EMB_PRESENT && BLE_TEST_MODE_SUPPORT)

struct hci_vs_set_mdm_switch_param_cmd
{
    ///nb of received packet window detection
    uint8_t  window;
    ///Threshold to switch from HP to LP in bad packet received
    uint8_t  thershold;
    ///Variable to enable or disable the switch mecanism
    bool  enable;
};

#if (BLE_HDT && RW_DEBUG)
/// HCI_VS_LE_HDT_En command parameters.
/*@TRACE*/
typedef struct
{
    /// Connection handle
    uint16_t conhdl;
    /// 40-bit physical channel address
    uint8_t pca[5];
    /// HDT TX Rate
    uint8_t tx_rate;
} hci_vs_le_hdt_en_cmd_t;
#endif // (BLE_HDT && RW_DEBUG)

#if (RW_DEBUG)
/// HCI DBG Meta Event indicating a SW assertion
/*@TRACE*/
typedef struct hci_dbg_assert_evt
{
    ///DBG Subevent code
    uint8_t subcode;
    /// Assert type (warning/error)
    //@trc_ref assert_type
    uint8_t type;
    /// Line number
    uint32_t line;
    /// Param0
    uint32_t param0;
    /// Param1
    uint32_t param1;
    /// File name
    uint8_t file[__ARRAY_EMPTY];
} hci_dbg_assert_evt_t;
#endif //(RW_DEBUG)

/// HCI LE Read PHY command
/*@TRACE*/
typedef struct hci_le_rd_phy_cmd
{
    /// Connection Handle
    uint16_t conhdl;
} hci_le_rd_phy_cmd_t;

/// HCI LE Set Default PHY Command
/*@TRACE*/
typedef struct hci_le_set_dft_phy_cmd
{
    /// Preferred PHYS selection
    uint8_t all_phys;
    /// Preferred PHYS for TX
    uint8_t tx_phys;
    /// Preferred PHYS for RX
    uint8_t rx_phys;
} hci_le_set_dft_phy_cmd_t;

/// HCI LE Set PHY Command
/*@TRACE*/
typedef struct hci_le_set_phy_cmd
{
    /// Connection Handle
    uint16_t conhdl;
    /// Preferred PHYS selection
    uint8_t all_phys;
    /// Preferred PHYS for TX
    uint8_t tx_phys;
    /// Preferred PHYS for RX
    uint8_t rx_phys;
    /// PHY options
    uint16_t phy_opt;
} hci_le_set_phy_cmd_t;

/// HCI LE Set Extended Advertising Parameters Command Complete Event
/*@TRACE*/
typedef struct hci_le_set_ext_adv_param_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Selected Tx power
    int8_t sel_tx_pwr;
} hci_le_set_ext_adv_param_cmd_cmp_evt_t;

/// HCI LE Receiver Test v2 Command
/*@TRACE*/
typedef struct hci_le_rx_test_v2_cmd
{
    /// RX channel, range: 0x00 to 0x27
    uint8_t rx_channel;
    /// PHY (@enum le_phy_value)
    uint8_t phy;
    /// Modulation index (0: standard | 1: stable)
    uint8_t mod_idx;
} hci_le_rx_test_v2_cmd_t;

/// HCI LE Transmitter Test v2 Command
/*@TRACE*/
typedef struct hci_le_tx_test_v2_cmd
{
    /// TX channel, range: 0x00 to 0x27
    uint8_t tx_channel;
    /// Length of test data in bytes, range: 0x00 to 0xFF
    uint8_t test_data_len;
    /**
     * Packet payload
     * 0x00 PRBS9 sequence "11111111100000111101" (in transmission order) as described in [Vol 6] Part F, Section 4.1.5
     * 0x01 Repeated "11110000" (in transmission order) sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x02 Repeated "10101010" (in transmission order) sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x03 PRBS15 sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x04 Repeated "11111111" (in transmission order) sequence
     * 0x05 Repeated "00000000" (in transmission order) sequence
     * 0x06 Repeated "00001111" (in transmission order) sequence
     * 0x07 Repeated "01010101" (in transmission order) sequence
     * 0x08-0xFF Reserved for future use
     */
    uint8_t pkt_payl;
    /// PHY (@enum le_phy_value)
    uint8_t phy;
} hci_le_tx_test_v2_cmd_t;

///HCI LE Set Advertising Set Random Address Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_adv_set_rand_addr_cmd
{
    /// Advertising handle
    uint8_t adv_hdl;
    /// Advertising random address
    struct bd_addr rand_addr;
} hci_le_set_adv_set_rand_addr_cmd_t;

///HCI LE Set Extended Advertising Parameters V1 Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_ext_adv_param_v1_cmd
{
    /// Advertising handle
    uint8_t adv_hdl;
    /// Advertising event properties
    uint16_t adv_evt_properties;
    /// Primary advertising minimum interval
    uint8_t prim_adv_intv_min[3];
    /// Primary advertising maximum interval
    uint8_t prim_adv_intv_max[3];
    /// Primary advertising channel map
    uint8_t prim_adv_ch_map;
    /// Own address type:  public=0 / random=1 / rpa_or_pub=2 / rpa_or_rnd=3
    uint8_t own_addr_type;
    /// Peer address type: public=0 / random=1
    uint8_t peer_addr_type;
    /// Peer Bluetooth device address
    struct bd_addr peer_addr;
    /// Advertising filter policy
    uint8_t adv_filt_policy;
    /// Advertising Tx power
    int8_t adv_tx_pwr;
    /// Primary advertising PHY
    uint8_t prim_adv_phy;
    /// Secondary advertising max skip
    uint8_t sec_adv_max_skip;
    /// Secondary advertising PHY
    uint8_t sec_adv_phy;
    /// Advertising SID
    uint8_t adv_sid;
    /// Scan request notification enable
    uint8_t scan_req_notif_en;
} hci_le_set_ext_adv_param_v1_cmd_t;

#if (BLE_CSSA)
///HCI LE Set Extended Advertising Parameters V2 Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_ext_adv_param_v2_cmd
{
    /// Advertising handle
    uint8_t adv_hdl;
    /// Advertising event properties
    uint16_t adv_evt_properties;
    /// Primary advertising minimum interval
    uint8_t prim_adv_intv_min[3];
    /// Primary advertising maximum interval
    uint8_t prim_adv_intv_max[3];
    /// Primary advertising channel map
    uint8_t prim_adv_ch_map;
    /// Own address type:  public=0 / random=1 / rpa_or_pub=2 / rpa_or_rnd=3
    uint8_t own_addr_type;
    /// Peer address type: public=0 / random=1
    uint8_t peer_addr_type;
    /// Peer Bluetooth device address
    struct bd_addr peer_addr;
    /// Advertising filter policy
    uint8_t adv_filt_policy;
    /// Advertising Tx power
    int8_t adv_tx_pwr;
    /// Primary advertising PHY
    uint8_t prim_adv_phy;
    /// Secondary advertising max skip
    uint8_t sec_adv_max_skip;
    /// Secondary advertising PHY
    uint8_t sec_adv_phy;
    /// Advertising SID
    uint8_t adv_sid;
    /// Scan request notification enable
    uint8_t scan_req_notif_en;
    /// Primary advertising PHY options
    uint8_t prim_adv_phy_opt;
    /// Secondary advertising PHY options
    uint8_t sec_adv_phy_opt;
} hci_le_set_ext_adv_param_v2_cmd_t;

/// Default HCI Set Extended Advertising Parameters Command is V2
#define hci_le_set_ext_adv_param_cmd hci_le_set_ext_adv_param_v2_cmd
#else //!(BLE_CSSA)
/// Default HCI Set Extended Advertising Parameters Command is V1
#define hci_le_set_ext_adv_param_cmd hci_le_set_ext_adv_param_v1_cmd
#endif //!(BLE_CSSA)

///HCI LE Set Extended Advertising Data Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_ext_adv_data_cmd
{
    /// Advertising handle
    uint8_t adv_hdl;
    /**
     *  Operation
     *  0x00 Intermediate fragment of fragmented extended advertising data
     *  0x01 First fragment of fragmented extended advertising data
     *  0x02 Last fragment of fragmented extended advertising data
     *  0x03 Complete extended advertising data
     *  0x04 [ID7300_r06] Unchanged data (just update the Advertising DID)
     *  All other values Reserved for future use
     */
    uint8_t operation;
    /**
     *  Fragment preference
     *  0x00 The Controller may fragment all Host advertising data
     *  0x01 The Controller should not fragment nor minimize fragmentation of Host advertising data
     *  All other values Reserved for future use
     */
    uint8_t frag_pref;
    /// Advertising Data Length (0-252 bytes)
    uint8_t data_len;
    /// Advertising data
    uint8_t data[__ARRAY_EMPTY];
} hci_le_set_ext_adv_data_cmd_t;

///HCI LE Set Extended Scan Response Data Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_ext_scan_rsp_data_cmd
{
    /// Advertising handle
    uint8_t adv_hdl;
    /**
     *  Operation
     *  0x00 Intermediate fragment of fragmented extended advertising data
     *  0x01 First fragment of fragmented extended advertising data
     *  0x02 Last fragment of fragmented extended advertising data
     *  0x03 Complete extended advertising data
     *  0x04 [ID7300_r06] Unchanged data (just update the Advertising DID)
     *  All other values Reserved for future use
     */
    uint8_t operation;
    /**
     *  Fragment preference
     *  0x00 The Controller may fragment all Host advertising data
     *  0x01 The Controller should not fragment nor minimize fragmentation of Host advertising data
     *  All other values Reserved for future use
     */
    uint8_t frag_pref;
    /// Scan Response Data Length (0-252 bytes)
    uint8_t data_len;
    /// Advertising data
    uint8_t data[__ARRAY_EMPTY];
} hci_le_set_ext_scan_rsp_data_cmd_t;

#if BLE_EMB_PRESENT || BLE_HOST_PRESENT

/// Advertising set enable parameters
/*@TRACE*/
typedef struct hci_ext_adv_set_en_param
{
    /// Advertising handle
    uint8_t adv_hdl;
    /// Duration (N * 10 ms), 0x0000 No advertising duration. Advertising to continue until the Host disables it.
    uint16_t duration;
    /// Maximum number of extended advertising events
    uint8_t max_ext_adv_evt;
} hci_ext_adv_set_en_param_t;

///HCI LE Set Extended Advertising Enable Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_ext_adv_en_cmd
{
    /// Enable
    uint8_t enable;
    /// Number of sets (1 - 0x3F)
    uint8_t nb_sets;
    /// Advertising Set enable parameters
    struct hci_ext_adv_set_en_param sets[BLE_ACTIVITY_MAX];
} hci_le_set_ext_adv_en_cmd_t;
#endif //BLE_EMB_PRESENT || BLE_HOST_PRESENT

///HCI LE Read Maximum Advertising Data Length Command complete event
/*@TRACE*/
typedef struct hci_le_rd_max_adv_data_len_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Maximum advertising data length
    uint16_t max_adv_data_len;
} hci_le_rd_max_adv_data_len_cmd_cmp_evt_t;

///HCI LE Remove Advertising Set Command parameters structure
/*@TRACE
 * hci_le_rmv_adv_set_cmd = hci_le_rem_adv_set_cmd*/
typedef struct hci_le_rem_adv_set_cmd
{
    /// Advertising handle
    uint8_t adv_hdl;
} hci_le_rem_adv_set_cmd_t;

///HCI LE Read Number of Supported Advertising Sets Command complete event
/*@TRACE*/
typedef struct hci_le_rd_nb_supp_adv_sets_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Number of supported advertising sets
    uint8_t nb_supp_adv_sets;
} hci_le_rd_nb_supp_adv_sets_cmd_cmp_evt_t;

///HCI LE Read Transmit Power Command complete event
/*@TRACE*/
typedef struct hci_le_rd_tx_pwr_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Minimum TX Power
    int8_t min_tx_pwr;
    /// Maximum TX Power
    int8_t max_tx_pwr;
} hci_le_rd_tx_pwr_cmd_cmp_evt_t;

///HCI LE Read RF Path Compensation Command complete event
/*@TRACE*/
typedef struct hci_le_rd_rf_path_comp_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// RF TX Path Compensation
    int16_t tx_path_comp;
    /// RF RX Path Compensation
    int16_t rx_path_comp;
} hci_le_rd_rf_path_comp_cmd_cmp_evt_t;

///HCI LE Write RF Path Compensation Command complete event
/*@TRACE*/
typedef struct hci_le_wr_rf_path_comp_cmd
{
    /// RF TX Path Compensation
    int16_t tx_path_comp;
    /// RF RX Path Compensation
    int16_t rx_path_comp;
} hci_le_wr_rf_path_comp_cmd_t;

#if (BLE_PER_ADV)
///HCI LE Set Periodic Advertising Parameters V1 Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_per_adv_param_v1_cmd
{
    /// Advertising handle
    uint8_t adv_hdl;
    /// Minimum advertising interval for periodic advertising (units of 1.25 ms)
    uint16_t adv_intv_min;
    /// Maximum advertising interval for periodic advertising (units of 1.25 ms)
    uint16_t adv_intv_max;
    /// Advertising properties
    uint16_t adv_prop;
} hci_le_set_per_adv_param_v1_cmd_t;

#if (BLE_PAWR)
///HCI LE Set Periodic Advertising Parameters V2 Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_per_adv_param_v2_cmd
{
    /// Advertising handle
    uint8_t adv_hdl;
    /// Minimum advertising interval for periodic advertising (units of 1.25 ms)
    uint16_t adv_intv_min;
    /// Maximum advertising interval for periodic advertising (units of 1.25 ms)
    uint16_t adv_intv_max;
    /// Advertising properties
    uint16_t adv_prop;
    /// Num Subevents
    uint8_t num_subevt;
    /// Subevent interval (units of 1.25ms)
    uint8_t subevt_intv;
    /// Response slot delay (in unit of 1.25ms)
    uint8_t rsp_slot_delay;
    /// Response slot spacing (in unit of 0.125ms)
    uint8_t rsp_slot_spacing;
    /// Num response slots
    uint8_t num_rsp_slots;
} hci_le_set_per_adv_param_v2_cmd_t;

/// Default HCI Set Periodic Advertising Parameters Command is V2
#define hci_le_set_per_adv_param_cmd hci_le_set_per_adv_param_v2_cmd
#else //!(BLE_PAWR)
/// Default HCI Set Periodic Advertising Parameters Command is V1
#define hci_le_set_per_adv_param_cmd hci_le_set_per_adv_param_v1_cmd
#endif //!(BLE_PAWR)

///HCI LE Set Periodic Advertising Data Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_per_adv_data_cmd
{
    /// Advertising handle
    uint8_t adv_hdl;
    /**
     *  Operation
     *  0x00 Intermediate fragment of fragmented periodic advertising data
     *  0x01 First fragment of fragmented periodic advertising data
     *  0x02 Last fragment of fragmented periodic advertising data
     *  0x03 Complete periodic advertising data
     *  0x04 Unchanged data, just update advertising DID
     *  All other values Reserved for future use
     */
    uint8_t operation;
    /// Advertising Data Length (0-252 bytes)
    uint8_t data_len;
    /// Advertising data
    uint8_t data[__ARRAY_EMPTY];
} hci_le_set_per_adv_data_cmd_t;

///HCI LE Set Periodic Advertising Enable Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_per_adv_en_cmd
{
    /**
     *  Enable (@see enum per_adv_en)
     *  Bit number 0: Enable periodic advertising
     *  Bit number 1: Include ADI field in AUX_SYNC_IND PDUs (BT_53)
     *  All other bits: Reserved for future use
     */
    uint8_t enable;
    /// Advertising handle
    uint8_t adv_hdl;
} hci_le_set_per_adv_en_cmd_t;
#endif // (BLE_PER_ADV)

#if (BT_53)
///HCI LE Set Data Related Address Changes Command parameters structure
/*@TRACE*/
typedef struct hci_le_set_data_related_addr_chg_cmd
{
    /// Advertising handle
    uint8_t adv_hdl;
    /**
     *  Change reasons
     *  Bit number 0: Change the address whenever the advertising data changes
     *  Bit number 1: Change the address whenever the scan response data changes
     *  All other bits: Reserved for future use
     */
    uint8_t chg_reasons;
} hci_le_set_data_related_addr_chg_cmd_t;
#endif // (BT_53)
#if (BLE_SUBRATING)
/// HCI LE Set Default Subrate Comamnd parameters structure
/*@TRACE*/
typedef struct hci_le_set_dft_subrate_cmd
{
    /// Subrate min
    uint16_t subrate_min;
    /// Subrate max
    uint16_t subrate_max;
    /// Max latency (N x con intv)
    uint16_t max_latency;
    /// Continuation number (N x con evts)
    uint16_t cont_num;
    /// Supervision timeout (N x 10ms)
    uint16_t supv_to;
} hci_le_set_dft_subrate_cmd_t;

/// HCI LE Subrate Request command parameters structure
/*@TRACE*/
typedef struct hci_le_subrate_req_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Subrate min
    uint16_t subrate_min;
    /// Subrate max
    uint16_t subrate_max;
    /// Max latency (N x con intv)
    uint16_t max_latency;
    /// Continuation number (N x con evts)
    uint16_t cont_num;
    /// Supervision timeout (N x 10ms)
    uint16_t supv_to;
} hci_le_subrate_req_cmd_t;

/// HCI LE Subrate Change event
/*@TRACE */
typedef struct hci_le_subrate_chg_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Status
    uint8_t  status;
    /// Connection handle
    uint16_t conhdl;
    /// Subrate factor (N x con intv)
    uint16_t subrate_factor;
    /// Peripheral latency (N x con evts)
    uint16_t periph_latency;
    /// Continuation number (N x con evts)
    uint16_t cont_num;
    /// Supervision timeout (N x 10ms)
    uint16_t supv_to;
} hci_le_subrate_chg_evt_t;
#endif // (BLE_SUBRATING)

#if (BLE_PAWR)
/// HCI LE Set Periodic Advertising Subevent Data command parameters structure
/*@TRACE*/
typedef struct hci_le_set_per_adv_subevt_data_cmd
{
    /// Advertising handle (range 0x00 - 0xEF)
    uint8_t adv_hdl;
    /// Number of subevent data (range 0x01 - 0x0F)
    uint8_t num_subevt;
    /// Subevent index of data (range 0x00 - 0x7F)
    uint8_t subevt;
    /// First response slot to be used in the subevent
    uint8_t rsp_slot_start;
    /// Number of response slots to be used
    uint8_t rsp_slot_cnt;
    /// Subevent data length in octets (range 0 - 251)
    uint8_t subevt_data_len;
    /// Subevent data
    uint8_t subevt_data[__ARRAY_EMPTY];
} hci_le_set_per_adv_subevt_data_cmd_t;

/// HCI DBG LE Set Periodic Advertising Subevent Data command parameters structure
/*@TRACE*/
typedef struct hci_dbg_le_set_per_adv_subevt_data_cmd
{
    /// Action (disable, enable, one-shot)
    uint8_t action;
    /// Advertising handle (range 0x00 - 0xEF)
    uint8_t adv_hdl;
    /// Subevent index of data (range 0x00 - 0x7F)
    uint8_t subevt;
    /// First response slot to be used in the subevent
    uint8_t rsp_slot_start;
    /// Number of response slots to be used
    uint8_t rsp_slot_cnt;
    /// Subevent data length in octets (range 0 - 251)
    uint8_t subevt_data_len;
    /// Subevent data
    uint8_t subevt_data[__ARRAY_EMPTY];
} hci_dbg_le_set_per_adv_subevt_data_cmd_t;

/// HCI DBG LE Set Periodic Advertising Response Data command parameters structure
/*@TRACE*/
typedef struct hci_dbg_le_set_per_adv_rsp_data_cmd
{
    /// Action (disable, enable, one-shot)
    uint8_t action;
    /// Sync handle (range 0x0000 - 0x0EFF)
    uint16_t sync_hdl;
    /// Subevent index of data (range 0x00 - 0x7F)
    uint8_t subevt;
    /// Response slot to be used in the subevent
    uint8_t rsp_slot;
    /// Response data length in octets (range 0 - 251)
    uint8_t rsp_data_len;
    /// Response data
    uint8_t rsp_data[__ARRAY_EMPTY];
} hci_dbg_le_set_per_adv_rsp_data_cmd_t;

/// HCI LE Set Periodic Advertising Response Data command parameters structure
/*@TRACE*/
typedef struct hci_le_set_per_adv_rsp_data_cmd
{
    /// Sync handle (range 0x0000 - 0x0EFF)
    uint16_t sync_hdl;
    /// Request event
    uint16_t req_evt;
    /// Request subevent
    uint8_t req_subevt;
    /// Response subevent (range 0x00 - 0x7F)
    uint8_t rsp_subevt;
    /// Response slot (range 0x00 - 0x7F)
    uint8_t rsp_slot;
    /// Response data length in octets (range 0 - 251)
    uint8_t rsp_data_len;
    /// Response data
    uint8_t rsp_data[__ARRAY_EMPTY];
} hci_le_set_per_adv_rsp_data_cmd_t;

/// HCI LE Set Periodic Sync Subevent command parameters structure
/*@TRACE*/
typedef struct hci_le_set_per_sync_subevt_cmd
{
    /// Sync handle (range 0x0000 - 0x0EFF)
    uint16_t sync_hdl;
    /// Periodic Advertising Properties
    uint16_t per_adv_properties;
    /// Number of subevents (range 0x01 - 0x80)
    uint8_t num_subevt;
    /// Subevent to synchronize with (range 0x00 - 0x7F)
    uint8_t subevt[__ARRAY_EMPTY];
} hci_le_set_per_sync_subevt_cmd_t;
#endif // (BLE_PAWR)

#if (BT_60)
/// HCI LE Read All Remote Features command parameters structure
/*@TRACE*/
typedef struct hci_le_rd_all_rem_feats_cmd
{
    /// Connection handle (0x000 to 0xEFF - connection handle, 0xFFF - CS test)
    uint16_t conhdl;
    /// Highest numbered Page requested (range 0-10)
    uint8_t pages_req;
} hci_le_rd_all_rem_feats_cmd_t;

/// HCI LE Read All Remote Features complete event structure
/*@TRACE*/
typedef struct hci_le_rd_all_rem_feats_cmp_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Status
    uint8_t status;
    /// Connection handle
    uint16_t conhdl;
    /// Max Remote Page
    uint8_t max_rem_page;
    /// Max Valid Page
    uint8_t max_valid_page;
    /// LE Features
    le_all_ext_features_t feats;
} hci_le_rd_all_rem_feats_cmp_evt_t;

/// HCI LE Read All Local Supported Features command complete event structure
/*@TRACE*/
typedef struct hci_le_rd_all_loc_supp_feats_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Max Page
    uint8_t max_page;
    /// LE Features
    le_all_ext_features_t feats;
} hci_le_rd_all_loc_supp_feats_cmd_cmp_evt_t;
#endif // (BT_60)

#if (BLE_CHSD)
/// HCI LE CS Security Enable command parameters structure
/*@TRACE*/
typedef struct hci_le_cs_sec_en_cmd
{
    /// Connection handle (0x000 to 0xEFF - connection handle, 0xFFF - CS test)
    uint16_t conhdl;
} hci_le_cs_sec_en_cmd_t;

/// HCI LE CS Set Channel Classification command parameters structure
/*@TRACE*/
typedef struct hci_le_cs_set_ch_class_cmd
{
    /// Channel classification
    struct le_ch_class channel;
} hci_le_cs_set_ch_class_cmd_t;

/// HCI LE CS Write Cached Remote Supported Capabilities command parameters structure
/*@TRACE*/
typedef struct hci_le_cs_wr_cached_rem_supp_cap_cmd
{
    /// Connection handle (0x000 to 0xEFF - connection handle, 0xFFF - CS test)
    uint16_t conhdl;
    /// Number of CS configurations supported per connection (from 1 to 4)
    uint8_t nb_cfg;
    /// Maximum number of consecutive CS procedures supported
    uint16_t max_consecutive_proc;
    /// Number of antennas supported (from 1 to 4)
    uint8_t nb_antennas;
    /// Maximum number of antenna paths supported (from 1 to 4)
    uint8_t max_antenna_paths;
    /// Roles supported
    uint8_t roles_bf;
    /// Optional modes supported
    uint8_t opt_modes_bf;
    /// RTT capability
    uint8_t rtt_capa_bf;
    /// RTT AA only N
    uint8_t rtt_aa_only_n;
    /// RTT Sounding N
    uint8_t rtt_sounding_n;
    /// RTT Random Payload N
    uint8_t rtt_random_payload_n;
    /// Optional NADM Sounding Capability
    uint16_t opt_nadm_sounding_capa_bf;
    /// Optional NADM Random Capability
    uint16_t opt_nadm_random_capa_bf;
    /// Optional CS_SYNC PHYs supported
    uint8_t opt_cs_sync_phy_bf;
    /// Optional subfeatures supported
    uint16_t opt_subfeatures_bf;
    /// Optional T_IP1 times supported
    uint16_t opt_t_ip1_times_bf;
    /// Optional T_IP2 times supported
    uint16_t opt_t_ip2_times_bf;
    /// Optional T_FCS times supported
    uint16_t opt_t_fcs_times_bf;
    /// Optional T_PM times supported
    uint16_t opt_t_pm_times_bf;
    /// Time in microseconds for the antenna switch period of the CS tones
    uint8_t t_sw_us;
    /// Optional TX SNR capability
    uint8_t tx_snr_bf;
} hci_le_cs_wr_cached_rem_supp_cap_cmd_t;

/// HCI LE CS Read Remote Supported Capabilities command parameters structure
/*@TRACE*/
typedef struct hci_le_cs_rd_rem_supp_cap_cmd
{
    /// Connection handle (0x000 to 0xEFF - connection handle, 0xFFF - CS test)
    uint16_t conhdl;
} hci_le_cs_rd_rem_supp_cap_cmd_t;

/// HCI LE CS Set Default Setings command parameters structure
/*@TRACE*/
typedef struct hci_le_cs_set_dft_settings_cmd
{
    /// Connection handle (0x000 to 0xEFF - connection handle, 0xFFF - CS test)
    uint16_t conhdl;
    /// Roles enable
    uint8_t roles_bf;
    /// Antenna identifier to be used for CS_SYNC packets by the local Controller
    uint8_t cs_sync_antenna_sel;
    /// Maximum TX power to be used for all CS transmissions (from -127 to 20 dBm)
    int8_t max_tx_power_dbm;
} hci_le_cs_set_dft_settings_cmd_t;

/// HCI LE CS Read Remote FAE Table command parameters structure
/*@TRACE*/
typedef struct hci_le_cs_rd_rem_fae_table_cmd
{
    /// Connection handle (0x000 to 0xEFF - connection handle, 0xFFF - CS test)
    uint16_t conhdl;
} hci_le_cs_rd_rem_fae_table_cmd_t;

/// HCI LE CS Write Cached Remote FAE Table command parameters structure
/*@TRACE*/
typedef struct hci_le_cs_wr_cached_rem_fae_table_cmd
{
    /// Connection handle (0x000 to 0xEFF - connection handle, 0xFFF - CS test)
    uint16_t conhdl;
    /// Remote FAE Table - per-channel mode 0 Frequency Actuation Error table
    int8_t remote_fae_table[BLE_CHSD_NB_CHANNEL_MAX];
} hci_le_cs_wr_cached_rem_fae_table_cmd_t;

/// HCI LE CS Create Config command parameters structure
/*@TRACE*/
typedef struct hci_le_cs_create_config_cmd
{
    /// Connection handle (0x000 to 0xEFF - connection handle, 0xFFF - CS test)
    uint16_t conhdl;
    /// Config ID (from 0 to 3)
    uint8_t config_id;
    /// Create context
    uint8_t create_context;
    /// Main mode type
    uint8_t main_mode_type;
    /// Sub mode type
    uint8_t sub_mode_type;
    /// Minimum number of CS main mode steps to be executed before a submode step
    uint8_t min_main_mode_steps;
    /// Maximum number of CS main mode steps to be executed before a submode step
    uint8_t max_main_mode_steps;
    /// Number of main mode steps taken from the end of the last CS subevent to be repeated at the beginning of the
    /// current CS subevent directly after the last mode 0 step of that event
    uint8_t main_mode_repetition;
    /// Number of CS mode 0 steps to be included at the beginning of each CS subevent
    uint8_t mode_0_steps;
    /// Role
    uint8_t role;
    /// RTT type
    uint8_t rtt_type;
    /// CS_SYNC PHY (see @le_phy_chsd_value enumeration)
    uint8_t cs_sync_phy;
    /// Channel map
    struct le_ch_class channel_map;
    /// The number of times the map represented by the Channel_Map field is to be cycled through for non-mode 0 steps
    /// within a CS procedure
    uint8_t channel_map_repetition;
    /// Channel selection type
    uint8_t channel_selection_type;
    /// Ch3c shape
    uint8_t ch3c_shape;
    /// Ch3c jump (Number of channels skipped in each rising and falling sequence)
    uint8_t ch3c_jump;
    /// Reserved
    uint8_t reserved;
} hci_le_cs_create_config_cmd_t;

/// HCI LE CS Remove Config command parameters structure
/*@TRACE*/
typedef struct hci_le_cs_remove_config_cmd
{
    /// Connection handle (0x000 to 0xEFF - connection handle, 0xFFF - CS test)
    uint16_t conhdl;
    /// Config ID (from 0 to 3)
    uint8_t config_id;
} hci_le_cs_remove_config_cmd_t;

/// HCI LE CS Set Procedure Parameters command parameters structure
/*@TRACE*/
typedef struct hci_le_cs_set_proc_parameters_cmd
{
    /// Connection handle (0x000 to 0xEFF - connection handle, 0xFFF - CS test)
    uint16_t conhdl;
    /// Config ID (from 0 to 3)
    uint8_t config_id;
    /// Maximum duration for each CS procedure (in slots)
    uint16_t max_proc_len_slot;
    /// Minimum number of connection events between consecutive CS procedures
    uint16_t min_proc_intv;
    /// Maximum number of connection events between consecutive CS procedures
    uint16_t max_proc_intv;
    /// Maximum number of CS procedures to be scheduled
    uint16_t max_proc_cnt;
    /// Minimum suggested duration for each CS subevent in microseconds (from 1250us to 4s)
    uint32_t min_subevent_len_us;
    /// Maximum suggested duration for each CS subevent in microseconds (from 1250us to 4s)
    uint32_t max_subevent_len_us;
    /// Tone antenna configuration selection
    uint8_t tone_antenna_cfg;
    /// PHY
    uint8_t phy;
    /// TX power delta
    uint8_t tx_power_delta;
    /// Preferred peer antenna
    uint8_t pref_peer_antenna_bf;
    /// SNR control adjustment (Initiator)
    uint8_t snr_control_initiator;
    /// SNR control adjustment (Reflector)
    uint8_t snr_control_reflector;
} hci_le_cs_set_proc_parameters_cmd_t;

/// HCI LE CS Procedure Enable command parameters structure
/*@TRACE*/
typedef struct hci_le_cs_proc_en_cmd
{
    /// Connection handle (0x000 to 0xEFF - connection handle, 0xFFF - CS test)
    uint16_t conhdl;
    /// Config ID (from 0 to 3)
    uint8_t config_id;
    /// Enable or disable
    uint8_t enable;
} hci_le_cs_proc_en_cmd_t;

/// HCI LE CS Test command parameters structure
/*@TRACE*/
typedef struct hci_le_cs_test_cmd
{
    /// Main mode type (range 1-3)
    uint8_t main_mode_type;
    /// Sub mode type (range 1-3, or 0xFF unused)
    uint8_t sub_mode_type;
    /// Main mode repetition (range 0-3 steps)
    uint8_t main_mode_repetition;
    /// Mode 0 steps (range 1-3 steps)
    uint8_t mode0_steps;
    /// Role: 0=initiator / 1=reflector
    uint8_t role;
    /**
     *  RTT type
     *  0x00 RTT AA Only
     *  0x01 RTT with 32-bit sounding sequence
     *  0x02 RTT with 96-bit sounding sequence
     *  0x03 RTT with 32-bit random sequence
     *  0x04 RTT with 64-bit random sequence
     *  0x05 RTT with 96-bit random sequence
     *  0x06 RTT with 128-bit random sequence
     *  All other values Reserved for future use
     */
    uint8_t rtt_type;
    /// CS sync phy (range 1-2, @see enum le_phy_value)
    uint8_t cs_sync_phy;
    /// CS sync antenna selection (antenna id, range 1-4)
    uint8_t cs_sync_antenna_sel;
    /// Subevent length, in units of usecs (range 1250us-4s)
    uint32_t subevt_len;
    /// Subevent interval, in units of 625us slots
    uint16_t subevt_intv;
    /// Maximum allowed number of subevents in the procedure
    uint8_t max_subevts;
    /// Transmit power level in dBm (0x7E: minimum | 0x7F: maximum | range: -127 to +20)
    int8_t tx_pwr_lvl;
    /// T_IP1_Time (time between RTT packets), in usecs - specific values only (0x0A, 0x14, 0x1E, 0x28, 0x32, 0x3C, 0x50, 0x91), All other values RFU
    uint8_t t_ip1;
    /// T_IP2_Time (time between CS tones), in usecs - specific values only (0x0A, 0x14, 0x1E, 0x28, 0x32, 0x3C, 0x50, 0x91), All other values RFU
    uint8_t t_ip2;
    /// T_FCS_Time (frequency change time), in usecs - specific values only (0x0F, 0x14, 0x1E, 0x28, 0x32, 0x3C, 0x50, 0x64, 0x78, 0x96), All other values RFU
    uint8_t t_fcs;
    /// T_PM_Time (phase measurement period of CS tones), in usecs - specific values only (0x0A, 0x14, 0x28), All other values RFU
    uint8_t t_pm;
    /// T_SW_Time (antenna switch period of CS tones), in usecs - specific values only (0x00, 0x01, 0x02, 0x04, 0x0A), All other values RFU
    uint8_t t_sw;
    /**
     *  Tone antenna configuration
     *  0x00 Paths:1, DevA antennas:1, DevB antennas:1 => 1:1
     *  0x01 Paths:2, DevA antennas:2, DevB antennas:1 => 2:1
     *  0x02 Paths:3, DevA antennas:3, DevB antennas:1 => 3:1
     *  0x03 Paths:4, DevA antennas:4, DevB antennas:1 => 4:1
     *  0x04 Paths:2, DevA antennas:1, DevB antennas:2 => 1:2
     *  0x05 Paths:3, DevA antennas:1, DevB antennas:3 => 1:3
     *  0x06 Paths:4, DevA antennas:1, DevB antennas:4 => 1:4
     *  0x07 Paths:4, DevA antennas:2, DevB antennas:2 => 2:2
     *  All other values Reserved for future use
     */
    uint8_t tone_antenna_cfg;
    /// Reserved
    uint8_t reserved;
    /// SNR control adjustment for CS_SYNC transmissions on the initiator
    uint8_t snr_initiator;
    /// SNR control adjustment for CS_SYNC transmissions on the reflector
    uint8_t snr_reflector;
    /// DRBG nonce - determines octet 14 & 15 of the initial value of the DRBG nonce
    uint16_t drbg_nonce;
    /// Channel Map Repetition
    uint8_t channel_map_repetition;
    /**
     *  Override configuration - specifies which configurations determined by override param data:
     *  Bit 0 Channel sequence
     *  Bit 2 Num main mode steps per sub mode
     *  Bit 3 Tone extensions (for mode2/3)
     *  Bit 4 Antenna permutation index (for mode2/3)
     *  Bit 5 Access addresses
     *  Bit 6 SS marker positions (for mode1/3)
     *  Bit 7 SS marker value (for mode1/3)
     *  Bit 8 CS SYNC user payload (for mode1/3)
     *  Bit 10 stable phase test
     *  All other bits Reserved for future use
     */
    uint16_t ovr_config_flags;
    /// Override parameters length (in bytes)
    uint8_t ovr_param_len;
    /// Override parameters data (extracted by handler, based on override configuration)
    uint8_t ovr_param_data[__ARRAY_EMPTY];
} hci_le_cs_test_cmd_t;

#if (RW_DEBUG)
/// HCI DBG Test AES DRBG AA function commannd parameters structure
/*@TRACE*/
typedef struct hci_dbg_test_aes_drbg_aa_func_cmd
{
    /// DRBG Key, 16 octets ordered lsb-msb 15-0
    uint8_t k_drbg[AES_KEY_LEN];
    /// DRBG Nonce, 16 octets ordered lsb-msb 0-15
    uint8_t v_drbg[AES_DRBG_NONCE_LEN];
    /// Step Count
    uint8_t step_cnt;
} hci_dbg_test_aes_drbg_aa_func_cmd_t;

/// HCI DBG Test AES DRBG AA function commannd complete event structure
/*@TRACE*/
typedef struct hci_dbg_test_aes_drbg_aa_func_cmd_cmp_evt
{
    /// Initiator access address
    uint32_t aa_initiator;
    /// Reflector access address
    uint32_t aa_reflector;
} hci_dbg_test_aes_drbg_aa_func_cmd_cmp_evt_t;

/// HCI DBG Test AES DRBG CR1 function commannd parameters structure
/*@TRACE*/
typedef struct hci_dbg_test_aes_drbg_cr1_func_cmd
{
    /// DRBG Key, 16 octets ordered lsb-msb 15-0
    uint8_t k_drbg[AES_KEY_LEN];
    /// DRBG Nonce, 16 octets ordered lsb-msb 0-15
    uint8_t v_drbg[AES_DRBG_NONCE_LEN];
    /// Step Count
    uint8_t step_cnt;
    /// Transaction ID
    uint8_t tid;
    /// Number of channels
    uint8_t nb_ch;
    /// Channel Array
    uint8_t channel_array[__ARRAY_EMPTY];
} hci_dbg_test_aes_drbg_cr1_func_cmd_t;

/// HCI DBG Test AES DRBG CR1 function commannd complete event structure
/*@TRACE*/
typedef struct hci_dbg_test_aes_drbg_cr1_func_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Number of channels
    uint8_t nb_ch;
    /// Shuffled Channel Array
    uint8_t shuffled_channel_array[__ARRAY_EMPTY];
} hci_dbg_test_aes_drbg_cr1_func_cmd_cmp_evt_t;

/// HCI DBG Test AES DRBG H9 function commannd parameters structure
/*@TRACE*/
typedef struct hci_dbg_test_aes_drbg_h9_func_cmd
{
    /// Initialization vector, 16 octets ordered lsb-msb 15-0
    uint8_t cs_iv[AES_DRBG_CS_IV_LEN];
    /// Initialization nonce, 8 octets ordered lsb-msb 7-0
    uint8_t cs_in[AES_DRBG_CS_IN_LEN];
    /// Personalization vector, 16 octets ordered lsb-msb 15-0
    uint8_t cs_pv[AES_DRBG_CS_PV_LEN];
} hci_dbg_test_aes_drbg_h9_func_cmd_t;

/// HCI DBG Test AES DRBG H9 function commannd complete event structure
/*@TRACE*/
typedef struct hci_dbg_test_aes_drbg_h9_func_cmd_cmp_evt
{
    /// DRBG Key, 16 octets ordered lsb-msb 15-0
    uint8_t k_drbg[AES_KEY_LEN];
    /// DRBG Nonce, 16 octets ordered lsb-msb 0-15
    uint8_t v_drbg[AES_DRBG_NONCE_LEN];
} hci_dbg_test_aes_drbg_h9_func_cmd_cmp_evt_t;
#endif //(RW_DEBUG)
#endif //(BLE_CHSD)

#if (BLE_FSU)
/// HCI LE Frame Space Update command parameters structure
/*@TRACE*/
typedef struct hci_le_frame_space_upd_cmd
{
    /// Connection handle (0x000 to 0xEFF)
    uint16_t conhdl;
    /// The minimum requested frame space value, in microseconds (0x0000 to 0x2710)
    uint16_t frame_space_min;
    /// The maximum requested frame space value, in microseconds (0x0000 to 0x2710)
    uint16_t frame_space_max;
    /// PHYS (bit field, see enum le_phy_mask)
    uint8_t phys;
    /// Spacing_Types (bit field, see enum fsu_spacing_types)
    uint16_t spacing_types;
} hci_le_frame_space_upd_cmd_t;
#endif //(BLE_FSU)

#if (BLE_ENH_TEST_MODE)
/// HCI LE Enable OTA UTP Mode command parameters structure
/*@TRACE*/
typedef struct hci_le_en_ota_utp_mode_cmd
{
    /// Enable Test Mode
    uint8_t en;
} hci_le_en_ota_utp_mode_cmd_t;

/// HCI LE UTP Send command parameters structure
/*@TRACE*/
typedef struct hci_le_utp_send_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Data Length
    uint8_t len;
    /// Data
    uint8_t data[__ARRAY_EMPTY];
} hci_le_utp_send_cmd_t;

/// HCI LE UTP receive event
/*@TRACE*/
typedef struct hci_le_utp_receive_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Connection handle
    uint16_t conhdl;
    /// Data Length
    uint8_t len;
    /// Data
    uint8_t data[__ARRAY_EMPTY];
} hci_le_utp_receive_evt_t;
#endif // (BLE_ENH_TEST_MODE)

///HCI LE Advertising Set Terminated event
/*@TRACE
 * hci_le_adv_set_terminated_evt = hci_le_adv_set_term_evt*/
typedef struct hci_le_adv_set_term_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Status
    uint8_t status;
    /// Advertising handle
    uint8_t adv_hdl;
    /// Connection handle
    uint16_t conhdl;
    /// Num_Completed_Extended_Advertising_Events
    uint8_t nb_cmp_ext_adv_evt;
} hci_le_adv_set_term_evt_t;

///HCI LE Scan Request Received event
/*@TRACE*/
typedef struct hci_le_scan_req_rcvd_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Advertising handle
    uint8_t adv_hdl;
    /// Scanner address type:  public=0 / random=1 / rpa_or_pub=2 / rpa_or_rnd=3
    uint8_t scan_addr_type;
    /// Scanner address
    struct bd_addr scan_addr;
} hci_le_scan_req_rcvd_evt_t;

#if (BLE_PER_ADV)
///HCI LE Read Periodic Advertiser List Size Command complete event
/*@TRACE*/
typedef struct hci_le_rd_per_adv_list_size_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    /// Periodic Advertiser List Size
    uint8_t size;
} hci_le_rd_per_adv_list_size_cmd_cmp_evt_t;
#endif // (BLE_PER_ADV)

#if (BLE_PAWR)
///HCI LE Set Periodic Advertising Parameters V2 Command complete event strucutre
/*@TRACE*/
typedef struct hci_le_set_per_adv_param_v2_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Advertising handle
    uint8_t adv_hdl;
} hci_le_set_per_adv_param_v2_cmd_cmp_evt_t;

///HCI LE Periodic Advertising Subevent Data Request event strucutre
/*@TRACE*/
typedef struct hci_le_per_adv_subevt_data_req_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Advertising handle (range 0x00 - 0xEF)
    uint8_t adv_hdl;
    /// First subevent that data is requested for (range 0x00 - 0x7F)
    uint8_t subevt_start;
    /// Number of subevents that data is requested for (range 0x01 - 0x80)
    uint8_t subevt_data_cnt;
} hci_le_per_adv_subevt_data_req_evt_t;

///HCI LE Periodic Advertising Response Report event strucutre
/*@TRACE*/
typedef struct hci_le_per_adv_rsp_report_evt
{
    /// LE Subevent code
    uint8_t  subcode;
    /// Advertising handle (range 0x00 - 0xEF)
    uint8_t adv_hdl;
    /// Subevent number (range 0x00 - 0x7F)
    uint8_t subevt;
    /// Tx status (0x00 transmitted, 0x01 not transmitted)
    uint8_t tx_status;
    /// Number of responses (range 0x00 - 0x19)
    uint8_t num_rsp;
    /// Tx power in dBm (range -127 - 20)
    uint8_t tx_pwr;
    /// RSSI in dBm (range -127 - 20, 0x7F not available)
    uint8_t rssi;
    /// CTE Type (0x00 AoA, 0x01 AoA 1 us slots, 0x02 AoA 2 us slots, oxFF no CTE)
    uint8_t cte_type;
    /// Response slot the data was received in (range 0x00 - 0xFF)
    uint8_t rsp_slot;
    /// Data status (0x00 Data complete, 0x01 Data incomplete, 0xFF Failed to receive)
    uint8_t data_status;
    /// Data length in octets
    uint8_t data_len;
    /// Data
    uint8_t data[PAWR_MAX_RSP_REPORT_DATA_LEN];
} hci_le_per_adv_rsp_report_evt_t;
#endif // (BLE_PAWR)

#if (BLE_CHSD)
/// HCI LE CS Read Remote Supported Capabilities Complete event
/*@TRACE*/
typedef struct hci_le_cs_rd_remote_capa_cmp_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Status
    uint8_t status;
    /// Connection handle
    uint16_t conhdl;
    /// Number of CS configurations supported per connection (from 1 to 4)
    uint8_t nb_cfg;
    /// Maximum number of consecutive CS procedures supported
    uint16_t max_consecutive_proc;
    /// Number of antennas supported (from 1 to 4)
    uint8_t nb_antennas;
    /// Maximum number of antenna paths supported (from 1 to 4)
    uint8_t max_antenna_paths;
    /// Roles supported
    uint8_t roles_bf;
    /// Optional modes supported
    uint8_t opt_modes_bf;
    /// RTT capability
    uint8_t rtt_capa_bf;
    /// RTT AA only N
    uint8_t rtt_aa_only_n;
    /// RTT Sounding N
    uint8_t rtt_sounding_n;
    /// RTT Random Payload N
    uint8_t rtt_random_payload_n;
    /// Optional NADM Sounding Capability
    uint16_t opt_nadm_sounding_capa_bf;
    /// Optional NADM Random Capability
    uint16_t opt_nadm_random_capa_bf;
    /// Optional CS_SYNC PHYs supported
    uint8_t opt_cs_sync_phy_bf;
    /// Optional subfeatures supported
    uint16_t opt_subfeatures_bf;
    /// Optional T_IP1 times supported
    uint16_t opt_t_ip1_times_bf;
    /// Optional T_IP2 times supported
    uint16_t opt_t_ip2_times_bf;
    /// Optional T_FCS times supported
    uint16_t opt_t_fcs_times_bf;
    /// Optional T_PM times supported
    uint16_t opt_t_pm_times_bf;
    /// Time in microseconds for the antenna switch period of the CS tones
    uint8_t t_sw_us;
    /// Optional TX SNR Capability
    uint8_t tx_snr_capa_bf;
} hci_le_cs_rd_remote_capa_cmp_evt_t;

/// HCI LE CS Read Local Supported Capabilities command Complete event
/*@TRACE*/
typedef struct hci_le_cs_rd_local_capa_cmd_cmp_evt
{
    /// Status
    uint8_t status;
    /// Number of CS configurations supported per connection (from 1 to 4)
    uint8_t nb_cfg;
    /// Maximum number of consecutive CS procedures supported
    uint16_t max_consecutive_proc;
    /// Number of antennas supported (from 1 to 4)
    uint8_t nb_antennas;
    /// Maximum number of antenna paths supported (from 1 to 4)
    uint8_t max_antenna_paths;
    /// Roles supported
    uint8_t roles_bf;
    /// Optional modes supported
    uint8_t opt_modes_bf;
    /// RTT capability
    uint8_t rtt_capa_bf;
    /// RTT AA only N
    uint8_t rtt_aa_only_n;
    /// RTT Sounding N
    uint8_t rtt_sounding_n;
    /// RTT Random Payload N
    uint8_t rtt_random_payload_n;
    /// Optional NADM Sounding Capability
    uint16_t opt_nadm_sounding_capa_bf;
    /// Optional NADM Random Capability
    uint16_t opt_nadm_random_capa_bf;
    /// Optional CS_SYNC PHYs supported
    uint8_t opt_cs_sync_phy_bf;
    /// Optional subfeatures supported
    uint16_t opt_subfeatures_bf;
    /// Optional T_IP1 times supported
    uint16_t opt_t_ip1_times_bf;
    /// Optional T_IP2 times supported
    uint16_t opt_t_ip2_times_bf;
    /// Optional T_FCS times supported
    uint16_t opt_t_fcs_times_bf;
    /// Optional T_PM times supported
    uint16_t opt_t_pm_times_bf;
    /// Time in microseconds for the antenna switch period of the CS tones
    uint8_t t_sw_us;
    /// Optional TX SNR Capability
    uint8_t tx_snr_capa_bf;
} hci_le_cs_rd_local_capa_cmd_cmp_evt_t;

/// HCI LE CS Read Remote FAE Table Complete event
/*@TRACE*/
typedef struct hci_le_cs_rd_remote_fae_table_cmp_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Status
    uint8_t status;
    /// Connection handle
    uint16_t conhdl;
    /// Remote FAE table
    int8_t remote_fae_table[BLE_CHSD_NB_CHANNEL_MAX];
} hci_le_cs_rd_remote_fae_table_cmp_evt_t;

/// HCI LE CS Security Enable Complete event
/*@TRACE*/
typedef struct hci_le_cs_security_enable_cmp_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Status
    uint8_t status;
    /// Connection handle
    uint16_t conhdl;
} hci_le_cs_security_enable_cmp_evt_t;

/// HCI LE CS Config Complete event
/*@TRACE*/
typedef struct hci_le_cs_config_cmp_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Status
    uint8_t status;
    /// Connection handle (0x000 to 0xEFF - connection handle, 0xFFF - CS test)
    uint16_t conhdl;
    /// Config ID (from 0 to 3)
    uint8_t config_id;
    /// Action
    uint8_t action;
    /// Main mode type
    uint8_t main_mode_type;
    /// Sub mode type
    uint8_t sub_mode_type;
    /// Minimum number of CS main mode steps to be executed before a submode step
    uint8_t min_main_mode_steps;
    /// Maximum number of CS main mode steps to be executed before a submode step
    uint8_t max_main_mode_steps;
    /// Number of main mode steps taken from the end of the last CS subevent to be repeated at the beginning of the
    /// current CS subevent directly after the last mode 0 step of that event
    uint8_t main_mode_repetition;
    /// Number of CS mode 0 steps to be included at the beginning of each CS subevent
    uint8_t mode_0_steps;
    /// Role
    uint8_t role;
    /// RTT type
    uint8_t rtt_type;
    /// CS_SYNC PHY
    uint8_t cs_sync_phy;
    /// Channel map
    struct le_ch_class channel_map;
    /// The number of times the map represented by the Channel_Map field is to be cycled through for non-mode 0 steps
    /// within a CS procedure
    uint8_t channel_map_repetition;
    /// Channel selection type
    uint8_t channel_selection_type;
    /// Ch3c shape
    uint8_t ch3c_shape;
    /// Ch3c jump (Number of channels skipped in each rising and falling sequence)
    uint8_t ch3c_jump;
    /// Reserved
    uint8_t reserved;
    /// Interlude time in microseconds between the RTT packets
    uint8_t t_ip1_us;
    /// Interlude time in microseconds between the CS tones
    uint8_t t_ip2_us;
    /// Time in microseconds for frequency changes
    uint8_t t_fcs_us;
    /// Time in microseconds for the phase measurement period of the CS tones
    uint8_t t_pm_us;
} hci_le_cs_config_cmp_evt_t;

/// HCI LE CS Procedure Enable Complete event
/*@TRACE*/
typedef struct hci_le_cs_proc_en_cmp_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Status
    uint8_t status;
    /// Connection handle
    uint16_t conhdl;
    /// Config ID (from 0 to 3)
    uint8_t config_id;
    /// State
    uint8_t state;
    /// Antenna configuration index (from 0 to 7)
    uint8_t tone_antenna_cfg;
    /// Selected TX Power (from -127 to 20 dBm)
    int8_t tx_power_dbm;
    /// Duration for each CS subevent in microseconds (from 1250us to 4s)
    uint32_t subevent_len_us;
    /// Number of CS subevents anchored off the same ACL connection event (from 1 to 16)
    uint8_t subevents_per_event;
    /// Time between consecutive CS subevents anchored off the same ACL connection event (in slots)
    uint16_t subevent_intv_slot;
    /// Number of ACL connection events between consecutive CS event anchor points
    uint16_t event_intv;
    /// Number of ACL connection events between consecutive CS procedure anchor points
    uint16_t proc_intv;
    /// Number of CS procedures to be scheduled
    uint16_t proc_cnt;
} hci_le_cs_proc_en_cmp_evt_t;

/// Description of CS result steps
/*@TRACE*/
struct hci_le_cs_step_result
{
    /// Step mode
    uint8_t mode;
    /// Step channel
    uint8_t channel;
    /// Step data length
    uint8_t len;
    /// Step data
    uint8_t data[__ARRAY_EMPTY];
};

/// HCI LE CS Subevent Result event
/*@TRACE*/
typedef struct hci_le_cs_subevt_result_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Connection handle
    uint16_t conhdl;
    /// Config ID
    uint8_t config_id;
    /// Starting ACL connection event count
    uint16_t start_acl_con_evt_cnt;
    /// Procedure counter
    uint16_t proc_cnt;
    /// Frequency compensation
    int16_t freq_comp;
    /// Reference power level
    int8_t ref_pwr;
    /// Procedure done status
    uint8_t proc_status;
    /// Subevent done status
    uint8_t subevt_status;
    /// Abort reason
    uint8_t abort_reason;
    /// Number of antenna paths
    uint8_t num_ant_path;
    /// Number of steps reported
    uint8_t num_steps;
    /// Step results @see struct hci_le_cs_step_result
    uint8_t step_data[HCI_LE_CS_SUBEVENT_RESULT_MAX_STEP_DATA];
} hci_le_cs_subevt_result_evt_t;

/// HCI LE CS Subevent Result Continue event
/*@TRACE*/
typedef struct hci_le_cs_subevt_result_cont_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Connection handle
    uint16_t conhdl;
    /// Config ID
    uint8_t config_id;
    /// Procedure done status
    uint8_t proc_status;
    /// Subevent done status
    uint8_t subevt_status;
    /// Abort reason
    uint8_t abort_reason;
    /// Number of antenna paths
    uint8_t num_ant_path;
    /// Number of steps reported
    uint8_t num_steps;
    /// Step results @see struct hci_le_cs_step_result
    uint8_t step_data[HCI_LE_CS_SUBEVENT_RESULT_CONT_MAX_STEP_DATA];
} hci_le_cs_subevt_result_cont_evt_t;

/// HCI LE CS Test End Complete event
/*@TRACE*/
typedef struct hci_le_cs_test_end_cmp_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Status
    uint8_t status;
} hci_le_cs_test_end_cmp_evt_t;
#endif //(BLE_CHSD)

#if (BLE_FSU)
/// HCI LE Frame Space Update Complete event
/*@TRACE*/
typedef struct hci_le_frame_space_upd_cmp_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Status
    uint8_t status;
    /// Connection handle
    uint16_t conhdl;
    /// Initiator (see enum fsu_initiator)
    uint8_t initiator;
    /// Frame_Space (in us, range: 0x0000 to 0x2710)
    uint16_t frame_space;
    /// PHYS (bit field, see enum le_phy_mask)
    uint8_t phys;
    /// Spacing_Types (bit field, see enum fsu_spacing_types)
    uint16_t spacing_types;

} hci_le_frame_space_upd_cmp_evt_t;
#endif // (BLE_FSU)

#if (BLE_HOST_PRESENT && BLE_MONITOR_ADV)
/// HCI LE Monitored Advertisers List read size Complete event
/*@TRACE*/
typedef struct
{
    /// LE Subevent code
    uint8_t subcode;
    /// Status
    uint8_t status;
    /// Number of entries inside the Monitored Advertisers List
    uint8_t number;

} hci_le_mal_read_size_cmd_cmp_evt_t;
#endif // (BLE_HOST_PRESENT && BLE_MONITOR_ADV)

///HCI LE Channel Selection Algorithm event
/*@TRACE*/
typedef struct hci_le_ch_sel_algo_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Connection handle
    uint16_t conhdl;
    /// Channel selection algorithm
    uint8_t ch_sel_algo;
} hci_le_ch_sel_algo_evt_t;

/// HCI LE read PHY Command complete event
/*@TRACE*/
typedef struct hci_le_rd_phy_cmd_cmp_evt
{
    ///Status of received command
    uint8_t status;
    /// Connection Handle
    uint16_t conhdl;
    /// Current configured PHY for TX
    //@trc_ref le_phy_value
    uint8_t tx_phy;
    /// Current configured PHY for RX
    //@trc_ref le_phy_value
    uint8_t rx_phy;
} hci_le_rd_phy_cmd_cmp_evt_t;

/// HCI LE PHY Update Complete event
/*@TRACE*/
typedef struct hci_le_phy_upd_cmp_evt
{
    ///LE Subevent code
    uint8_t subcode;
    ///Status of received command
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///TX phy chosen
    //@trc_ref le_phy_value
    uint8_t tx_phy;
    ///RX phy chosen
    //@trc_ref le_phy_value
    uint8_t rx_phy;
} hci_le_phy_upd_cmp_evt_t;

#if (BLE_CONLESS_CTE_RX)
///HCI LE Connectionless IQ Report event
/*@TRACE*/
typedef struct hci_le_conless_iq_report_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Sync handle
    uint16_t sync_hdl;
    /// Data channel index
    uint8_t channel_idx;
    /// RSSI (in 0.1 dBm)
    int16_t rssi;
    /// RSSI antenna ID
    uint8_t rssi_antenna_id;
    /// CTE type (0: AOA | 1: AOD-1us | 2: AOD-2us)
    uint8_t cte_type;
    /// Slot durations (1: 1us | 2: 2us)
    uint8_t slot_dur;
    /// Packet status
    uint8_t pkt_status;
    /// paEventCounter
    uint16_t pa_evt_cnt;
    /// Sample_count
    uint8_t sample_cnt;
    /// I/Q sample
    struct
    {
        /// I sample
        int8_t i;
        /// Q sample
        int8_t q;
    } iq_sample[MAX_SAMPLE_CNT];
} hci_le_conless_iq_report_evt_t;
#endif // (BLE_CONLESS_CTE_RX)

#if (BLE_CON_CTE_REQ)
///HCI LE Connection IQ Report event
/*@TRACE*/
typedef struct hci_le_con_iq_report_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Connection handle
    uint16_t conhdl;
    /// Rx PHY  (@see enum le_phy_value)
    uint8_t rx_phy;
    /// Data channel index
    uint8_t data_channel_idx;
    /// RSSI (in 0.1 dBm)
    int16_t rssi;
    /// RSSI antenna ID
    uint8_t rssi_antenna_id;
    /// CTE type (0: AOA | 1: AOD-1us | 2: AOD-2us)
    uint8_t cte_type;
    /// Slot durations (1: 1us | 2: 2us)
    uint8_t slot_dur;
    /// Packet status
    uint8_t pkt_status;
    /// connEventCounter
    uint16_t con_evt_cnt;
    /// Sample_count
    uint8_t sample_cnt;
    /// I/Q sample
    struct
    {
        /// I sample
        int8_t i;
        /// Q sample
        int8_t q;
    } iq_sample[MAX_SAMPLE_CNT];
} hci_le_con_iq_report_evt_t;

///HCI LE CTE Request Failed event
/*@TRACE*/
typedef struct hci_le_cte_req_failed_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// Status
    uint8_t status;
    /// Connection handle
    uint16_t conhdl;
} hci_le_cte_req_failed_evt_t;
#endif // (BLE_CON_CTE_REQ)

///Commands Opcodes: OGF(6b) | OCF(10b)
/* Some Abbreviation used in names:
 *  - LK   = Link Key
 *  - RD   = Read
 *  - WR   = Write
 *  - REM  = Remote
 *  - STG  = Settings
 *  - CON  = Connection
 *  - CHG  = Change
 *  - DFT  = Default
 *  - PER  = Periodic
 *  - CHAN = Channel
 *  - REQ  = Request
 */

/// Used to read the maximum size of the data portion of ACL data packets and isochronous data packets
/// no parameter on command, only need to specify returned parameters
/*@TRACE*/
typedef struct hci_le_rd_buf_size_v2_cmd_cmp_evt
{
    /// 0x00 - Request succeed ; 0x01-0xFF Failed reason
    uint8_t status;
    /// Maximum length (in octets) of the data portion of each HCI ACL (0x0000: No dedicated LE buffer, 0x0001-0x001A RFU, Range 0x001B-0xFFFF)
    uint16_t le_acl_data_packet_length;
    /// The total number of HCI ACL data packets that can be stored in the data buffers of the Controller (0x00: No dedicated LE buffer, Range 0x01-0xFF)
    uint8_t total_num_le_acl_data_packets;
    /// The maximum length (in octets) of the data portion of each HCI ISO data packet (0x0000: No dedicated ISO buffer, Range 0x0001-0xFFFF)
    uint16_t iso_data_packet_length;
    /// The total number of HCI ISO data packets that can be stored in the ISO buffers of the Controller (0x00: No dedicated ISO buffer, Range 0x01-0xFF)
    uint8_t total_num_iso_data_packets;
} hci_le_rd_buf_size_v2_cmd_cmp_evt_t;

#if (BLE_HOST_PRESENT && BLE_MONITOR_ADV)
///HCI LE Monitored Advertisers List Report event
typedef struct
{
    /// Subevent code
    uint8_t subevent_code;
    /// Address type - 0=public/1=random
    uint8_t addr_type;
    /// BD address
    struct bd_addr addr;
    /// Condition
    uint8_t condition;
} hci_le_mal_report_evt_t;
#endif // (BLE_HOST_PRESENT && BLE_MONITOR_ADV)

#if (BLE_CIS || BLE_BIS)
/// Command Complete event of HCI_LE_RD_ISO_TX_SYNC_CMD
/*@TRACE*/
typedef struct hci_le_rd_iso_tx_sync_cmd_cmp_evt
{
    /// 0x00 - Command succeeded ; 0x01-0xFF Failed reason
    uint8_t status;
    /// Connection handle of the CIS or BIS (12 bits meaningful - Range 0x0000-0x0EFF)
    uint16_t conhdl;
    /// The packet sequence number of an SDU (Range 0x0000-0xFFFF)
    uint16_t pkt_seq_nb;
    /// The time stamp, in microseconds, of the reference anchor point of a transmitted SDU (Range 0x00000000-0xFFFFFFFF)
    uint32_t time_stamp;
    /// The time offset, in microseconds, that is associated with a transmitted SDU (Range 0x000000-0xFFFFFF)
    uint32_t time_offset;
} hci_le_rd_iso_tx_sync_cmd_cmp_evt_t;
#endif // (BLE_CIS || BLE_BIS)

#if (BLE_CIS)
/// Description of CIS parameters
/*@TRACE*/
typedef struct hci_le_cis_param
{
    /// Used to identify a Connected Isochronous Stream. (Range 0x00-0xEF)
    uint8_t cis_id;
    /// Maximum size of an SDU in octets from the master Host (12 bits meaningful - Range 0x000-0xFFF).
    uint16_t max_sdu_m2s;
    /// Maximum size of an SDU in octets from the master Host (12 bits meaningful - Range 0x000-0xFFF).
    uint16_t max_sdu_s2m;
    /// Master to slave PHY, bit 0: 1Mbps, bit 1: 2Mbps, bit 2: LE-Coded
    uint8_t phy_m2s;
    /// Slave to master PHY, bit 0: 1Mbps, bit 1: 2Mbps, bit 2: LE-Coded
    uint8_t phy_s2m;
    /// Maximum number of times every CIS Data PDU should be retransmitted from the master to slave (range 0x00-0x0F)
    uint8_t rtn_m2s;
    /// Maximum number of times every CIS Data PDU should be retransmitted from the slave to master (range 0x00-0x0F)
    uint8_t rtn_s2m;
} hci_le_cis_param_t;

/// Used by a master Host to set the parameters of all Connected Isochronous Streams associated with a Connected
/// Isochronous Group in the Controller
/*@TRACE*/
typedef struct hci_le_set_cig_params_cmd
{
    /// Used to identify the Connected Isochronous Group (range 0x00 to 0xEF).
    uint8_t cig_id;
    /// Interval in microseconds of periodic SDUs master to slave (20 bits meaningful Range 0x000FF - 0xFFFFF).
    uint32_t sdu_interval_m2s;
    /// Interval in microseconds of periodic SDUs slave to master (20 bits meaningful Range 0x000FF - 0xFFFFF).
    uint32_t sdu_interval_s2m;
    /// Slave Clock Accuracy:
    /// 0x00: 251 ppm to 500 ppm,  0x01: 151 ppm to 250 ppm, 0x02: 101 ppm to 150 ppm, 0x03: 76 ppm to 100 ppm
    /// 0x04:  51 ppm to  75 ppm,  0x05:  31 ppm to  50 ppm, 0x06:  21 ppm to  30 ppm, 0x07:  0 ppm to  20 ppm
    uint8_t sca;
    /// Scheduling method, 0: Sequential, 1: Interleaved
    uint8_t packing;
    /// ISOAL Framing mode, 0: Unframed, 1: Framed
    uint8_t framing;
    /// The maximum time in milliseconds for an SDU to be transported from the master Controller to slave Controller.
    /// (range 0x0005 to 0x0FA0)
    uint16_t max_trans_latency_m2s;
    /// The maximum time in milliseconds for an SDU to be transported from the slave Controller to master Controller.
    /// (range 0x0005 to 0x0FA0)
    uint16_t max_trans_latency_s2m;
    /// Total number of CISs in the CIG being added or modified. (Range 0x01-0x10)
    uint8_t cis_count;
    /// CIS Parameters
    struct hci_le_cis_param params[0x10];
} hci_le_set_cig_params_cmd_t;

/// Command Complete event of HCI_LE_SET_CIG_PARAMS_CMD
/*@TRACE*/
typedef struct hci_le_set_cig_params_cmd_cmp_evt
{
    /// 0x00 - The Broadcast Isochronous Group has been completed ; 0x01-0xFF Failed reason
    uint8_t status;
    /// Used to identify the Connected Isochronous Group. (Range 0x00-0xEF)
    uint8_t cig_id;
    /// Total number of CISs added or modified by this command (Range 0x01-0x10)
    uint8_t cis_count;
    /// List of connection handles of CISs in the CIG. The connection handle list order is the same as the list order
    /// of the CISs in the command  (Range 0x0000-0x0EFF)
    uint16_t conhdl[0x10];
} hci_le_set_cig_params_cmd_cmp_evt_t;

/// Description of CIS parameters
/*@TRACE*/
typedef struct hci_le_cis_test_param
{
    /// Used to identify a Connected Isochronous Stream. (Range 0x00-0xEF)
    uint8_t cis_id;
    /// Maximum number of subevents in each interval of Connected Isochronous Stream (Range 0x00-0x1F)
    uint8_t nse;
    /// Maximum size of an SDU in octets from the master Host (12 bits meaningful - Range 0x000-0xFFF).
    uint16_t max_sdu_m2s;
    /// Maximum size of an SDU in octets from the master Host (12 bits meaningful - Range 0x000-0xFFF).
    uint16_t max_sdu_s2m;
    /// Maximum size of payload from master Link Layer to slave Link Layer (Range 0x01-0xFB).
    uint16_t max_pdu_m2s;
    /// Maximum size of payload from slave Link Layer to master Link Layer (Range 0x01-0xFB).
    uint16_t max_pdu_s2m;
    /// Master to slave PHY, bit 0: 1Mbps, bit 1: 2Mbps, bit 2: LE-Coded
    uint8_t phy_m2s;
    /// Slave to master PHY, bit 0: 1Mbps, bit 1: 2Mbps, bit 2: LE-Coded
    uint8_t phy_s2m;
    /// The burst number for master to slave transmission (Range 0x00-0x0F).
    /// 0x00 - No isochronous data from the master to the slave
    uint8_t bn_m2s;
    /// The burst number for slave to master transmission (Range 0x00-0x0F).
    /// 0x00 - No isochronous data from the slave to the master
    uint8_t  bn_s2m;
} hci_le_cis_test_param_t;

/// Used by a master Host to set the parameters of all Connected Isochronous Streams associated with a Connected
/// Isochronous Group in the Controller
/*@TRACE*/
typedef struct hci_le_set_cig_params_test_cmd
{
    /// Used to identify the Connected Isochronous Group. (Range 0x00-0xEF)
    uint8_t cig_id;
    /// Interval in microseconds of periodic SDUs master to slave (20 bits meaningful Range 0x000FF - 0xFFFFF).
    uint32_t sdu_interval_m2s;
    /// Interval in microseconds of periodic SDUs slave to master (20 bits meaningful Range 0x000FF - 0xFFFFF).
    uint32_t sdu_interval_s2m;
    /// The maximum flush timeout for each payload from the master (Range 0x01-0x1F).
    uint8_t ft_m2s;
    /// The maximum flush timeout for each payload from the slave (Range 0x01-0x1F).
    uint8_t ft_s2m;
    /// The duration of the isochronous interval in multiples of 1.25 ms (Range 0x0004-0xC80).
    uint16_t iso_interval;
    /// Slave Clock Accuracy:
    /// 0x00: 251 ppm to 500 ppm,  0x01: 151 ppm to 250 ppm, 0x02: 101 ppm to 150 ppm, 0x03: 76 ppm to 100 ppm
    /// 0x04:  51 ppm to  75 ppm,  0x05:  31 ppm to  50 ppm, 0x06:  21 ppm to  30 ppm, 0x07:  0 ppm to  20 ppm
    uint8_t sca;
    /// Scheduling method, 0: Sequential, 1: Interleaved
    uint8_t packing;
    /// ISOAL Framing mode, 0: Unframed, 1: Framed
    uint8_t framing;
    /// Total number of CISs in the CIG being added or modified. (Range 0x00-0x10)
    uint8_t cis_count;
    /// CIS Parameters
    struct hci_le_cis_test_param params[0x10];
} hci_le_set_cig_params_test_cmd_t;

/// Command Complete event of HCI_LE_SET_CIG_PARAMS_TEST_CMD
/*@TRACE*/
typedef struct hci_le_set_cig_params_test_cmd_cmp_evt
{
    /// 0x00 - The Broadcast Isochronous Group has been completed ; 0x01-0xFF Failed reason
    uint8_t status;
    /// Used to identify the Connected Isochronous Group. (Range 0x00-0xEF)
    uint8_t cig_id;
    /// Total number of CISs added or modified by this command (Range 0x00-0x10)
    uint8_t cis_count;
    /// List of connection handles of CISs in the CIG. The connection handle list order is the same as the list order
    /// of the CISs in the command  (Range 0x0000-0x0EFF)
    uint16_t conhdl[0x10];
} hci_le_set_cig_params_test_cmd_cmp_evt_t;

/// Create CIS parameter
/*@TRACE*/
typedef struct hci_le_cis_create_param
{
    /// Connection handle of Connected Isochronous Stream (Range 0x0000-0x0EFF)
    uint16_t cis_conhdl;
    /// Connection handle of ACL Link (Range 0x0000-0x0EFF)
    uint16_t acl_conhdl;
} hci_le_cis_create_param_t;

/// Used by the master Host to establish one or more Connected Isochronous Streams using the connections identified
/// by the ACL_Connection_Handle
/*@TRACE*/
typedef struct hci_le_create_cis_cmd
{
    /// Total number of CISs to be established. (Range 0x01-0x1F)
    uint8_t cis_count;
    /// Create CIS Parameters
    struct hci_le_cis_create_param params[0x1F];
} hci_le_create_cis_cmd_t;

/// Removes all the Connected Isochronous Streams which have previously been set up in the Controller by using the
/// HCI_LE_Set_CIG_Parameters or the HCI_LE_Set_CIG_Parameters_Test command.
/*@TRACE*/
typedef struct hci_le_remove_cig_cmd
{
    /// Used to identify the Connected Isochronous Group. (Range 0x00-0xEF)
    uint8_t cig_id;
} hci_le_remove_cig_cmd_t;

/// Command Complete event of HCI_LE_REMOVE_CIG_CMD
/*@TRACE*/
typedef struct hci_le_remove_cig_cmd_cmp_evt
{
    /// 0x00 - The HCI_LE_Remove_CIG command succeeded ; 0x01-0xFF Failed reason
    uint8_t status;
    /// Identifier of a CIG. (Range 0x00-0xEF)
    uint8_t cig_id;
} hci_le_remove_cig_cmd_cmp_evt_t;

/// Used by the slave Host to inform the Controller to accept the request for the Connected Isochronous Stream that is
/// identified by the Connection_Handle
/*@TRACE*/
typedef struct hci_le_accept_cis_req_cmd
{
    /// Connection handle of the Connected Isochronous Stream (Range: 0x0000-0x0EFF)
    uint16_t conhdl;
} hci_le_accept_cis_req_cmd_t;

/// Used by the slave Host to reject the request for the Connected Isochronous Stream that is identified by the
/// Connection_Handle
/*@TRACE*/
typedef struct hci_le_reject_cis_req_cmd
{
    /// Connection handle of the Connected Isochronous Stream (Range: 0x0000-0x0EFF)
    uint16_t conhdl;
    /// The reason for rejecting the Connected Isochronous Stream
    uint8_t reason;
} hci_le_reject_cis_req_cmd_t;
#endif // (BLE_CIS)

#if (BLE_BIS)
/// Used to transmit data PDUs of one or more Broadcast Isochronous Streams of the Broadcast Isochronous Group
/*@TRACE*/
typedef struct hci_le_create_big_cmd
{
    /// BIG_Handle is used to identify the BIS Group. (Range 0x00-0xEF)
    uint8_t big_hdl;
    /// Used to identify the Periodic Advertising set (Range 0x00-0xEF)
    uint8_t adv_hdl;
    /// Total number of Broadcast Isochronous Streams in the Broadcast Isochronous Group. (Range 0x01-0x1F)
    uint8_t num_bis;
    /// Interval of periodic SDUs in microseconds (20 bits meaningful - range 0x000FF-0xFFFFF).
    uint32_t sdu_interval;
    /// Maximum size of an SDU (12 bits meaningful Range 0x000 to 0xFFF).
    uint16_t max_sdu;
    /// The maximum time in milliseconds for a SDU to be transported (Range 0x0000-0x0FA0)
    uint16_t trans_latency;
    /// Number of times every BIS Data PDU should be retransmitted (Range 0x00-0x0F)
    uint8_t rtn;
    /// PHY, bit 0: 1Mbps, bit 1: 2Mbps, bit 2: LE-Coded
    uint8_t phy;
    /// Scheduling method, 0: Sequential, 1: Interleaved
    uint8_t packing;
    /// ISOAL Framing mode, 0: Unframed, 1: Framed
    uint8_t framing;
    /// 0: Unencrypted, 1: Encrypted
    uint8_t encryption;
    /// Code used for deriving the session key for encrypting and decrypting payloads of BISs.
    uint8_t broadcast_code[KEY_LEN];
} hci_le_create_big_cmd_t;

/// Prepares the CL-Master Controller to set up a new BIG.
/*@TRACE*/
typedef struct hci_le_create_big_test_cmd
{
    /// BIG_Handle is used to identify the BIS Group. (Range 0x00-0xEF)
    uint8_t big_hdl;
    /// Used to identify the Periodic Advertising set (Range 0x00-0xEF)
    uint8_t adv_hdl;
    /// Total number of Broadcast Isochronous Streams in the Broadcast Isochronous Group. (Range 0x01-0x1F)
    uint8_t num_bis;
    /// Interval of periodic SDUs in microseconds (20 bits meaningful - range 0x000FF-0xFFFFF).
    uint32_t sdu_interval;
    /// Isochronous Interval in multiples of 1.25 ms (Range 0x0004-0xC80)
    uint16_t iso_interval;
    /// Total number of subevents in each interval of each BIS in the BIG (Range 0x01-0x1F)
    uint8_t nse;
    /// Maximum size of an SDU (12 bits meaningful Range 0x000 to 0xFFF).
    uint16_t max_sdu;
    /// Maximum size of payload (Range 0x01-0xFB)
    uint16_t max_pdu;
    /// PHY, bit 0: 1Mbps, bit 1: 2Mbps, bit 2: LE-Coded
    uint8_t phy;
    /// Scheduling method, 0: Sequential, 1: Interleaved
    uint8_t packing;
    /// ISOAL Framing mode, 0: Unframed, 1: Framed
    uint8_t framing;
    /// Number of new payloads in each interval for each Broadcast Isochronous Stream (Burst Number) (Range 0x01-0x07)
    uint8_t bn;
    /// Number of times the scheduled payload(s) should be transmitted (Range 0x01-0x0F)
    uint8_t irc;
    /// Isochronous Interval spacing of payloads transmitted in the Pre_Transmission_Subevents (Range 0x00-0x0F)
    uint8_t pto;
    /// 0: Unencrypted, 1: Encrypted
    uint8_t encryption;
    /// Code used for encrypting or decrypting payloads of an encrypted Broadcast Isochronous Stream
    uint8_t broadcast_code[KEY_LEN];
} hci_le_create_big_test_cmd_t;

/// Terminate transmission of all Broadcast Isochronous Streams of a Broadcast Isochronous Group or to cancel
/// the process of creating a Broadcast Isochronous Group
/*@TRACE*/
typedef struct hci_le_terminate_big_cmd
{
    /// BIG_Handle is used to identify the BIS Group. (Range 0x00-0xEF)
    uint8_t big_hdl;
    /// Termination reason
    uint8_t reason;
} hci_le_terminate_big_cmd_t;

/// Used to synchronize and receive PDUs from one or more Broadcast Isochronous Streams within a Broadcast Isochronous
/// Group.
/*@TRACE*/
typedef struct hci_le_big_create_sync_cmd
{
    /// BIG_Handle is used to identify the BIS Group. (Range 0x00-0xEF)
    uint8_t big_hdl;
    /// Used to identify the periodic advertising set (Range 0x0000-0x0EFF)
    uint16_t sync_hdl;
    /// 0: Unencrypted, 1: Encrypted
    uint8_t encryption;
    /// 128-bit code used for deriving the session key for decrypting payloads of encrypted BISs.
    uint8_t broadcast_code[16];
    /// Maximum number of subevents that should be used to receive data payloads in each isochronous interval (0x00-0xFF)
    /// 0x00 - The Controller can  schedule reception of any number of subevents up to NSE
    uint8_t mse;
    /// Synchronization timeout for a Broadcast Isochronous Group (step 10ms, range 0x000A-0x4000)
    uint16_t big_sync_timeout;
    /// Total number of BISs to synchronize. (Range 0x01-0x1F)
    uint8_t num_bis;
    /// List of indices of BISs (bis_id Range 0x01-0x1F)
    uint8_t bis_id[0x1F];
} hci_le_big_create_sync_cmd_t;

/// Stop synchronization with Broadcast Isochronous Streams or to cancel the process of synchronization to Broadcast Isochronous Streams
/*@TRACE*/
typedef struct hci_le_big_terminate_sync_cmd
{
    /// BIG_Handle is used to identify the BIS Group. (Range 0x00-0xEF)
    uint8_t big_hdl;
} hci_le_big_terminate_sync_cmd_t;

/// Command Complete event of HCI_LE_BIG_TERMINATE_SYNC_CMD
/*@TRACE*/
typedef struct hci_le_big_terminate_sync_cmd_cmp_evt
{
    /// 0x00 - Command succeeded ; 0x01-0xFF Failed reason
    uint8_t status;
    /// BIG_Handle is used to identify the BIS Group. (Range 0x00-0xEF)
    uint8_t big_hdl;
} hci_le_big_terminate_sync_cmd_cmp_evt_t;
#endif // (BLE_BIS)

/// Stop synchronization with Broadcast Isochronous Streams or to cancel the process of synchronization to Broadcast Isochronous Streams
/*@TRACE*/
typedef struct hci_le_req_peer_sca_cmd
{
    /// Connection handle of the CIS or BIS (12 bits meaningful - Range 0x0000-0x0EFF)
    uint16_t conhdl;
} hci_le_req_peer_sca_cmd_t;

#if (BLE_CIS || BLE_BIS)
/// The command is used to identify and create the isochronous data path between the Host and the Controller.
/// It can also be used to configure a codec for each data path.
/*@TRACE*/
typedef struct hci_le_setup_iso_data_path_cmd
{
    /// Connection handle of a CIS or BIS. (Range 0x0000-0x0EFF)
    uint16_t conhdl;
    /// Data path direction
    /// 0x00       Input (Host to Controller)
    /// 0x01       Output (Controller to Host)
    /// Others     Reserved for future use
    uint8_t data_path_direction;
    /// Data path ID
    /// 0x00       HCI
    /// 0x01-0xFE  Logical_Channel_Number. The meaning of the logical channel is vendor specific.
    /// 0xFF       Reserved for future use
    uint8_t data_path_id;
    /// Codec ID (5 octets)
    uint8_t codec_id[CODEC_ID_LEN];
    /// Controller delay in ms (Range: 0x000000 to 0x3D0900)
    uint32_t ctrl_delay;
    /// Codec configuration length
    uint8_t codec_cfg_len;
    /// Codec configuration
    uint8_t codec_cfg[__ARRAY_EMPTY];
} hci_le_setup_iso_data_path_cmd_t;

/// Command Complete event of HCI_LE_SETUP_ISO_DATA_PATH_CMD
/*@TRACE*/
typedef struct hci_le_setup_iso_data_path_cmd_cmp_evt
{
    /// 0x00 - Command succeeded ; 0x01-0xFF Failed reason
    uint8_t status;
    /// Connection handle of the CIS or BIS (12 bits meaningful - Range 0x0000-0x0EFF)
    uint16_t conhdl;
} hci_le_setup_iso_data_path_cmd_cmp_evt_t;

/// The command remove the data path between the Host and Controller and vice versa.
/*@TRACE*/
typedef struct hci_le_remove_iso_data_path_cmd
{
    /// Connection handle of a CIS or BIS. (Range 0x0000-0x0EFF)
    uint16_t conhdl;
    /// Data path direction
    /// bit 0      Input (Host to Controller)
    /// bit 1      Output (Controller to Host)
    /// Others     Reserved for future use
    uint8_t data_path_direction;
} hci_le_remove_iso_data_path_cmd_t;

/// Command Complete event of HCI_LE_REMOVE_ISO_DATA_PATH_CMD
/*@TRACE*/
typedef struct hci_le_remove_iso_data_path_cmd_cmp_evt
{
    /// 0x00 - Command succeeded ; 0x01-0xFF Failed reason
    uint8_t status;
    /// Connection handle of the CIS or BIS (12 bits meaningful - Range 0x0000-0x0EFF)
    uint16_t conhdl;
} hci_le_remove_iso_data_path_cmd_cmp_evt_t;

/// Used to configure an established CIS or BIS specified by the Connection_Handle parameter, and transmit test payloads which are generated by the Controller.
/*@TRACE*/
typedef struct hci_le_iso_tx_test_cmd
{
    /// Connection handle of the CIS or BIS (12 bits meaningful - Range 0x0000-0x0EFF)
    uint16_t conhdl;
    /// 0x00: Zero length payload 0x01: Variable length payload 0x02: Maximum length payload
    /// All other values: RFU
    uint8_t payload_type;
} hci_le_iso_tx_test_cmd_t;

/// Used to configure an established CIS or a synchronized BIG specified by the Connection_Handle parameter to receive payloads.
/*@TRACE*/
typedef struct hci_le_iso_rx_test_cmd
{
    /// Connection handle of the CIS or BIS. (Range 0x0000-0x0EFF)
    uint16_t conhdl;
    /// 0x00: Zero length payload 0x01: Variable length payload 0x02: Maximum length payload
    /// All other values: RFU
    uint8_t payload_type;
} hci_le_iso_rx_test_cmd_t;

/// Used to read the test counters in the Controller which are configured in ISO Receive Test mode for a CIS or BIS specified by the Connection_Handle.
/*@TRACE*/
typedef struct hci_le_iso_read_test_counters_cmd
{
    /// Connection handle of the CIS or BIS. (Range 0x0000-0x0EFF)
    uint16_t conhdl;
} hci_le_iso_read_test_counters_cmd_t;

/// Command Complete event of HCI_LE_ISO_READ_TEST_COUNTERS_CMD
/*@TRACE*/
typedef struct hci_le_iso_read_test_counters_cmd_cmp_evt
{
    /// 0x00 - Command succeeded ; 0x01-0xFF Failed reason
    uint8_t status;
    /// Connection handle of the CIS or BIS (12 bits meaningful - Range 0x0000-0x0EFF)
    uint16_t conhdl;
    /// Number in the Received_Packet_Count (0xXXXXXXXX)
    uint32_t received_packet_count;
    /// Number in the Missed_Packet_Count (0xXXXXXXXX)
    uint32_t missed_packet_count;
    /// Number in the Failed_Packet_Count (0xXXXXXXXX)
    uint32_t failed_packet_count;
} hci_le_iso_read_test_counters_cmd_cmp_evt_t;

/// Used to terminate the ISO Transmit and/or Receive Test mode for a CIS or BIS specified by the Connection_Handle parameter but does not terminate the CIS or BIS.
/*@TRACE*/
typedef struct hci_le_iso_test_end_cmd
{
    /// Connection handle of the CIS or BIS. (Range 0x0000-0x0EFF)
    uint16_t conhdl;
} hci_le_iso_test_end_cmd_t;

/// Command Complete event of HCI_LE_ISO_TEST_END_CMD
/*@TRACE*/
typedef struct hci_le_iso_test_end_cmd_cmp_evt
{
    /// 0x00 - Command succeeded ; 0x01-0xFF Failed reason
    uint8_t status;
    /// Connection handle of the CIS or BIS (12 bits meaningful - Range 0x0000-0x0EFF)
    uint16_t conhdl;
    /// Number in the Received_Packet_Count (0xXXXXXXXX)
    uint32_t received_packet_count;
    /// Number in the Missed_Packet_Count (0xXXXXXXXX)
    uint32_t missed_packet_count;
    /// Number in the Failed_Packet_Count (0xXXXXXXXX)
    uint32_t failed_packet_count;
} hci_le_iso_test_end_cmd_cmp_evt_t;

/// Returns the values of various counters related to link quality that are associated with the isochronous stream specified by the Connection_Handle parameter.
/*@TRACE*/
typedef struct hci_le_rd_iso_link_quality_cmd
{
    /// Connection handle of the CIS or BIS. (Range 0x0000-0x0EFF)
    uint16_t conhdl;
} hci_le_rd_iso_link_quality_cmd_t;

/// Set the margin between ISO sub-events.
/*@TRACE*/
struct hci_vs_iso_subevt_margin_set_cmd
{
    /// Sub-event margin between ISO sub-events, in units of us.
    uint32_t subevt_margin;
};
#endif // (BLE_CIS || BLE_BIS)

/// Used by the Host to set or clear a bit of the feature controlled by the Host in the Link Layer FeatureSet stored in the Controller.
typedef struct hci_le_set_host_feature_cmd
{
    /// Bit position in the FeatureSet. (Range 0x00-0x3F)
    uint8_t bit_number;
    /// Bit value. (0x00: feature disabled 0x01: feature enabled All other values: RFU)
    uint8_t bit_value;
} hci_le_set_host_feature_cmd_t;

#if (BLE_CIS || BLE_BIS)
/// Command Complete event of HCI_LE_RD_ISO_LINK_QUALITY_CMD
/*@TRACE*/
typedef struct hci_le_rd_iso_link_quality_cmd_cmp_evt
{
    /// 0x00 - Command succeeded ; 0x01-0xFF Failed reason
    uint8_t status;
    /// Connection handle of the CIS or BIS (12 bits meaningful - Range 0x0000-0x0EFF)
    uint16_t conhdl;
    /// Value of the Tx_UnACKed_Packets counter (0xXXXXXXXX)
    uint32_t tx_unacked_packets;
    /// Value of the Tx_Flushed_Packets counter (0xXXXXXXXX)
    uint32_t tx_flushed_packets;
    /// Value of the Tx_Last_Subevent_Packets counter (0xXXXXXXXX)
    uint32_t tx_last_subevent_packets;
    /// Value of the Retransmitted_Packets counter (0xXXXXXXXX)
    uint32_t retransmitted_packets;
    /// Value of the CRC_Error_Packets counter (0xXXXXXXXX)
    uint32_t crc_error_packets;
    /// Value of the Rx_Unreceived_Packets counter (0xXXXXXXXX)
    uint32_t rx_unreceived_packets;
    /// Value of the Duplicate_Packets counter (0xXXXXXXXX)
    uint32_t duplicate_packets;
} hci_le_rd_iso_link_quality_cmd_cmp_evt_t;
#endif // (BLE_CIS || BLE_BIS)

/**************************************************************************************
 **************                   HCI EVENT TRUCTURES                  ****************
 **************************************************************************************/

/// Basic LE event sutrcture
/*@TRACE*/
typedef struct hci_le_basic_evt
{
    /// LE Subevent code
    uint8_t subcode;
} hci_le_basic_evt_t;

#if (BLE_CIS)
/// Event indicates that the Connected Isochronous Stream with the Connection_Handle has been established.
/*@TRACE*/
typedef struct hci_le_cis_established_v1_evt
{
    ///LE Subevent code
    uint8_t subcode;
    /// 0x00 - The Broadcast Isochronous Group has been completed ; 0x01-0xFF Failed reason
    uint8_t status;
    /// Connection handle of the Connected Isochronous Stream (Range: 0x0000-0x0EFF)
    uint16_t conhdl;
    /// The CIG synchronization delay time in microseconds
    uint32_t cig_sync_delay;
    /// The CIS synchronization delay time in microseconds
    uint32_t cis_sync_delay;
    /// The maximum time, in microseconds, for transmission of SDUs of all CISes from master to slave
    /// (range 0x0000EA to 0x7FFFFF)
    uint32_t trans_latency_m2s;
    /// The maximum time, in microseconds, for transmission of SDUs of all CISes from slave to master
    /// (range 0x0000EA to 0x7FFFFF)
    uint32_t trans_latency_s2m;
    /// Master to slave PHY, 0x01: 1Mbps, 0x02: 2Mbps, 0x03: LE-Coded
    uint8_t phy_m2s;
    /// Slave to master PHY, 0x01: 1Mbps, 0x02: 2Mbps, 0x03: LE-Coded
    uint8_t phy_s2m;
    /// Maximum number of subevents in each isochronous event (Range: 0x01-0x1E)
    uint8_t nse;
    /// The burst number for master to slave transmission (0x00: no isochronous data from the master to the slave, range 0x01-0x0F)
    uint8_t bn_m2s;
    /// The burst number for slave to master transmission (0x00: no isochronous data from the slave to the master, range 0x01-0x0F)
    uint8_t bn_s2m;
    /// The flush timeout, in multiples of the ISO_Interval, for each payload sent from the master to the slave (Range: 0x01-0x1F)
    uint8_t ft_m2s;
    /// The flush timeout, in multiples of the ISO_Interval, for each payload sent from the slave to the master (Range: 0x01-0x1F)
    uint8_t ft_s2m;
    /// Maximum size, in octets, of the payload from master to slave (Range: 0x00-0xFB)
    uint16_t  max_pdu_m2s;
    /// Maximum size, in octets, of the payload from slave to master (Range: 0x00-0xFB)
    uint16_t  max_pdu_s2m;
    /// ISO interval (1.25ms unit, range: 5ms to 4s)
    uint16_t iso_interval;
} hci_le_cis_established_v1_evt_t;

/// Event indicates that the Connected Isochronous Stream with the Connection_Handle has been established.
/*@TRACE*/
typedef struct hci_le_cis_request_evt
{
    ///LE Subevent code
    uint8_t subcode;
    /// Connection handle of the ACL (Range: 0x0000-0x0EFF)
    uint16_t acl_conhdl;
    /// Connection handle of the Connected Isochronous Stream (Range: 0x0000-0x0EFF)
    uint16_t cis_conhdl;
    /// Used to identify the Connected Isochronous Group. (Range 0x00-0xEF)
    uint8_t cig_id;
    /// Used to identify a Connected Isochronous Stream. (Range 0x00-0xEF)
    uint8_t cis_id;
} hci_le_cis_request_evt_t;

/// Event indicates that the Connected Isochronous Stream with the Connection_Handle has been established.
/*@TRACE*/
typedef struct hci_le_cis_established_v2_evt
{
    ///LE Subevent code
    uint8_t subcode;
    /// 0x00 - The Broadcast Isochronous Group has been completed ; 0x01-0xFF Failed reason
    uint8_t status;
    /// Connection handle of the Connected Isochronous Stream (Range: 0x0000-0x0EFF)
    uint16_t conhdl;
    /// The CIG synchronization delay time in microseconds
    uint32_t cig_sync_delay;
    /// The CIS synchronization delay time in microseconds
    uint32_t cis_sync_delay;
    /// The maximum time, in microseconds, for transmission of SDUs of all CISes from master to slave
    /// (range 0x0000EA to 0x7FFFFF)
    uint32_t trans_latency_m2s;
    /// The maximum time, in microseconds, for transmission of SDUs of all CISes from slave to master
    /// (range 0x0000EA to 0x7FFFFF)
    uint32_t trans_latency_s2m;
    /// Master to slave PHY, 0x01: 1Mbps, 0x02: 2Mbps, 0x03: LE-Coded
    uint8_t phy_m2s;
    /// Slave to master PHY, 0x01: 1Mbps, 0x02: 2Mbps, 0x03: LE-Coded
    uint8_t phy_s2m;
    /// Maximum number of subevents in each isochronous event (Range: 0x01-0x1E)
    uint8_t nse;
    /// The burst number for master to slave transmission (0x00: no isochronous data from the master to the slave, range 0x01-0x0F)
    uint8_t bn_m2s;
    /// The burst number for slave to master transmission (0x00: no isochronous data from the slave to the master, range 0x01-0x0F)
    uint8_t bn_s2m;
    /// The flush timeout, in multiples of the ISO_Interval, for each payload sent from the master to the slave (Range: 0x01-0x1F)
    uint8_t ft_m2s;
    /// The flush timeout, in multiples of the ISO_Interval, for each payload sent from the slave to the master (Range: 0x01-0x1F)
    uint8_t ft_s2m;
    /// Maximum size, in octets, of the payload from master to slave (Range: 0x00-0xFB)
    uint16_t  max_pdu_m2s;
    /// Maximum size, in octets, of the payload from slave to master (Range: 0x00-0xFB)
    uint16_t  max_pdu_s2m;
    /// ISO interval (1.25ms unit, range: 5ms to 4s)
    uint16_t iso_interval;
    /// Time, in microseconds, between the start of consecutive subevents in a CIS event (Range: 0x000190 to ISO_Interval×1250 – 1 )
    uint32_t sub_interval;
    /// Maximum size, in octets, of the payload from the Central's Host (Range: 0x0000-0x0FFF)
    uint16_t max_sdu_m2s;
    /// Maximum size, in octets, of the payload from the Peripheral's Host (Range: 0x0000-0x0FFF)
    uint16_t max_sdu_s2m;
    /// Time, in microseconds, between the start of consecutive SDUs sent by the Central (Range: 0x000000 to 0x0FFFFF)
    uint32_t sdu_Interval_m2s;
    /// Time, in microseconds, between the start of consecutive SDUs sent by the Peripheral (Range: 0x000000 to 0x0FFFFF)
    uint32_t sdu_Interval_s2m;
    /// ISOAL Framing mode, 0: Unframed, 1: Framed
    uint8_t framing;
} hci_le_cis_established_v2_evt_t;
#endif // (BLE_CIS)

#if (BLE_BIS)
/// Event Generated in the Isochronous Broadcaster when all the Broadcast Isochronous Streams in the Group are being
/// transmitted
/*@TRACE*/
typedef struct hci_le_create_big_cmp_evt
{
    ///LE Subevent code
    uint8_t subcode;
    /// 0x00 - The Broadcast Isochronous Group has been completed ; 0x01-0xFF Failed reason
    uint8_t status;
    /// BIG_Handle is used to identify the BIS Group. (Range 0x00-0xEF)
    uint8_t big_hdl;
    /// Transmission delay time in microseconds of all BISs in the BIG (in us range 0x0000EA-0x7FFFFF)
    uint32_t big_sync_delay;
    /// The maximum delay time, in microseconds, for transmission of SDUs of all BISes (in us range 0x0000EA-0x7FFFFF)
    uint32_t big_trans_latency;
    /// PHY used, bit 0: 1Mbps, bit 1: 2Mbps, bit 2: LE-Coded
    uint8_t phy;
    /// The number of subevents in each BIS event in the BIG, range 0x01-0x1E
    uint8_t nse;
    /// The number of new payloads in each BIS event, range 0x01-0x07
    uint8_t bn;
    /// Offset used for pre-transmissions, range 0x00-0x0F
    uint8_t pto;
    /// The number of times a payload is transmitted in a BIS event, range 0x01-0x0F
    uint8_t irc;
    /// Maximum size of the payload in octets, range 0x00-0xFB
    uint16_t  max_pdu;
    /// ISO interval (1.25ms unit, range: 5ms to 4s)
    uint16_t iso_interval;
    /// Total number of BISs in the BIG (Range 0x01-0x1F)
    uint8_t num_bis;
    /// The connection handles of the BISs (Range 0x0000-0x0EFF)
    uint16_t conhdl[0x1F];
} hci_le_create_big_cmp_evt_t;

/// Event is generated when transmissions of all the Broadcast Isochronous Streams in the Broadcast Isochronous Group
/// are terminated
/*@TRACE*/
typedef struct hci_le_terminate_big_cmp_evt
{
    ///LE Subevent code
    uint8_t subcode;
    /// BIG_Handle is used to identify the BIS Group. (Range 0x00-0xEF)
    uint8_t big_hdl;
    /// Reason for termination (0xXX)
    uint8_t reason;
} hci_le_terminate_big_cmp_evt_t;

/// Event is generated in a scanning device when the Controller has synchronized with the requested Broadcast
/// Isochronous Streams.
/*@TRACE*/
typedef struct hci_le_big_sync_est_evt
{
    ///LE Subevent code
    uint8_t subcode;
    /// 0x00 - The Broadcast Isochronous Group has been completed ; 0x01-0xFF Failed reason
    uint8_t status;
    /// BIG_Handle is used to identify the BIS Group. (Range 0x00-0xEF)
    uint8_t big_hdl;
    /// The maximum delay time, in microseconds, for transmission of SDUs of all BISes (in us range 0x0000EA-0x7FFFFF)
    uint32_t big_trans_latency;
    /// The number of subevents in each BIS event in the BIG, range 0x01-0x1E
    uint8_t nse;
    /// The number of new payloads in each BIS event, range 0x01-0x07
    uint8_t bn;
    /// Offset used for pre-transmissions, range 0x00-0x0F
    uint8_t pto;
    /// The number of times a payload is transmitted in a BIS event, range 0x01-0x0F
    uint8_t irc;
    /// Maximum size of the payload in octets, range 0x00-0xFB
    uint16_t max_pdu;
    /// ISO interval (1.25ms unit, range: 5ms to 4s)
    uint16_t iso_interval;
    /// Total number of BISs in the BIG (Range 0x01-0x1F)
    uint8_t num_bis;
    /// The connection handles of the BISs (Range 0x0000-0x0EFF)
    uint16_t conhdl[0x1F];
} hci_le_big_sync_est_evt_t;

/// Event indicates that the Controller has not received any payload from any Broadcast Isochronous Stream within a
/// Broadcast Isochronous Group to which it was synchronized within the timeout period (BIS_Sync_Timeout).
/*@TRACE*/
typedef struct hci_le_big_sync_lost_evt
{
    ///LE Subevent code
    uint8_t subcode;
    /// BIG_Handle is used to identify the BIS Group. (Range 0x00-0xEF)
    uint8_t big_hdl;
    /// Reason of synchronization to BIG termination
    uint8_t reason;
} hci_le_big_sync_lost_evt_t;
#endif // (BLE_BIS)

/// Event indicates that the HCI_LE_Request_Peer_SCA command has been completed.
/*@TRACE*/
typedef struct hci_le_req_peer_sca_cmp_evt
{
    ///LE Subevent code
    uint8_t subcode;
    /// 0x00 - The Peer_Clock_Accuracy parameter is successfully received ; 0x01-0xFF Failed reason
    uint8_t status;
    /// Connection handle of the ACL (12 bits meaningful Range 0x0000-0x0EFF)
    uint16_t conhdl;
    /// Peer clock accuracy (@see enum SCA)
    uint8_t peer_clock_accuracy;
} hci_le_req_peer_sca_cmp_evt_t;

#if (BLE_BIS)
/// Event is generated in a scanning device when the Controller has received an advertising PDU that contained a BIGInfo field.
/*@TRACE*/
typedef struct hci_le_big_info_adv_report_evt
{
    ///LE Subevent code
    uint8_t subcode;
    /// Sync_Handle identifying the periodic advertising train (Range 0x0000-0x0EFF)
    uint16_t sync_hdl;
    /// Value of the Num_BIS subfield of the BIGInfo field (Range 0x01-0x1F)
    uint8_t num_bis;
    /// Value of the NSE subfield of the BIGInfo field (Range 0x01-0x1F)
    uint8_t nse;
    /// Value of the ISO_Interval subfield of the BIGInfo field (0xXXXX)
    uint16_t iso_interval;
    /// Value of the BN subfield of the BIGInfo field (Range 0x01-0x07)
    uint8_t bn;
    /// Value of the PTO subfield of the BIGInfo field (Range 0x00-0x0F)
    uint8_t pto;
    /// Value of the IRC subfield of the BIGInfo field (Range 0x01-0x0F)
    uint8_t irc;
    /// Value of the Max_PDU subfield of the BIGInfo field (Range 0x0000-0x00FB)
    uint16_t max_pdu;
    /// Value of the SDU_Interval subfield of the BIGInfo field (Range 0x0000FF-0x0FFFFF)
    uint8_t sdu_interval[3];
    /// Value of the Max_SDU subfield of the BIGInfo field (Range 0x0000-0x0FFF)
    uint16_t max_sdu;
    /// Value of the PHY subfield of the BIGInfo field (0x01: 1M, 0x02: 2M, 0x03: Coded, All other values: RFU)
    uint8_t phy;
    /// Value of the Framing subfield of the BIGInfo field (0x00: Unframed, 0x01: Framed, All other values: RFU)
    uint8_t framing;
    /// Value of the Encryption subfield of the BIGInfo field (0x00: Unencrypted, 0x01: Encrypted, All other values: RFU)
    uint8_t encryption;
} hci_le_big_info_adv_report_evt_t;
#endif // (BLE_BIS)


#if ((BLE_EMB_PRESENT && !HOST_PRESENT) || (!EMB_PRESENT && BLE_HOST_PRESENT))
///HCI VS LE Decrypt Command parameters structure
/*@TRACE*/
typedef struct hci_vs_le_decrypt_cmd
{
    ///Long term key structure
    struct ltk key;
    ///Pointer to buffer with encrypt data to decrypt - 16 bytes
    uint8_t encrypt_data[16];
} hci_vs_le_decrypt_cmd_t;
///HCI VS LE Decrypt Command complete event structure
/*@TRACE*/
typedef struct hci_vs_le_decrypt_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Decrypted  data to return to command source.
    uint8_t plain_data[ENC_DATA_LEN];
} hci_vs_le_decrypt_cmd_cmp_evt_t;
#endif // ((BLE_EMB_PRESENT && !HOST_PRESENT) || (!EMB_PRESENT && BLE_HOST_PRESENT))

#if (BLE_CH_SCAN_SUPPORT)
///HCI command complete event structure for Channel Scan End
/*@TRACE*/
typedef struct hci_vs_le_ch_scan_end_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
} hci_vs_le_ch_scan_end_cmd_cmp_evt_t;

/// HCI LE Set Channel Scan Command
typedef struct hci_vs_le_ch_scan_cmd
{
    /// Scan window duration (in us)
    uint32_t win_duration;
    /// Scan event duration (in us)
    uint32_t scan_duration;
    /// Scan interval (in 1.25ms frame)
    uint16_t intv;
    /// Channel map
    struct le_ch_map ch_map;
} hci_vs_le_ch_scan_cmd_t;
#endif // (BLE_CH_SCAN_SUPPORT)

#if (BLE_PWR_CTRL)
/// HCI LE Set Tx Power Command
typedef struct hci_vs_le_set_tx_pwr_cmd
{
    /// Connection handle
    uint16_t con_hdl;
    /// TX power (in dBm)
    int8_t tx_pwr;
} hci_vs_le_set_tx_pwr_cmd_t;

/// HCI command complete event structure for Set Tx Power Command
/*@TRACE*/
typedef struct hci_vs_le_set_tx_pwr_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    /// TX power (in dBm)
    int8_t new_tx_pwr;
} hci_vs_le_set_tx_pwr_cmd_cmp_evt_t;

#endif // (BLE_PWR_CTRL)

#if (MAX_NB_SYNC || TWS_LE_FWD_PRIMARY)
/// The command is used to identify and create a BT audio data path between the Host and the Controller.
/// It can also be used to configure a codec for each data path.
typedef struct hci_vs_bt_setup_audio_data_path_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Data path direction
    /// 0x00       Input (Host to Controller)
    /// 0x01       Output (Controller to Host)
    /// Others     Reserved for future use
    uint8_t  data_path_direction;
    /// Data path ID
    /// 0x00       HCI
    /// 0x01-0xFE  Logical_Channel_Number. The meaning of the logical channel is vendor specific.
    /// 0xFF       Reserved for future use
    uint8_t  data_path_id;
    /// Codec ID (5 octets)
    uint8_t codec_id[CODEC_ID_LEN];
    /// Controller delay in ms (Range: 0x000000 to 0x3D0900)
    uint32_t ctrl_delay;
    /// Codec configuration length
    uint8_t codec_cfg_len;
    /// Codec configuration
    uint8_t codec_cfg[__ARRAY_EMPTY];
} hci_vs_bt_setup_audio_data_path_cmd_t;

/// The command remove the BT audio data path between the Host and Controller and vice versa.
/*@TRACE*/
typedef struct hci_vs_bt_remove_audio_data_path_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Data path direction
    /// bit 0      Input (Host to Controller)
    /// bit 1      Output (Controller to Host)
    /// Others     Reserved for future use
    uint8_t  data_path_direction;
} hci_vs_bt_remove_audio_data_path_cmd_t;
#endif // (MAX_NB_SYNC || TWS_LE_FWD_PRIMARY)

#if (TWS_LE_FWD_PRIMARY)
/// HCI command structure for the HCI VS BT Set Audio Route command
/*@TRACE*/
typedef struct hci_vs_bt_set_audio_route_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// L2CAP channel ID
    uint16_t l2cap_chan_id;
} hci_vs_bt_set_audio_route_cmd_t;

/// HCI command structure for the HCI VS BT Remove Audio Route command
/*@TRACE*/
typedef struct hci_vs_bt_remove_audio_route_cmd
{
    /// Connection handle
    uint16_t conhdl;
} hci_vs_bt_remove_audio_route_cmd_t;

#if (BLE_CENTRAL)
/// HCI command structure for the HCI VS LE Start Follow eSCO command
/*@TRACE*/
typedef struct hci_vs_le_start_follow_esco_cmd
{
    /// LE Connection handle
    uint16_t le_conhdl;
    /// eSCO connection handle
    uint16_t esco_conhdl;
    /// Reserved time in microsecond
    /// Shall not exceed (TeSCO - WeSCO)
    uint16_t reserved_time;
} hci_vs_le_start_follow_esco_cmd_t;

/// HCI command structure for the HCI VS LE Stop Follow eSCO command
/*@TRACE*/
typedef struct hci_vs_le_stop_follow_esco_cmd
{
    /// LE Connection handle
    uint16_t le_conhdl;
} hci_vs_le_stop_follow_esco_cmd_t;
#endif // (BLE_CENTRAL)
#endif //(TWS_LE_FWD_PRIMARY)

#if (BLE_UPPER_TESTER)
/// HCI command structure for the HCI VS LE Test Mode command
/*@TRACE*/
typedef struct hci_vs_le_test_mode_cmd
{
    /// LE Connection handle
    uint16_t le_conhdl;
    /// Data Length
    uint8_t len;
    /// Data
    uint8_t data[__ARRAY_EMPTY];
} hci_vs_le_test_mode_cmd_t;

/// HCI VS LE Test Mode event
/*@TRACE*/
typedef struct hci_vs_le_test_mode_evt
{
    /// LE Subevent code
    uint8_t subcode;
    /// LE Connection handle
    uint16_t le_conhdl;
    /// Data Length
    uint8_t len;
    /// Data
    uint8_t data[__ARRAY_EMPTY];
} hci_vs_le_test_mode_evt_t;
#endif // (BLE_UPPER_TESTER)

#if (RW_DEBUG)
/// HCI LE Connection event trace Command
typedef struct hci_dbg_le_con_evt_trace_cmd
{
    /// Connection handle
    uint16_t con_hdl;
    /// Number of events to check (1..255)
    uint8_t nb_events;
} hci_dbg_le_con_evt_trace_cmd_t;

/// HCI LE Connection event trace Complete Event
typedef struct hci_dbg_le_con_evt_trace_cmd_cmp_evt
{
    /// DBG Subevent code
    uint8_t subcode;
    /// Connection handle
    uint16_t con_hdl;
    /// Number of events to trace
    uint8_t nb_events;
    /// Data trace
    uint16_t data[HCI_CON_MAX_TRACE_NB_EVENT];
} hci_dbg_le_con_evt_trace_cmd_cmp_evt_t;
#endif // (RW_DEBUG)

#if (BLE_HOST_PRESENT && BLE_MONITOR_ADV)
/// HCI LE Monitored Advertisers List enable and disable Commands
typedef struct
{
    /// Enable value 0 for disable, 1 for enable
    uint8_t enable;
} hci_le_mal_enable_cmd_t;

/// HCI LE Monitored Advertisers List add Command
typedef struct
{
    /// Address type - 0=public/1=random
    uint8_t addr_type;
    /// BD address
    struct bd_addr addr;
    /// Low RSSI threshold value (Range -127 to +20)
    uint8_t rssi_low;
    /// High RSSI threshold value (Range -127 to +20)
    uint8_t rssi_high;
    /// Timeout used for low rssi report (in seconds range 0x01 to 0xFF)
    uint8_t timeout;
} hci_le_mal_add_cmd_t;

/// HCI LE Monitored Advertisers List remove Command
typedef struct
{
    /// Address type - 0=public/1=random
    uint8_t addr_type;
    /// BD address
    struct bd_addr addr;
} hci_le_mal_remove_cmd_t;

#endif // (BLE_HOST_PRESENT && BLE_MONITOR_ADV)
/// @} CO_BT
#endif // CO_HCI_H_

