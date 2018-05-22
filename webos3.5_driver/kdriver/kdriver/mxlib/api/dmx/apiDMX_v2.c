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

////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file   apiDMX_v2.c
/// @brief  Demux 1.0 API wrapper for Demux 2.0
/// @author MStar Semiconductor,Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "utopia.h"
#include "apiDMX_private.h"
#include "apiDMX.h"

#ifdef MSOS_TYPE_LINUX_KERNEL
#include <linux/string.h>
#else
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#define DMX_V2DBGMSG(x)                                 //x

#if 1

#define TSPlog	printf

#else

#ifdef MSOS_TYPE_LINUX_KERNEL

#define TSPlog(fmt, ...) { struct file *filep; char buf[255];  \
filep=(struct file *)filp_open("/tmp/tsplog",O_CREAT | O_APPEND | O_RDWR,0);  \
memset(buf, 0x00, 255);  \
sprintf(buf, fmt, ##__VA_ARGS__);   \
filep->f_op->write(filep,buf,strlen(buf),&filep->f_pos);  \
filp_close(filep,0);  \
}

#else

#define TSPlog(fmt, ...) { FILE *fp;  \
fp = fopen("/tmp/tsplog", "a+t");  \
fprintf(fp, fmt, ##__VA_ARGS__);   \
fclose(fp);  \
}

#endif

#endif

#define DMX_TASK_STACK_SIZE                             4096
#define DMX_MAX_PVR_ENG                                 8
#define DMX_MAX_SECFLT_NUM                              256
//------------------------------------------------------------------------------
// Local Variable
//------------------------------------------------------------------------------
#ifdef DMX_UTOPIA_20
static void*                        _pInstantDmx        = NULL;
static void*                        _pAttributeDmx      = NULL;
#endif

//#define DMX_UTOPIA_2K
#ifdef DMX_UTOPIA_2K

typedef struct _DMX_SEC_CB_PARAM
{
    DMX_EVENT                   Event;              ///< DMX section event
    DMX_NotifyCb                pNotify;            ///< DMX section callback
    DMX_Type2NotifyCb           pType2Notify;       ///< DMX section callback
    MS_U32                      Type2NotifyParam1;  ///< DMX section callback
}DMX_SEC_CB_PARAM;

static MS_U8                        _u8DmxTspStack[DMX_TASK_STACK_SIZE];
static MS_S32                       _s32DmxTspTaskId    = -1;
static MS_U8                        _u8TspThrState      = 0;
static MS_S32                       _Dmx2K_Mutex        = -1;

static DMX_NotifyCb                 _PvrEngCb[DMX_MAX_PVR_ENG]      = {[0 ... (DMX_MAX_PVR_ENG-1)] = NULL};     //< DMX Pvr callback
static DMX_SEC_CB_PARAM             _SecCbParam[DMX_MAX_SECFLT_NUM];

#define DMX2K_MUTEX_TIMEOUT                           10000
#define _DMX2K_LOCK()                                 while(!MsOS_ObtainMutex(_Dmx2K_Mutex, DMX2K_MUTEX_TIMEOUT)) \
                                                      {   \
                                                            printf("[%s][%d] DMX2K mutex get error\n", __FUNCTION__, __LINE__);   \
                                                            MsOS_DelayTask(1);  \
                                                      }

#define _DMX2K_UNLOCK()                               do{  \
                                                            if(!MsOS_ReleaseMutex(_Dmx2K_Mutex))   \
                                                            {   \
                                                                printf("[%s][%d] DMX2K mutex free error\n", __FUNCTION__, __LINE__);   \
                                                            }   \
                                                      }while(0);

#endif //DMX_UTOPIA_2K

//------------------------------------------------------------------------------
// Internal implementation
//------------------------------------------------------------------------------

#ifdef DMX_UTOPIA_20
static MS_BOOL _DMX_IOCtrlOpen(void)
{
    if(_pInstantDmx == NULL)
    {
        #ifdef DMX_UTOPIA_2K
        if(UtopiaOpen(MODULE_DMX|KERNEL_MODE, &_pInstantDmx, 0, _pAttributeDmx) !=  UTOPIA_STATUS_SUCCESS)
        #else
        if(UtopiaOpen(MODULE_DMX, &_pInstantDmx, 0, _pAttributeDmx) !=  UTOPIA_STATUS_SUCCESS)
        #endif
        {
            printf("[MAPI DMX][%06d] Open DMX Lib fail\n", __LINE__);
            return FALSE;
        }

        #ifdef MSOS_TYPE_LINUX_KERNEL
        printf("[%s] _pInstantDmx %p \n", __FUNCTION__, _pInstantDmx);
        #endif
    }

    return TRUE;
}

#ifdef DMX_UTOPIA_2K

static void _DMX_TSP_Proc_Task(void)
{
    DMX_TSP_IOSIGNAL stTspIoSig;
    MS_U32           u32ii =0, u32jj = 0, u32FltId = 0;
    MS_U32           u32PidFltNum = 0, u32PidGroup = 0;
    MS_U32           u32Mask = 0;
    DMX_SEC_CB_PARAM stCbParam;

    while(_pInstantDmx == NULL)
    {
        MsOS_DelayTask(1);
    }

    MApi_DMX_GetCap(DMX_CAP_PID_FILTER_NUM, (void*)&u32PidFltNum);

    while((_pInstantDmx != NULL) && (_u8TspThrState == 0))
    {
        if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_WaitTspCbEvt, (void*)&stTspIoSig) != UTOPIA_STATUS_SUCCESS)
        {
            continue;
        }
        if(stTspIoSig.u32DmxEvt == 0)
        {
            continue;
        }

        //PVR CB process
        if((stTspIoSig.u32DmxEvt & (DMX_EVENT_PVRBUF_FULL|DMX_EVENT_PVR1BUF_FULL)) && (_PvrEngCb[0] != NULL))
        {
            if(stTspIoSig.u32PvrBufFullFlag[0] == 0xFF)
            {
                _PvrEngCb[0](0xFF, DMX_EVENT_PVR1BUF_FULL);
            }
            else
            {
                if(stTspIoSig.u32PvrBufFullFlag[0] & 0x01)
                {
                    _PvrEngCb[0](0, DMX_EVENT_PVRBUF_FULL);
                }
                else if(stTspIoSig.u32PvrBufFullFlag[0] & 0x01)
                {
                    _PvrEngCb[u32ii](1, DMX_EVENT_PVRBUF_FULL);
                }
            }
        }

        //Section CB process
        if(stTspIoSig.u32DmxEvt & (DMX_EVENT_DATA_READY|DMX_EVENT_BUF_OVERFLOW))
        {
            u32PidGroup = u32PidFltNum >> 5;
            for(u32ii = 0; u32ii < u32PidGroup; u32ii++)
            {
                u32FltId = u32ii << 5;
                u32Mask  = 1;
                for(u32jj = 0; u32jj < 32; u32jj++)
                {
                    if((stTspIoSig.u32SecRdyFlag[u32ii] | stTspIoSig.u32SecOvfFlag[u32ii]) == 0)
                    {
                        break;
                    }

                    if(stTspIoSig.u32SecRdyFlag[u32ii] & u32Mask)
                    {                        
                        _DMX2K_LOCK();
                        stCbParam.Event = _SecCbParam[u32FltId].Event;
                        stCbParam.pNotify = _SecCbParam[u32FltId].pNotify;
                        stCbParam.pType2Notify = _SecCbParam[u32FltId].pType2Notify;
                        stCbParam.Type2NotifyParam1 = _SecCbParam[u32FltId].Type2NotifyParam1;
                        _DMX2K_UNLOCK();      
                        
                        if(stCbParam.Event & DMX_EVENT_CB_MASK)
                        {               
                            if(((stCbParam.Event & DMX_EVENT_CB_SELF_TYPE2) == DMX_EVENT_CB_SELF_TYPE2) && (stCbParam.pType2Notify))
                            {
                                stCbParam.pType2Notify(u32FltId, DMX_EVENT_DATA_READY ,stCbParam.Type2NotifyParam1);
                            }
                            else if(stCbParam.pNotify)
                            {
                                stCbParam.pNotify(u32FltId, DMX_EVENT_DATA_READY);
                            }
                        }
                        stTspIoSig.u32SecRdyFlag[u32ii] &= ~u32Mask;

                    }
                    if(stTspIoSig.u32SecOvfFlag[u32ii] & u32Mask)
                    {                       
                        _DMX2K_LOCK();
                        stCbParam.Event = _SecCbParam[u32FltId].Event;
                        stCbParam.pNotify = _SecCbParam[u32FltId].pNotify;
                        stCbParam.pType2Notify = _SecCbParam[u32FltId].pType2Notify;
                        stCbParam.Type2NotifyParam1 = _SecCbParam[u32FltId].Type2NotifyParam1;
                        _DMX2K_UNLOCK();      
                        
                        if(stCbParam.Event & DMX_EVENT_CB_MASK)
                        {
                            if(((stCbParam.Event&DMX_EVENT_CB_SELF_TYPE2) == DMX_EVENT_CB_SELF_TYPE2) && (stCbParam.pType2Notify))
                            {
                                stCbParam.pType2Notify(u32FltId, DMX_EVENT_BUF_OVERFLOW ,stCbParam.Type2NotifyParam1);
                            }
                            else if(stCbParam.pNotify)
                            {
                                stCbParam.pNotify(u32FltId, DMX_EVENT_BUF_OVERFLOW);
                            }
                        }
                        stTspIoSig.u32SecOvfFlag[u32ii] &= ~u32Mask;
                    }
                    u32FltId++;
                    u32Mask <<= 1;

                }

            }
        }
    }

    _u8TspThrState = 2;
}
#endif

#endif //end of DMX_UTOPIA_2K

DMX_FILTER_STATUS MApi_DMX_SetFW(MS_PHYADDR pFwAddr, MS_U32 u32FwSize)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s][%d](%lx, %lx)\n", __FUNCTION__, __LINE__, pFwAddr, u32FwSize));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = pFwAddr;
    u32param[1] = u32FwSize;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_SetFw, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%06d] DMX Init fail\n", __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK) 
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_SetFW(pFwAddr, u32FwSize);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Init(void)
{

#ifdef DMX_UTOPIA_20
    #ifdef DMX_UTOPIA_2K
    int ii = 0;
    char dmx_tsp_prc_task[] = "DMX_TSP_Task";
    #endif

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__)); 

    MS_U32 u32param = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
    {
        return DMX_FILTER_STATUS_ERROR;
    }

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Init_1_0, (void*)&u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%06d] DMX Init fail\n", __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    #ifdef DMX_UTOPIA_2K
    _Dmx2K_Mutex = MsOS_CreateMutex(E_MSOS_FIFO, (char*)"DMX2K Mutex", MSOS_PROCESS_SHARED);
    if (-1 == _Dmx2K_Mutex)
    {
        printf("[%s][%06d] create DMX2K mutex fail\n", __FUNCTION__, __LINE__);
        return DMX_FILTER_STATUS_ERROR;
    }

    for(ii = 0; ii < DMX_MAX_SECFLT_NUM; ii++)
    {
        _SecCbParam[ii].Event = 0;
        _SecCbParam[ii].pNotify = NULL;
        _SecCbParam[ii].pType2Notify = NULL;
        _SecCbParam[ii].Type2NotifyParam1 = 0;
    }
    //printf("[%s][%d] %lx\n", __FUNCTION__, __LINE__, (MS_U32)_s32DmxTspTaskId);
    if(_s32DmxTspTaskId == -1)
    {
        _s32DmxTspTaskId = MsOS_CreateTask((TaskEntry)_DMX_TSP_Proc_Task,
                           (MS_U32)NULL,
                           E_TASK_PRI_MEDIUM,
                           TRUE,
                           _u8DmxTspStack,
                           DMX_TASK_STACK_SIZE,
                           dmx_tsp_prc_task);
    }
    //printf("[%s][%d] %lx\n", __FUNCTION__, __LINE__, (MS_U32)_s32DmxTspTaskId);
    #endif

    if(u32param != (MS_U32)DMX_FILTER_STATUS_OK) 
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Init();

#endif

}

DMX_FILTER_STATUS MApi_DMX_TSPInit(DMX_TSPParam *param)
{
    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

#ifdef DMX_UTOPIA_20

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSPInit_1_0, param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%06d] DMX Init fail\n", __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(param->u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_TSPInit(param);

#endif
}

DMX_FILTER_STATUS MApi_DMX_Exit(void)	
{
#ifdef DMX_UTOPIA_20
    #ifdef DMX_UTOPIA_2K
    MS_U32 u32Timer;
    #endif

    MS_U32 u32param = 0;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    #ifdef DMX_UTOPIA_2K
    _u8TspThrState = 1;
    #endif

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Exit, (void*)&u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%06d] DMX Exit fail\n", __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    #ifdef DMX_UTOPIA_2K
    u32Timer = MsOS_GetSystemTime();
    while(_s32DmxTspTaskId > 0)
    {
        if(_u8TspThrState == 2)
            break;

        if (MsOS_GetSystemTime()-u32Timer > 10000)
        {
            DMX_V2DBGMSG(printf("[%s][%d] TSP Proc task exit timeout!!!!!!\n", __FUNCTION__, __LINE__));
            break;
        }
    }

    if(_s32DmxTspTaskId != -1)
    {
        MsOS_DeleteTask(_s32DmxTspTaskId);
        _s32DmxTspTaskId = -1;
    }
    _u8TspThrState = 0;

    if (_Dmx2K_Mutex != -1 )
    {
        _DMX2K_LOCK();
        _DMX2K_UNLOCK();
        MsOS_DeleteMutex(_Dmx2K_Mutex);
        _Dmx2K_Mutex = -1;
    }
    #endif

    if(UtopiaClose(_pInstantDmx) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%06d] DMX Close fail\n", __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    _pInstantDmx        = NULL;
    _pAttributeDmx      = NULL;

    if(u32param != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Exit();
#endif
}

DMX_FILTER_STATUS MApi_DMX_Reset(void)
{
#ifdef DMX_UTOPIA_20

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));    
    MS_U32 u32param = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Reset, (void*)&u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%06d] DMX Reset fail\n", __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Reset();

#endif
}

DMX_FILTER_STATUS MApi_DMX_ForceExit(void)
{
#ifdef DMX_UTOPIA_20

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));    
    MS_U32 u32param = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;
    
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_ForceExit, (void*)&u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%06d] DMX Reset fail\n", __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_ForceExit();

#endif
}

DMX_FILTER_STATUS MApi_DMX_Resume(MS_U32 u32FWAddr, MS_U32 u32FWSize)
{

#ifdef DMX_UTOPIA_20

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    DMX_TSPParam tspparam = {0};

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    tspparam.phyFWAddr = u32FWAddr;
    tspparam.u32FWSize = u32FWSize;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Resume, (void*)&tspparam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%06d] DMX Resume fail\n", __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(tspparam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Resume(u32FWAddr, u32FWSize);

#endif
}

DMX_FILTER_STATUS MApi_DMX_Suspend(void)
{
#ifdef DMX_UTOPIA_20

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));
    MS_U32 u32param = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Suspend, (void*)&u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%06d] DMX Suspend fail\n", __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Suspend();

#endif
}

DMX_FILTER_STATUS MApi_DMX_ReleaseSemaphone(void)
{
#ifdef DMX_UTOPIA_20

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));
    MS_U32 u32param = 0;
    
    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_ReleseHWSemp, (void*)&u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%06d] DMX Release HW lock semaphore fail\n", __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
    
#else

    return _MApi_DMX_ReleaseSemaphone();

#endif
}

DMX_FILTER_STATUS MApi_DMX_Get_FlowInput_Status(DMX_FLOW DmxFlow, DMX_FLOW_INPUT *pDmxFlowInput, MS_BOOL *pbClkInv, MS_BOOL *pbExtSync, MS_BOOL *pbParallel)
{

#ifdef DMX_UTOPIA_20

    DMX_FLOW_PARAM flowparam ={0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    //initiate the parameters
    *pDmxFlowInput = DMX_FLOW_INPUT_MEM;
    *pbClkInv      = FALSE;
    *pbExtSync     = FALSE;
    *pbParallel    = FALSE;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    flowparam.DmxFlow       = DmxFlow;
    flowparam.DmxFlowInput  = DMX_FLOW_INPUT_DEMOD;
    flowparam.bClkInv       = FALSE;
    flowparam.bExtSync      = FALSE;
    flowparam.bParallel     = FALSE;
    flowparam.bSet          = FALSE;

    //initiate the parameters
    *pDmxFlowInput = DMX_FLOW_INPUT_MEM;
    *pbClkInv      = FALSE;
    *pbExtSync     = FALSE;
    *pbParallel    = FALSE;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Flow,(void*)&flowparam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pDmxFlowInput  = flowparam.DmxFlowInput;
    *pbClkInv       = flowparam.bClkInv;
    *pbExtSync      = flowparam.bExtSync;
    *pbParallel     = flowparam.bParallel;

    if(flowparam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Get_FlowInput_Status(DmxFlow, pDmxFlowInput, pbClkInv, pbExtSync, pbParallel);

#endif
}

DMX_FILTER_STATUS MApi_DMX_FlowSet(DMX_FLOW DmxFlow, DMX_FLOW_INPUT DmxFlowInput, MS_BOOL bClkInv, MS_BOOL bExtSync, MS_BOOL bParallel)
{
#ifdef DMX_UTOPIA_20
    DMX_FLOW_PARAM flowparam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    flowparam.bSet              = TRUE;
    flowparam.DmxFlow           = DmxFlow;
    flowparam.DmxFlowInput      = DmxFlowInput;
    flowparam.bClkInv           = bClkInv;
    flowparam.bExtSync          = bExtSync;
    flowparam.bParallel         = bParallel;
    flowparam.u32CAEngSel       = 0;

    if(DmxFlowInput == DMX_FLOW_INPUT_MEM_NOPASSCA)
    {
        flowparam.u32CAEngSel   = 0xFFFFFFFF;
    }
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Flow,(void*)&flowparam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(flowparam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else

    return _MApi_DMX_FlowSet(DmxFlow, DmxFlowInput, bClkInv, bExtSync, bParallel);

#endif
}

DMX_FILTER_STATUS MApi_DMX_PVR_FlowSet(DMX_PVR_ENG Eng, DMX_TSIF ePvrSrcTSIf, MS_BOOL bDscmbRec)
{
#ifdef DMX_UTOPIA_20
    DMX_PVRFLOWPARAM pvrflowparam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    pvrflowparam.bSet = TRUE;
    pvrflowparam.eEng = Eng;
    pvrflowparam.eSrcTSIf = ePvrSrcTSIf;
    pvrflowparam.bDscmbRec = bDscmbRec;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_PVR_Flow,(void*)&pvrflowparam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(pvrflowparam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_PVR_FlowSet(Eng, ePvrSrcTSIf, bDscmbRec);

#endif
}

DMX_FILTER_STATUS MApi_DMX_Open(DMX_FILTER_TYPE DmxFltType, MS_U8* pu8DmxId)
{
#ifdef DMX_UTOPIA_20
    DMX_FLT_TYPE_PARAM fltparam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu8DmxId = 0xFF;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    fltparam.DmxFltType = DmxFltType;
    fltparam.u32DmxId = 0xFF;

    //initiate the parameters
    *pu8DmxId = 0xFF;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Open,(void*)&fltparam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu8DmxId = (MS_U8)(fltparam.u32DmxId & 0xFF);

    if(fltparam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Open(DmxFltType, pu8DmxId);

#endif
}

DMX_FILTER_STATUS MApi_DMX_Close(MS_U8 u8DmxId)
{
#ifdef DMX_UTOPIA_20
    
    MS_U32 u32param[2] = {0, 0};
    u32param[0] = ((MS_U32)u8DmxId) & 0xFF;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));   

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    while(1)
    {
        if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Close,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
        {
            DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));        
            return DMX_FILTER_STATUS_ERROR;
        }
        if(u32param[1] == (MS_U32)DMX_FILTER_STATUS_OK)
        {
            break;
        }
        MsOS_DelayTask(4);
    }

    #ifdef DMX_UTOPIA_2K
    _DMX2K_LOCK();
    _SecCbParam[u8DmxId].Event = 0;
    _SecCbParam[u8DmxId].pNotify = NULL;
    _SecCbParam[u8DmxId].pType2Notify = NULL;
    _SecCbParam[u8DmxId].Type2NotifyParam1 = 0;
    _DMX2K_UNLOCK();
    #endif
    
    if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Close(u8DmxId);

#endif
}

DMX_FILTER_STATUS MApi_DMX_Start(MS_U8 u8DmxId)
{
#ifdef DMX_UTOPIA_20
    
    MS_U32 u32param[2] = {0, 0};

    u32param[0] = ((MS_U32)u8DmxId) & 0xFF;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;
    
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Start,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else
    return _MApi_DMX_Start(u8DmxId);
#endif
}

DMX_FILTER_STATUS MApi_DMX_Stop(MS_U8 u8DmxId)
{
#ifdef DMX_UTOPIA_20
    MS_U32 u32param[2] = {0, 0};

    u32param[0] = ((MS_U32)u8DmxId) & 0xFF;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;    
    
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Stop,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
    
#else

    return _MApi_DMX_Stop(u8DmxId);

#endif
}

DMX_FILTER_STATUS MApi_DMX_IsStart(MS_U8 u8DmxId, MS_BOOL* pbEnable)
{
#ifdef DMX_UTOPIA_20
    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pbEnable = FALSE;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = ((MS_U32)u8DmxId) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_IsStart,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pbEnable = (MS_BOOL)(u32param[1] & 0xFF);

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_IsStart(u8DmxId, pbEnable);

#endif
}

DMX_FILTER_STATUS MApi_DMX_Get_FltScmbSts(DMX_FILTER_TYPE FltSrc, MS_U32 u32FltGroupId, MS_U32 PidFltId, MS_U32 *pu32ScmbSts)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[5] = {0, 0, 0, 0, 0};

    *pu32ScmbSts = 0;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = (MS_U32)FltSrc;
    u32param[1] = u32FltGroupId;
    u32param[2] = PidFltId;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_GetFltScmbSts,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32ScmbSts = u32param[3];

    if(u32param[4] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Get_FltScmbSts(FltSrc, u32FltGroupId, PidFltId, pu32ScmbSts);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Get_PesScmbSts(MS_U8 u8DmxId, MS_U8* pu8scmb)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[3] = {0, 0, 0};

    *pu8scmb = 0;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = ((MS_U32)u8DmxId) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_GetPESScmbSts,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu8scmb = (MS_U8)(u32param[1] & 0xFF);

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else

    return _MApi_DMX_Get_PesScmbSts(u8DmxId, pu8scmb);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Get_TsScmbSts(MS_U8 u8DmxId, MS_U8* pu8ScmSts)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu8ScmSts = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = ((MS_U32)u8DmxId) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_GetTsScmbSts,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu8ScmSts = (MS_U8)(u32param[1] & 0xFF);

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
    
#else

    return _MApi_DMX_Get_TsScmbSts(u8DmxId, pu8ScmSts);

#endif

}

DMX_FILTER_STATUS MApi_DMX_ChkAlive(void)
{
#ifdef DMX_UTOPIA_20
    MS_U32 u32param = 0;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;
   
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_ChkFwAlive, (void*)&u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
    
#else

    return _MApi_DMX_ChkAlive();

#endif

}

DMX_FILTER_STATUS MApi_DMX_Info(MS_U8 u8DmxId, DMX_Flt_info* pDmxFltInfo, DMX_FILTER_TYPE* pDmxFltType, MS_BOOL bSet)
{
#ifdef DMX_UTOPIA_20

    DMX_FLT_INFO_PARAM infoparam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    infoparam.u32DmxId      = ((MS_U32)u8DmxId) & 0xFF;
    infoparam.pDmxFltInfo   = pDmxFltInfo;
    infoparam.pDmxFltType   = pDmxFltType;
    infoparam.bSet          = bSet;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Info,(void*)&infoparam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

 #ifdef DMX_UTOPIA_2K
    if(bSet == TRUE)
    {
    	DMX_Sect_info* pInfo = &(pDmxFltInfo->Info.SectInfo);

		//printf("info %d: type: %lx,  %lx , %p , %p , %ld\n", (int)u8DmxId, *pDmxFltType,
        //  pInfo->Event, pInfo->pNotify, pInfo->pType2Notify, pInfo->Type2NotifyParam1);
    	switch(*pDmxFltType & ~DMX_FILTER_FLT_MASK)
    	{
    		case DMX_FILTER_TYPE_SECTION:
			case DMX_FILTER_TYPE_PES:
			case DMX_FILTER_TYPE_PACKET:
			case DMX_FILTER_TYPE_PCR:
			case DMX_FILTER_TYPE_TELETEXT:
			case DMX_FILTER_TYPE_SCMBCHK:
			#if (!(defined(TSP_VERSION) && ((TSP_VERSION == TSP_VER_2_0) || (TSP_VERSION == TSP_VER_4_0))))
			case DMX_FILTER_TYPE_SECTION_VER:
			#endif
                //_DMX2K_LOCK();
            	_SecCbParam[u8DmxId].Event = pInfo->Event;
            	_SecCbParam[u8DmxId].pNotify = pInfo->pNotify;
            	_SecCbParam[u8DmxId].pType2Notify = pInfo->pType2Notify;
            	_SecCbParam[u8DmxId].Type2NotifyParam1 = pInfo->Type2NotifyParam1;
                //_DMX2K_UNLOCK();
				break;
			default:
				break;
        }
    }
#endif

    if(infoparam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;  

#else

    return _MApi_DMX_Info(u8DmxId, pDmxFltInfo, pDmxFltType, bSet);

#endif
}

DMX_FILTER_STATUS MApi_DMX_Pid(MS_U8 u8DmxId, MS_U16* pu16Pid, MS_BOOL bSet)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));


    if(bSet == FALSE)
    {
        *pu16Pid = 0x1FFF;
    }

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = ((MS_U32)u8DmxId) & 0xFF;
    u32param[1] = ((MS_U32)(*pu16Pid)) & 0xFFFF;
    u32param[2] = ((MS_U32)bSet) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pid,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }
    if(bSet == FALSE)
    {
        *pu16Pid = (MS_U16)u32param[1];
    }

    if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pid(u8DmxId, pu16Pid, bSet);

#endif
}

//-------------------------------------------------------------------------------------------------
/// Set and activate a demux filter
/// @param u16Pid \b IN: PID to set
/// @param DmxFltType \b IN: the demux filer type to set
/// @param pu8DmxId \b OUT: the demux filer Id
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_StartFilter(MS_U16 u16Pid, DMX_FILTER_TYPE DmxFltType, MS_U8 *pu8DmxId)
{
    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    //DMX_ASSERT((pu8DmxId != NULL || u16Pid < DMX_PID_NULL), DMX_DBGMSG(DMX_DBG_ERR, printf("[MAPI DMX][%06d] Bad dmxid or pid\n", __LINE__)));

    if (DMX_FILTER_STATUS_OK == MApi_DMX_Open(DmxFltType, pu8DmxId))
    {
        MApi_DMX_Pid(*pu8DmxId, &u16Pid, TRUE);
        MApi_DMX_Start(*pu8DmxId);
    }
    return DMX_FILTER_STATUS_OK;
}

DMX_FILTER_STATUS MApi_DMX_SectPatternSet(MS_U8 u8DmxId, MS_U8* pPattern, MS_U8* pMask, MS_U8 *pu8NotMask, MS_U32 MatchSize)
{
#ifdef DMX_UTOPIA_20

    DMX_FLT_SEC_PAT_PARAM SecPatParam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    SecPatParam.u32DmxId        = ((MS_U32)u8DmxId) & 0xFF;
    SecPatParam.pu8Pattern      = pPattern;
    SecPatParam.pu8Mask         = pMask;
    SecPatParam.pu8NotMask      = pu8NotMask;
    SecPatParam.u32MatchSize    = MatchSize;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Sec_SetPattern,(void*)&SecPatParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(SecPatParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_SectPatternSet(u8DmxId, pPattern, pMask, pu8NotMask, MatchSize);

#endif
}

DMX_FILTER_STATUS MApi_DMX_SectReset(MS_U8 u8DmxId)
{
#ifdef DMX_UTOPIA_20
    MS_U32 u32param[2] = {0, 0};
    u32param[0] = ((MS_U32)u8DmxId) & 0xFF;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));    
    
    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Sec_Reset,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_SectReset(u8DmxId);

#endif
}

DMX_FILTER_STATUS MApi_DMX_SectReadSet(MS_U8 u8DmxId, MS_PHYADDR Read)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));    
    
    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = ((MS_U32)u8DmxId) & 0xFF;
    u32param[1] = Read;
    u32param[2] = 1;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Sec_ReadAddr,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else

    return _MApi_DMX_SectReadSet(u8DmxId, Read);

#endif
}

DMX_FILTER_STATUS MApi_DMX_SectReadGet(MS_U8 u8DmxId, MS_PHYADDR* pRead)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pRead = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = ((MS_U32)u8DmxId) & 0xFF;
    u32param[2] = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Sec_ReadAddr,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pRead = u32param[1];

    if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
    
#else

    return _MApi_DMX_SectReadGet(u8DmxId, pRead);

#endif
}

DMX_FILTER_STATUS MApi_DMX_SectWriteGet(MS_U8 u8DmxId, MS_PHYADDR* pWrite)
{
#ifdef DMX_UTOPIA_20
    
    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pWrite = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = ((MS_U32)u8DmxId) & 0xFF;
    u32param[1] = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Sec_GetWriteAddr,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pWrite = u32param[1];

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_SectWriteGet(u8DmxId, pWrite);

#endif
}

DMX_FILTER_STATUS MApi_DMX_SectStartGet(MS_U8 u8DmxId, MS_PHYADDR* pStart)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pStart = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = ((MS_U32)u8DmxId) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Sec_GetStartAddr,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pStart = u32param[1];

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
    
#else

    return _MApi_DMX_SectWriteGet(u8DmxId, pStart);

#endif
}

DMX_FILTER_STATUS MApi_DMX_SectEndGet(MS_U8 u8DmxId, MS_PHYADDR* pEnd)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pEnd = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = ((MS_U32)u8DmxId) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Sec_GetEndAddr,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pEnd = u32param[1];

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_SectEndGet(u8DmxId, pEnd);

#endif
}

DMX_FILTER_STATUS MApi_DMX_SetFwDataAddr(MS_U32 u32DataAddr, MS_U32 u32size)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = u32DataAddr;
    u32param[1] = u32size;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_SetFwMiuDataAddr,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_SetFwDataAddr(u32DataAddr, u32size);

#endif
}

DMX_FILTER_STATUS MApi_DMX_TTX_WriteGet(MS_U8 u8DmxId, MS_PHYADDR* pWrite)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pWrite = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = ((MS_U32)u8DmxId) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_GetTTXWrite,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pWrite = u32param[1];

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_TTX_WriteGet(u8DmxId, pWrite);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_SetPlaybackStamp(MS_U32 u32Stamp)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d (%d)\n", __FUNCTION__, __LINE__, (int)u32Stamp));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = 0;
    u32param[1] = u32Stamp;
    u32param[2] = 1;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_PlayStamp,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pvr_SetPlaybackStamp(u32Stamp);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Eng_SetPlaybackStamp(DMX_FILEIN_PATH ePath,MS_U32 u32Stamp)
{
#ifdef DMX_UTOPIA_20
	
	MS_U32 u32param[4] = {0, 0, 0, 0};
	
	DMX_V2DBGMSG(printf("[%s] %d (%d)\n", __FUNCTION__, __LINE__, (int)u32Stamp));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	u32param[0] = (MS_U32)ePath;
	u32param[1] = u32Stamp;
	u32param[2] = 1;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_PlayStamp,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}
	
    if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_Filein_Eng_SetPlaybackStamp(ePath, u32Stamp);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_GetPlaybackStamp(MS_U32* pu32Stamp)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d (0x%x)\n", __FUNCTION__, __LINE__, (int)pu32Stamp));

    *pu32Stamp = 0;
    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = 0;
    u32param[1] = 0;
    u32param[2] = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_PlayStamp,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32Stamp = u32param[1];

    if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else

    return _MApi_DMX_Pvr_GetPlaybackStamp(pu32Stamp);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Eng_GetPlaybackStamp(DMX_FILEIN_PATH ePath,MS_U32* pu32Stamp)
{

#ifdef DMX_UTOPIA_20
	
	MS_U32 u32param[4] = {0, 0, 0, 0};
	
	DMX_V2DBGMSG(printf("[%s] %d (0x%x)\n", __FUNCTION__, __LINE__, (int)pu32Stamp));

	*pu32Stamp = 0;
	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	u32param[0] = (MS_U32)ePath;
	u32param[1] = 0;
	u32param[2] = 0;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_PlayStamp,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}

	*pu32Stamp = u32param[1];
	
	if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_Filein_Eng_GetPlaybackStamp(ePath, pu32Stamp);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_TimeStampEnable(void)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[3] = {0, 0 ,0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = 0;
    u32param[1] = 1;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_TimestampMode,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pvr_TimeStampEnable();

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Eng_PlaybackTimeStampEnable(DMX_FILEIN_PATH ePath)
{
#ifdef DMX_UTOPIA_20
	
	MS_U32 u32param[3] = {0, 0, 0};
	
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	u32param[0] = (MS_U32)ePath;
	u32param[1] = 1;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_TimestampMode,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}
	
    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_Filein_Eng_PlaybackTimeStampEnable(ePath);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_TimeStampDisable(void)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = 0;
    u32param[1] = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_TimestampMode,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pvr_TimeStampDisable();

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Eng_PlaybackTimeStampDisable(DMX_FILEIN_PATH ePath)
{
#ifdef DMX_UTOPIA_20
	
	MS_U32 u32param[3] = {0, 0, 0};
	
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	u32param[0] = (MS_U32)ePath;
	u32param[1] = 0;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_TimestampMode,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}
	
    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_Filein_Eng_PlaybackTimeStampDisable(ePath);

#endif

}

DMX_FILTER_STATUS MApi_DMX_PvrCA_Eng_Pid_Open(DMX_PVR_ENG Eng, MS_U32 Pid, MS_U8* pu8DmxId, MS_U8 u8ShareKeyType)
{
#ifdef DMX_UTOPIA_20

    DMX_PVR_FLTTYPE_PARAM PVRFltParam = {0};

    DMX_V2DBGMSG(printf("[%s](%d, 0x%08lx, 0x%p) %d\n", __FUNCTION__, (int)Eng, Pid, pu8DmxId, __LINE__));

    *pu8DmxId = 0xFF;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    PVRFltParam.u32EngId = ((MS_U32)Eng) & 0xFF;
    PVRFltParam.u32DmxId = 0xFF;
    PVRFltParam.u32Pid = Pid;
    PVRFltParam.u32SelShareFlt = u8ShareKeyType;
    PVRFltParam.FltSrc = DMX_FILTER_SOURCE_TYPE_LIVE;
    PVRFltParam.u32CAEngSel = ((MS_U32)Eng) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_PidOpen,(void*)&PVRFltParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu8DmxId = (MS_U8)(PVRFltParam.u32DmxId & 0xFF);

    if(PVRFltParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_PvrCA_Eng_Pid_Open(Eng, Pid, pu8DmxId, u8ShareKeyType);

#endif

}

DMX_FILTER_STATUS MApi_DMX_PvrCA_Eng_Pid_Close(DMX_PVR_ENG Eng, MS_U8 u8DmxId)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s](%d, %d) %d\n", __FUNCTION__, (int)Eng, (int)u8DmxId, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = ((MS_U32)Eng) & 0xFF;
    u32Param[1] = ((MS_U32)u8DmxId) & 0xFF;
    u32Param[2] = 1;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_PidClose,(void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_PvrCA_Eng_Pid_Close(Eng, u8DmxId);

#endif

}

//PVR CA API
//-------------------------------------------------------------------------------------------------------------
// Version 1.0
// PVRCA pair function of PID filter close and open
// 1. MApi_DMX_FlowSet(DMX_FLOW_PVRCA) / MApi_DMX_Pvr_Close ==> Duplicate all FLT0 to FLT1 / Close all FLT1  (PVR1FLT_NOTUSE<->PVR1FLT_CA)
// 2. MApi_DMX_PvrCA_Pid_Open / MApi_DMX_PvrCA_Pid_Close ==> Reserved / Un-reserved FLT1 of record enable (PVR1FLT_CA<->PVR1FLT_CAPVR)
// 3. MApi_DMX_Open / MApi_DMX_Close ==>  If TS1(PVRCA enable), also allocate FIL1 or free FLT1, but if recording, not free FLT1(PVR1FLT_CA->PVR1FLT_NOTUSE)
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/// Open PVR PID filter for recording streamble stream
/// @param Pid                  \b IN: Set the PID to record
/// @param pu8DmxId       \b IN: The pointer stored the PID filter index
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_PvrCA_Pid_Open(MS_U32 Pid, MS_U8* pu8DmxId)
{
   DMX_V2DBGMSG(printf("[%s](0x%08lx, 0x%p) %d\n", __FUNCTION__, Pid, pu8DmxId, __LINE__));

#if (defined(TSP_VERSION) && ((TSP_VERSION == TSP_VER_2_0) || (TSP_VERSION == TSP_VER_4_0)))
    return (MApi_DMX_Pvr_Pid_Open(Pid, pu8DmxId));
#else
    return (MApi_DMX_PvrCA_Eng_Pid_Open(DMX_PVR_EGN0, Pid, pu8DmxId, 0));
#endif
}

//do nothing, before close path switch to TS1
//-------------------------------------------------------------------------------------------------
/// Close PVR PID filter for recording streamble stream
/// @param Pid                  \b IN: Set the PID to record
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_PvrCA_Pid_Close(MS_U8 u8DmxId)
{
     DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

#if (defined(TSP_VERSION) && ((TSP_VERSION == TSP_VER_2_0) || (TSP_VERSION == TSP_VER_4_0)))
    return (MApi_DMX_Pvr_Pid_Close(u8DmxId));
#else
    return (MApi_DMX_PvrCA_Eng_Pid_Close(DMX_PVR_EGN0, u8DmxId));
#endif
}

//-------------------------------------------------------------------------------------------------
/// Start to record scramble stream
/// @param bPvrAll                  \b IN: If TRUE, recording all stream data; if false, recording stream data by PIDs
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_PvrCA_Start(MS_BOOL bPvrAll)
{
     DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

#if (defined(TSP_VERSION) && ((TSP_VERSION == TSP_VER_2_0) || (TSP_VERSION == TSP_VER_4_0)))
    return (MApi_DMX_Pvr_Start(bPvrAll));
#else
    return (MApi_DMX_PvrCA_Eng_Start(DMX_PVR_EGN0, bPvrAll));
#endif
}

//-------------------------------------------------------------------------------------------------
/// Stop to record scramble stream
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_PvrCA_Stop(void)
{
     DMX_V2DBGMSG( printf("[%s] %d\n", __FUNCTION__, __LINE__));

#if (defined(TSP_VERSION) && ((TSP_VERSION == TSP_VER_2_0) || (TSP_VERSION == TSP_VER_4_0)))
    return (MApi_DMX_Pvr_Stop());
#else
    return (MApi_DMX_PvrCA_Eng_Stop(DMX_PVR_EGN0));
#endif
}


DMX_FILTER_STATUS MApi_DMX_PvrCA_Eng_Start(DMX_PVR_ENG Eng, MS_BOOL bPvrAll)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s](%d, %d) %d\n", __FUNCTION__, (int)Eng, (int)bPvrAll, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = ((MS_U32)Eng) & 0xFF;
    u32Param[1] = ((MS_U32)bPvrAll) & 0xFF;
    u32Param[2] = 1;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_Start,(void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_PvrCA_Eng_Start(Eng, bPvrAll);

#endif

}

DMX_FILTER_STATUS MApi_DMX_PvrCA_Eng_Stop(DMX_PVR_ENG Eng)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0, 0, 0};
    
    DMX_V2DBGMSG(printf("[%s](%d) %d\n", __FUNCTION__, (int)Eng, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = ((MS_U32)Eng) & 0xFF;
    u32Param[1] = 1;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_Stop,(void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_PvrCA_Eng_Stop(Eng);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Info(DMX_Filein_info *pFileinInfo)
{

#ifdef DMX_UTOPIA_20

    DMX_FILE_INFO_PARAM FiInfoParam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    FiInfoParam.u32EngId = 0;
    FiInfoParam.pFileinInfo = pFileinInfo;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_Info,(void*)&FiInfoParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(FiInfoParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Filein_Info(pFileinInfo);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Eng_Info(DMX_FILEIN_PATH ePath, DMX_Filein_info *pFileinInfo)
{

#ifdef DMX_UTOPIA_20

	DMX_FILE_INFO_PARAM FiInfoParam = {0};

	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	FiInfoParam.u32EngId = (MS_U32)ePath;
	FiInfoParam.pFileinInfo = pFileinInfo;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_Eng_Info,(void*)&FiInfoParam) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}
	
	if(FiInfoParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_Filein_Eng_Info(ePath, pFileinInfo);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Start(DMX_FILEIN_DST Dst, MS_PHYADDR pBuf, MS_U32 u32BufSize)
{

#ifdef DMX_UTOPIA_20

    DMX_FILE_START_PARAM FiStartParam ={0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    FiStartParam.u32EngId = 0;
    FiStartParam.Dst = Dst;
    FiStartParam.pBuf = pBuf;
    FiStartParam.u32BufSize = u32BufSize;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_Start,(void*)&FiStartParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(FiStartParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Filein_Start(Dst, pBuf, u32BufSize);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Eng_Start(DMX_FILEIN_PATH ePath, DMX_FILEIN_DST Dst, MS_PHYADDR pBuf, MS_U32 u32BufSize)
{
#ifdef DMX_UTOPIA_20

	DMX_FILE_START_PARAM FiStartParam = {0};

	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	FiStartParam.u32EngId = (MS_U32)ePath;
	FiStartParam.Dst = Dst;
	FiStartParam.pBuf = pBuf;
	FiStartParam.u32BufSize = u32BufSize;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_Start,(void*)&FiStartParam) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}
	
	if(FiStartParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_Filein_Eng_Start(ePath, Dst, pBuf, u32BufSize);

#endif

}


DMX_FILTER_STATUS MApi_DMX_Filein_Stop(void)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[2] = {0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_Stop,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Filein_Stop();

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Eng_Stop(DMX_FILEIN_PATH ePath)
{

#ifdef DMX_UTOPIA_20
	
	MS_U32 u32param[2] = {0, 0};

    u32param[0] = (MS_U32)ePath;
	
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;
	
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_Stop,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}
	
    if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_Filein_Eng_Stop(ePath);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Pause(void)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[2] = {0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_Pause,(void*)&u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
    
#else

    return _MApi_DMX_Filein_Pause();

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Eng_Pause(DMX_FILEIN_PATH ePath)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[2] = {0, 0};

    u32param[0] = (MS_U32)ePath;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_Pause,(void*)&u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Filein_Eng_Pause(ePath);

#endif

}


DMX_FILTER_STATUS MApi_DMX_Filein_Resume(void)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[2] = {0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));   
    
    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_Resume,(void*)&u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
    
#else

    return _MApi_DMX_Filein_Resume();

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Eng_Resume(DMX_FILEIN_PATH ePath)
{

#ifdef DMX_UTOPIA_20
	
	MS_U32 u32param[2] = {0, 0};

    u32param[0] = (MS_U32)ePath;
	
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_Resume,(void*)&u32param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}
	
	if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_Filein_Eng_Resume(ePath);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_CMDQ_Reset(void)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[2] = {0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_CmdQReset,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Filein_CMDQ_Reset();

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Eng_CMDQ_Reset(DMX_FILEIN_PATH ePath)
{

#ifdef DMX_UTOPIA_20
	
	MS_U32 u32param[2] = {0, 0};

    u32param[0] = (MS_U32)ePath;
	
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_CmdQReset,(void*)&u32param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}
	
	if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_Filein_Eng_CMDQ_Reset(ePath);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_CMDQ_GetEmptyNum(MS_U32 *pu32EmptySlot)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32EmptySlot = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_CmdQGetEmptyNum,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32EmptySlot = u32param[1];

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Filein_CMDQ_GetEmptyNum(pu32EmptySlot);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Eng_CMDQ_GetEmptyNum(DMX_FILEIN_PATH ePath, MS_U32 *pu32EmptySlot)
{

#ifdef DMX_UTOPIA_20
	
	MS_U32 u32param[3] = {0, 0, 0};
	
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	*pu32EmptySlot = 0;

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	u32param[0] = (MS_U32)ePath;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_CmdQGetEmptyNum,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}

	*pu32EmptySlot = u32param[1];
	
	if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else
	
	return _MApi_DMX_Filein_Eng_CMDQ_GetEmptyNum(ePath, pu32EmptySlot);
	
#endif
	
}

DMX_FILTER_STATUS MApi_DMX_Filein_CMDQ_FIFOWriteLevel(MS_U8 *pu8CmdQStatus)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu8CmdQStatus = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_CmdQGetFifoLevel,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu8CmdQStatus = (MS_U8)(u32param[1]);

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Filein_CMDQ_FIFOWriteLevel(pu8CmdQStatus);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Eng_CMDQ_FIFOWriteLevel(DMX_FILEIN_PATH ePath, MS_U8 *pu8CmdQStatus)
{

#ifdef DMX_UTOPIA_20
	
	MS_U32 u32param[3] = {0, 0, 0};
	
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	*pu8CmdQStatus = 0;

	u32param[0] = (MS_U32)ePath;
	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_CmdQGetFifoLevel,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}

	*pu8CmdQStatus = (MS_U8)(u32param[1]);
	
	if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_Filein_Eng_CMDQ_FIFOWriteLevel(ePath, pu8CmdQStatus);

#endif

}

MS_BOOL MApi_DMX_Filein_IsIdle(void)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[2] = {0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return FALSE;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_IsIdle,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return FALSE;
    }

    return ((MS_BOOL)u32param[1]);

#else

    return _MApi_DMX_Filein_IsIdle();

#endif

}

MS_BOOL MApi_DMX_Filein_Eng_IsIdle(DMX_FILEIN_PATH ePath)
{

#ifdef DMX_UTOPIA_20

	MS_U32 u32param[2] = {0, 0};

	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return FALSE;

	u32param[0] = (MS_U32)ePath;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_IsIdle,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return FALSE;
	}

	return ((MS_BOOL)u32param[1]);

#else

	return _MApi_DMX_Filein_Eng_IsIdle(ePath);

#endif

}


MS_BOOL MApi_DMX_Filein_IsBusy(void)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[2] = {0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return FALSE;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_IsBusy,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return FALSE;
    }

    return ((MS_BOOL)u32param[1]);

#else

    return _MApi_DMX_Filein_IsBusy();

#endif

}

MS_BOOL MApi_DMX_Filein_Eng_IsBusy(DMX_FILEIN_PATH ePath)
{

#ifdef DMX_UTOPIA_20

	MS_U32 u32param[2] = {0, 0};

	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return FALSE;

	u32param[0] = (MS_U32)ePath;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_IsBusy,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return FALSE;
	}

	return ((MS_BOOL)u32param[1]);

#else

	return _MApi_DMX_Filein_Eng_IsBusy(ePath);

#endif

}

MS_BOOL MApi_DMX_Filein_IsPause(void)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[2] = {0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return FALSE;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_IsPause,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return FALSE;
    }

    return ((MS_BOOL)u32param[1]);

#else

    return _MApi_DMX_Filein_IsPause();

#endif

}

MS_BOOL MApi_DMX_Filein_Eng_IsPause(DMX_FILEIN_PATH ePath)
{

#ifdef DMX_UTOPIA_20

	MS_U32 u32param[2] = {0, 0};

	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return FALSE;

	u32param[0] = (MS_U32)ePath;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_IsPause,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return FALSE;
	}

	return ((MS_BOOL)u32param[1]);

#else

	return _MApi_DMX_Filein_Eng_IsPause(ePath);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Open_MultiFlt(DMX_FILTER_TYPE DmxFltType , MS_U8* pu8DmxId, MS_U8 u8TargetDmxId)
{
#ifdef DMX_UTOPIA_20
		
	MS_U32 u32param[4] = {0, 0, 0, 0};
		
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	*pu8DmxId = 0;

	if(_DMX_IOCtrlOpen() == FALSE)
		return FALSE;

	u32param[0] = (MS_U32)DmxFltType;
	u32param[2] = ((MS_U32)u8TargetDmxId) & 0xFF;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Open_MultiFlt,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}

	*pu8DmxId = (MS_U8)u32param[1];
		
	if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
		
#else

	return _MApi_DMX_Open_MultiFlt(DmxFltType, pu8DmxId, u8TargetDmxId);

#endif

}

//-------------------------------------------------------------------------------------------------
/// Get STC counter
/// @param pu32Stc32 \b OUT: STC counter (bit 32)
/// @param pu32Stc \b OUT: STC counter (bit 31 to bit 0)
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_Stc_Get(MS_U32* pu32Stc32, MS_U32* pu32Stc)
{
    return MApi_DMX_Stc_Eng_Get(0, pu32Stc32, pu32Stc);
}

//-------------------------------------------------------------------------------------------------
/// Set STC counter
/// @param u32Stc32 \b IN: STC counter (bit 32)
/// @param u32Stc \b IN: STC counter (bit 31 to bit 0)
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_Stc_Set(MS_U32 u32Stc32, MS_U32 u32Stc)
{
    return MApi_DMX_Stc_Eng_Set(0, u32Stc32, u32Stc);
}

DMX_FILTER_STATUS MApi_DMX_Stc_Eng_Get(MS_U8 u8Eng, MS_U32* pu32Stc32, MS_U32* pu32Stc)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32Stc32 = 0;
    *pu32Stc = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = ((MS_U32)u8Eng) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Stc_Get,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32Stc32 = u32param[1];
    *pu32Stc = u32param[2];

    if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else

    return _MApi_DMX_Stc_Eng_Get(u8Eng, pu32Stc32, pu32Stc);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Stc_Eng_Set(MS_U8 u8Eng, MS_U32 u32Stc32, MS_U32 u32Stc)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = ((MS_U32)u8Eng) & 0xFF;
    u32param[1] = u32Stc32;
    u32param[2] = u32Stc;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Stc_Set,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Stc_Eng_Set(u8Eng, u32Stc32, u32Stc);

#endif

}

DMX_FILTER_STATUS MApi_DMX_STC_UpdateCtrl(MS_U8 u8Eng, eStcUpdateCtrlMode eMode)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = ((MS_U32)u8Eng) & 0xFF;
    u32param[1] = ((MS_U32)eMode) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Stc_UpdateCtrl,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_STC_UpdateCtrl(u8Eng, eMode);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Stc_Eng_SetOffset(MS_U32 u32Eng, MS_U32 u32Offset, MS_BOOL bAdd)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = u32Eng;
    u32param[1] = u32Offset;
    u32param[2] = ((MS_U32)bAdd) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Stc_SetOffset,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Stc_Eng_SetOffset(u32Eng, u32Offset, bAdd);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Stc_Clk_Adjust(MS_U32 u32EngId, MS_BOOL bUpClk, MS_U32 u32Percentage)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = u32EngId;
    u32param[1] = ((MS_U32)bUpClk) & 0xFF;
    u32param[2] = u32Percentage;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Stc_ClkAdjust,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Stc_Clk_Adjust(u32EngId, bUpClk, u32Percentage);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Stc_Select(DMX_FILTER_TYPE eFltSrc, MS_U32 u32StcEng)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = eFltSrc;
    u32param[1] = u32StcEng;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_STCEng_Sel,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Stc_Select(eFltSrc, u32StcEng);

#endif
}

DMX_FILTER_STATUS MApi_DMX_Pcr_Get(MS_U32* pu32Pcr32, MS_U32* pu32Pcr)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32Pcr32 = 0;
    *pu32Pcr = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_GetPcr,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32Pcr32 = u32param[1];
    *pu32Pcr = u32param[2];

    if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pcr_Get(pu32Pcr32, pu32Pcr);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pcr_Eng_Get(MS_U8 u8PcrEng, MS_U32* pu32Pcr32, MS_U32* pu32Pcr)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[4] = {0, 0, 0, 0};

        DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

        *pu32Pcr32 = 0;
        *pu32Pcr = 0;

        if(_DMX_IOCtrlOpen() == FALSE)
            return DMX_FILTER_STATUS_ERROR;

        u32param[0] = ((MS_U32)u8PcrEng) & 0xFF;
        if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_GetPcr,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
        {
            DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
            return DMX_FILTER_STATUS_ERROR;
        }

        *pu32Pcr32 = u32param[1];
        *pu32Pcr = u32param[2];

    if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

        return _MApi_DMX_Pcr_Eng_Get(u8PcrEng, pu32Pcr32, pu32Pcr);

#endif
}

DMX_FILTER_STATUS MApi_DMX_Proc(MS_U8 u8DmxId, DMX_EVENT* pEvent)      // for non-OS TSP scheduling
{

#ifdef DMX_UTOPIA_20

    DMX_FLT_EVENT_PARAM FltEventParam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    FltEventParam.u32DmxId = ((MS_U32)u8DmxId & 0xFF);
    FltEventParam.pEvent = pEvent;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Proc,(void*)&FltEventParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(FltEventParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Proc(u8DmxId, pEvent);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Parl_Invert(DMX_FLOW DmxFlow, MS_BOOL bInvert)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = (MS_U32)DmxFlow;
    u32param[1] = ((MS_U32)bInvert & 0xFF);
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Flow_ParlInvert,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Parl_Invert(DmxFlow, bInvert);

#endif

}

DMX_FILTER_STATUS MApi_DMX_CopyData(MS_U8 u8DmxId, MS_U8* pu8Buf, MS_U32 u32BufSize, MS_U32* pu32ActualSize, MS_U32* pu32RmnSize, DMX_CheckCb pfCheckCB)
{

#ifdef DMX_UTOPIA_20

    DMX_FLT_COPY_PARAM FltCopyParam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32ActualSize = 0;
    *pu32RmnSize = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    FltCopyParam.u32DmxId       = ((MS_U32)u8DmxId) & 0xFF;
    FltCopyParam.pu8Buf         = pu8Buf;
    FltCopyParam.u32BufSize     = u32BufSize;
    FltCopyParam.pfCheckCB      = pfCheckCB;
    FltCopyParam.u32ActualSize  = 0;
    FltCopyParam.u32RmnSize     = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_CopyData,(void*)&FltCopyParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32ActualSize = FltCopyParam.u32ActualSize;
    *pu32RmnSize = FltCopyParam.u32RmnSize;

    if(FltCopyParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_CopyData(u8DmxId, pu8Buf, u32BufSize, pu32ActualSize, pu32RmnSize, pfCheckCB);

#endif

}

#if (!(defined(TSP_VERSION) && ((TSP_VERSION == TSP_VER_2_0) || (TSP_VERSION == TSP_VER_4_0))))

void MApi_DMX_WriteProtect_Enable(MS_BOOL bEnable, MS_U32* pu32StartAddr, MS_U32* pu32EndAddr)
{

#ifdef DMX_UTOPIA_20

    DMX_WP_PARAM stWPParam;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return;

    stWPParam.bEnable       = bEnable;
    stWPParam.pu32StartAddr = pu32StartAddr;
    stWPParam.pu32EndAddr   = pu32EndAddr;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_WProtectEnable,(void*)&stWPParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return;
    }

#else

    _MApi_DMX_WriteProtect_Enable(bEnable, pu32StartAddr, pu32EndAddr);

#endif

}

void MApi_DMX_OrzWriteProtect_Enable(MS_BOOL bEnable, MS_U32 u32StartAddr, MS_U32 u32EndAddr)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return;

    u32Param[0] = ((MS_U32)bEnable) & 0xFF;
    u32Param[1] = (MS_U32)u32StartAddr;
    u32Param[2] = (MS_U32)u32EndAddr;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_OrzWProtectEnable,(void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return;
    }

#else

    _MApi_DMX_OrzWriteProtect_Enable(bEnable, u32StartAddr, u32EndAddr);

#endif

}

DMX_FILTER_STATUS MApi_DMX_FlowEnable(DMX_FLOW DmxFlow, MS_BOOL bEnable)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = (MS_U32)DmxFlow;
    u32Param[1] = ((MS_U32)bEnable) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Flow_ENABLE,(void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_FlowEnable(DmxFlow, bEnable);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Flow_DscmbEng(DMX_TSIF eTsIf, MS_U32* pu32EngId, MS_BOOL bSet)
{
#ifdef DMX_UTOPIA_20
    
    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    DMX_FLOW_DSCMBENG_PARAM stDmxDscmbEng = {0};

    stDmxDscmbEng.eTsif = eTsIf;
    stDmxDscmbEng.bSet = bSet;
    if(bSet == TRUE)
    {
        stDmxDscmbEng.u32DscmbEng = *pu32EngId;
    }
    else
    {
        *pu32EngId = 0xFF;
    }

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FlowDscmbEng,(void*)&stDmxDscmbEng) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(bSet == FALSE)
    {
        *pu32EngId = stDmxDscmbEng.u32DscmbEng;
    }
    
    if(stDmxDscmbEng.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK; 

#else

    return _MApi_DMX_Flow_DscmbEng(eTsIf, pu32EngId, bSet);

#endif

}

DMX_FILTER_STATUS MApi_DMX_TsOutputPadCfg(DMX_FLOW_OUTPUT_PAD eOutPad, DMX_FLOW_INPUT eInSrcPad, MS_BOOL bInParallel, MS_U32 u32ResvNum, MS_U32 *pu32Resv)
{

#ifdef DMX_UTOPIA_20

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    DMX_OutputPad_Cfg stOutPadCfg = {0};

    stOutPadCfg.eOutPad = eOutPad;
    stOutPadCfg.eInSrcPad = eInSrcPad;
    stOutPadCfg.bInParallel = bInParallel;
    stOutPadCfg.u32ResvNum = u32ResvNum;
    stOutPadCfg.pu32Resv = pu32Resv;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_OutPadCfg,(void*)&stOutPadCfg) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }
    
    if(stOutPadCfg.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK; 
    
#else

    return _MApi_DMX_TsOutputPadCfg(eOutPad, eInSrcPad, bInParallel, u32ResvNum, pu32Resv);

#endif

}

DMX_FILTER_STATUS MApi_DMX_TsS2POutputClkPhase(MS_U16 u16Val, MS_BOOL bEnable, MS_U32 u32S2pOpt)
{
#ifdef DMX_UTOPIA_20
    
    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));
    
    MS_U32 u32param[4] = {0, 0, 0, 0};

    u32param[0] = ((MS_U32)u16Val) & 0xFFFF;
    u32param[1] = ((MS_U32)bEnable) & 0xFF;
    u32param[2] = u32S2pOpt;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;
    
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TsOutPhase,(void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }
        
    if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK; 
        
#else

    return _MApi_DMX_TsS2POutputClkPhase(u16Val, bEnable, u32S2pOpt);

#endif
    
}

DMX_FILTER_STATUS MApi_DMX_Read_DropPktCnt(MS_U16* pu16ADropCnt, MS_U16* pu16VDropCnt)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu16ADropCnt = 0;
    *pu16VDropCnt = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_ReadDropCount,(void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu16ADropCnt = (MS_U16)(u32Param[0]);
    *pu16VDropCnt = (MS_U16)(u32Param[1]);

    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK; 

#else

    return _MApi_DMX_Read_DropPktCnt(pu16ADropCnt, pu16VDropCnt);

#endif

}

#endif

DMX_FILTER_STATUS MApi_DMX_SetOwner(MS_U8 u8DmxIdStart, MS_U8 u8DmxIdEnd, MS_BOOL bOwner)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = ((MS_U32)u8DmxIdStart) & 0xFF;
    u32Param[1] = ((MS_U32)u8DmxIdEnd) & 0xFF;
    u32Param[2] = ((MS_U32)bOwner) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_SetOwner,(void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_SetOwner(u8DmxIdStart, u8DmxIdEnd, bOwner);

#endif

}

DMX_FILTER_STATUS MApi_DMX_GetOwner(MS_U8 u8DmxId, MS_BOOL* pbOwner)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pbOwner = FALSE;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = ((MS_U32)u8DmxId) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_GetOwner,(void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pbOwner = (MS_BOOL)u32Param[1];

    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_GetOwner(u8DmxId, pbOwner);

#endif

}

DMX_FILTER_STATUS MApi_DMX_GetAccess(MS_U32 u32Try)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[2] = {0, 0};

    u32Param[0]= u32Try;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_GetAccess,(void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_GetAccess(u32Try);

#endif

}

DMX_FILTER_STATUS MApi_DMX_ReleaseAccess(void)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param = 0;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_ReleaseAccess, (void*)&u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_ReleaseAccess();

#endif

}

DMX_FILTER_STATUS MApi_DMX_SetHK(MS_BOOL bIsHK)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32Data[2] = {0, 0};

    u32Data[0]= (MS_U32)bIsHK;

    DMX_V2DBGMSG(printf("[%s][%d](%d)\n", __FUNCTION__, __LINE__, (int)bIsHK));

    if(_DMX_IOCtrlOpen() == FALSE)
    {
        return DMX_FILTER_STATUS_ERROR;
    }

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_SetHK, (void*)u32Data) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Data[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_SetHK(bIsHK);

#endif

}

DMX_FILTER_STATUS MApi_DMX_AVFifo_Reset(DMX_FILTER_TYPE DmxFltType, MS_BOOL bReset)
{

#ifdef DMX_UTOPIA_20

    DMX_AVFIFO_PARAM AvfifoParam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    AvfifoParam.DmxFltType = DmxFltType;
    AvfifoParam.u32Data = ((MS_U32)bReset) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_AVFIFO_Reset, (void*)&AvfifoParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(AvfifoParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else

    return _MApi_DMX_AVFifo_Reset(DmxFltType, bReset);

#endif

}

DMX_FILTER_STATUS MApi_DMX_AVFifo_Status(DMX_FILTER_TYPE DmxFltType, DMX_FIFO_STATUS* u32FifoLevel)
{

#ifdef DMX_UTOPIA_20

    DMX_AVFIFO_PARAM AvfifoParam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *u32FifoLevel = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    AvfifoParam.DmxFltType  = DmxFltType;
    AvfifoParam.u32Data     = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_AVFIFO_Status, (void*)&AvfifoParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *u32FifoLevel = AvfifoParam.u32Data;

    if(AvfifoParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_AVFifo_Status(DmxFltType, u32FifoLevel);

#endif

}

DMX_FILTER_STATUS MApi_DMX_GetLibVer(const MSIF_Version **ppVersion)
{
    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

#ifndef UTOPIA_STRIP  
    return _MApi_DMX_GetLibVer(ppVersion);
#else
    return DMX_FILTER_STATUS_ERROR;
#endif
}

DMX_FILTER_STATUS MApi_DMX_GetCap(DMX_QUERY_TYPE DmxQueryType, void* pOutput)
{

#ifdef DMX_UTOPIA_20

    DMX_CAP_PARAM CapParam = {0};

    DMX_V2DBGMSG(printf("[%s][%d](%d , %p)\n", __FUNCTION__, __LINE__, (int)DmxQueryType, pOutput));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    CapParam.Quetype = DmxQueryType;
    CapParam.pdata = pOutput;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_GetCap, (void*)&CapParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }
    
    if(CapParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_GetCap(DmxQueryType, pOutput);

#endif

}

DMX_FILTER_STATUS MApi_DMX_GetCap_Ex(char* pstrQueryType, int inStrlen, void* pOutput, int* pOutSize)
{

#ifdef DMX_UTOPIA_20

    DMX_CAP_EX_PARAM CapExParam ={0};

    DMX_V2DBGMSG(printf("[%s][%d](%s , %p)\n", __FUNCTION__, __LINE__, pstrQueryType, pOutput));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    CapExParam.InputStrLen = inStrlen;
    CapExParam.StrQuetype = pstrQueryType;
    CapExParam.OutputSize = 0;
    CapExParam.pdata = pOutput;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_GetCapEx, (void*)&CapExParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pOutSize = CapExParam.OutputSize;
    
    if(CapExParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;   

#else

    return _MApi_DMX_GetCap_Ex(pstrQueryType, inStrlen, pOutput, pOutSize);

#endif

}


DMX_FILTER_STATUS MApi_DMX_SetBurstLen(DMX_BURSTTYPE BurstType)
{

#ifdef DMX_UTOPIA_20

    DMX_BURSTTYPE type = BurstType;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_SetBurstLen, (void*)&type) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_SetBurstLen(BurstType);

#endif

}

DMX_FILTER_STATUS MApi_DMX_GetFileInTimeStamp(MS_U32 *pu32FileInTS)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32FileInTS = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_GetFileTimeStamp, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32FileInTS = u32Param[1];

    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_GetFileInTimeStamp(pu32FileInTS);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Eng_GetFileInTimeStamp(DMX_FILEIN_PATH ePath, MS_U32 *pu32FileInTS)
{

#ifdef DMX_UTOPIA_20
	
	MS_U32 u32Param[3] = {0, 0, 0};
	
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	*pu32FileInTS = 0;

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	u32Param[0] = (MS_U32)ePath;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_GetFileTimeStamp, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}

	*pu32FileInTS = u32Param[1];
	
    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_Filein_Eng_GetFileInTimeStamp(ePath, pu32FileInTS);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_GetReadAddr(MS_U32* pu32Read)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32Read = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_GetReadAddr, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32Read = u32Param[1];

    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else

    return _MApi_DMX_Filein_GetReadAddr(pu32Read);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Eng_GetReadAddr(DMX_FILEIN_PATH ePath, MS_U32* pu32Read)
{

#ifdef DMX_UTOPIA_20
	
	MS_U32 u32Param[3] = {0, 0, 0};
	
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	*pu32Read = 0;

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	u32Param[0] = (MS_U32)ePath;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_GetReadAddr, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}

	*pu32Read = u32Param[1];
	
	if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_Filein_Eng_GetReadAddr(ePath, pu32Read);

#endif

}

void MApi_DMX_BypassFileInTimeStamp(MS_BOOL bbypass)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[2] = {0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return;

    u32Param[1] = ((MS_U32)bbypass) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_BypassFileTimestamp, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return;
    }

#else

    _MApi_DMX_BypassFileInTimeStamp(bbypass);

#endif

}

void MApi_DMX_Filein_Eng_BypassFileInTimeStamp(DMX_FILEIN_PATH ePath, MS_BOOL bbypass)
{

#ifdef DMX_UTOPIA_20

	MS_U32 u32Param[2] = {0, 0};

	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return;

	u32Param[0] = (MS_U32)ePath;
	u32Param[1] = ((MS_U32)bbypass) & 0xFF;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_BypassFileTimestamp, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return;
	}

#else

	_MApi_DMX_Filein_Eng_BypassFileInTimeStamp(ePath, bbypass);

#endif

}

DMX_FILTER_STATUS MApi_TSP_Get_FW_VER(MS_U32* u32FWVer)
{
    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

#ifndef UTOPIA_STRIP     
    return _MApi_TSP_Get_FW_VER(u32FWVer);
#else    
    return DMX_FILTER_STATUS_ERROR;
#endif
}

DMX_FILTER_STATUS MApi_DMX_RemoveDupAVPkt(MS_BOOL bEnable)
{

#ifdef DMX_UTOPIA_20

    DMX_AVFIFO_DROP_PARAM AvFifoDrop ={0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    AvFifoDrop.bAllFifo = TRUE;
    AvFifoDrop.bEnable = bEnable;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_RemoveDupAVFifoPkt, (void*)&AvFifoDrop) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(AvFifoDrop.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_RemoveDupAVPkt(bEnable);

#endif

}

DMX_FILTER_STATUS MApi_DMX_RemoveDupAVFifoPkt(DMX_FILTER_TYPE DmxFltType, MS_BOOL bEnable)
{

#ifdef DMX_UTOPIA_20

    DMX_AVFIFO_DROP_PARAM AvFifoDrop = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    AvFifoDrop.bAllFifo   = FALSE;
    AvFifoDrop.bEnable    = bEnable;
    AvFifoDrop.DmxFltType = DmxFltType;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_RemoveDupAVFifoPkt, (void*)&AvFifoDrop) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(AvFifoDrop.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_RemoveDupAVFifoPkt(DmxFltType, bEnable);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Change_FilterSource(MS_U8 u8DmxId, DMX_FILTER_TYPE DmxFltSrcType)
{

#ifdef DMX_UTOPIA_20

    DMX_FLT_TYPE_PARAM FltTypeParam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    FltTypeParam.u32DmxId = ((MS_U32)u8DmxId) & 0xFF;
    FltTypeParam.DmxFltType = DmxFltSrcType;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_ChangeFltSrc, (void*)&FltTypeParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(FltTypeParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Change_FilterSource(u8DmxId, DmxFltSrcType);

#endif

}

DMX_FILTER_STATUS MApi_DMX_SetDbgLevel(DMX_DBGMSG_LEVEL level)
{

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[2] = {0, 0};

    u32param[0] = (MS_U32)level;
    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_SetDbgLevel, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_SetDbgLevel(level);

#endif

}

DMX_FILTER_STATUS MApi_DMX_STC64_Mode_Enable(MS_BOOL bEnable)
{

#ifdef DMX_UTOPIA_20

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    MS_U32 u32Param[2] = {0, 0};

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_STC64ModeEnable, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_STC64_Mode_Enable(bEnable);

#endif

}

MS_BOOL MApi_DMX_MMFI_Filein_IsIdle(DMX_MMFI_PATH ePath)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[2] = {0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return FALSE;

    u32param[0] = (MS_U32)ePath;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_IsIdle, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return FALSE;
    }

    return (MS_BOOL)(u32param[1]);

#else

    return _MApi_DMX_MMFI_Filein_IsIdle(ePath);

#endif

}

MS_BOOL MApi_DMX_MMFI_Filein_IsBusy(DMX_MMFI_PATH ePath)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[2] = {0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return FALSE;

    u32param[0] = (MS_U32)ePath;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_IsBusy, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return FALSE;
    }

    return (MS_BOOL)(u32param[1]);

#else

    return _MApi_DMX_MMFI_Filein_IsBusy(ePath);

#endif

}

DMX_FILTER_STATUS MApi_DMX_MMFI_Filein_CMDQ_Reset(DMX_MMFI_PATH ePath)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[2] = {0, 0};

    u32param[0] = (MS_U32)ePath;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_CmdQReset, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_MMFI_Filein_CMDQ_Reset(ePath);

#endif

}

DMX_FILTER_STATUS MApi_DMX_MMFI_Filein_CMDQ_GetEmptyNum(DMX_MMFI_PATH ePath, MS_U32 *pu32EmptySlot)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32EmptySlot = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = (MS_U32)ePath;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_GetEmptyNum, (void*)&u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32EmptySlot = u32param[1];

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_MMFI_Filein_CMDQ_GetEmptyNum(ePath, pu32EmptySlot);

#endif

}

DMX_FILTER_STATUS MApi_DMX_MMFI_Filein_Start(DMX_MMFI_DST eDst, MS_PHYADDR pBuf, MS_U32 u32BufSize)
{

#ifdef DMX_UTOPIA_20

    DMX_MMFI_START_PARAM FltStartParam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    FltStartParam.Dst = eDst;
    FltStartParam.pBuf = pBuf;
    FltStartParam.u32BufSize = u32BufSize;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_Start, (void*)&FltStartParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(FltStartParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_MMFI_Filein_Start(eDst, pBuf, u32BufSize);

#endif

}

DMX_FILTER_STATUS MApi_DMX_MMFI_GetFileInTimeStamp(DMX_MMFI_PATH ePath, MS_U32 *pu32FileInTS)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32FileInTS = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = (MS_U32)ePath;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_GetFileTimeStamp, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32FileInTS = u32param[1];

    
    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_MMFI_GetFileInTimeStamp(ePath, pu32FileInTS);

#endif

}

DMX_FILTER_STATUS MApi_DMX_MMFI_Pid_Open(DMX_MMFI_FLTTYPE flttype, MS_U16 u16Pid, MS_U8* pu8DmxId)
{

#ifdef DMX_UTOPIA_20

    DMX_MMFI_FLT_PARAM MmfiFltParam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu8DmxId = 0xFF;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    MmfiFltParam.flttype = (DMX_MMFI_FLTTYPE)flttype;
    MmfiFltParam.u32DmxId = 0xFF;
    MmfiFltParam.u32Pid = ((MS_U32)u16Pid) & 0xFFFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_PidOpen, (void*)&MmfiFltParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu8DmxId = (MS_U8)(MmfiFltParam.u32DmxId & 0xFF);

    if(MmfiFltParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_MMFI_Pid_Open(flttype, u16Pid, pu8DmxId);

#endif

}

DMX_FILTER_STATUS MApi_DMX_MMFI_Pid_Close(MS_U8 u8DmxId)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[1] = ((MS_U32)u8DmxId) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_PidClose, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else

    return _MApi_DMX_MMFI_Pid_Close(u8DmxId);

#endif

}

DMX_FILTER_STATUS MApi_DMX_MMFI_Filein_CMDQ_FIFOWriteLevel(DMX_MMFI_PATH ePath, MS_U8 *pu8CmdQStatus)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu8CmdQStatus = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = (MS_U32)ePath;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_GetFifoLevel, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu8CmdQStatus = (MS_U8)u32Param[1];
    
    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_MMFI_Filein_CMDQ_FIFOWriteLevel(ePath, pu8CmdQStatus);

#endif

}

DMX_FILTER_STATUS MApi_DMX_MMFI_SetPlaybackTimeStamp(DMX_MMFI_PATH ePath, MS_U32 u32pcr2)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = (MS_U32)ePath;
    u32Param[1] = u32pcr2;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_PlayStamp_Set, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_MMFI_SetPlaybackTimeStamp(ePath, u32pcr2);

#endif

}

DMX_FILTER_STATUS MApi_DMX_MMFI_GetPlaybackTimeStamp(DMX_MMFI_PATH ePath, MS_U32 *pu32pcr2)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32pcr2 = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = (MS_U32)ePath;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_PlayStamp_Get, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32pcr2 = u32Param[1];

    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_MMFI_GetPlaybackTimeStamp(ePath, pu32pcr2);

#endif

}

DMX_FILTER_STATUS MApi_DMX_MMFI_RemoveDupAVPkt(MS_BOOL bEnable)
{
#ifdef DMX_UTOPIA_20

    DMX_AVFIFO_DROP_PARAM AVFifoDropParam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    AVFifoDropParam.DmxFltType = 0;
    AVFifoDropParam.bAllFifo = TRUE;
    AVFifoDropParam.bEnable = bEnable;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_RemoveDupAVPkt, (void*)&AVFifoDropParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(AVFifoDropParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_MMFI_RemoveDupAVPkt(bEnable);

#endif

}

DMX_FILTER_STATUS MApi_DMX_MMFI_MOBF_Enable(DMX_MMFI_PATH ePath, MS_BOOL bEnable, MS_U32 u32key)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = ((MS_U32)ePath) & 0xFF;
    u32param[1] = u32key;
    u32param[2] = ((MS_U32)bEnable) & 0xFF;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_SetMOBFKey, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_MMFI_MOBF_Enable(ePath, bEnable, u32key);

#endif

}

DMX_FILTER_STATUS MApi_DMX_MMFI_TimeStampClk(DMX_MMFI_PATH ePath, DMX_TimeStamp_Clk eClk)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = (MS_U32)ePath;
    u32Param[1] = (MS_U32)eClk;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_SetTimeStampClk, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_MMFI_TimeStampClk(ePath, eClk);

#endif

}

DMX_FILTER_STATUS MApi_DMX_MMFI_TimeStampEnable(DMX_MMFI_PATH ePath)
{

#ifdef DMX_UTOPIA_20
	
	MS_U32 u32Param[3] = {0, 0, 0};
	
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	u32Param[0] = (MS_U32)ePath;
	u32Param[1] = 1;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_Timestamp_En, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}
	
	if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_MMFI_TimeStampEnable(ePath);

#endif

}

DMX_FILTER_STATUS MApi_DMX_MMFI_TimeStampDisable(DMX_MMFI_PATH ePath)
{

#ifdef DMX_UTOPIA_20
	
	MS_U32 u32Param[3] = {0, 0, 0};
	
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	u32Param[0] = (MS_U32)ePath;
	u32Param[1] = 0;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_Timestamp_En, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}
	
	if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_MMFI_TimeStampDisable(ePath);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_MOBF_Enable(MS_U8 u8Eng, MS_BOOL bEnable, MS_U32 u32key0, MS_U32 u32key1)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[5] = {0, 0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = ((MS_U32)u8Eng) & 0xFF;
    u32Param[1] = u32key0;
    u32Param[2] = u32key1;
    u32Param[3] = ((MS_U32)bEnable) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_SetMobfKey, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[4] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pvr_Eng_MOBF_Enable(u8Eng, bEnable, u32key0, u32key1);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_MOBF_Enable(MS_BOOL bEnable, MS_U32 u32key)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[1] = ((MS_U32)bEnable) & 0xFF;
    u32Param[2] = u32key;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_SetMobfKey, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Filein_MOBF_Enable(bEnable, u32key);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Eng_MOBF_Enable(DMX_FILEIN_PATH ePath, MS_BOOL bEnable, MS_U32 u32key)
{

#ifdef DMX_UTOPIA_20
	
	MS_U32 u32Param[4] = {0, 0, 0, 0};
	
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	u32Param[0] = (MS_U32)ePath;
	u32Param[1] = ((MS_U32)bEnable) & 0xFF;
	u32Param[2] = u32key;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_SetMobfKey, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}
	
	if(u32Param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_Filein_Eng_MOBF_Enable(ePath, bEnable, u32key);

#endif

}


DMX_FILTER_STATUS MApi_DMX_MMFI_Filein_Info(DMX_MMFI_PATH ePath, DMX_Filein_info *pFileinInfo)
{

#ifdef DMX_UTOPIA_20

	DMX_FILE_INFO_PARAM stparam = {0};

	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	stparam.u32EngId = (MS_U32)ePath;
	stparam.pFileinInfo = pFileinInfo;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_Info, (void*)&stparam) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}
	
	if(stparam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_MMFI_Filein_Info(ePath, pFileinInfo);

#endif

}

void MApi_DMX_MMFI_Filein_BypassTimeStamp(DMX_MMFI_PATH ePath, MS_BOOL bbypass)
{

#ifdef DMX_UTOPIA_20

	MS_U32 u32param[2] = {0, 0};

	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return;

	u32param[0] = (MS_U32)ePath;
	u32param[1] = ((MS_U32)bbypass) & 0xFF;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MMFI_BypassStamp, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return;
	}		
		
#else

	_MApi_DMX_MMFI_Filein_BypassTimeStamp(ePath, bbypass);

#endif

}

DMX_FILTER_STATUS MApi_DMX_GetDbgPortInfo(MS_U32 u32DbgSel,MS_U32* u32DbgInfo)
{

#ifdef DMX_UTOPIA_20
			
	MS_U32 u32param[3] = {0, 0, 0};
			
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	u32param[0] = u32DbgSel;
	u32param[1] = 0;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Get_DbgPortInfo, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}

	*u32DbgInfo = u32param[1];
		
	if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
			
#else

	return _MApi_DMX_GetDbgPortInfo(u32DbgSel, u32DbgInfo);

#endif

}


DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_Open(DMX_PVR_ENG Eng ,DMX_Pvr_info* pPvrInfo)
{
#ifdef DMX_UTOPIA_20

    DMX_PVR_INFO_PARAM Pvrinfo = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    #ifdef DMX_UTOPIA_2K
    _PvrEngCb[Eng]   = NULL;
    #endif

    Pvrinfo.u32EngId = Eng;
    Pvrinfo.pPvrInfo = pPvrInfo;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_Open, (void*)&Pvrinfo) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    #ifdef DMX_UTOPIA_2K
    _PvrEngCb[Eng]   = pPvrInfo->pNotify;
    #endif

    if(Pvrinfo.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pvr_Eng_Open(Eng, pPvrInfo);

#endif

}

// PVR API
//-------------------------------------------------------------------------------------------------
/// Set up the recording parameters
/// @param pInfo \b IN: the recording parameters
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_Pvr_Open(DMX_Pvr_info* pInfo)
{
    return MApi_DMX_Pvr_Eng_Open(DMX_PVR_EGN0, pInfo);
}

//-------------------------------------------------------------------------------------------------
/// Clear the recording parameters
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_Pvr_Close(void)
{
    return MApi_DMX_Pvr_Eng_Close(DMX_PVR_EGN0);
}

//-------------------------------------------------------------------------------------------------
/// Set the PID to be recorded
/// @param Pid \b IN: The target PID to be recorded
/// @param pu8DmxId \b OUT: The demux filter Id for this recorded PID
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_Pvr_Pid_Open(MS_U32 Pid, MS_U8* pu8DmxId)
{
    return MApi_DMX_Pvr_Eng_Pid_Open(DMX_PVR_EGN0, Pid, pu8DmxId);
}

//-------------------------------------------------------------------------------------------------
/// Free the PID to be recorded
/// @param u8DmxId \b IN: The demux filter Id from MApi_DMX_Pvr_Pid_Open
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_Pvr_Pid_Close(MS_U8 u8DmxId)
{
    return MApi_DMX_Pvr_Eng_Pid_Close(DMX_PVR_EGN0, u8DmxId);
}

//-------------------------------------------------------------------------------------------------
/// Start to record
/// @param bPvrAll \b IN: TRUE to record original bit stream
///                       FALSE to recording the PIDs of interests
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_Pvr_Start(MS_BOOL bPvrAll)
{
    return MApi_DMX_Pvr_Eng_Start(DMX_PVR_EGN0, bPvrAll);
}

//-------------------------------------------------------------------------------------------------
/// Stop recording
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_Pvr_Stop(void)
{
    return MApi_DMX_Pvr_Eng_Stop(DMX_PVR_EGN0);
}

//-------------------------------------------------------------------------------------------------
/// Pause/Resume PVR engine
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_Pvr_Pause(MS_BOOL bPause)
{
#if defined(TSP_VERSION) && (TSP_VERSION == TSP_VER_2_0)
    MDrv_RASP_PVR_Pause(DMX_PVR_EGN0, bPause);
#elif defined(TSP_VERSION) && (TSP_VERSION == TSP_VER_4_0)
    MDrv_TSP_PVR_Eng_Pause(DMX_PVR_EGN0,bPause);
#else
    printf("[%s][%d] Not Implement Yet !\n",__FUNCTION__,__LINE__);
#endif
    return DMX_FILTER_STATUS_OK;
}

//-------------------------------------------------------------------------------------------------
/// Get write pointer of recording buffer
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_Pvr_WriteGet(MS_PHYADDR* pu32Write)
{
    return MApi_DMX_Pvr_Eng_WriteGet(DMX_PVR_EGN0, pu32Write);
}

//-------------------------------------------------------------------------------------------------
/// Set recording packet mode
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_PVR_SetPacketMode(MS_BOOL bSet)
{
    return MApi_DMX_Pvr_Eng_SetPacketMode(DMX_PVR_EGN0, bSet);
}

//-------------------------------------------------------------------------------------------------
/// Set recording starting time stamp
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_Pvr_SetRecordStamp(MS_U32 u32Stamp)
{
    return MApi_DMX_Pvr_Eng_SetRecordStamp(DMX_PVR_EGN0, u32Stamp);
}

//-------------------------------------------------------------------------------------------------
/// Get current recording time stamp
/// @return DMX_FILTER_STATUS_OK - Success
/// @return DMX_FILTER_STATUS_ERROR - Failure
/// @note
//-------------------------------------------------------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_Pvr_GetRecordStamp(MS_U32* pu32Stamp)
{
    return MApi_DMX_Pvr_Eng_GetRecordStamp(DMX_PVR_EGN0, pu32Stamp);
}

DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_Close(DMX_PVR_ENG Eng)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[2] = {0, 0};

    u32Param[0] = ((MS_U32)Eng) & 0xFF;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_Close, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    #ifdef DMX_UTOPIA_2K
    _PvrEngCb[Eng]   = NULL;
    #endif

    if(u32Param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pvr_Eng_Close(Eng);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_Pid_Open(DMX_PVR_ENG Eng ,MS_U32 Pid, MS_U8* pu8DmxId)
{

#ifdef DMX_UTOPIA_20

    DMX_PVR_FLTTYPE_PARAM PvrFltTypeParam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu8DmxId = 0xFF;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    PvrFltTypeParam.u32EngId = ((MS_U32)Eng) & 0xFF;
    PvrFltTypeParam.u32Pid = Pid;
    PvrFltTypeParam.u32CAEngSel = 0xFFFFFFFF;
    PvrFltTypeParam.u32SelShareFlt = 0;
    PvrFltTypeParam.u32DmxId = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_PidOpen, (void*)&PvrFltTypeParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu8DmxId = (MS_U8)(PvrFltTypeParam.u32DmxId & 0xFF);

    if(PvrFltTypeParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pvr_Eng_Pid_Open(Eng, Pid, pu8DmxId);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_Pid_Close(DMX_PVR_ENG Eng ,MS_U8 u8DmxId)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = ((MS_U32)Eng) & 0xFF;
    u32Param[1] = ((MS_U32)u8DmxId) & 0xFF;
    u32Param[2] = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_PidClose, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pvr_Eng_Pid_Close(Eng, u8DmxId);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_Start(DMX_PVR_ENG Eng, MS_BOOL bPvrAll)
{

#ifdef DMX_UTOPIA_20
    MS_U32 u32Param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = ((MS_U32)Eng) & 0xFF;
    u32Param[1] = ((MS_U32)bPvrAll) & 0xFF;
    u32Param[2] = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_Start, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else

    return _MApi_DMX_Pvr_Eng_Start(Eng, bPvrAll);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_Stop(DMX_PVR_ENG Eng)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));


    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = ((MS_U32)Eng) & 0xFF;
    u32Param[1] = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_Stop, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pvr_Eng_Stop(Eng);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_WriteGet(DMX_PVR_ENG Eng ,MS_U32* pu32Write)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32Write = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = ((MS_U32)Eng) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_GetWriteAddr, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32Write = u32Param[1];

    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pvr_Eng_WriteGet(Eng, pu32Write);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_SetPacketMode(DMX_PVR_ENG Eng ,MS_BOOL bSet)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = ((MS_U32)Eng) & 0xFF;
    u32Param[1] = ((MS_U32)bSet) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_SetPktMode, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pvr_Eng_SetPacketMode(Eng, bSet);

#endif
}

DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_SetRecordStamp(DMX_PVR_ENG Eng ,MS_U32 u32Stamp)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = ((MS_U32)Eng) & 0xFF;
    u32Param[1] = u32Stamp;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_SetRecordStamp, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pvr_Eng_SetRecordStamp(Eng, u32Stamp);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_GetRecordStamp(DMX_PVR_ENG Eng ,MS_U32* pu32Stamp)
{

#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32Stamp = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = ((MS_U32)Eng) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_GetRecordStamp, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32Stamp = u32Param[1];

    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pvr_Eng_GetRecordStamp(Eng, pu32Stamp);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_SetPlaybackStampClk(MS_U8 u8Eng, DMX_TimeStamp_Clk eClkSrc)
{
#ifdef DMX_UTOPIA_20

    DMX_TIMESTAMP_CLK_PARAM TimestampClk ={0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    TimestampClk.u32EngId = ((MS_U32)u8Eng) & 0xFF;
    TimestampClk.eClk = eClkSrc;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FI_SetPlaybackStampClk, (void*)&TimestampClk) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(TimestampClk.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else

    return _MApi_DMX_Pvr_Eng_SetPlaybackStampClk(u8Eng, eClkSrc);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_SetRecordStampClk(DMX_PVR_ENG Eng, DMX_TimeStamp_Clk eClkSrc)
{

#ifdef DMX_UTOPIA_20

    DMX_TIMESTAMP_CLK_PARAM TimestampClk ={0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    TimestampClk.u32EngId = ((MS_U32)Eng) & 0xFF;
    TimestampClk.eClk = eClkSrc;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_SetRecordStampClk, (void*)&TimestampClk) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(TimestampClk.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pvr_Eng_SetRecordStampClk(Eng, eClkSrc);

#endif

}

DMX_FILTER_STATUS MApi_DMX_LiveSrcSwitch(DMX_FILTER_TYPE TgtFlow)
{
#ifdef DMX_UTOPIA_20

    DMX_FLT_TYPE_PARAM FltTypeParam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    FltTypeParam.u32DmxId = 0L;
    FltTypeParam.DmxFltType = TgtFlow;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_ChangeFltSrc, (void*)&FltTypeParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(FltTypeParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_LiveSrcSwitch(TgtFlow);

#endif

}

DMX_FILTER_STATUS MApi_DMX_AU_BD_Mode_Enable(MS_BOOL bEnable)
{
#ifdef DMX_UTOPIA_20

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    MS_U32 u32param[2] = {0, 0};

    u32param[0] = ((MS_U32)bEnable) & 0xFF;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_AUBD_Enable, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_AU_BD_Mode_Enable(bEnable);

#endif

}

DMX_FILTER_STATUS MApi_DMX_CMD_Run(MS_U32 u32Cmd, MS_U32 u32Config, MS_U32 u32DataNum, void *pData)
{
#ifdef DMX_UTOPIA_20

    DMX_CMDRUN_PARAM CmdRunParam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    CmdRunParam.u32Cmd = u32Cmd;
    CmdRunParam.u32Config = u32Config;
    CmdRunParam.u32DataNum = u32DataNum;
    CmdRunParam.pData = pData;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_CmdRun, (void*)&CmdRunParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(CmdRunParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_CMD_Run(u32Cmd, u32Config, u32DataNum, pData);

#endif
}

MS_U32 MApi_DMX_SetPowerState(EN_POWER_MODE u16PowerState, MS_U32 u32FWAddr, MS_U32 u32FWSize)
{
#ifdef DMX_UTOPIA_20

    DMX_POWSTATE_PARAM PowSteParam ={0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    PowSteParam.u16PowerState = u16PowerState;
    PowSteParam.u32FWAddr = u32FWAddr;
    PowSteParam.u32FWSize = u32FWSize;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_SetPowerState, (void*)&PowSteParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(PowSteParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_SetPowerState(u16PowerState, u32FWAddr, u32FWSize);

#endif
}

DMX_FILTER_STATUS MApi_DMX_SetPacketMode(DMX_FLOW DmxFlow, DMX_PacketMode ePktMode)
{
#ifdef DMX_UTOPIA_20

    DMX_PKT_MODE_PARAM stPktMode = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    stPktMode.eFlow = DmxFlow;
    stPktMode.ePktMode = ePktMode;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_SetPktMode, (void*)&stPktMode) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(stPktMode.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_SetPacketMode(DmxFlow, ePktMode);

#endif
}

DMX_FILTER_STATUS MApi_DMX_SetMergeStrSyncByte(MS_U32 u32SrcID, MS_U8 u8SyncByte)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32Param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = u32SrcID;
    u32Param[1] = (MS_U32)u8SyncByte;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_SetMergeStrSync, (void*)&u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_SetMergeStrSyncByte(u32SrcID, u8SyncByte);

#endif
}

DMX_FILTER_STATUS MApi_DMX_TSO_Filein_Info(MS_U8 u8Eng, DMX_Filein_info *pFileinInfo)
{
#ifdef DMX_UTOPIA_20

    DMX_FILE_INFO_PARAM stFileInfo = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    stFileInfo.u32EngId = (MS_U32)u8Eng;
    stFileInfo.pFileinInfo = pFileinInfo;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_FileInfo, (void*)&stFileInfo) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(stFileInfo.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_TSO_Filein_Info(u8Eng, pFileinInfo);
#endif

}

MS_BOOL MApi_DMX_TSO_Filein_IsIdle(MS_U8 u8Eng)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32parem[2] = {0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return FALSE;

    u32parem[0] = (MS_U32)u8Eng;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_FileIsIdle, (void*)u32parem) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    return (MS_BOOL)(u32parem[1]);

#else

    return _MApi_DMX_TSO_Filein_IsIdle(u8Eng);
#endif

}

DMX_FILTER_STATUS MApi_DMX_TSO_Filein_CMDQ_GetEmptyNum(MS_U8 u8Eng, MS_U32 *pu32EmptySlot)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32parem[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32EmptySlot = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32parem[0] = (MS_U32)u8Eng;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_FileGetCmdQEmptyNum, (void*)u32parem) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32EmptySlot = u32parem[1];

    if(u32parem[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_TSO_Filein_CMDQ_GetEmptyNum(u8Eng, pu32EmptySlot);
#endif

}

DMX_FILTER_STATUS MApi_DMX_TSO_Filein_CMDQ_Reset(MS_U8 u8Eng)
{
#ifdef DMX_UTOPIA_20
    
    MS_U32 u32parem[2] = {0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;
    
    u32parem[0] = ((MS_U32)u8Eng) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_CmdQReset, (void*)u32parem) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }   
    
    if(u32parem[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
    
#else

    return _MApi_DMX_TSO_Filein_CMDQ_Reset(u8Eng);
#endif

}

DMX_FILTER_STATUS MApi_DMX_TSO_Filein_Start(MS_U8 u8Eng, MS_PHYADDR pBuf, MS_U32 u32BufSize)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32parem[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32parem[0] = (MS_U32)u8Eng;
    u32parem[1] = pBuf;
    u32parem[2] = u32BufSize;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_FileStart, (void*)u32parem) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32parem[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_TSO_Filein_Start(u8Eng, pBuf, u32BufSize);
#endif

}

DMX_FILTER_STATUS MApi_DMX_TSO_Filein_Stop(MS_U8 u8Eng)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32parem[2] = {0, 0};

    u32parem[0] = ((MS_U32)u8Eng) & 0xFF;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_FileStop, (void*)u32parem) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32parem[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_TSO_Filein_Stop(u8Eng);
#endif

}

DMX_FILTER_STATUS MApi_DMX_TSO_SetPlaybackTimeStamp(MS_U8 u8Eng, MS_U32 u32Stamp)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32parem[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32parem[0] = ((MS_U32)u8Eng) & 0xFF;
    u32parem[1] = u32Stamp;
    u32parem[2] = 1;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_FilePlayStamp, (void*)u32parem) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32parem[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_TSO_SetPlaybackTimeStamp(u8Eng, u32Stamp);
#endif

}

DMX_FILTER_STATUS MApi_DMX_TSO_GetPlaybackStamp(MS_U8 u8Eng, MS_U32* pu32Stamp)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32parem[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32Stamp = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32parem[0] = ((MS_U32)u8Eng) & 0xFF;
    u32parem[2] = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_FilePlayStamp, (void*)u32parem) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32Stamp = u32parem[1];

    if(u32parem[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_TSO_GetPlaybackStamp(u8Eng, pu32Stamp);

#endif

}

DMX_FILTER_STATUS MApi_DMX_TSO_GetFileInTimeStamp(MS_U8 u8Eng, MS_U32 *pu32Stamp)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32parem[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32Stamp = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32parem[0] = (MS_U32)u8Eng;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_FileGetTimeStamp, (void*)u32parem) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32Stamp = u32parem[1];

    if(u32parem[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else

    return _MApi_DMX_TSO_GetFileInTimeStamp(u8Eng, pu32Stamp);

#endif

}

DMX_FILTER_STATUS MApi_DMX_TSO_BypassFileInTimeStamp(MS_U8 u8Eng, MS_BOOL bbypass)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32parem[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32parem[0] = (MS_U32)u8Eng;
    u32parem[1] = (MS_U32)bbypass;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_FileBypassStamp, (void*)u32parem) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32parem[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_TSO_BypassFileInTimeStamp(u8Eng, bbypass);

#endif

}

DMX_FILTER_STATUS MApi_DMX_TSO_TimeStampEnable(MS_U8 u8Eng)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32parem[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32parem[0] = (MS_U32)u8Eng;
    u32parem[1] = 1;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_FileTimeStampEnable, (void*)u32parem) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32parem[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else

    return _MApi_DMX_TSO_TimeStampEnable(u8Eng);

#endif

}

DMX_FILTER_STATUS MApi_DMX_TSO_TimeStampDisable(MS_U8 u8Eng)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32parem[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32parem[0] = (MS_U32)u8Eng;
    u32parem[1] = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_FileTimeStampEnable, (void*)u32parem) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32parem[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_TSO_TimeStampDisable(u8Eng);
#endif

}

DMX_FILTER_STATUS MApi_DMX_TSO_SetOutClk(MS_U8 u8Eng, DMX_TSO_OutClk eTsOutClk, DMX_TSO_OutClkSrc eTsOutClkSrc, MS_U16 u16DivNum, MS_BOOL bClkInv)
{
#ifdef DMX_UTOPIA_20

    DMX_TSO_OUTCLK_PARAM stParam;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    stParam.u32Res = 0;
    stParam.u32Eng = (MS_U32)u8Eng;
    stParam.eTsoOutClk = eTsOutClk;
    stParam.eTsoOutClkSrc = eTsOutClkSrc;
    stParam.u16DivNum = u16DivNum;
    stParam.bClkInv = bClkInv;
    stParam.bSet = TRUE;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_OutClk, (void*)&stParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(stParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_TSO_SetOutClk(u8Eng, eTsOutClk, eTsOutClkSrc, u16DivNum, bClkInv);
#endif

}

DMX_FILTER_STATUS MApi_DMX_TSO_OutputEnable(MS_U8 u8Eng, MS_BOOL bEnable)
{
#ifdef DMX_UTOPIA_20
	
	MS_U32 u32Param[3] = {0, 0, 0};
	
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	u32Param[0] = ((MS_U32)u8Eng) & 0xFF;
	u32Param[1] = ((MS_U32)bEnable) & 0xFF;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_OutputEn, (void*)&u32Param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}

	if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_TSO_OutputEnable(u8Eng, bEnable);
#endif

}

DMX_FILTER_STATUS MApi_DMX_TSO_LocalStreamId(MS_U8 u8Eng, DMX_TSIF eIf, MS_U8* pu8StrId, MS_BOOL bSet)
{
#ifdef DMX_UTOPIA_20

    DMX_TSO_LOCSTRID_PARAM stParam = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(bSet == FALSE)
    {
        *pu8StrId = 0xFF;
    }
    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    stParam.u8Eng = u8Eng;
    stParam.eIf = eIf;
    stParam.pu8StrId = pu8StrId;
    stParam.bSet = bSet;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_LocStrId, (void*)&stParam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(stParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else

    return _MApi_DMX_TSO_LocalStreamId(u8Eng, eIf, pu8StrId, bSet);
#endif

}

DMX_FILTER_STATUS MApi_DMX_TSO_SVQBuf_Set(MS_U8 u8Eng, MS_PHYADDR u32Addr, MS_U32 u32BufSize)
{
#ifdef DMX_UTOPIA_20
    
    MS_U32 u32parem[4] = {0, 0, 0, 0};
    
    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32parem[0] = (MS_U32)(u8Eng & 0xFF);
    u32parem[1] = u32Addr;
    u32parem[2] = u32BufSize;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_SvqBufSet, (void*)u32parem) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }
    
    if(u32parem[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;    
#else

    return _MApi_DMX_TSO_SVQBuf_Set(u8Eng, u32Addr, u32BufSize);
#endif
}

DMX_FILTER_STATUS MApi_DMX_TSO_Flow_InputCfg(DMX_TSO_InputCfg* pstInputCfg)
{
#ifdef DMX_UTOPIA_20

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_InputCfg, (void*)pstInputCfg) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}
	
	if(pstInputCfg->u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_TSO_Flow_InputCfg(pstInputCfg);

#endif
}

DMX_FILTER_STATUS MApi_DMX_TSO_Flow_OutputCfg(DMX_TSO_OutputCfg* pstOutputCfg)
{
#ifdef DMX_UTOPIA_20

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_OutputCfg, (void*)pstOutputCfg) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
    }

	if(pstOutputCfg->u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_TSO_Flow_OutputCfg(pstOutputCfg);

#endif
}

DMX_FILTER_STATUS MApi_DMX_TSO_Configure(DMX_TSO_Cfg* pstTsoCfg)
{
#ifdef DMX_UTOPIA_20
    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TSO_Configure, (void*)pstTsoCfg) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }
    if(pstTsoCfg->u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else
    return _MApi_DMX_TSO_Configure(pstTsoCfg);
#endif
}

DMX_FILTER_STATUS MApi_DMX_FQ_SetFltRushPass(MS_U8 u8DmxId, MS_U8 u8Enable)
{
#ifdef DMX_UTOPIA_20
    
    MS_U32 u32parem[4] = {0, 0, 0, 0};

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;
    u32parem[0] = (MS_U32)u8DmxId;
    u32parem[1] = (MS_U32)u8Enable;
    u32parem[2] = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FQ_SetFltRushPass, (void*)u32parem) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }
        
    if(u32parem[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else
        return _MApi_DMX_FQ_SetFltRushPass(u8DmxId, u8Enable);
#endif

}

DMX_FILTER_STATUS MApi_DMX_FQ_Init(MS_U32 u32FQEng, DMX_FQ_Info* pFQInfo)
{
#ifdef DMX_UTOPIA_20
        DMX_FQ_INIT_PARAM stParam = {0};
        if(_DMX_IOCtrlOpen() == FALSE)
            return DMX_FILTER_STATUS_ERROR;
        stParam.u32Eng= u32FQEng;
        stParam.pInfo = pFQInfo;
        if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FQ_Init, (void*)&stParam) != UTOPIA_STATUS_SUCCESS)
        {
            DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
            return DMX_FILTER_STATUS_ERROR;
        }
    
        if(stParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
            return DMX_FILTER_STATUS_ERROR;
        else
            return DMX_FILTER_STATUS_OK;
    
#else
        return _MApi_DMX_FQ_Init(u32FQEng, pFQInfo);
#endif

}

DMX_FILTER_STATUS MApi_DMX_FQ_Exit(MS_U32 u32FQEng)
{
#ifdef DMX_UTOPIA_20
        MS_U32 u32parem[3] = {0, 0, 0};
        if(_DMX_IOCtrlOpen() == FALSE)
            return DMX_FILTER_STATUS_ERROR;
        u32parem[0] = (MS_U32)u32FQEng;
        u32parem[1] = 0;

        if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FQ_Exit, (void*)&u32parem) != UTOPIA_STATUS_SUCCESS)
        {
            DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
            return DMX_FILTER_STATUS_ERROR;
        }
    
        if(u32parem[2] != (MS_U32)DMX_FILTER_STATUS_OK)
            return DMX_FILTER_STATUS_ERROR;
        else
            return DMX_FILTER_STATUS_OK;
    
#else    
        return _MApi_DMX_FQ_Exit(u32FQEng);
#endif

}

DMX_FILTER_STATUS MApi_DMX_FQ_RushEnable(MS_U32 u32FQEng)
{
#ifdef DMX_UTOPIA_20
    
        MS_U32 u32parem[3] = {0, 0, 0};
        if(_DMX_IOCtrlOpen() == FALSE)
            return DMX_FILTER_STATUS_ERROR;
        u32parem[0] = (MS_U32)u32FQEng;
        u32parem[1] = 0;

        if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FQ_RushEnable, (void*)&u32parem) != UTOPIA_STATUS_SUCCESS)
        {
            DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
            return DMX_FILTER_STATUS_ERROR;
        }
    
        if(u32parem[2] != (MS_U32)DMX_FILTER_STATUS_OK)
            return DMX_FILTER_STATUS_ERROR;
        else
            return DMX_FILTER_STATUS_OK;
    
#else
        return _MApi_DMX_FQ_RushEnable(u32FQEng);
#endif

}

DMX_FILTER_STATUS MApi_DMX_FQ_SkipRushData(MS_U32 u32FQEng, DMX_FQ_SkipPath eSkipPath)
{
#ifdef DMX_UTOPIA_20

        DMX_FQ_SKIP_RUSH_DATA_PARAM stParam = {0};
        if(_DMX_IOCtrlOpen() == FALSE)
            return DMX_FILTER_STATUS_ERROR;
        stParam.u32Eng = u32FQEng;
        stParam.eSkipPath = eSkipPath;
        if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_FQ_SkipRushData, (void*)&stParam) != UTOPIA_STATUS_SUCCESS)
        {
            DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
            return DMX_FILTER_STATUS_ERROR;
        }
    
        if(stParam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
            return DMX_FILTER_STATUS_ERROR;
        else
            return DMX_FILTER_STATUS_OK;

#else
        return _MApi_DMX_FQ_SkipRushData(u32FQEng, eSkipPath);
#endif

}

DMX_FILTER_STATUS MApi_DMX_Get_DisContiCnt(DMX_DisContiCnt_info* pDmxInfo, MS_U32 *pu32Cnt)
{
#ifdef DMX_UTOPIA_20

    DMX_PKT_DISCONCNT_PARAM Param = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32Cnt = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    Param.pInfo = pDmxInfo;
    Param.u32Cnt = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_DBG_GET_DISCONCNT, (void*)&Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32Cnt = Param.u32Cnt;

    if(Param.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Get_DisContiCnt(pDmxInfo, pu32Cnt);
#endif

}

DMX_FILTER_STATUS MApi_DMX_Get_DropPktCnt(DMX_DropPktCnt_info* pDmxInfo, MS_U32 *pu32Cnt)
{
#ifdef DMX_UTOPIA_20

    DMX_PKT_DROPCNT_PARAM Param ={0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32Cnt = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    Param.pInfo = pDmxInfo;
    Param.u32Cnt = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_DBG_GET_DROPPKTCNT, (void*)&Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32Cnt = Param.u32Cnt;

    if(Param.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else

    return _MApi_DMX_Get_DropPktCnt(pDmxInfo, pu32Cnt);
#endif

}

DMX_FILTER_STATUS MApi_DMX_Get_LockPktCnt(DMX_LockPktCnt_info* pDmxInfo, MS_U32 *pu32Cnt)
{
#ifdef DMX_UTOPIA_20

    DMX_PKT_LOCKCNT_PARAM Param ={0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32Cnt = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    Param.pInfo = pDmxInfo;
    Param.u32Cnt = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_DBG_GET_LOCKPKTCNT, (void*)&Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32Cnt = Param.u32Cnt;

    if(Param.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else

    return _MApi_DMX_Get_LockPktCnt(pDmxInfo, pu32Cnt);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Get_AVPktCnt(DMX_AVPktCnt_info *DmxInfo, MS_U32 *pu32Cnt)
{
#ifdef DMX_UTOPIA_20

    DMX_PKT_AVCNT_PARAM Param = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32Cnt = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    Param.pInfo = DmxInfo;
    Param.u32Cnt = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_DBG_GET_AVPKTCNT, (void*)&Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32Cnt = Param.u32Cnt;

    if(Param.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Get_AVPktCnt(DmxInfo, pu32Cnt);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Get_SecTEI_PktCount(DMX_FILTER_TYPE FltSrc, MS_U32* pu32PktCnt)
{
#ifdef DMX_UTOPIA_20

    DMX_PKT_SECTEI_PARAM Param = {0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32PktCnt = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    Param.eType = FltSrc;
    Param.u32Cnt = 0;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_GET_SECTEI_PKTCNT, (void*)&Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32PktCnt = Param.u32Cnt;

    if(Param.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Get_SecTEI_PktCount(FltSrc, pu32PktCnt);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Reset_SecTEI_PktCount(DMX_FILTER_TYPE FltSrc)
{
#ifdef DMX_UTOPIA_20

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    MS_U32 u32param[2] = {0, 0};

    u32param[0] = (MS_U32)FltSrc;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_RESET_SECTEI_PKTCNT, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Reset_SecTEI_PktCount(FltSrc);

#endif
}

DMX_FILTER_STATUS MApi_DMX_Get_SecDisCont_PktCount(MS_U32 u32DmxID, MS_U32* pu32PktCnt)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pu32PktCnt = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = u32DmxID;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_GET_SECDISCON_PKTCNT, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32PktCnt = u32param[1];

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Get_SecDisCont_PktCount(u32DmxID, pu32PktCnt);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Reset_SecDisCont_PktCount(MS_U32 u32DmxID)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[2] = {0, 0};

    u32param[0] = u32DmxID;

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_RESET_SECDISCON_PKTCNT, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
#else

    return _MApi_DMX_Reset_SecDisCont_PktCount(u32DmxID);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_CallbackSize(DMX_PVR_ENG Eng, MS_U32* pu32CallbackSize, MS_BOOL bSet)
{
#ifdef DMX_UTOPIA_20
    
    MS_U32 u32param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(bSet == FALSE)
    {
        *pu32CallbackSize = 0;
    }

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = (MS_U32)Eng;
    u32param[2] = (MS_U32)bSet;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_CBSize, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pu32CallbackSize = u32param[1];

    if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pvr_Eng_CallbackSize(Eng, pu32CallbackSize, bSet);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_SetCaMode(DMX_PVR_ENG Eng, DMX_CA_PVRMODE eCaMode, MS_BOOL bspsEnable)
{
#ifdef DMX_UTOPIA_20

    MS_U32 u32param[4] = {0, 0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = (MS_U32)Eng;
    u32param[1] = (MS_U32)eCaMode;
    u32param[2] = (MS_U32)bspsEnable;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_SetCaMode, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(u32param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_Pvr_Eng_SetCaMode(Eng, eCaMode, bspsEnable);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_IsStart(DMX_PVR_ENG Eng, MS_BOOL *pbIsStart)
{
#ifdef DMX_UTOPIA_20
    
    MS_U32 u32param[3] = {0, 0, 0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    *pbIsStart = FALSE;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32param[0] = (MS_U32)Eng;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_IsStart, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    *pbIsStart = (MS_BOOL)u32param[1];

    if(u32param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK; 
#else

    return _MApi_DMX_Pvr_Eng_IsStart(Eng, pbIsStart);

#endif

}


DMX_FILTER_STATUS MApi_DMX_Get_Intr_Count(MS_U32* pu32Count)
{
#ifdef DMX_UTOPIA_20        
    
    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));    
    
    MS_U32 u32param[2] = {0, 0};

    *pu32Count = 0;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_GetIntCnt, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }
    
    *pu32Count = u32param[0];

    if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
    
#else

    return _MApi_DMX_Get_Intr_Count(pu32Count);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Drop_Enable(MS_BOOL bSet)
{
#ifdef DMX_UTOPIA_20        
        
    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));    
        
    MS_U32 u32param[2] = {0, 0};

    u32param[0] = ((MS_U32)bSet) & 0xFF;

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;
    
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_DropEn, (void*)u32param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }
        
    if(u32param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
        
#else

    return _MApi_DMX_Drop_Enable(bSet);

#endif
}

DMX_FILTER_STATUS MApi_DMX_VQ_Enable(MS_BOOL bEnable)
{
#ifdef DMX_UTOPIA_20

    return DMX_FILTER_STATUS_ERROR;

#else

    return _MApi_DMX_VQ_Enable(bEnable);

#endif
}

DMX_FILTER_STATUS MApi_DMX_TEI_RemoveErrorPkt(DMX_TEI_RmPktType eDmxPktType, MS_BOOL bEnable)
{
#ifdef DMX_UTOPIA_20        

    MS_U32 u32Param[3] = {0, 0, 0};
                
    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));    
                
    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = (DMX_TEI_RmPktType)eDmxPktType;
    u32Param[1] = ((MS_U32)bEnable) & 0xFF;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_TeiRmErrPkt, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }
                
    if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
                
#else

    return _MApi_DMX_TEI_RemoveErrorPkt(eDmxPktType, bEnable);

#endif
}

DMX_FILTER_STATUS MApi_DMX_SetFwDbgParam(MS_U32 u32Addr, MS_U32 u32Size, MS_U32 u32DbgWord)
{
#ifdef DMX_UTOPIA_20        
    
MS_U32 u32Param[4] = {0, 0, 0, 0};
                    
    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));    
                    
    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = u32Addr;
    u32Param[1] = u32Size;
    u32Param[2] = u32DbgWord;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_SetFwDbgParam, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }
                   
    if(u32Param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
                    
#else

    return _MApi_DMX_SetFwDbgParam(u32Addr, u32Size, u32DbgWord);

#endif
}

DMX_FILTER_STATUS MApi_DMX_SetAVPause(MS_BOOL bSet)
 {
#ifdef DMX_UTOPIA_20

    return DMX_FILTER_STATUS_ERROR;

#else

    return _MApi_DMX_SetAVPause(bSet);

#endif
}

DMX_FILTER_STATUS MApi_DMX_Pvr_MOBF_Enable(MS_BOOL bEnable, MS_U32 u32key0, MS_U32 u32key1)
{
#ifdef DMX_UTOPIA_20        
    
MS_U32 u32Param[4] = {0, 0, 0, 0};
                    
    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));    
                    
    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    u32Param[0] = ((MS_U32)bEnable) & 0xFF;
    u32Param[1] = u32key0;
    u32Param[2] = u32key1;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_PVR_MOBFEn, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }
                   
    if(u32Param[3] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
                    
#else

    return _MApi_DMX_Pvr_MOBF_Enable(bEnable, u32key0, u32key1);

#endif
}

DMX_FILTER_STATUS MApi_DMX_Filein_Pvr_Eng_Pid_Open(DMX_PVR_ENG Eng, MS_U32 Pid, MS_U8* pu8DmxId, MS_U8 u8ShareKeyType)
{
#ifdef DMX_UTOPIA_20
		
	MS_U32 u32Param[5] = {0, 0, 0, 0, 0};
		
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	*pu8DmxId = 0xFF;

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	u32Param[0] = (MS_U32)Eng;
	u32Param[1] = Pid;
	u32Param[3] = ((MS_U32)u8ShareKeyType) & 0xFF;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_File_PVR_PidOpen, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}

	*pu8DmxId = (MS_U8)u32Param[2];
		
    if(u32Param[4] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;		
#else

	return _MApi_DMX_Filein_Pvr_Eng_Pid_Open(Eng, Pid, pu8DmxId, u8ShareKeyType);

#endif
}

DMX_FILTER_STATUS MApi_DMX_Filein_Pvr_Eng_Pid_Close(DMX_PVR_ENG Eng, MS_U8 u8DmxId)
{
#ifdef DMX_UTOPIA_20
	
	MS_U32 u32Param[3] = {0, 0, 0};
	
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	u32Param[0] = (MS_U32)Eng;
	u32Param[1] = ((MS_U32)u8DmxId) & 0xFF;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_File_PVR_PidClose, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}
	
	if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
	
#else

	return _MApi_DMX_Filein_Pvr_Eng_Pid_Close(Eng, u8DmxId);

#endif
}

DMX_FILTER_STATUS MApi_DMX_Filein_Pvr_Eng_Start(DMX_PVR_ENG Eng, MS_BOOL bPvrAll)
{
#ifdef DMX_UTOPIA_20
		
	MS_U32 u32Param[3] = {0, 0, 0};
		
    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));
		
    if(_DMX_IOCtrlOpen() == FALSE)
	    return DMX_FILTER_STATUS_ERROR;
		
	u32Param[0] = (MS_U32)Eng;
	u32Param[1] = ((MS_U32)bPvrAll) & 0xFF;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_File_PVR_Start, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}
		
	if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
		
#else

		return _MApi_DMX_Filein_Pvr_Eng_Start(Eng, bPvrAll);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Filein_Pvr_Eng_Stop(DMX_PVR_ENG Eng)
{
#ifdef DMX_UTOPIA_20
			
	MS_U32 u32Param[2] = {0, 0};

    u32Param[0] = ((MS_U32)Eng) & 0xFF;
			
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;
			
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_File_PVR_Stop, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}
			
	if(u32Param[1] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
			
#else

	return _MApi_DMX_Filein_Pvr_Eng_Stop(Eng);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_Pause(DMX_PVR_ENG Eng ,MS_BOOL bPause)
{
#ifdef DMX_UTOPIA_20
				
	MS_U32 u32Param[3]  = {0, 0, 0};
				
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	u32Param[0] = (MS_U32)Eng;
	u32Param[1] = ((MS_U32)bPause) & 0xFF;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_Pause, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}
				
	if(u32Param[2] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
				
#else

	return _MApi_DMX_Pvr_Eng_Pause(Eng, bPause);

#endif

}

DMX_FILTER_STATUS MApi_DMX_Pvr_Eng_Pid(DMX_PVR_ENG Eng ,MS_U8 u8DmxId , MS_U32 *Pid, MS_BOOL bSet)
{
#ifdef DMX_UTOPIA_20
					
	MS_U32 u32Param[5]	= {0, 0, 0, 0, 0};
					
	DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

	if(_DMX_IOCtrlOpen() == FALSE)
		return DMX_FILTER_STATUS_ERROR;

	u32Param[0] = (MS_U32)Eng;
	u32Param[1] = ((MS_U32)u8DmxId) & 0xFF;
	u32Param[2] = *Pid;
	u32Param[3] = ((MS_U32)bSet) & 0xFF;
	if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_Pvr_Pid, (void*)u32Param) != UTOPIA_STATUS_SUCCESS)
	{
		DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
		return DMX_FILTER_STATUS_ERROR;
	}

	if(bSet == FALSE)
	{
		*Pid = u32Param[2];
	}
	
	if(u32Param[4] != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
	    return DMX_FILTER_STATUS_OK;
					
#else

	return MApi_DMX_Pvr_Eng_Pid(Eng, u8DmxId, Pid, bSet);

#endif

}


DMX_FILTER_STATUS MApi_DMX_DropScmbPkt(DMX_FILTER_TYPE pDmxFltType, MS_BOOL bEnable)
{
#ifdef DMX_UTOPIA_20
    DMX_DROP_SCMB_PKT_PARAM infoparam ={0};

    DMX_V2DBGMSG(printf("[%s] %d\n", __FUNCTION__, __LINE__));

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    infoparam.eType         = pDmxFltType;
    infoparam.bEnable       = bEnable;

    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_DropScmbPkt,(void*)&infoparam) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }
    if(infoparam.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;

#else

    return _MApi_DMX_DropScmbPkt(pDmxFltType,bEnable);

#endif
}

//-----------------------------------------------------
// Merge stream API
//-----------------------------------------------------
DMX_FILTER_STATUS MApi_DMX_MStr_SyncByte(DMX_TSIF eIf, MS_U8 u8StrId, MS_U8* pu8SyncByte, MS_BOOL bSet)
{
#ifdef DMX_UTOPIA_20
    DMX_MSTR_SYNCBY_PARAM param = {0};

    if(_DMX_IOCtrlOpen() == FALSE)
        return DMX_FILTER_STATUS_ERROR;

    param.eIf = eIf;
    param.u8StrId = u8StrId;
    param.pu8SyncByte = pu8SyncByte;
    param.bSet = bSet;
    if (UtopiaIoctl(_pInstantDmx, MApi_CMD_DMX_MStr_SyncBy, (void*)&param) != UTOPIA_STATUS_SUCCESS)
    {
        DMX_V2DBGMSG(printf("[MAPI DMX][%s][%06d] fail\n", __FUNCTION__, __LINE__));
        return DMX_FILTER_STATUS_ERROR;
    }

    if(param.u32Res != (MS_U32)DMX_FILTER_STATUS_OK)
        return DMX_FILTER_STATUS_ERROR;
    else
        return DMX_FILTER_STATUS_OK;
    
#else

    return _MApi_DMX_MStr_SyncByte(eIf, u8StrId, pu8SyncByte, bSet);

#endif
}

