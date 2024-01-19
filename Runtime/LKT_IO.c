/************************************************************

                Flatcode.c


This file implements all the module entry points: 

int rtIdentify( P_IDENTITY_BLOCK* ppIdentityBlock );
int rtLoad(   UINT32 ScanRate, UINT32* rDirectCalls );
int rtOpen(   LPDRIVER_INST lpNet, P_ERR_PARAM lpErrors);
int rtReload( LPDRIVER_INST lpNet, P_ERR_PARAM lpErrors);
int rtOnLine( LPDRIVER_INST lpNet, P_ERR_PARAM lpErrors);
int rtInput(  LPDRIVER_INST lpNet);
int rtOutput( LPDRIVER_INST lpNet);
int rtSpecial(LPDRIVER_INST lpNet, LPSPECIAL_INST lpData);
int rtOffLine(LPDRIVER_INST lpNet, P_ERR_PARAM  lpErrors);
int rtClose(  LPDRIVER_INST lpNet, P_ERR_PARAM  lpErrors);
int rtUnload( );

**************************************************************/


#include "stdafx.h"


/*********************************************************/
/*                                                       */
/*                Flatcode Sample Program                */
/*                                                       */
/*********************************************************/ 
                 
#include <rt.h>

#include "vlcport.h"
#include "CSFlat.h"     // FCoreSup
#include "DCFlat.h"     // FDriverCore
#include "driver.h"

#include "version.h"
#include "auxrut.h"
#include "LKT_IO.h"   // DUAL_PORT
#include "task.h"
#include "card.h"
#include "dmcgalil.h"

/************************************************************/



int rtIdentify( P_IDENTITY_BLOCK* ppIdentityBlock )
{
    static IDENTITY_BLOCK IdentityBlock; 
    IdentityBlock.DriverId   = DriverLKT_IO;
    IdentityBlock.DriverVers = LKT_IOVERS;
    IdentityBlock.pName      = PRODUCT_NAME ", " PRODUCT_VERSION;
    *ppIdentityBlock = &IdentityBlock;
    return 0;
}

int rtLoad( UINT32 ScanRate, UINT32* rDirectCalls )
{
    // Executing the LOAD PROJECT command

    #if defined( _DEBUG )
        SetDebuggingFlag( 1 );  // Disable the VLC watchdog, so we can step through our code. 
    #endif  // _DEBUG


    // Use direct calls for very fast applications.  
    // With the appropriate bit set, Input(), Output() and/or Special()
    //  can be directly called from the engine thread, 
    //  saving the delay introduced by a task switch. 
    // Note:  Functions exectuted in the engine thread cannot call 
    //  some C stdlib APIs, like sprintf(), malloc(), ...
    
    // *rDirectCalls = ( DIRECT_INPUT | DIRECT_OUTPUT | DIRECT_SPECIAL );

    EROP( "rtLoad() ScanRate=%d, rDirectCalls=%x", ScanRate, *rDirectCalls, 0, 0 );

    return 0;
}

int rtOpen( LPDRIVER_INST pNet, P_ERR_PARAM pErrors)
{
    // Executing the LOAD PROJECT command

    int rc = SUCCESS;
//	UINT32	DprAddr;

    // rc = test();  // Example on how to use the call back functions implemented in the csFlat.lib

    if( pNet->Sentenial != RT3_SENTINEL )
        rc = IDS_VLCRTERR_ALIGNMENT;

    if( rc == SUCCESS )
    {
        UINT32* pSentenial = BuildUiotPointer( pNet->ofsSentenial );
        if( *pSentenial != RT3_SENTINEL )
            rc = IDS_VLCRTERR_ALIGNMENT;
    }

    EROP( "rtOpen() pNet=%p, pErrors=%p, PortAddress=%x", pNet, pErrors, pNet->PortAddress, 0 );

    if( rc == SUCCESS )
    {
        pNet->pDeviceList = BuildUiotPointer( pNet->ofsDeviceList );
        pNet->pInputList  = BuildUiotPointer( pNet->ofsInputList  );
        pNet->pOutputList = BuildUiotPointer( pNet->ofsOutputList );
    }

    if( !pNet->bSimulate )
	{
        if( rc == SUCCESS )
        {

            
//				DprAddr= pNet->DualPortAddress << 4;

			pNet->pDpr = AllocateDpr( pNet->DualPortAddress , DPR_TOTAL_SIZE );
            if( pNet->pDpr == NULL )
                rc = IDS_VLCRTERR_CREATE_DESCRIPTOR;
        }

        if( rc == SUCCESS )
        {
            LPDEVICE_INST pDevice = pNet->pDeviceList;
            LPDEVICE_IO   pInput  = pNet->pInputList;
            LPDEVICE_IO   pOutput = pNet->pOutputList;
            DUAL_PORT*    pDpr    = pNet->pDpr;

            for( ; pDevice->Type; pDevice++ )
                pDevice->pName = BuildUiotPointer( pDevice++->ofsName );


            for( ; pInput->Size; pInput++ )
            {
                // TO DO: Generate here a pointer to the dpr input area for this device
                // In our example all devices have 8 input bytes
				//default
				//			m_Input.pSrc		= (void*) m_DeviceInst.IOAddress;
				//			m_Input.pDst		= (void*)m_DeviceInst.IOAddress;

			switch(pInput->Type)
			{
			case DEVICE_4W_INPUT_G:
		    case DEVICE_2W_INPUT_G:
		    case DEVICE_1W_INPUT_G:
		    case DEVICE_1B_INPUT_G:

			    // pInput->pSrc = (void*)( pDpr + pInput->IODpr) ;

				//
				// pInput->pSrc = (void*)&pDpr->DprRange[ (UINT32)pInput->pSrc ];
				pInput->pSrc = (void*)&pDpr->DprRange[ (UINT32) pInput->IODpr ];

				break;


			}
				pInput->pDst = BuildUiotPointer( pInput->ofsUiot );            
			}

				
            for( ; pOutput->Size; pOutput++ )
            {
                // TO DO: Generate here a pointer to the dpr output area for this device
                // In our example all devices have 8 output bytes
			switch(pOutput->Type)
			{
			case DEVICE_4W_OUTPUT_G:
			case DEVICE_2W_OUTPUT_G:
			case DEVICE_1W_OUTPUT_G:
			case DEVICE_1B_OUTPUT_G:

                // pOutput->pDst = (void*) (pDpr + pOutput->IODpr); 
					
					pOutput->pDst = (void*)&pDpr->DprRange[ (UINT32) pOutput->IODpr ];
				
				break;
			}
				pOutput->pSrc = BuildUiotPointer( pOutput->ofsUiot );            
            } 
        }
        
        if( rc == SUCCESS )
            rc = Init( pNet->pDpr, pNet->DprHWTests, pErrors);

        if( rc == SUCCESS )
            rc = CreateBackgroundTask(pNet);
	}

    return rc;
}
int rtReload( LPDRIVER_INST pNet, P_ERR_PARAM pErrors)
{
    // Executing the LOAD PROJECT command
    EROP( "rtReload() pNet=%p, pErrors=%p", pNet, pErrors, 0, 0);
    if( !pNet->bSimulate )
    {
        InitLinkedList(&pNet->Pend);
        InitLinkedList(&pNet->Done);
    }

    // make sure pNet is in the same state as after rtOpen(). 
    return 0;
}

int rtOnLine( LPDRIVER_INST pNet, P_ERR_PARAM pErrors)
{
    // Executing the START PROJECT command
    int rc = SUCCESS;

    EROP( "rtOnLine() pNet=%p, pErrors=%p", pNet, pErrors, 0, 0 );
    pNet->bFirstCycle = 1;
    pNet->bGoOffLine  = 0;

    if( !pNet->bSimulate )
    {
        /* Check all devices. If critical devices are offline,  rc = IDS_FLATCODE_DEVICE_OFFLINE */

        rc = TestConfig( pNet, pErrors);
        
        /* 
            If we have a watchdog with a time consuming start procedure, start it here. 
            If it takes much shorter than a scan cycle to start it, start it in the first input cycle        
         */
    }

    return rc;

}


int rtInput( LPDRIVER_INST pNet )  // EVERY SCAN START OF SCAN

{

    // This is the beginning of the VLC scan cycle
    if( !pNet->bSimulate )
    {
        // Copy new input data from the hw to the driver input image in the UIOT. 
        LPDEVICE_IO pInput		= pNet->pInputList;


        for(; pInput->Size; pInput++)
           if( pInput->bUsed ) 
            InCardCopy( pInput->pDst, pInput->pSrc, pInput->Size, pInput->Type, pInput->IOSwap );

// READ

        /* 
        if( pNet->bFirstCycle )
            Start the watchdog.
        else
            KickWD(dp);     kick watchdog, if any
        */

        VerifyDoneList(&pNet->Done);    // Flush the completed background functions
    }

    EROP( "rtInput() pNet=%p", pNet, 0, 0, 0 );

    return SUCCESS;
}


int rtOutput( LPDRIVER_INST pNet)  // EVERY END OF SCAN 
{
    // This is the end of the VLC scan cycle
    if( !pNet->bSimulate )
    {
        // Copy new output data from the UIOT driver output image to the hw.
        LPDEVICE_IO pOutput = pNet->pOutputList;
    
        for(; pOutput->Size; pOutput++) 
            if( pOutput->bUsed )
                OutCardCopy( pOutput->pDst, pOutput->pSrc, pOutput->Size,pOutput->Type,pOutput->IOSwap);


			// SEND CHAR
        if( pNet->bFirstCycle )     /* first Output() ? */
        {
            /*  Only now we have a valid output image in the DPR. 
                EnableOutputs(dp);  enable outputs (if our hardware lets us) 
             */
            pNet->bFirstCycle = 0;
        }       
    }

    EROP( "rtOutput() pNet=%p", pNet, 0, 0, 0 );

    return SUCCESS;
}

int rtSpecial( LPDRIVER_INST pNet, LPSPECIAL_INST pData)
{
    // A trapeziodal block has been hit.

    UINT16  Result = 0;
    UINT16  Status = VLCFNCSTAT_OK;
    
    EROP( "rtSpecial() pNet=%p, pData=%p", pNet, pData, 0, 0 );

    if( !pNet->bSimulate )
    {
        int  FunctionId = pData->User.paramHeader.FunctionId;
        switch( FunctionId ) 
        {


		case DRVF_PORT_COMMAND:
                
				BuildDMA( &pData->User.paramCommandDMA, pNet->DualPortAddress);
				break;

		case DRVF_PORT_START:
				PortStart( &pData->User.paramMisc );
				break;
		case DRVF_PORT_STOP:
				PortStop( &pData->User.paramMisc );
				break;
            

            case DRVF_PORT_INPUT:
                PortInput( &pData->User.paramPort );
								

                    break;

            case DRVF_PORT_OUTPUT:
                PortOutput( &pData->User.paramPort );
		
				break;

            case DRVF_PEEK:
                    DoPeekCommand( pNet, &pData->User.paramCommand );
                    break;

            case DRVF_POKE:
                    DoPokeCommand( pNet, &pData->User.paramCommand );
					break;


            case DRVF_PORT_TX:

					PortGalilTx( &pData->User.paramPortGalilSplit, pNet->PortAddress );

					break;



			case DRVF_PORT_RX:
					PortGalilRx( &pData->User.paramPortGalilSplit, pNet->PortAddress );

					break;


            case DRVF_PORT_GALIL:
				// try bkg task see; task.c, DoAsyncSpecialFunction
				Status = Pend( pNet, pData );
				
//				PortGalil( &pData->User.paramPortGalil, pNet->PortAddress );
                   break;
            default:
                    Status = VLCFNCSTAT_WRONGPARAM;
                    break;
        }
    
        EROP("Special();  FunId= %d, Status= %d, pData= %p", FunctionId, Status, pData, 0);
    }
    else
    {
		UINT16* pResult = BuildUiotPointer( pData->User.paramHeader.ofsResult );
        if( pResult )   // some functions may not have the Result param implemented
		    *pResult = (UINT32) SUCCESS;

        Status = VLCFNCSTAT_SIMULATE;
    }

    if( pData->User.paramHeader.ofsStatus )   // some functions may not have the status param implemented
	{
		UINT16* pStatus = BuildUiotPointer( pData->User.paramHeader.ofsStatus );
		*pStatus = Status;
	}
    
    return SUCCESS;
}

int rtOffLine( LPDRIVER_INST pNet, P_ERR_PARAM pErrors)
{
    // Executing the STOP PROJECT command
    int rc = SUCCESS;

    EROP( "rtOffLine() pNet=%p, pErrors=%p", pNet, pErrors, 0, 0 );

    pNet->bGoOffLine = 1;
    if( !pNet->bSimulate )
    {
        rc = WaitForAllFunctionCompletion(pNet);  /* wait for the backgroung task to calm down */
        
        if( rc == SUCCESS )
        {
            /*
            DUAL_PORT far *  dp  = (DUAL_PORT far *)pNet->pDpr;
            if( pNet->StopState == 1 )
                rc = stop scanning;
    
            DisableOutputs(dp, &pNet->trans_count);
            DisableWD(dp); 
            */

        LPDEVICE_IO pOutput = pNet->pOutputList;
        if( pNet->StopState == 1 )
		{
        for(; pOutput->Size; pOutput++) 
            if( pOutput->bUsed )
                ZeroCardCopy( pOutput->pDst, pOutput->pSrc, pOutput->Size,pOutput->Type);
		}

        }
        
    }    

    EROP("rtOffLine(). exit  rc= %d", rc, 0, 0, 0);

    return rc;
}

/*   if Open() fails, Close() is not automatically called for this instance.
     if lpErrors == NULL, do not report any error, keep the Open()'s error code and params.  
 */ 
int rtClose( LPDRIVER_INST pNet, P_ERR_PARAM pErrors)
{
    // Executing the UNLOAD PROJECT command
    if( !pNet->bSimulate )
    {
        EROP("rtClose(). start. pNet= %p", pNet, 0, 0, 0);
        /*
        {
            DUAL_PORT far* const dp = (DUAL_PORT far *)pNet->pDpr;     / * pointer to the dualport * /
            Reset the board;
        }
        */
        
        //DeleteInterruptTask( pNet );
        DeleteBackgroundTask( pNet );
    
        if( pNet->pDpr )
        {
            FreeDpr( pNet->pDpr );
            pNet->pDpr = NULL;
        }
    }

    EROP( "rtClose() pNet=%p, pErrors=%p", pNet, pErrors, 0, 0 );
    return SUCCESS;
}

int rtUnload( LPUIOT lpUiot )
{
    // Executing the UNLOAD PROJECT command

	
        // Stop DMA channel
		StopDma(0);

		StopDma(1);
		


    EROP( "rtUnload()", 0,0,0,0 );
    return 0;
}




