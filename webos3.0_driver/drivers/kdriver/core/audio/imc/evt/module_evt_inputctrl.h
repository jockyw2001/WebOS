/*
	SIC LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
	Copyright(c) 2013 by LG Electronics Inc.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	version 2 as published by the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.
*/




/******************************************************************************
  Header File Guarder
 ******************************************************************************/
#ifndef __MODULE_EVT_INPUTCTRL_H__
#define __MODULE_EVT_INPUTCTRL_H__

#include "common/adec_media_type.h"
#include "imc/adec_imc_evt.h"

#define INPUTCTRL_EVT_SYSTEM_DELAY			ADEC_EVT_MOD_ID(EVT_INPUT_CTRL, 0x00) //SE_EVT_DETECT_TONE

/**
* System Delay Event ������ ����ü.
*/
typedef struct _InputctrlEvtSystemDelay
{
   unsigned int         total_system_delay;
   unsigned int			inputctrl_delay;
}InputctrlEvtSystemDelay;

#endif //__MODULE_EVT_INPUTCTRL_H__
