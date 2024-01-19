/********************************************************************

                                Card.c

    Interface specific code. 
    This file only should touch the hardware.

*********************************************************************/


#include "stdafx.h"

#include <rt.h>
#include <string.h>     // strlen()
#include <stdio.h>      // sprintf()
#include <stdlib.h>


#include "vlcport.h"
#include "dcflat.h"     // EROP()
#include "driver.h"     /* SEMAPHORE */
#include "errors.h"     /* IDS_RT_DP_RW_TEST                     */
#include "auxrut.h"     /* StartTimeout(), IsTimeout(), EROP     */
#include "flatcode.h"   /* DUAL_PORT                             */
#include "card.h"       /* Init()                                */

#include "dmcgalil.h"

/******************* Card specific  Functions  *******************************/


/******************* Initialization  *******************************/




typedef unsigned char   uchar;


/* Defines for accessing the upper and lower byte of an integer. */
#define LOW_BYTE(x)         (x & 0x00FF)
#define HI_BYTE(x)          ((x & 0xFF00) >> 8)
#define PAGE(x)             ((x & 0xF000) >> 12)
#define HI_OFFSET(x)          ((x & 0x0FF0) >>4)
#define LO_OFFSET(x)          ((x & 0x000F) << 4)

/* Quick-access registers and ports for each DMA channel. */
uchar MaskReg[8]   = { 0x0A, 0x0A, 0x0A, 0x0A, 0xD4, 0xD4, 0xD4, 0xD4 };
uchar ModeReg[8]   = { 0x0B, 0x0B, 0x0B, 0x0B, 0xD6, 0xD6, 0xD6, 0xD6 };
uchar ClearReg[8]  = { 0x0C, 0x0C, 0x0C, 0x0C, 0xD8, 0xD8, 0xD8, 0xD8 };

uchar PagePort[8]  = { 0x87, 0x83, 0x81, 0x82, 0x8F, 0x8B, 0x89, 0x8A };
uchar AddrPort[8]  = { 0x00, 0x02, 0x04, 0x06, 0xC0, 0xC4, 0xC8, 0xCC };
uchar CountPort[8] = { 0x01, 0x03, 0x05, 0x07, 0xC2, 0xC6, 0xCA, 0xCE };



void    BuildDMA( SPECIAL_INST_COMMAND_DMA* const pData, UINT32 Page)
{
	int		rc  = SUCCESS;

	UINT16	mode;
	const UINT16 DMA_channel = pData->wDMAchannel;
	const UINT16 DMA_length = pData->lDMALength;
	const UINT16 DMA_address = Page >> 4;
	
    UINT16* pResult   = BuildUiotPointer( pData->Header.ofsResult );

		/* First, make sure our 'mode' is using the DMA channel specified. */

		mode = DMA_channel | pData->wTMode | pData->wInitMode 
			   |pData->wAddrMode | pData-> wMMode;


		/* Set up the DMA channel so we can use it.  This tells the DMA */
		/* that we're going to be using this channel.  (It's masked) */
		outbyte(MaskReg[DMA_channel], 0x04 | DMA_channel);

		/* Clear any data transfers that are currently executing. */
		outbyte(ClearReg[DMA_channel], 0x00);

		/* Send the specified mode to the DMA. */
		outbyte(ModeReg[DMA_channel], mode);

		/* Send the offset address.  The first byte is the low base offset, the */
		/* second byte is the high offset. */
		outbyte(AddrPort[DMA_channel], LO_OFFSET( DMA_address));
		outbyte(AddrPort[DMA_channel], HI_OFFSET(DMA_address));

		/* Send the physical page that the data lies on. */
		outbyte(PagePort[DMA_channel], PAGE(DMA_address));

		/* Send the length of the data.  Again, low byte first. */
		outbyte(CountPort[DMA_channel], LOW_BYTE(DMA_length));
		outbyte(CountPort[DMA_channel], HI_BYTE(DMA_length));

		/* Ok, we're done.  Enable the DMA channel (clear the mask). */
		// outportb(MaskReg[DMA_channel], DMA_channel);
		// use another function to enable the dma

// page fault therefore removed ? ? ?
//  *pResult = rc;

}


static int TestAndFill(UINT8* pc, const int Size, const int test, const int fill)   /* test == 1  --> no test */
{
    int i  = 0;
    for(; i < Size;  *pc++ = fill, i++)
    {
        int c = *pc & 255;
        if(test != 1  &&  test != c)
        {
            EROP("Ram Error.  Address %p, is 0x%02x, and should be 0x%02x", pc, c, test, 0);
            return IDS_FLATCODE_HW_TEST;
        }
    }
    return SUCCESS;
}


int  Init( DUAL_PORT* const dp, const UINT16 DprHWTests, P_ERR_PARAM const lpErrors)
{
    int rc = SUCCESS;

    if(DprHWTests == HWTEST_RW)
    {
        UINT8* const pc = (UINT8*)dp;

        TestAndFill(pc, DPR_TOTAL_SIZE, 1, 0xff);                     /* write 0xff overall       */
        rc = TestAndFill(pc, DPR_TOTAL_SIZE, 0xff, 0x55);             /* test 0xff and write 0x55 */
        if(rc == SUCCESS) rc = TestAndFill(pc, DPR_TOTAL_SIZE, 0x55, 0xaa);
        if(rc == SUCCESS) rc = TestAndFill(pc, DPR_TOTAL_SIZE, 0xaa, 0x00);
    }

    return rc;
}



/****************************************************************************************
    IN:     pName   --> pointer to the device user name
            Address --> device's network address
            pBuf    --> pointer to the destination buffer
            szBuf   --> size of the destination buffer

    OUT:    *pBuf   <-- "Address xx (usr_name)".  
    Note:   The device user name may be truncated!
*/
static void LoadDeviceName( char* pName, UINT16 Address, char* pBuf, size_t szBuf )
{
    if( szBuf && (pBuf != NULL) )
    {
        char* format = "Address %d";

        *pBuf = '\0';

        if( szBuf > (strlen(format)+3) )    /* Address may need more digits */
        {
            size_t  len;

            sprintf(pBuf, format, Address & 0xffff);

            len = strlen( pBuf ); 

            if( pName && ((szBuf - len) > 10) )     /* if we still have 10 free bytes  */
            {
                strcat(pBuf, " (");
                len += 2;
                strncat( pBuf, pName, szBuf-len-2 );
                *(pBuf + szBuf - 2) = '\0';
                strcat( pBuf, ")" );
            }
        }
    }
}



int  TestConfig( LPDRIVER_INST const pNet, P_ERR_PARAM const lpErrors )
{
    int rc = SUCCESS;

    LPDEVICE_INST pDevice = (LPDEVICE_INST)pNet->pDeviceList;
        
    for( ; pDevice->Type && (rc == SUCCESS); pDevice++ )
    {
        DUAL_PORT* const dp = (DUAL_PORT*)pNet->pDpr;     /* pointer to the dualport */

        pDevice->bPresent = 1;

        /*
        Check pDevice. 
        if( the device is not on the network )
            pDevice->bPresent = 0;
        */
        
 //       if( pDevice->StnAddress == 5 )     /* let's say for example, device 5 is offline */
 //           pDevice->bPresent = 0;
        
        if( !pDevice->bPresent && pDevice->bCritical)
        {
            LoadDeviceName( pDevice->pName, pDevice->StnAddress, lpErrors->Param3, sizeof(lpErrors->Param3) );
            rc = IDS_FLATCODE_DEVICE_OFFLINE; 
        }
    }

    return rc;
}


/********************* runtime specific card access functions ********************/

void DoPeekCommand( const LPDRIVER_INST pNet, SPECIAL_INST_COMMAND* const pData )
{
    int     rc       = SUCCESS;
    UINT8*  const dp = (UINT8*)pNet->pDpr; 
    const UINT32     dpSize   = DPR_TOTAL_SIZE; 
    const UINT32     dpOffset = pData->Address; 
    const LPPTBUFFER pRBuffer = &pData->RBuffer;
    const UINT32     Length   = pData->WLength;
    UINT16* pResult = BuildUiotPointer( pData->Header.ofsResult );

    if( !Length )
    {
        rc = IDS_FLATCODE_RW_ZERO;
    } 
    else if( Length > pRBuffer->Size )
    {
        rc = IDS_FLATCODE_READ_SIZE;
    }
    else if( dpOffset + Length > dpSize )
    {
        rc = IDS_FLATCODE_DPR_OUT;
    }
    else
    {
        UINT8* p = BuildUiotPointer( pRBuffer->Offset );
  //      OutCardCopy( p, dp+dpOffset, Length);
    }
    
    *pResult = rc;


}

void DoPokeCommand( const LPDRIVER_INST pNet, SPECIAL_INST_COMMAND* const pData )
{
    int          rc  = SUCCESS;
    UINT8* const dp  = (UINT8*)pNet->pDpr; 
    const UINT32     dpSize   = DPR_TOTAL_SIZE; 
    const UINT32     dpOffset = pData->Address; 
    const LPPTBUFFER pWBuffer = &pData->WBuffer;
    const UINT32     Length   = pData->WLength;
    UINT16* pResult = BuildUiotPointer( pData->Header.ofsResult );


    if( !Length )
    {
        rc = IDS_FLATCODE_RW_ZERO;
    } 
    else if( Length > pWBuffer->Size )
    {
        rc = IDS_FLATCODE_WRITE_SIZE;
    }
    else if( dpOffset + Length > dpSize )
    {
        rc = IDS_FLATCODE_DPR_OUT;
    }
    else
    {
        UINT8* p = BuildUiotPointer( pWBuffer->Offset );
 //       CardCopy( dp+dpOffset, p, Length);
    }
    
    *pResult = rc;
}



void GetDriverStatus( const LPDRIVER_INST pNet, SPECIAL_INST_COMMAND* const pData )
{
//    const DUAL_PORT* const dp = (DUAL_PORT*)pNet->pDpr;
//    UINT16* pDriverStatus = BuildUiotPointer( pData->ofsDDevStatus );
//    *pDriverStatus = dp->NetStatus;
}


void GetDeviceStatus( const LPDRIVER_INST pNet, SPECIAL_INST_COMMAND* const pData )
{
 /*   int    rc       = SUCCESS;
    UINT16 Address  = pData->Address;
    UINT16* pResult = BuildUiotPointer( pData->Header.ofsResult );
    
    if( Address < MAX_DEVICES )
    {
        UINT16* pDeviceStatus = BuildUiotPointer( pData->ofsDDevStatus );
        const DUAL_PORT* const dp = (DUAL_PORT*)pNet->pDpr;
        *pDeviceStatus = dp->DevStatus[Address];
    }
    else 
        rc = IDS_FLATCODE_INVALID_ADDERSS;
        
    *pResult = rc;
*/
}


/*
 *   Long lasting function, asynchronusely processed, called by BackgroundTask().
 *   Copies WriteBuffer into ReadBuffer, one character per second. 
 */
void DoCommand( const LPDRIVER_INST pNet, SPECIAL_INST* const pData )
{
    int rc = SUCCESS;

    SPECIAL_INST_COMMAND* const pWork = &pData->Work.paramCommand;

    UINT16* pResult = BuildUiotPointer( pWork->Header.ofsResult );

    const LPPTBUFFER pWBuffer = &pWork->WBuffer;
    const LPPTBUFFER pRBuffer = &pWork->RBuffer;

    UINT8* Src = BuildUiotPointer( pWBuffer->Offset );
    UINT8* Dst = BuildUiotPointer( pRBuffer->Offset );
    UINT16 Length  = pWork->WLength;
    UINT16 Address = pWork->Address;
    
    if( Address >= MAX_DEVICES )
    {
        rc = IDS_FLATCODE_INVALID_ADDERSS;
    }
    else if( Address == 5 )
    {
        rc = IDS_FLATCODE_DEVICE_OFFLINE;
    }
    else if( !Length )
    {
        rc = IDS_FLATCODE_RW_ZERO;
    } 
    else if( Length > pWBuffer->Size )
    {
        rc = IDS_FLATCODE_WRITE_SIZE;
    }
    else if( Length > pRBuffer->Size )
    {
        rc = IDS_FLATCODE_READ_SIZE;
    }
    else
    {
        UINT16* pRLength = BuildUiotPointer( pWork->ofsRLength );

        int    Timeout  = pWork->Timeout * 1000;        /* how much time we can afford to wait */
        UINT16 n = 0;

        pData->MarkTime = StartTimeout(Timeout);        /* milisecond when it should complete  */

        while( (rc == SUCCESS) && (n < Length) && !pNet->bGoOffLine  )
        {
            *Dst++    = *Src++;
            *pRLength = ++n;
    
            if( IsTimeout( pData->MarkTime ) )          /* if not timeout yet, sleeps 1 tick */
                rc = IDS_FLATCODE_TIMEOUT;
            else
            {
                //Delay(100);      // sleep 100 ms
                Delay(1000);     // sleep 1 second 
            }
        }
    }
    
    *pResult = rc;
}


void PortStart( SPECIAL_INST_MISC* const pData)
{

    int   rc  = SUCCESS;
        const UINT16 DMA_channel = pData->wDMAchannel;
    
    UINT16* pResult   = BuildUiotPointer( pData->Header.ofsResult );


        /* Ok, we're done.  Enable the DMA channel (clear the mask). */
                 outbyte(MaskReg[DMA_channel], DMA_channel);

// page fault therefore removed ? ? ?
// *pResult = rc;


}


void    PortStop( SPECIAL_INST_MISC* const pData)
{

    int   rc  = SUCCESS;
        const UINT16 DMA_channel = pData->wDMAchannel;
    
    UINT16* pResult   = BuildUiotPointer( pData->Header.ofsResult );


        // Stop DMA channel
                         outbyte(MaskReg[DMA_channel], 0x04 | DMA_channel);

// page fault therefore removed ? ? ?
// *pResult = rc;


}

// galil tx rx rt function.

// ofsRxValue        string
// ofsTxValue        string

void    PortGalil( SPECIAL_INST_PORT_GALIL* const pData, UINT16 Addr)
{
    char  rx[256];
    int   rc  = SUCCESS;

	UINT8* OutValue = BuildUiotPointer(pData->ofsTxValue);                // string pointer 
	UINT8* InValue  = BuildUiotPointer(pData->ofsRxValue);                // 

	UINT16* pResult   = BuildUiotPointer( pData->Header.ofsResult );
	pData->Address= Addr;


	if( Addr >= 0x100 && Addr < 0x1000 )
    {
				SetAddress(Addr);
                ClearBuffer();

                SendString(OutValue);
                rc = WaitForResponse();
                if(rc==0)
                {
						strcpy(InValue, "                             ");

                        // need to check len(<255) and time out and update rc
                        // return string needs null termination
                         strcpy(rx,GetResponse());
						 if(strcmp(rx, "")==0)
						 {
							 rx[0]=0;
							 strcpy(InValue, "");
						 }
						 else
						 {
							 rx[strlen(rx)-2]='\0';
							 strcpy(InValue, &rx[1]);
						 }

//						 printf("addr <%d> tx <%s> rx <%s>", Addr, OutValue, InValue);
                }
//			 printf("addr <%d> tx <%s> rx <%s>", Addr, OutValue, InValue);
	}
    else 
        rc = IDS_FLATCODE_INVALID_ADDERSS;

    *pResult = rc;
}

void PortInput( SPECIAL_INST_PORT* const pData)
{
    int   rc  = SUCCESS;

    UINT16 const Addr = pData->Address;
    UINT16* pResult   = BuildUiotPointer( pData->Header.ofsResult );
    
    if( Addr >= 0x100 && Addr < 0x1000 )
    {
        UINT16* pInValue = BuildUiotPointer( pData->ofsInValue );

        switch(pData->Length)
        {
            case 1: 
                                        *pInValue = inbyte( Addr );
                                        break;

            case 2: 
                                        *pInValue = inhword( Addr );
                                        break;

            case 4: 
                                        // *pInValue = inword( Addr );
                                        break;
        }
    }
    else 
        rc = IDS_FLATCODE_INVALID_ADDERSS;

    *pResult = rc;
}


void PortOutput( SPECIAL_INST_PORT* const pData)
{
    int   rc  = SUCCESS;

    UINT16 const Addr = pData->Address;
    UINT16* pResult   = BuildUiotPointer( pData->Header.ofsResult );
    
    if( Addr >= 0x100 && Addr < 0x1000 )
    {
        const UINT16 OutValue = pData->OutValue;
        switch(pData->Length)
        {
            case 1: 
                                        outbyte( Addr, OutValue );
                                        break;

            case 2: 
                    outhword( Addr, OutValue );
                                        break;

            case 4: 
                    // outword( Addr, OutValue );
                                        break;
        }

  }
    else 
        rc = IDS_FLATCODE_INVALID_ADDERSS;

    *pResult = rc;
}







