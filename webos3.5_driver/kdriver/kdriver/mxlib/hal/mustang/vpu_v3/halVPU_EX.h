//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _HAL_VPU_EX_H_
#define _HAL_VPU_EX_H_

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#ifndef MSOS_TYPE_NUTTX

#if defined(REDLION_LINUX_KERNEL_ENVI)
#define ENABLE_VPU_MUTEX_PROTECTION         0
#define VPU_DEFAULT_MUTEX_TIMEOUT           0xFFFFFFFFUL
#define VPU_ENABLE_BDMA_FW_FLASH_2_SDRAM    0
#else
#define ENABLE_VPU_MUTEX_PROTECTION         1
#define VPU_DEFAULT_MUTEX_TIMEOUT           MSOS_WAIT_FOREVER

    #if defined(FW_EXTERNAL_BIN)
    #define VPU_ENABLE_EMBEDDED_FW_BINARY       0
    #define VPU_ENABLE_BDMA_FW_FLASH_2_SDRAM    1
    #else
    #define VPU_ENABLE_EMBEDDED_FW_BINARY       1
    #define VPU_ENABLE_BDMA_FW_FLASH_2_SDRAM    0
    #endif

#endif
#define VPU_FORCE_MIU_MODE  1
#define HVD_ENABLE_IQMEM  0
#define VPU_IQMEM_BASE  0xe0000000
#define ENABLE_DECOMPRESS_FUNCTION          TRUE

#define VPU_CLOCK_240MHZ                    BITS(4:2,0)
#define VPU_CLOCK_216MHZ                    BITS(4:2,1)
#define VPU_CLOCK_192MHZ                    BITS(4:2,2)
#define VPU_CLOCK_XTAL                      BITS(4:2,3)
#define VPU_CLOCK_320MHZ                    BITS(4:2,4)
#define VPU_CLOCK_288MHZ                    BITS(4:2,5)


#define VPU_IQMEM_BASE  0xe0000000

#define VPU_ENABLE_MOBF_TEST  0



#define VPU_HI_MBOX0        0
#define VPU_HI_MBOX1        1
#define VPU_RISC_MBOX0      2
#define VPU_RISC_MBOX1      3


#define VPU_EX_TimerDelayMS(x)                  \
    do                                          \
    {                                           \
        volatile MS_U32 ticks = 0;              \
        while (ticks < (((MS_U32) (x)) << 13))  \
        {                                       \
            ticks++;                            \
        }                                       \
    } while(0)

#ifdef VDEC3
#define HAL_VPU_INVALID_BBU_ID 0xFFFFFFFF
#define VPU_MAX_DEC_NUM 2
#else
#define VPU_MAX_DEC_NUM 2
#endif
#define CHECK_NULL_PTR(vbbu_addr) (((void *)(vbbu_addr)) == NULL)

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_HAL_HVD_STREAM_NONE = 0x0,

    //Support TSP/TS/File mode
    E_HAL_HVD_MAIN_STREAM_BASE = 0x10,
    E_HAL_HVD_MAIN_STREAM0 = E_HAL_HVD_MAIN_STREAM_BASE,
    E_HAL_HVD_MAIN_STREAM_MAX,

    //Only support file mode
    E_HAL_HVD_SUB_STREAM_BASE   = 0x20,
    E_HAL_HVD_SUB_STREAM0 = E_HAL_HVD_SUB_STREAM_BASE,
    E_HAL_HVD_SUB_STREAM1,
    E_HAL_HVD_SUB_STREAM_MAX,

#ifdef VDEC3
    E_HAL_HVD_N_STREAM_BASE = 0x40,
    E_HAL_HVD_N_STREAM0 = E_HAL_HVD_N_STREAM_BASE,
    E_HAL_HVD_N_STREAM_MAX = E_HAL_HVD_N_STREAM0 + VPU_MAX_DEC_NUM,
#endif

    //Only support MVC stream
    E_HAL_HVD_MVC_STREAM_BASE = 0xF0,
    E_HAL_HVD_MVC_Main_View = E_HAL_HVD_MVC_STREAM_BASE,
    E_HAL_HVD_MVC_Sub_View,
    E_HAL_HVD_MVC_STREAM_MAX,
} HAL_HVD_StreamId;

typedef enum
{
    E_VPU_EX_DECODER_NONE = 0,
    E_VPU_EX_DECODER_MVD,
    E_VPU_EX_DECODER_HVD,
    E_VPU_EX_DECODER_MJPEG,
    E_VPU_EX_DECODER_RVD,
    E_VPU_EX_DECODER_MVC,
    E_VPU_EX_DECODER_VP8,
#ifdef VDEC3
    E_VPU_EX_DECODER_EVD,
#if SUPPORT_G2VP9
    E_VPU_EX_DECODER_G2VP9,
#endif
#endif
} VPU_EX_DecoderType;

typedef enum
{
    E_VPU_EX_CLOCK_240MHZ = VPU_CLOCK_240MHZ,
    E_VPU_EX_CLOCK_216MHZ   = VPU_CLOCK_216MHZ,
    E_VPU_EX_CLOCK_192MHZ   = VPU_CLOCK_192MHZ,
    E_VPU_EX_CLOCK_XTAL   = VPU_CLOCK_XTAL,
    E_VPU_EX_CLOCK_320MHZ   = VPU_CLOCK_320MHZ,
    E_VPU_EX_CLOCK_288MHZ   = VPU_CLOCK_288MHZ,
} VPU_EX_ClockSpeed;

typedef enum
{
    E_HAL_VPU_STREAM_NONE = 0x0,

    //Support TSP/TS File/File mode
    E_HAL_VPU_MAIN_STREAM_BASE = 0x10,
    E_HAL_VPU_MAIN_STREAM0 = E_HAL_VPU_MAIN_STREAM_BASE,
    E_HAL_VPU_MAIN_STREAM_MAX,

    //Only support file mode
    E_HAL_VPU_SUB_STREAM_BASE = 0x20,
    E_HAL_VPU_SUB_STREAM0 = E_HAL_VPU_SUB_STREAM_BASE,
    E_HAL_VPU_SUB_STREAM_MAX,

#ifdef VDEC3
    E_HAL_VPU_N_STREAM_BASE = 0x40,
    E_HAL_VPU_N_STREAM0 = E_HAL_VPU_N_STREAM_BASE,
    E_HAL_VPU_N_STREAM_MAX = E_HAL_VPU_N_STREAM0 + VPU_MAX_DEC_NUM,
#endif

    //Only support MVC stream
    E_HAL_VPU_MVC_STREAM_BASE = 0xF0,
    E_HAL_VPU_MVC_MAIN_VIEW = E_HAL_VPU_MVC_STREAM_BASE,
    E_HAL_VPU_MVC_SUB_VIEW,
    E_HAL_VPU_MVC_STREAM_MAX,
} HAL_VPU_StreamId;

typedef enum
{
    //Support TSP/TS/File mode
    E_HAL_VPU_MAIN_STREAM,

    //Only support file mode
    E_HAL_VPU_SUB_STREAM,

    //Only support MVC mode
    E_HAL_VPU_MVC_STREAM,

#ifdef VDEC3
    E_HAL_VPU_N_STREAM,
#endif
} HAL_VPU_StreamType;

typedef enum
{
    //Support TSP/TS/File mode
    E_VPU_EX_INPUT_TSP,
    //Only support file mode
    E_VPU_EX_INPUT_FILE,
    E_VPU_EX_INPUT_NONE,
} VPU_EX_SourceType;

typedef enum
{
    E_VPU_EX_UART_LEVEL_NONE = 0,      ///< Disable all uart message.
    E_VPU_EX_UART_LEVEL_ERR,           ///< Only output error message
    E_VPU_EX_UART_LEVEL_INFO,          ///< output general message, and above.
    E_VPU_EX_UART_LEVEL_DBG,           ///< output debug message, and above.
    E_VPU_EX_UART_LEVEL_TRACE,         ///< output function trace message, and above.
    E_VPU_EX_UART_LEVEL_FW,            ///< output FW message, and above.
} VPU_EX_UartLevel;

typedef enum
{
    E_VPU_EX_FW_VER_CTRLR = 0,
    E_VPU_EX_FW_VER_MVD_FW,
    E_VPU_EX_FW_VER_HVD_FW,
    E_VPU_EX_FW_VER_MVD_IF,
    E_VPU_EX_FW_VER_HVD_IF,
} VPU_EX_FWVerType;

/// DecodeMode for f/w tasks
typedef enum
{
    E_VPU_DEC_MODE_DUAL_INDIE,                     ///< Two independent tasks
    E_VPU_DEC_MODE_DUAL_3D,                        ///< Two dependent tasks for 3D
    E_VPU_DEC_MODE_SINGLE,                         ///< One task use the whole SRAM
    E_VPU_DEC_MODE_AUTO_MCU,
    E_VPU_DEC_MODE_MVC = E_VPU_DEC_MODE_SINGLE,
} VPU_EX_DecMode;

/// CmdMode for KOREA3D or PIP mode
typedef enum
{
    //Group1:Set Korea3DTV mode
    E_VPU_CMD_MODE_KR3D_BASE  = 0x0000,
    E_VPU_CMD_MODE_KR3D_INTERLACE = E_VPU_CMD_MODE_KR3D_BASE,
    E_VPU_CMD_MODE_KR3D_FORCE_P,
    E_VPU_CMD_MODE_KR3D_INTERLACE_TWO_PITCH,
    E_VPU_CMD_MODE_KR3D_FORCE_P_TWO_PITCH,

    //Group2:Set PIP mode
    E_VPU_CMD_MODE_PIP_BASE = 0x1000,
    E_VPU_CMD_MODE_PIP_SYNC_INDIE = E_VPU_CMD_MODE_PIP_BASE,
    E_VPU_CMD_MODE_PIP_SYNC_MAIN_STC,
    E_VPU_CMD_MODE_PIP_SYNC_SWITCH
} VPU_EX_CmdMode;

typedef struct
{
    VPU_EX_ClockSpeed   eClockSpeed;
    MS_BOOL             bClockInv;
    MS_S32              s32VPUMutexID;
    MS_U32              u32VPUMutexTimeout;
    MS_U8               u8MiuSel;
} VPU_EX_InitParam;

#define CMA_DRV_DIRECT_INIT
#ifdef CMA_DRV_DIRECT_INIT
/// input source select enumerator
typedef enum
{
    ///DTV mode
    E_VPU_EX_SRC_MODE_DTV = 0,
    ///TS file mode
    E_VPU_EX_SRC_MODE_TS_FILE,
    ///generic file mode
    E_VPU_EX_SRC_MODE_FILE,
    /// TS file and dual ES buffer mode
    E_VPU_EX_SRC_MODE_TS_FILE_DUAL_ES,
    ///generic file and dual ES buffer mode
    E_VPU_EX_SRC_MODE_FILE_DUAL_ES,
} VPU_EX_SrcMode;
#endif

/// codec type enumerator
typedef enum
{
    ///unsupported codec type
    E_VPU_EX_CODEC_TYPE_NONE = 0,
    ///MPEG 1/2
    E_VPU_EX_CODEC_TYPE_MPEG2,
    ///H263 (short video header)
    E_VPU_EX_CODEC_TYPE_H263,
    ///MPEG4 (default)
    E_VPU_EX_CODEC_TYPE_MPEG4,
    ///MPEG4 (Divx311)
    E_VPU_EX_CODEC_TYPE_DIVX311,
    ///MPEG4 (Divx412)
    E_VPU_EX_CODEC_TYPE_DIVX412,
    ///FLV
    E_VPU_EX_CODEC_TYPE_FLV,
    ///VC1 advanced profile (VC1)
    E_VPU_EX_CODEC_TYPE_VC1_ADV,
    ///VC1 main profile (RCV)
    E_VPU_EX_CODEC_TYPE_VC1_MAIN,
    ///Real Video version 8
    E_VPU_EX_CODEC_TYPE_RV8,
    ///Real Video version 9 and 10
    E_VPU_EX_CODEC_TYPE_RV9,
    ///H264
    E_VPU_EX_CODEC_TYPE_H264,
    ///AVS
    E_VPU_EX_CODEC_TYPE_AVS,
    ///MJPEG
    E_VPU_EX_CODEC_TYPE_MJPEG,
    ///MVC
    E_VPU_EX_CODEC_TYPE_MVC,
    ///VP8
    E_VPU_EX_CODEC_TYPE_VP8,
    ///HEVC
    E_VPU_EX_CODEC_TYPE_HEVC,
    ///VP9
    E_VPU_EX_CODEC_TYPE_VP9,
    E_VPU_EX_CODEC_TYPE_NUM
} VPU_EX_CodecType;

typedef struct
{
    MS_U32              u32Id;
    HAL_VPU_StreamId    eVpuId;
    VPU_EX_SourceType   eSrcType;
    VPU_EX_DecoderType  eDecType;
    MS_U8               u8HalId;  // hal MVD/HVD id
    MS_U32              u32HeapSize;
} VPU_EX_TaskInfo;

typedef struct
{
    MS_U32 u32DstAddr;
    MS_U32 u32DstSize;
    MS_U32 u32BinSize;
    MS_U32 u32BinAddr;
    MS_U8  u8SrcType;
} VPU_EX_FWCodeCfg;

typedef struct
{
    MS_U32  u32DstAddr;
    MS_U32  u32BinAddr;
    MS_U32  u32BinSize;
    MS_U32  u32FrameBufAddr;
    MS_U32  u32VLCTableOffset;
} VPU_EX_VLCTblCfg;

#ifdef VDEC3
typedef struct
{
    MS_U32  u32FrameBufAddr;
    MS_U32  u32FrameBufSize;
} VPU_EX_FBCfg;
#endif

/// VPU init parameters for dual decoder
typedef struct
{
    VPU_EX_FWCodeCfg   *pFWCodeCfg;
    VPU_EX_TaskInfo    *pTaskInfo;
    VPU_EX_VLCTblCfg   *pVLCCfg;
#ifdef VDEC3
    VPU_EX_FBCfg       *pFBCfg;
#endif
} VPU_EX_NDecInitPara;

typedef struct
{
    MS_U8  u8DecMod;
    MS_U8  u8CodecCnt;
    MS_U8  u8CodecType[VPU_MAX_DEC_NUM];
    MS_U8  u8ArgSize;
    MS_U32 u32Arg;
} VPU_EX_DecModCfg;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
MS_BOOL     HAL_VPU_EX_SetDecodeMode(VPU_EX_DecModCfg *pstCfg);
MS_BOOL     HAL_VPU_EX_SetSingleDecodeMode(MS_BOOL bEnable);
#ifdef VDEC3
MS_BOOL     HAL_VPU_EX_TaskCreate(MS_U32 u32Id, VPU_EX_NDecInitPara *pInitPara, MS_BOOL bFWdecideFB, MS_U32 u32BBUId);
#else
MS_BOOL     HAL_VPU_EX_TaskCreate(MS_U32 u32Id, VPU_EX_NDecInitPara *pInitPara);
#endif
MS_BOOL     HAL_VPU_EX_TaskDelete(MS_U32 u32Id, VPU_EX_NDecInitPara *pInitPara);
MS_BOOL     HAL_VPU_EX_SetFWReload(MS_BOOL bReload);

MS_BOOL     HAL_VPU_EX_LoadCode(VPU_EX_FWCodeCfg *pFWCodeCfg);
void        HAL_VPU_EX_InitRegBase(MS_U32 u32RegBase);

HAL_VPU_StreamId HAL_VPU_EX_GetFreeStream(HAL_VPU_StreamType eStreamType);
MS_BOOL     HAL_VPU_EX_Init(VPU_EX_InitParam *InitParams);
MS_BOOL     HAL_VPU_EX_DeInit(void);
void        HAL_VPU_EX_PowerCtrl(MS_BOOL bEnable);
void        HAL_VPU_EX_MIU_RW_Protect(MS_BOOL bEnable);
MS_BOOL     HAL_VPU_EX_CPUSetting(MS_U32 u32StAddr);
MS_BOOL     HAL_VPU_EX_SwRst(MS_BOOL bCheckMauIdle);
void        HAL_VPU_EX_SwRstRelse(void);
void        HAL_VPU_EX_SwRelseMAU(void);
MS_U32      HAL_VPU_EX_MemRead(MS_U32 u32Address);
MS_BOOL     HAL_VPU_EX_MemWrite(MS_U32 u32Address, MS_U32 u32Value);
MS_BOOL     HAL_VPU_EX_MBoxRdy(MS_U32 u32type);
MS_BOOL     HAL_VPU_EX_MBoxRead(MS_U32 u32type, MS_U32 *u32Msg);
void        HAL_VPU_EX_MBoxClear(MS_U32 u32type);
MS_BOOL     HAL_VPU_EX_MBoxSend(MS_U32 u32type, MS_U32 u32Msg);
MS_U32      HAL_VPU_EX_GetProgCnt(void);
MS_U8       HAL_VPU_EX_GetTaskId(MS_U32 u32Id);
void        HAL_VPU_EX_SetShareInfoAddr(MS_U32 u32Id, MS_U32 u32ShmAddr);
MS_U32      HAL_VPU_EX_GetShareInfoAddr(MS_U32 u32Id);
MS_U32      HAL_VPU_EX_GetVsyncAddrOffset(MS_U32 u32Id);
MS_U32      HAL_VPU_EX_GetVsyncExtAddrOffset(MS_U32 u32Id);
MS_BOOL     HAL_VPU_EX_IsPowered(void);
MS_BOOL     HAL_VPU_EX_IsRsted(void);
MS_BOOL     HAL_VPU_EX_IsEVDR2(void);
MS_BOOL     HAL_VPU_EX_MVDInUsed(void);
MS_BOOL     HAL_VPU_EX_HVDInUsed(void);
#ifdef VDEC3
MS_BOOL     HAL_VPU_EX_EVDInUsed(void);
#if SUPPORT_G2VP9
MS_BOOL     HAL_VPU_EX_G2VP9InUsed(void);
#endif
#endif
void        HAL_VPU_EX_IQMemSetDAMode(MS_BOOL bEnable);
void        HAL_VPU_EX_SetDbgLevel(VPU_EX_UartLevel eLevel);
MS_U32      HAL_VPU_EX_GetFWVer(MS_U32 u32Id, VPU_EX_FWVerType eVerType);
MS_BOOL HAL_VPU_EX_Init_Share_Mem(void);
MS_BOOL HAL_VPU_EX_CHIP_Capability(void* pHWCap);

MS_U32 HAL_VPU_EX_MIU1BASE(void);
MS_U32 HAL_VPU_EX_GetSHMAddr(void);
MS_BOOL HAL_VPU_EX_EnableSecurityMode(MS_BOOL enable);
MS_BOOL HAL_VPU_EX_REE_SetSHMBaseAddr(void);
MS_BOOL HAL_VPU_EX_REE_RegisterMBX(void);
MS_BOOL HAL_VPU_Set_MBX_param(MS_U8 u8APIMbxMsgClass);

#ifdef VDEC3
typedef enum
{
    E_HVD_CMDQ_CMD,
    E_HVD_CMDQ_ARG,
} HVD_COMMAND_QUEUE_TYPE;

typedef enum
{
    E_HVD_COMMAND_QUEUE_SEND_SUCCESSFUL,
    E_HVD_COMMAND_QUEUE_NOT_INITIALED,
    E_HVD_COMMAND_QUEUE_FULL,
    E_HVD_COMMAND_QUEUE_SEND_FAIL,
} HVD_DRAM_COMMAND_QUEUE_SEND_STATUS;


typedef struct
{
    MS_U32 u32Offset;       ///< Packet offset from bitstream buffer base address. unit: byte.
    MS_U32 u32Length;       ///< Packet size. unit: byte.   ==> Move _VDEC_EX_ReparseVP8Packet to FW
    MS_U64 u64TimeStamp;    ///< Packet time stamp. unit: ms.
    MS_U32 u32ID_L;         ///< Packet ID low part.
    MS_U32 u32ID_H;         ///< Packet ID high part.
} HAL_VPU_EX_PacketInfo;
// *****************Virtual BBU function*****************
MS_BOOL HAL_VPU_EX_Push2VBBU(MS_U32 u32Id, HAL_VPU_EX_PacketInfo *stVpuPkt, MS_U32 u32VBBUAddr);
MS_U32 HAL_VPU_EX_GetVBBUVacancy(MS_U32 u32VBBUAddr);
MS_U32 HAL_VPU_EX_GetESReadPtr(MS_U32 u32Id, MS_U32 u32VBBUAddr);
MS_U32 HAL_VPU_EX_GetESWritePtr(MS_U32 u32Id, MS_U32 u32VBBUAddr);
MS_BOOL HAL_VPU_EX_IsVBBUEmpty(MS_U32 u32VBBUAddr);
// *****************General dram command queue function*****************
MS_BOOL HAL_VPU_EX_IsMailBoxCMD(MS_U32 u32Cmd);
MS_BOOL HAL_VPU_EX_IsDisplayQueueCMD(MS_U32 u32Cmd);
MS_BOOL HAL_VPU_EX_DRAMCMDQueueSend(MS_U32 u32DramAddr, MS_U32 u32Msg);
// *****************Dram command queue function*****************
MS_BOOL HAL_VPU_EX_DRAMCMDQueueIsEmpty(void *cmd_queue);
MS_BOOL HAL_VPU_EX_DRAMCMDQueueIsFull(void *cmd_queue);
HVD_DRAM_COMMAND_QUEUE_SEND_STATUS HAL_VPU_EX_DRAMStreamCMDQueueSend(MS_U32 u32Id, void *cmd_queue, HVD_COMMAND_QUEUE_TYPE u8CmdType, MS_U32 u32Msg);
// *****************Display dram command queue  function*****************
MS_BOOL HAL_VPU_EX_DRAMDispCMDQueueIsEmpty(void *cmd_queue);
MS_BOOL HAL_VPU_EX_DRAMDispCMDQueueIsFull(void *cmd_queue);
HVD_DRAM_COMMAND_QUEUE_SEND_STATUS HAL_VPU_EX_DRAMStreamDispCMDQueueSend(MS_U32 u32Id, void *cmd_queue, HVD_COMMAND_QUEUE_TYPE u8CmdType, MS_U32 u32Msg);
// *****************General purpose function*****************
MS_BOOL HAL_VPU_EX_SetBitstreamBufAddress(MS_U32 u32BsAddr);
MS_U32 HAL_VPU_EX_GetBBUId(MS_U32 u32Id, VPU_EX_TaskInfo *pTaskInfo, MS_BOOL bIsNstreamMode);
MS_BOOL HAL_VPU_EX_FreeBBUId(MS_U32 u32Id, MS_U32 u32BBUId, VPU_EX_TaskInfo *pTaskInfo);
#endif
#ifdef VDEC3_FB
MS_BOOL HAL_VPU_EX_LoadVLCTable(VPU_EX_VLCTblCfg *pVlcCfg, MS_U8 u8FwSrcType);
#endif
#ifdef SUPPORT_VDEC_STR
void HAL_VPU_EX_ResumeFW(void);
MS_BOOL HAL_VPU_EX_CheckSTRState(MS_U8 u8TaskId);
MS_U32 HAL_VPU_EX_GetFWAddr(void);
MS_U32 HAL_VPU_EX_GetStrOffset(MS_U32 u32CodeBufSize);
void HAL_VPU_EX_EnableSTRResume(MS_U8 u8TaskId,MS_BOOL bEnable);
#endif
MS_BOOL HAL_VPU_EX_IsSTRResume(MS_U8 u8TaskId);
MS_BOOL HAL_VPU_EX_IsSTR(MS_U8 u8TaskId);

MS_U8   HAL_VPU_EX_CheckFreeStream(void);

MS_SIZE HAL_VPU_EX_GetFrameBufferDefaultSize(VPU_EX_CodecType eCodecType);
#ifdef CMA_DRV_DIRECT_INIT
// *****************CMA function*****************
MS_BOOL HAL_VPU_EX_GetCMAMemSize(VPU_EX_CodecType eCodecType, VPU_EX_SrcMode eSrcMode,
    MS_U64 *offset, MS_SIZE *length, MS_U64 total_length, MS_SIZE unUseSize);
#endif


#else
typedef struct
{
    MS_U32 Bitstream_Addr_Main;
    MS_U32 Bitstream_Len_Main;
    MS_U32 Bitstream_Addr_Sub;
    MS_U32 Bitstream_Len_Sub;
    MS_U32 MIU1_BaseAddr;
} VPU_EX_LOCK_DOWN_REGISTER;


MS_BOOL HAL_VPU_EX_LoadCodeInSecure(MS_U32 addr);
MS_BOOL HAL_VPU_EX_SetLockDownRegister(void* param);
MS_U32 HAL_VPU_EX_GetSHM_Offset(void);

#endif
#endif // _HAL_VPU_EX_H_
