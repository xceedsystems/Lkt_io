/****************************************************************************

                            Driver.h

    FLATCODE Driver specfic UIOT structures and adjunct to the network node

*****************************************************************************/



#ifndef __DRIVER_H__
#define __DRIVER_H__



// "FLATCODE" id num. Make sure this id is unique accross all VLC C-modules and drivers.
#define DriverLKT_IO       0x5e7f8d45L


/*
    Version 01.0002
    Bump this version # every time DRIVER_INST, DEVICE_INST or
    DEVICE_IO structs have changed. 
    This will force old projects to be recompiled before execution. 
*/
#define LKT_IOVERS         0x00010002L



//  Network config struct id
#define NETCONFIG_ID        DriverLKT_IO

/***************************************************************

                            version.h

    Use this file to set your company logo, the name and 
    the version of your product

 ***************************************************************/


#ifndef __VERSION_H__
#define __VERSION_H__


#define COMPANYNAME             "Xceed systems."
#define LEGALCOPYRIGHT          "Copyright 2007 Xceed Systems."

#include "globcom.h"
        
// product version info 
#define PRODUCT_MAJOR_VERSION   6
#define PRODUCT_MINOR_VERSION   0
#define PRODUCT_FIX_VERSION     0                               // Not in string if zero
#define PRODUCT_BUILD_NUMBER    4                               // Nonzero if engineering or alpha/beta release
#define PRODUCT_VERSION         "6.0 Build 4" OPTION_STR	    // last chg 5/10/99
#define PRODUCT_NAME            "I/O Driver v4"

// Empty on release 
#define FILE_STR

// FILE version info
#define FILE_MAJOR_VERSION      PRODUCT_MAJOR_VERSION
#define FILE_MINOR_VERSION      PRODUCT_MINOR_VERSION
#define FILE_FIX_VERSION        PRODUCT_FIX_VERSION
#define FILE_BUILD_NUMBER       PRODUCT_BUILD_NUMBER
#define FILE_VERSION            PRODUCT_VERSION

#ifdef WIN32

// GUI version info
#ifdef _DEBUG
#define FILE_NAMEEXT            "IO.IO3"      // if you are using MFC, add the suffix 'D'
#else
#define FILE_NAMEEXT            "IO.IO3"
#endif
#define FILE_DESCRIPTION        "NT side of the Driver" OPTION_STR

#else   // !WIN32

// Runtime version info
#define FILE_NAMEEXT            "IO.RT3"
#define FILE_DESCRIPTION        "INtime side of the Driver" OPTION_STR

#endif  // WIN32


#define FILE_NAMEINT            "IO"


#endif                          //  __VERSION_H__

	


/* 
    Network config struct version
    Bump this version # every time NETCONFIG got new fields. 
    NetPass1() will force drivers in old projects to be reconfigured. 
    If old fields and NETCONFIG size are preserved, 
    configuration in old fields will be kept. 
*/
#define NETCONFIG_VERS      0x00010002L



// Device  config struct id
#define DEVCONFIG_ID        0x12345678L    



/*
    Device  config struct version
    Bump this version # every time DEVCONFIG got new fields. 
    NetPass1() will force devices in old projects to be reconfigured. 
    If old fields and DEVCONFIG size are preserved, 
    configuration in old fields will be kept. 
*/
#define DEVCONFIG_VERS      0x00010002L     


// load this value in DRIVER_INST for rt checking
#define RT3_SENTINEL        0x55667788L


// max 4 networks can be controlled by 1 PC
#define  MAX_DRV_INSTANCES          1       



#include  "errors.h"


#ifndef APSTUDIO_READONLY_SYMBOLS



#ifndef VLCPORT_H__
#include "vlcport.h"
#pragma warning( disable: 4244 )
#endif

#ifndef DATASEG_H__
#include "dataseg.h"
#endif


/*****************************************************************************************

    This file keeps all necessary definitions needed for our flatcode driver.
    This is a driver build for didactical purposes. It can be used as a starting point 
    when we need to create a new real driver.
    The FLATCODE driver assumes we have an IO network. 
    The network is controlled by a card and can have up to MAX_DEVICES devices. 
    The card is accesible by a dual port ram and a port address.
    The DRP is DPR_TOTAL_SIZE bytes large and contains a control space, an input space and
    an output space.
    To keep things simple, our devices have all the same size: 8 bytes. 
    They are mapped in the DPR IO spaces based on their network address: device 0's input
    area can be found at the beginning of the DPR input space, device 1's input area is 
    8 bytes farther...  The same mechanism works for devices' output points. 
    In order to see input tags changing, we have to use an external application 
    that writes in the DPR input space.  We can also use the POKE special function to write there. 
    When generating the driver we can change DPR_OUTPUT_OFF to match DPR_INPUT_OFF. 
    Input space will then overlap the output space, so in the VLC project all input tags 
    will be animated by their correspondent output tags.    

    Configuring the driver:
    1.  Choose a DPR address.
    2.  Choose a port address (didactic purpose only: will not be used)
    3.  Choose an interrupt level (didactic purpose only: the interrupt routine does nothing)
    4.  Skip HW tests. We may want to control the thoroughness of initial hw tests.
    5.  Simulate:  if on, there will be no attempt to touch the hardware.
    6.  Watchdog:  if on, the card's watchdog must be cyclicly kicked at run time.
    7.  Cyclic Input Read:  if on,  we update UIOT input image every Input() call.
                            if off, we have to rely on some hardware features telling us if 
                                    any input changed.
    
    Configuring devices:
    1.  Choose a link address (0 ... 127). This determines allocation in the DPR IO space
    2.  Critical:  if on, this device must be active at Online()    
    

    There are 5 different driver models we are studying:
    Model1:     No special functions at all.    (Simulate)
    Model2:     Only sync s.f. No background task.  (PID, Utility, ...)
    Model3:     Sequential async s.f. processing:  Pend & Done lists. (ex. ABKTX, MTL, ...)
                    DRIVER_INST needs  MarkTime.
    Model4:     Simoultaneous async s.f. processing: Pend, Run, Done lists (ex. DHPLUS, ...)
                    The hw supports commands with reply. 
                    New commands can be launched while others are waiting for their replies.
    Model5:     Paralel sequential s.f. processing. 
                    The hw supports a fixed # of channels that can accept commands.
                    Commands executed on different channels can run in paralel.
                    Commands executed on a channel are performed on a FIFO basis.
                    Pend[], Done lists    (DATALOG)
                    DRIVER_INST needs  MarkTime[].
    

    Here is an example for model 3:    
    
*****************************************************************************************/



//************   1 byte alignment typedef structures !!!   ************

#pragma BYTE_ALIGN(_SPECIAL_INST)
typedef  struct _SPECIAL_INST*  LPSPECIAL_INST_;
typedef                 UINT8*  LPUINT8;

typedef  selector   SEMAPHORE;

typedef  UINT32     UIOTREF2CHAR;
typedef  UINT32     UIOTREF2SINT8;
typedef  UINT32     UIOTREF2UINT8;
typedef  UINT32     UIOTREF2SINT16;
typedef  UINT32     UIOTREF2UINT16;
typedef  UINT32     UIOTREF2SINT32;
typedef  UINT32     UIOTREF2UINT32;
typedef  UINT32     UIOTREF2DOUBLE;
typedef  UINT32     UIOTREF2VOID;


#pragma BYTE_ALIGN(_LINKED_LIST)
typedef struct _LINKED_LIST 
{
    LPSPECIAL_INST_     pHead;      // Pointer to the first element in the linked list
    LPSPECIAL_INST_     pTail;      // Pointer to the last element in the linked list
    SEMAPHORE           Semaphore;  // Semaphore that locks the SPECIAL_INST list
    UINT16              uCounter;   // How many items are enqueued here
} LINKED_LIST, *LPLINKED_LIST; 
#pragma BYTE_NORMAL()


#pragma BYTE_ALIGN(_PTBUFFER)
typedef struct _PTBUFFER 
{
    UIOTREF2VOID    Offset;  
    UINT32          Size;           // Use PTBUFFER type for PT_BUFFERs
} PTBUFFER, * LPPTBUFFER;           // Its size is 8 bytes
#pragma BYTE_NORMAL()


#pragma BYTE_ALIGN(_TASK)
typedef struct _TASK 
{
    UINT16      hTask;          // background/interrupt task handle
    SEMAPHORE   Semaphore;      // Where the background task waits
    void*       pStack;         // Pointer to the stack allocated to the task
    UINT16      bBusy;          // True if Special I/O Task is working on packet, used during shutdown
    UINT16      Error;          // error code for the task's init sequence
    void*       IrqThunk;       // pointer to the interrupt routine
    UINT16      level;          // irmx encoded IRQ
    UINT16      align;
} TASK, * LPTASK; 
#pragma BYTE_NORMAL()





#pragma BYTE_ALIGN(_DEVICE_INST)         // 1 byte alignment
typedef struct _DEVICE_INST
{
    UIOTREF2VOID ofsName;       // UIOT offset to the device name generated at compile time
    UINT16       StnAddress;       // device's network address
    UINT16       Type;          // DEVICE_4W_INPUT, ... Never 0 !!!
     UINT16       ProductCode;     
     UINT8        bCritical;     // if 1 --> device must be online when load and go
     UINT8        bPresent;      // if 1 --> device was online when load and go
     UINT16		IOAddress;          // added
    UINT16		IOSwap;				// added 
    char*        pName;         // Usable UIOT pointer to the device name generated at runtime based on ofsName.
} DEVICE_INST, *LPDEVICE_INST;
#pragma BYTE_NORMAL()


#pragma BYTE_ALIGN(_DEVICE_IO)  // 1 byte alignment
typedef struct _DEVICE_IO       // Specifies the UIOT offset and the size for each device
{
    void*        pSrc;          // DPR  offset/pointer for input devices || UIOT offset/pointer for output devices
    void*        pDst;          // UIOT offset/pointer for input devices || DPR  offset/pointer for output devices
    UINT16       Size;          // device input or output size.  Never 0 !!!
    UINT16       StnAddress;       // device's network address
    UIOTREF2VOID ofsUiot;       // ofsDst for input devices or ofsSrc for output devices
    UINT8        bPresent;      // If the device missed at OnLine(), skip it
    UINT8        bUsed;         // If no I/O tags defined in the UIOT, skip it
    UINT16		IOSwap;				// added 
    UINT16      Type;
    UINT16      IODpr;

} DEVICE_IO, *LPDEVICE_IO;            
#pragma BYTE_NORMAL()


#pragma BYTE_ALIGN(_DRIVER_INST) 
typedef struct _DRIVER_INST 
{
    NETWORK Net;

        // Compile-time Static variables.  This structure maps to UIOT

    UIOTREF2VOID    ofsDeviceList;      // Where the DEVICE_INST list starts.
    UIOTREF2VOID    ofsInputList;       // Where in the DEVICE_IO  Input list starts
    UIOTREF2VOID    ofsOutputList;      // Where in the DEVICE_IO Output list starts
    UINT16          PortAddress;        // 0x250,  0x254,  ...
    UINT16          align;
    UINT32          DualPortAddress;    // 0xd0000, 0xd1000, ...
    UINT16          IrqLevel;           // 0 ... 15
    UINT16          BaudRate;           // BAUDRATE_125, BAUDRATE_250_...
    UINT16          StopState;          // SS_HOLD_LAST_STATE,  SS_ZERO
    UINT16          bSimulate;          // =0 --> interface card must be present
    UINT16          bWatchdog;          // =1 --> kick the watchdog
    UINT16          DprHWTests;         // HWTEST_RW, HWTEST_OFF
    UINT16          InputRead;          // INPUT_READ_CYCLIC, INPUT_READ_COS,
    UINT16          align1;
    
        // Run-time Dynamic Variables
    LPDEVICE_INST   pDeviceList;        // Where the DEVICE_INST list starts.
    LPDEVICE_IO     pInputList;         // Where in the DEVICE_IO  Input list starts
    LPDEVICE_IO     pOutputList;        // Where in the DEVICE_IO Output list starts

    void*           pDpr;               // (DUAL_PORT*) - iRmx ptr to where the board is in physical memory
    UINT16          bFirstCycle;        // Set by OnLine(), reset by Output(). Read by Input() and Output()
    UINT16          bGoOffLine;         // Tell all the bkg functions to shutdown

    LINKED_LIST     Pend;               // Pointer to the linked list of pending functions
    LINKED_LIST     Done;               // Pointer to the linked list of done  functions

    TASK            BackgroundTask;     // controls for the background task
    TASK            InterruptTask;      // controls for the interrupt task

    UIOTREF2UINT32  ofsSentenial;       // 0x55667788 - display this value using Soft Scope to check corrct map
    UINT32          Sentenial;          // 0x55667788 - display this value using Soft Scope to check corrct map

} DRIVER_INST, *LPDRIVER_INST;    
#pragma BYTE_NORMAL() 


#pragma BYTE_ALIGN( _SPECIAL_INST_HEADER )      // Must be first block in all paremeter blocks
typedef struct _SPECIAL_INST_HEADER
{       // Compile-time Static variables.  This structure maps to .rcd descrition
                                    // off, sz, ob.sz
    UINT16          FunctionId;     //  0    2   2L   PT_CONST  --> UINT16, _SIZE 2L
    UINT16          align;          //  2    2
    UIOTREF2UINT16  ofsStatus;      //  4    4   2L   PT_REF    --> tag's offset in the UIOT
    UIOTREF2UINT16  ofsResult;      //  8    4   2L   PT_REF    --> tag's offset in the UIOT
} SPECIAL_INST_HEADER;              //      12 == sizeof( SPECIAL_INST_HEADER )
#pragma BYTE_NORMAL()
/*
    Note: beacuse all functions have an Id field and a return status, we can standardize them 
    at offsets 0 and 4. This is especially helpful when using customized parameter structures 
    to better match function particularities and to save memory. 
*/


#pragma BYTE_ALIGN( _SPECIAL_INST_COMMAND_DMA ) 
typedef struct _SPECIAL_INST_COMMAND_DMA
{       // Compile-time Static variables.  This structure maps to .rcd descrition
                                        // off, sz, ob.sz
    SPECIAL_INST_HEADER Header;         //  0   12        the header must always be first
    UINT16             wDMAchannel;    // 12    2   2L   PT_VALUE
    UINT16             wDPRaddress;    // 14    2   2L   PT_VALUE
    UIOTREF2UINT16      lDMALength;     // 16    4   2L   PT_REF    --> tag's offset in the UIOT
    UINT16              wTMode ;        // 20    2   2L   PT_VALUE  --> UINT16, _SIZE 2L
    UINT16              wInitMode;      // 22    2   2L
    UINT16              wAddrMode;      // 24    2   2L   
	UINT16              wMMode;         // 26    2   2L   

} SPECIAL_INST_COMMAND_DMA;                 //      48 == sizeof( SPECIAL_INST_COMMAND )
#pragma BYTE_NORMAL()

#pragma BYTE_ALIGN( _SPECIAL_INST_COMMAND ) 
typedef struct _SPECIAL_INST_COMMAND
{       // Compile-time Static variables.  This structure maps to .rcd descrition
                                        // off, sz, ob.sz
    SPECIAL_INST_HEADER Header;         //  0   12        the header must always be first
    UINT32              Address;        // 12    4   4L   PT_VALUE, PT_DEVICE --> UINT32
    UIOTREF2UINT16      ofsDDevStatus;  // 16    4   2L   PT_REF    --> tag's offset in the UIOT
    PTBUFFER            RBuffer;        // 20    8   8L   PT_BUFFER --> tag's offset in the UIOT
    PTBUFFER            WBuffer;        // 28    8   8L   PT_BUFFER --> tag's offset in the UIOT
    UIOTREF2UINT16      ofsRLength;     // 36    4   2L   PT_REF    --> tag's offset in the UIOT
    UINT16              WLength;        // 40    2   2L   PT_VALUE  --> UINT16, _SIZE 2L
    UINT16              align;          // 42    2
    UINT32              Timeout;        // 44    4   4L   PT_VALUE  --> UINT32, _SIZE 4L
} SPECIAL_INST_COMMAND;                 //      48 == sizeof( SPECIAL_INST_COMMAND )
#pragma BYTE_NORMAL()



#pragma BYTE_ALIGN(_SPECIAL_INST_PORT)  // we may have substitutes for SPECIAL_INST_PARAM
typedef struct _SPECIAL_INST_PORT
{       // Compile-time Static variables.  This structure maps to .rcd descrition
                                        // off, sz, ob.sz
    SPECIAL_INST_HEADER Header;         //  0   12        the header must always be first
    UINT16              Address;        // 12    2   2L   PT_VALUE  --> UINT16, _SIZE 2L
    UINT16              Length;         // 14    2   2L   PT_VALUE  --> UINT16, _SIZE 2L
    UIOTREF2UINT16      ofsInValue;     // 16    4   2L   PT_REF    --> tag's offset in the UIOT 
    UINT16              OutValue;       // 20    2   2L   PT_VALUE  --> UINT16, _SIZE 2L
    UINT16              align;          // 22    2   2L   PT_VALUE  --> UINT16, _SIZE 2L

} SPECIAL_INST_PORT;                    //      24 == sizeof( SPECIAL_INST_PORT )
#pragma BYTE_NORMAL()

#pragma BYTE_ALIGN(_SPECIAL_INST_PORT_GALIL)  
typedef struct _SPECIAL_INST_PORT_GALIL
{       // Compile-time Static variables.  This structure maps to .rcd descrition
                                        // off, sz, ob.sz
    SPECIAL_INST_HEADER Header;         //  0   12        the header must always be first
    UINT16              Address;        // 12    2   2L   PT_VALUE  --> UINT16, _SIZE 2L
    UINT16              Length;         // 14    2   2L   PT_VALUE  --> UINT16, _SIZE 2L
    UIOTREF2CHAR        ofsTxValue;     // 16    4   2L   PT_REF    --> tag's offset in the UIOT 
	UIOTREF2CHAR		ofsRxValue;		// 20	 4  
    UINT16              OutValue;       // 24	 2
    UINT16              align;          // 26	 2

} SPECIAL_INST_PORT_GALIL;                    //      28 == sizeof( SPECIAL_INST_PORT )
#pragma BYTE_NORMAL()

#pragma BYTE_ALIGN(_SPECIAL_INST_PORT_GALIL_SPLIT)  
typedef struct _SPECIAL_INST_PORT_GALIL_SPLIT
{       // Compile-time Static variables.  This structure maps to .rcd descrition
                                        // off, sz, ob.sz
    SPECIAL_INST_HEADER Header;         //  0   12        the header must always be first
    UINT16              Address;        // 12    2   2L   PT_VALUE  --> UINT16, _SIZE 2L
    UIOTREF2CHAR        ofsGalilValue;  // 14    4   2L   PT_REF    --> tag's offset in the UIOT 
	UINT16              align;          // 18	 2

} SPECIAL_INST_PORT_GALIL_SPLIT;                    //      20 == sizeof( SPECIAL_INST_PORT )
#pragma BYTE_NORMAL()



#pragma BYTE_ALIGN(_SPECIAL_INST_MISC)
typedef struct _SPECIAL_INST_MISC
{       // Compile-time Static variables.  This structure maps to .rcd description
                                        // off, sz, ob.sz
    SPECIAL_INST_HEADER Header;         //  0   12        the header must always be first
    UINT16             wDMAchannel;     // 12    2   2L   PT_VALUE
} SPECIAL_INST_MISC;                    //      14 == sizeof( SPECIAL_INST_PORT )
#pragma BYTE_NORMAL()



typedef union _SPECIAL_INST_PARAM
{       // Compile-time Static variables.  This structure maps to .rcd descrition
                                        // off, sz
    SPECIAL_INST_HEADER				paramHeader;		//  0   12
    SPECIAL_INST_COMMAND			paramCommand;		//  0   48
    SPECIAL_INST_COMMAND_DMA		paramCommandDMA;	//  0   48
    SPECIAL_INST_PORT				paramPort;			//  0   24
    SPECIAL_INST_PORT_GALIL			paramPortGalil;		//  0   28   added for galil tx/rx
	SPECIAL_INST_PORT_GALIL_SPLIT	paramPortGalilSplit;	// added for tx and rx
	SPECIAL_INST_MISC				paramMisc;

} SPECIAL_INST_PARAM;                   //      48 == sizeof(SPECIAL_INST_PARAM)


typedef struct _SPECIAL_INST
{       // Compile-time Static variables.  This structure maps to .rcd descrition
                                        // off,  sz
    SPECIAL_INST_PARAM  User;           //   0   48
    SPECIAL_INST_PARAM  Work;           //  48   48

        // generic, same for all drivers having asyncronous special functions
    UINT32                MarkTime;     //  96    4  when this s.f. must be complete
    SINT16                Status;       // 100    2
    UINT16                Busy;         // 102    2    
    struct _SPECIAL_INST* pNext;        // 104    4

} SPECIAL_INST, *LPSPECIAL_INST;        //      108 == sizeof( SPECIAL_INST )

/*
Note1: This struct is declared 1 byte aligned on top of file. The struct description is 
       evaluated by the "Runtime" sub-project only.  
       The 'Gui' subproject evaluates the SPECIAL_INST parameter block as presented 
       by the FNC_... definitions. 

Note2: For a very simple function module,  SPECIAL_INST is sufficient.  
       Parameter fields can be described directly in SPECIAL_INST.  
       SPECIAL_INST_PARAM, SPECIAL_INST_PORT, SPECIAL_INST_COMMAND and SPECIAL_INST_HEADER 
       are optional.  They have been defined here only to show a more complex example. 

Note3: In order to save memory SPECIAL_INST can be used only for asynchronous special functions. 
       SPECIAL_INST_COMMAND, SPECIAL_INST_PORT, or even SPECIAL_INST_HEADER 
       will do the job for synchronous special functions. 
       Make sure the correct param block size is declared NET(DEV)_FUNC_TYPE paragraph (p#2).

Note4: Because asynchronous functions are executed concurenlty with the flowchart code, 
       it is safer to provide a copy of the parameter block, to be used by the background thread. 
       This is why we have introduced the 'User' and 'Work' areas. 
       'User' is the area marked by the compiler to be filled in every time a function 
       is called. When the function is posted for execution, 'User' is copied into 'Work' 
       and 'Work' is what the background sees.
       Make sure the fields in 'User' and 'Header' match the FNC_... definitions. 
       It is a good idea to have them both mapped at offset 0.

Note5: The Runtime Special() entry point offers a pointer to the associated SPECIAL_INST. 
       Depending on the FunctionId, the right parameter layout will be selected. 
       This can be implemented in 3 ways: 
       a. Define 1 layout only large enough to encompass all parameters needed by any function. 
       b. Define 1 layout for every function, and cast to the right one based on the FunctionId. 
       c. Define 1 layout for every function, store them into a union and select the right 
          union branch based on the FunctionId. 
       Our current implementation is a mixture of a. and c. and should be optimal 
       for consumed memory and code complexity. 
*/


#ifdef WINVER          // This is for MSVC compiler


#ifndef DRVRUTIL_H__
#include "drvrutil.h"   // SS_ZERO
#endif

// What we put into the database for network config

#pragma BYTE_ALIGN(_NETCONFIG)     // 1 byte alignment
typedef struct _NETCONFIG
{
    UINT32           DriverId;          //  0  NETCONFIG_ID
    UINT16           DriverVersMinor;   //  4  LOW(  NETCONFIG_VERS )
    UINT16           DriverVersMajor;   //  6  HIGH( NETCONFIG_VERS )
    UINT32           DualPortAddress;   //  8  0xd0000, 0xd1000, ...
    UINT16           PortAddress;       // 12  0x250,  0x254,  ...
    UINT16           IrqLevel;          // 14  0 ... 15
    UINT16           BaudRate;          // 16  BAUDRATE_125, BAUDRATE_250, ...
    STOP_STATE_TYPES StopState;         // 18  SS_HOLD_LAST_STATE,  SS_ZERO
    //UINT16           StopState;       // 18  0 --> keep scanning, 1 --> stop scanning

    UINT16           bSimulate;         // 20  =0 --> interface card must be present
    UINT16           bWatchdog;         // 22  =1 --> kick the watchdog
    UINT16           DprHWTests;        // 24  HWTEST_RW, HWTEST_OFF
    UINT16           InputRead;         // 26  INPUT_READ_CYCLIC, INPUT_READ_COS,
    
    UINT16           reserved1;         // 28  add new fields without changing NETCONFIG size
    UINT16           reserved2;         // 30  
    UINT16           reserved3;         // 32  
    UINT16           reserved4;         // 34  
    UINT16           reserved5;         // 36  
    UINT16           reserved6;         // 38  
    UINT16           reserved7;         // 40  
    UINT16           reserved8;         // 42  
} NETCONFIG;                            // 44  == NET_CONFIG_SIZE == sizeof(NETCONFIG)
#pragma BYTE_NORMAL()


#pragma BYTE_ALIGN(_DEVCONFIG)  // 1 byte alignment
typedef struct _DEVCONFIG
{                               // Byte Offset
    UINT32  DriverId;           //  0 NETCONFIG_ID
    UINT32  DeviceId;           //  4 DEVCONFIG_ID
    UINT16  DeviceVersMinor;    //  6 LOW(  DEVCONFIG_VERS )
    UINT16  DeviceVersMajor;    //  8 HIGH( DEVCONFIG_VERS )
    UINT16  StnAddress;            // 12 device's address on the link
    UINT16  bCritical;          // 14 =1 --> this device must be present on the link
    UINT8   ProductCode[8];     // 16 edit field. 16#ffff
    UINT16  IOAddress;       // 24 add new fields without changing DEVCONFIG size
    UINT16  IOSwap;          // 26 
    UINT16  reserved1;          // 28 
    UINT16  reserved2;          // 30 
} DEVCONFIG;                    // 32 == DEVCONFIG_SIZE == sizeof(DEVCONFIG)
#pragma BYTE_NORMAL()
/*
    Note: The reserved fields will be used for future developpment. 
    They ensure compatibility with projects generated by older versions of this driver.
*/


#endif      // WINVER


#endif      // ! APSTUDIO_READONLY_SYMBOLS

/* 
    Defines for .rcd file 
    Arithmetic expressions are allowed to define RC and RCD constants, 
    when  ONLY using + and -.  
    It is a good idea to have them encapsulated in ( ).
    Never use * and /.  The RC compiler silently ignores them.
*/


// SPECIAL_INST offsets & sizes
#define FNC_HD_FUNCTIONID           0L 
#define FNC_HD_FUNCTIONID_SIZE          2L      // PT_CONST  --> size 2L    
#define FNC_HD_STATUS               4L 
#define FNC_HD_STATUS_SIZE              2L      // PT_REF --> size of the object pointed to
#define FNC_HD_RESULT               8L 
#define FNC_HD_RESULT_SIZE              2L      // PT_REF --> size of the object pointed to

// dma def.
#define FNC_DMA_CHANNEL				12L 
#define FNC_DMA_CHANNEL_SIZE			2L      
#define FNC_DMA_DPR					14L				// offset now
#define FNC_DMA_DPR_SIZE				2L      
#define FNC_DMA_LENGTH				16L 
#define FNC_DMA_LENGTH_SIZE				4L     
#define FNC_DMA_TMODE				20L 
#define FNC_DMA_TMODE_SIZE				2L 
#define FNC_DMA_INITMODE			22L 
#define FNC_DMA_INITMODE_SIZE			2L 
#define FNC_DMA_ADDRMODE			24L 
#define FNC_DMA_ADDRMODE_SIZE			2L 
#define FNC_DMA_MODE				26L 
#define FNC_DMA_MODE_SIZE				2L 

#define FNC_DMA_SPECIAL_INST_SIZE       28



#define FNC_MISC_DMA				12L 
#define FNC_MISC_DMA_SIZE			2L 



#define FNC_CM_ADDRESS              12L    
#define FNC_CM_ADDRESS_SIZE             4L      // PT_VALUE, PT_DEVICE  --> size 4L    
#define FNC_CM_DDSTATUS             16L    
#define FNC_CM_DDSTATUS_SIZE            2L      // PT_REF --> size of the object pointed to
#define FNC_CM_RBUFFER              20L 
#define FNC_CM_RBUFFER_SIZE             8L      // PT_BUFFER --> size 8L
#define FNC_CM_WBUFFER              28L 
#define FNC_CM_WBUFFER_SIZE             8L      // PT_BUFFER --> size 8L
#define FNC_CM_RLENGTH              36L 
#define FNC_CM_RLENGTH_SIZE             2L      // PT_REF --> size of the object pointed to
#define FNC_CM_WLENGTH              40L 
#define FNC_CM_WLENGTH_SIZE             2L      // PT_VALUE --> 2L
#define FNC_CM_TIMEOUT              44L    
#define FNC_CM_TIMEOUT_SIZE             4L      // PT_VALUE  --> size 4L    

#define FNC_PO_ADDRESS              12L    
#define FNC_PO_ADDRESS_SIZE             2L      // PT_VALUE --> 2L
#define FNC_PO_LENGTH               14L    
#define FNC_PO_LENGTH_SIZE              2L      // PT_VALUE --> 2L
#define FNC_PO_IN_VALUE             16L 
#define FNC_PO_IN_VALUE_SIZE            2L      // PT_REF --> size of the object pointed to
#define FNC_PO_OUT_VALUE            20L    
#define FNC_PO_OUT_VALUE_SIZE           2L      // PT_VALUE --> 2L

#define FNC_PO_GALIL_ADDRESS              12L    
#define FNC_PO_GALIL_ADDRESS_SIZE             2L      // PT_VALUE --> 2L
#define FNC_PO_GALIL_LENGTH               14L    
#define FNC_PO_GALIL_LENGTH_SIZE              2L      // PT_VALUE --> 2L
#define FNC_PO_GALIL_TX_VALUE             16L 
#define FNC_PO_GALIL_TX_VALUE_SIZE            4L      // PT_buffer --> size of the object pointed to
#define FNC_PO_GALIL_RX_VALUE             20L 
#define FNC_PO_GALIL_RX_VALUE_SIZE            4L      // PT_buffer --> size of the object pointed to

#define FNC_PO_GALIL_OUT_VALUE            24L    
#define FNC_PO_GALIL_OUT_VALUE_SIZE           2L      // PT_VALUE --> 2L

#define FNC_PO_GALIL_SPECIAL_INST_SIZE        28
#define FNC_HD_SPECIAL_INST_SIZE        12
// #define FNC_CM_SPECIAL_INST_SIZE        48
#define FNC_PO_SPECIAL_INST_SIZE        28     // 28,24
#define FNC_SPECIAL_INST_SIZE           108



#define FNC_PO_GALIL_ADDRESS              12L    
#define FNC_PO_GALIL_ADDRESS_SIZE          2L      // PT_VALUE --> 2L
#define FNC_PO_GALIL_VALUE				  14L 
#define FNC_PO_GALIL_VALUE_SIZE            4L      // PT_buffer --> size of the object pointed to


// NETCONFIG offsets & sizes
#define NET_ID                      0 
#define NET_ID_SIZE                     32 
#define NET_VERS                    4 
#define NET_VERS_SIZE                   32 
#define NET_DPADR                   8 
#define NET_DPADR_SIZE                  32 
#define NET_PORT                    12
#define NET_PORT_SIZE                   16 
#define NET_IRQ                     14
#define NET_IRQ_SIZE                    16 
#define NET_BAUDRATE                16 
#define NET_BAUDRATE_SIZE               16 
#define NET_STOPSTATE               18 
#define NET_STOPSTATE_SIZE              16 
#define NET_SIMULATE                20 
#define NET_SIMULATE_SIZE               16 
#define NET_WATCHDOG                22 
#define NET_WATCHDOG_SIZE               16 
#define NET_HWTEST                  24 
#define NET_HWTEST_SIZE                 16 
#define NET_INPUTREAD               26 
#define NET_INPUTREAD_SIZE              16 
#define NETCONFIG_SIZE              44 

// DEVICECONFIG offsets & sizes
#define DEV_DRVID                   0
#define DEV_DRVID_SIZE                  32
#define DEV_ID                      4
#define DEV_ID_SIZE                     32
#define DEV_VERS                    8
#define DEV_VERS_SIZE                   32
#define DEV_ADDRESS                 12 
#define DEV_ADDRESS_SIZE                16 
#define DEV_CRITICAL                14 
#define DEV_CRITICAL_SIZE               16 
#define DEV_PCODE                   16 
#define DEV_PCODE_SIZE                  64 
#define DEV_IO_ADDRESS              24 
#define DEV_IO_ADDRESS_SIZE            16 
#define DEV_IO_SWAP                 26 
#define DEV_IO_SWAP_SIZE                16 

#define DEVCONFIG_SIZE              32 


/////////////



// Dual port ram layout

#define  DPR_CONTROL_OFF        0
#define  DPR_CONTROL_SIZE       2048
#define  DPR_INPUT_OFF          2048    // where the input image can be found in the dpr
#define  DPR_INPUT_SIZE         1024    // 1kbyte =  MAX_DEVICES * 8bytes input devices
#define  DPR_OUTPUT_OFF         3072    // where the output image can be found in the dpr
//#define  DPR_OUTPUT_OFF       2048    // for didactic purposes use 2048 --> outputs will be looped back in inputs
#define  DPR_OUTPUT_SIZE        1024    // 1kbyte =  MAX_DEVICES * 8bytes input devices
#define  DPR_TOTAL_SIZE         4096    // 4 kbytes
#define  MAX_DEVICES            128     // max 128 devices allowed by our didactical network

#define  MIN_DEVICES            0L     // max 128 devices allowed by our didactical network
#define  STEP_DEVICES           1L     // max 128 devices allowed by our didactical network
#define  DEF_DEVICES            0L     // max 128 devices allowed by our didactical network

#define  DPADR_MIN              0x80000L
#define  DPADR_MAX              0xFF000L
#define  DPADR_STEP             0x01000L    // increments
#define  DPADR_DEFAULT          0x80000L

#define  PORT_MIN               0x100
#define  PORT_MAX               0x3ff
#define  PORT_STEP              1           // 4 bytes increments
#define  PORT_DEFAULT           824

#define  NO_IRQ                 0L
#define  HWTEST_RW              1
#define  HWTEST_OFF             0
#define  INPUT_READ_COS         1
#define  INPUT_READ_CYCLIC      0

#define  BAUDRATE_125           1
#define  BAUDRATE_250           2
#define  BAUDRATE_500           3

#define  DEVICE_4W_INPUT            10      // 64bits input
#define  DEVICE_4W_OUTPUT           11      // 64bits output
// NOT USED
#define  DEVICE_4W_IORO             13      // 64bits input or  64bits output
#define  DEVICE_4W_IANDO            12      // 64bits input and 64bits output
// 
#define  DEVICE_2W_INPUT            14      // 32bits input
#define  DEVICE_2W_OUTPUT           15      // 32bits output
#define  DEVICE_1W_INPUT            16      // 16bits input
#define  DEVICE_1W_OUTPUT           17      // 16bits output
#define  DEVICE_1B_INPUT            18      // 8bits input
#define  DEVICE_1B_OUTPUT           19      // 8bits output

#define  DEVICE_4W_INPUT_G            20      // 64bits input
#define  DEVICE_4W_OUTPUT_G           21      // 64bits output
#define  DEVICE_2W_INPUT_G            24      // 32bits input
#define  DEVICE_2W_OUTPUT_G           25      // 32bits output
#define  DEVICE_1W_INPUT_G            26      // 16bits input
#define  DEVICE_1W_OUTPUT_G           27      // 16bits output
#define  DEVICE_1B_INPUT_G            28      // 8bits input
#define  DEVICE_1B_OUTPUT_G           29      // 8bits output


#define  DRIVER_FUNC                2000    // special driver functions ids
#define  DRVF_GET_DRVSTAT           2100    // functions at driver level
#define  DRVF_GET_DEVSTAT           2101    
#define  DRVF_COMMAND               2102    
#define  DRVF_PORT_INPUT            2103    
#define  DRVF_PORT_OUTPUT           2104    
#define  DRVF_PEEK                  2105    
#define  DRVF_POKE                  2106    

#define  DEVICE_FUNC                2010    // special device functions ids
#define  DEVF_GET_DEVSTAT           2200    // functions at device level

#define  MAX_LENGTH                 400




//////////////////new///////////////////////////

// ADDED FOR MORE FUNCTION    
#define  DRVF_PORT_START            2108    
#define  DRVF_PORT_STOP             2109   
#define  DRVF_PORT_COMMAND          2111    
#define  DRVF_PORT_GALIL            2112
#define  DRVF_PORT_TX         2115
#define  DRVF_PORT_RX         2116
// #define  DRVF_PORT_GALIL_BK         2115

#endif       // __DRIVER_H__ 

/*

    DRVF_PORT_INPUT+LONG_PAD,           // UINT32  Function id, Add 0L to ensure UINT32
        "Port Input\0",                 // short name 
        "Synchronous special call. Reads an input port. Result codes:\r\n"
        "0: Function completed.\r\n"
        "2: Invalid port address.\r\n"
        "\0",
        0L, 0L, 0L, 0L,                 // Expansion 
        IDD_DRVF_PORT_INPUT,            // DlgId     
        IDC_PB_HELP,                    // HelpId    
        IDH_DRVF_PORT_INPUT+LONG_PAD,	// HelpIdx   
        FNC_SPECIAL_INST_SIZE,          // TotalSize 
        NET_PARAM_TYPE, DRVF_PORT_INPUT, // p.#8      
    DRIVER_SENTENIAL

    DRVF_PORT_OUTPUT+LONG_PAD,          // UINT32  Function id, Add 0L to ensure UINT32
        "Port Output\0",                // short name 
        "Synchronous special call. Writes to an output port. Result codes:\r\n"
        "0: Function completed.\r\n"
        "2: Invalid port address.\r\n"
        "\0",
        0L, 0L, 0L, 0L,                 // Expansion 
        IDD_DRVF_PORT_OUTPUT,           // DlgId     
        IDC_PB_HELP,                    // HelpId    
        IDH_DRVF_PORT_OUTPUT+LONG_PAD,	// HelpIdx   
        FNC_SPECIAL_INST_SIZE,          // TotalSize 
        NET_PARAM_TYPE, DRVF_PORT_OUTPUT, // p.#8      
    DRIVER_SENTENIAL

	*/