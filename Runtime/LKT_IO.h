/********************************************************************

            Ctoolkit.h             

   This file contains defintions for all of the common structures 
   and type definitions necessary to describe our hardware

**********************************************************************/



#ifndef  __LKT_IO_H__
#define  __LKT_IO_H__



/*============================================================================

 STATUS_BYTE - 

=============================================================================*/



/*=============================================================================

  DP_HEADER   - defines the structure of the dual port memory header.

=============================================================================*/

#pragma BYTE_ALIGN(_DUAL_PORT)  /* 1 byte alignement  */
typedef struct _DUAL_PORT
{
//     UINT16 NetStatus;                                  /* :0000 */
//     UINT16 NetCommand;                                 /* :0002 */
//     UINT16 Watchdog;                                   /* :0004 */
//     UINT16 reserved;                                   /* :0006 */
//     UINT16 DevStatus[MAX_DEVICES];                     /* :0008 */
//     UINT8  filler[DPR_CONTROL_SIZE-MAX_DEVICES*2-8];   /* :0108 */
//     UINT8  Input[DPR_INPUT_SIZE];                      /* :0800 */
//     UINT8  Output[DPR_OUTPUT_SIZE];                    /* :0c00 */
	   UINT8  DprRange[DPR_TOTAL_SIZE];                    /* :0~1K */
} DUAL_PORT;
#pragma BYTE_NORMAL()



/*****************************************************************************
**
**          NON-DUALPORT STRUCTURE DEFINTIONS
**
*****************************************************************************/



#endif            /* __FLATCODE_H__ */

