/*
 * SIC LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
 * Copyright(c) 2013 by LG Electronics Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */


/** @file
 *
 *  driver interface header for pvr device. ( used only within kdriver )
 *	pvr device will teach you how to make device driver with new platform.
 *
 *  @author		kyungbin.pak
 *  @version	1.0
 *  @date		2009.11.15
 *
 *  @addtogroup lg1150_pvr
 *	@{
 */

#ifndef	_PVR_DRV_H_
#define	_PVR_DRV_H_


/*----------------------------------------------------------------------------------------
	Control Constants
----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------
    File Inclusions
----------------------------------------------------------------------------------------*/
#include "debug_util.h"
#include "pvr_cfg.h"
#include "pvr_kapi.h"

#ifdef	__cplusplus
extern "C"
{
#endif /* __cplusplus */

/*----------------------------------------------------------------------------------------
	Constant Definitions
----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------
	Macro Definitions
----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------
    Type Definitions
----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------
	Extern Function Prototype Declaration
----------------------------------------------------------------------------------------*/
extern	void     PVR_PreInit(void);
extern	int      PVR_Init(void);
extern	void     PVR_Cleanup(void);

/*----------------------------------------------------------------------------------------
	Extern Variables
----------------------------------------------------------------------------------------*/


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif /* _PVR_DRV_H_ */

/** @} */