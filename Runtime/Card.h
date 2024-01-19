/***************************************************************

                Card.h             

   This file contains the interface to the manufacturer code

****************************************************************/


#ifndef  __CARD_H__
#define  __CARD_H__

int     Init( void* const dp, UINT16 const DprHWTests, P_ERR_PARAM const lpErrors);
int     TestConfig( LPDRIVER_INST const pNet, P_ERR_PARAM const lpErrors);
void    PortInput(  SPECIAL_INST_PORT* const pData);
void    PortOutput( SPECIAL_INST_PORT* const pData);
void    PortGalil( SPECIAL_INST_PORT_GALIL* const pData, UINT16 Addr);
void    PortGalilTx( SPECIAL_INST_PORT_GALIL_SPLIT* const pData, UINT16 Addr);
void    PortGalilRx( SPECIAL_INST_PORT_GALIL_SPLIT* const pData, UINT16 Addr);



void    GetDriverStatus(const LPDRIVER_INST pNet, SPECIAL_INST_COMMAND* const pData );
void    GetDeviceStatus(const LPDRIVER_INST pNet, SPECIAL_INST_COMMAND* const pData );
void    DoPeekCommand(  const LPDRIVER_INST pNet, SPECIAL_INST_COMMAND* const pData );
void    DoPokeCommand(  const LPDRIVER_INST pNet, SPECIAL_INST_COMMAND* const pData );
void    DoCommand(      const LPDRIVER_INST pNet, SPECIAL_INST* const pData );

void    PortStart( SPECIAL_INST_MISC* const pData);
void    PortStop( SPECIAL_INST_MISC* const pData);
void    BuildDMA( SPECIAL_INST_COMMAND_DMA* const pData, UINT32 Page);

void DprCardCopy(VOID *Dest, VOID *Source, UINT32 Size);

void StopDma (UINT8 Channel);


#define LOW_BYTE(x)         (x & 0x00FF)
#define HI_BYTE(x)          ((x & 0xFF00) >> 8)
#define PAGE(x)             ((x & 0xF000) >> 12)
#define HI_OFFSET(x)          ((x & 0x0FF0) >>4)
#define LO_OFFSET(x)          ((x & 0x000F) << 4)

/* Quick-access registers and ports for each DMA channel. */


#endif      /* __CARD_H__ */

