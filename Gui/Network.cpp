/*********************************************************************

                        network.cpp

**********************************************************************/


#include "stdafx.h"

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>          // sprintf()

#include "drvrsym.h"
#include "drvrio.h"         // HStreamAbs
#include "resource.h"
#include "driver.h"         // FLATCODE driver specifics
#include "goodies.h"        // interface to goodies.cpp
#include "network.h"        // interface to network.cpp




/*********************************************************************************************/
/****************************** Usefull FLATCODE classes *************************************/




/*********************************************************************************************/
/****************************** CNetwork Implementation **************************************/

CNetwork::CNetwork( LPCSTR pNetName )
    : m_arrDevice( pNetName ) 
{
    m_pName             = pNetName;
    m_uConfigTotal      = 0;
    m_uInputTotal       = 0;
    m_uOutputTotal      = 0;
    m_uNameTotal        = 0;
    m_uDeviceCnt        = 0;
    m_uDeviceInputCnt   = 0;
    m_uDeviceOutputCnt  = 0;
    memset( &m_NetInst, 0, sizeof(DRIVER_INST) );
    DebugString( "creating network %s\n", m_pName );
}

CNetwork::~CNetwork()
{
    DebugString( "removing network  %s\n", m_pName );
}


int CNetwork::IsOKPortAddress()
{
    return IsBetween( m_NetInst.PortAddress, PORT_MIN, PORT_MAX) && !(m_NetInst.PortAddress % PORT_STEP);
}


int CNetwork::IsOKDPR()
{
    return 1;
		// IsBetween(m_NetInst.DualPortAddress, DPADR_MIN, DPADR_MAX) && !(m_NetInst.DualPortAddress % DPADR_STEP);
}


// make sure  m_NetInst  can safely be used at run time
int CNetwork::NetValid()
{
    int rc = SUCCESS;
    
    if( !IsOKPortAddress() )
    {
        char buf[32];
        sprintf( buf, "%x", m_NetInst.PortAddress );
        Erop( IDS_CP_INVALID_PORT, m_pName, "", "", buf);
        rc = FAILURE;
    }
        
    if( !IsOKDPR() )
    {
        char buf[32];
        sprintf( buf, "%x", m_NetInst.DualPortAddress );
        Erop( IDS_CP_INVALID_DPR, m_pName, "", "", buf);
        rc = FAILURE;
    }

   // if(!IsBetween( m_NetInst.IrqLevel, 0, NO_IRQ))
 //   {
  //      Erop( IDS_CP_INVALID_IRQLEVEL, m_pName, "", "", m_NetInst.IrqLevel & 0xff);
  //      rc = FAILURE;
  //  }

    if( (m_NetInst.StopState != SS_HOLD_LAST_STATE) && (m_NetInst.StopState != SS_ZERO) )
    {
        Erop( IDS_CP_INVALID_STOPSTATE, m_pName, "", "", "");
        rc = FAILURE;
    }
        
    if( (m_NetInst.BaudRate != BAUDRATE_125) && 
        (m_NetInst.BaudRate != BAUDRATE_250) && 
        (m_NetInst.BaudRate != BAUDRATE_500) )
    {
        Erop( IDS_CP_INVALID_BAUDRATE, m_pName, "", "", "");
        rc = FAILURE;
    }

    return rc;
}


// test if the driver is configured and load config info into DRIVER_INST
int CNetwork::LoadNetwork()
{
    int rc = SUCCESS;

    NETCONFIG    NetConfig;     // where config data is loaded from symdb
    UINT16       Size;          // the actual size of the config block as recored in symdb

    memset( &NetConfig, 0, sizeof(NETCONFIG) );
    rc = ListConfigGet( NetConfig, Size );

    if( rc == SUCCESS )
    {
        if( Size                      == sizeof(NETCONFIG)           &&  
            NetConfig.DriverId        == NETCONFIG_ID                &&  
            NetConfig.DriverVersMajor == ( NETCONFIG_VERS >> 16    ) &&  
            NetConfig.DriverVersMinor >= ( NETCONFIG_VERS & 0xffff )
          )
        {
            m_NetInst.DualPortAddress = NetConfig.DualPortAddress;
            m_NetInst.PortAddress     = NetConfig.PortAddress;
            m_NetInst.IrqLevel        = NetConfig.IrqLevel;
            m_NetInst.BaudRate        = NetConfig.BaudRate;
            m_NetInst.StopState       = NetConfig.StopState;
            m_NetInst.bSimulate       = NetConfig.bSimulate; 
            m_NetInst.bWatchdog       = NetConfig.bWatchdog; 
            m_NetInst.DprHWTests      = NetConfig.DprHWTests; 
            m_NetInst.InputRead       = NetConfig.InputRead; 
            m_NetInst.Sentenial       = RT3_SENTINEL;
        }
        else
        {
            rc = IDS_CP_DRIVER_NOT_CONFIGURED;
            Erop( rc, m_pName, "", "", "" );
        }    
    }
    else
    {
        rc = IDS_CP_CANNOT_GET_NETCONFIG;
        Erop( rc, m_pName, "", "", "" );
    }


    return rc;
}



int CNetwork::Load()
{
    int rc = LoadNetwork();

    if( rc == SUCCESS )
        rc = NetValid();

    if( ListStart() )    /* scan all devices */
    {
        do 
        {
            HDriverSym* pDev = ListGet();

            CDevice* pNewDevice = new CDevice( pDev );

            int rc1 = pNewDevice->Load( m_uNameTotal, m_uInputTotal, m_uOutputTotal );

            if( rc1 == SUCCESS )
            {
                rc1 = m_arrDevice.Check( pNewDevice );
            }

            if( rc1 == SUCCESS )
            {
                m_arrDevice.Add( pNewDevice );
                m_uNameTotal   += strlen( pNewDevice->m_pName ) + 1;

                if( pNewDevice->GetInputSize() )            // if the device has input capabilities 
                {
                    m_uDeviceInputCnt++;                                // add a DEVICE_IO record even if no tags are defined
                    m_uInputTotal  += pNewDevice->GetUiotInputSize();   // reserve UIOT input space only if there are defined tags
                }

                if( pNewDevice->GetOutputSize() )           // if the device has output capabilities 
                {
                    m_uDeviceOutputCnt++;                               // add a DEVICE_IO record even if no tags are defined
                    m_uOutputTotal += pNewDevice->GetUiotOutputSize();  // reserve UIOT output space only if there are defined tags
                }

                m_uDeviceCnt++;
            }
            else
            {
                delete pNewDevice;      // also deletes  *pDev
                rc = FAILURE;
            }

        } while( ListNext());
    }
    ListEnd();

    DebugString( "CNetwork::Load(). Name=%s, rc=%d\n", m_pName, rc );
    
    return rc;
}


int CNetwork::Check( CNetwork* pNetwork )      // makes sure "this" can live with pNetwork
{
    int rc = SUCCESS;

    if( m_NetInst.PortAddress == pNetwork->m_NetInst.PortAddress )
    {
        char b[32];
        sprintf(b, "%x", m_NetInst.PortAddress );
        Erop( IDS_CP_SAME_PORT, m_pName, pNetwork->m_pName, b, 0);
        rc = FAILURE;
    }
    if( m_NetInst.DualPortAddress == pNetwork->m_NetInst.DualPortAddress )
    {
        char b[32];
        sprintf(b, "%x", m_NetInst.DualPortAddress );
        Erop( IDS_CP_SAME_DPRADR, m_pName, pNetwork->m_pName, b, 0);
        rc = FAILURE;
    }
 //   if( m_NetInst.IrqLevel == pNetwork->m_NetInst.IrqLevel )
  //  {
 //       Erop( IDS_CP_SAME_IRQ, m_pName, pNetwork->m_pName, m_NetInst.IrqLevel & 0xff, 0);
 //       rc = FAILURE;
 //   }

    return rc; 
}


void CNetwork::ReportSizes( UINT32& rConfigTotal, UINT32& rInputTotal, UINT32& rOutputTotal )
{
    /*   The config block passed to the runtime will contain:
     *   1.  our DRIVER_INST struct
     *   2.  a DEVICE_INST array with an element for each defined device + a terminator element
     *   3.  a DEVICE_IO array with an element for each defined device having  input capabilities + a terminator element
     *   4.  a DEVICE_IO array with an element for each defined device having output capabilities + a terminator element
     *   5.  the zero terminated names of all defined devices
     *   6.  RT3_SENTENIAL, for validity checking.
     */

    m_uConfigTotal = sizeof(DRIVER_INST) - sizeof( NETWORK)          + 
                    (m_uDeviceCnt       + 1) * sizeof( DEVICE_INST ) +
                    (m_uDeviceInputCnt  + 1) * sizeof( DEVICE_IO   ) +
                    (m_uDeviceOutputCnt + 1) * sizeof( DEVICE_IO   ) +
                    m_uNameTotal  +
                    sizeof( UINT32 );       // RT3_SENTENIAL

    rConfigTotal = m_uConfigTotal;
    rInputTotal  = m_uInputTotal;
    rOutputTotal = m_uOutputTotal;
}


// called in NetPass2 when we have real UIOT offsets for Config, Input and Output areas
void CNetwork::UpdateOffsets( UINT32 CfgOfs, UINT32 InputOfs, UINT32 OutputOfs, UINT32 IODelta )
{
                                                            // Prepare for runtime validity checking
    m_NetInst.ofsSentenial = CfgOfs + m_uConfigTotal - sizeof(UINT32);

    CfgOfs += sizeof(DRIVER_INST) - sizeof( NETWORK);       // offset of the first DEVICE_INST block

    m_NetInst.ofsDeviceList = CfgOfs;
    CfgOfs += (m_uDeviceCnt+1) * sizeof( DEVICE_INST );     // offset of the first DEVICE_IO input block

    m_NetInst.ofsInputList = CfgOfs;
    CfgOfs += (m_uDeviceInputCnt+1) * sizeof( DEVICE_IO );  // offset of the first DEVICE_IO output block

    m_NetInst.ofsOutputList = CfgOfs;
    CfgOfs += (m_uDeviceOutputCnt+1) * sizeof( DEVICE_IO ); // offset of the first device name

    m_arrDevice.UpdateOffsets( CfgOfs, InputOfs, OutputOfs, IODelta );
}



// Called in NetPass2 to write config data in the config area
// The # of written bytes must match the ConfigTotal value declared in NetPass1
void CNetwork::WriteConfigData( HStreamAbs& rFileHandle )
{
    // Write the config block as defined by 'ReportSizes'
    // The config block will be read by the runtime

    UINT32 uConfigTotal = 0;

    {
        char*  pCfg  = (char*) & m_NetInst + sizeof(NETWORK);
        UINT16 szCfg = sizeof(DRIVER_INST) - sizeof(NETWORK);
        rFileHandle.Write( pCfg, szCfg );
        uConfigTotal += szCfg;
    }

    for( int DevIndex = 0; DevIndex < m_arrDevice.GetSize() ; DevIndex++ )
    {
        DEVICE_INST* pDeviceInst = &m_arrDevice[ DevIndex ]->m_DeviceInst;
        rFileHandle.Write( pDeviceInst, sizeof( DEVICE_INST ) );
        uConfigTotal += sizeof( DEVICE_INST );
    }
    {
        DEVICE_INST Terminator;
        memset( &Terminator, 0, sizeof(DEVICE_INST) );
        rFileHandle.Write( &Terminator, sizeof( DEVICE_INST ) );
        uConfigTotal += sizeof( DEVICE_INST );
    }

    for( DevIndex = 0; DevIndex < m_arrDevice.GetSize() ; DevIndex++ )
    {
        DEVICE_IO* pInput = &m_arrDevice[ DevIndex ]->m_Input;
        if( pInput->Size )
        {
            rFileHandle.Write( pInput, sizeof( DEVICE_IO ) );
            uConfigTotal += sizeof( DEVICE_IO );
        }
    }
    {
        DEVICE_IO Terminator;
        memset( &Terminator, 0, sizeof(DEVICE_IO) );
        rFileHandle.Write( &Terminator, sizeof( DEVICE_IO ) );
        uConfigTotal += sizeof( DEVICE_IO );
    }

    for( DevIndex = 0; DevIndex < m_arrDevice.GetSize() ; DevIndex++ )
    {
        DEVICE_IO* pOutput = &m_arrDevice[ DevIndex ]->m_Output;
        if( pOutput->Size )
        {
            rFileHandle.Write( pOutput, sizeof( DEVICE_IO ) );
            uConfigTotal += sizeof( DEVICE_IO );
        }
    }
    {
        DEVICE_IO Terminator;
        memset( &Terminator, 0, sizeof(DEVICE_IO) );
        rFileHandle.Write( &Terminator, sizeof( DEVICE_IO ) );
        uConfigTotal += sizeof( DEVICE_IO );
    }

    for( DevIndex = 0; DevIndex < m_arrDevice.GetSize() ; DevIndex++ )
    {
        LPCSTR pName  = m_arrDevice[ DevIndex ]->m_pName;
        size_t szName = strlen( pName );
        rFileHandle.Write( pName, (UINT16)strlen( pName ) );
        char Terminator = 0;
        rFileHandle.Write( &Terminator, sizeof( char ) );
        uConfigTotal += szName + 1;
    }

    {
        UINT32 RT3Sentenial = RT3_SENTINEL;
        rFileHandle.Write( &RT3Sentenial, sizeof( UINT32 ) );
        uConfigTotal += sizeof( UINT32 );
    }

    ASSERT( uConfigTotal == m_uConfigTotal );
}



int  CNetwork::ListConfigGet( NETCONFIG& Netconfig, UINT16& rBytesRet )
{
    return ListNetworkConfigGet( m_pName, &Netconfig, sizeof(NETCONFIG), rBytesRet );
}

int  CNetwork::ListConfigPut( NETCONFIG& Netconfig )
{
    return ListNetworkConfigPut( m_pName, &Netconfig, sizeof(NETCONFIG) );
}

BOOL CNetwork::ListStart()
{
    return ListNetworkStart( m_pName );
}



/************************************************************************************/

CNetworkArray::CNetworkArray( DRIVER_KEY NetKey, LPCSTR ProjPath ) 
{
    SetSize( 0, MAX_DRV_INSTANCES );
    DebugString( "Creating CNetworkArray. Path=%s\n", ProjPath);
    m_NetKey   = NetKey;    // FLATCODE_DRIVER may be useful when more than 1 driver described in the rcd file
    m_ProjPath = ProjPath;  // full path to the project directory
}

CNetworkArray::~CNetworkArray()  
{ 
    DebugString( "removing CNetworkArray\n", "");
    for(int i = 0; i < GetSize(); i++ )
        delete GetAt( i );
    RemoveAll(); 
}


int CNetworkArray::Check( CNetwork* pNetwork )
{
    int rc = SUCCESS;

    for(int i = 0; i < GetSize(); i++ )
    {
        CNetwork* pNet = GetAt( i );
        int       rc1  = pNetwork->Check( pNet );

        if( rc1 != SUCCESS )
            rc = FAILURE;
    }

    return rc; 
}


CNetwork*  CNetworkArray::GetNetwork( LPCSTR lpNetName )
{
    for(int i = 0; i < GetSize(); i++ )
    {
        CNetwork* pNetwork = GetAt( i );
        if( pNetwork->m_pName == lpNetName )
            return pNetwork;
    }

    return NULL;
}

/**************************** End of CNetwork Implementation *********************************/
/*********************************************************************************************/



/************************** End of Usefull FLATCODE classes **********************************/
/*********************************************************************************************/


