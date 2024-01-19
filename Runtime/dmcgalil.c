//----------------------------------------------------
//
// Filename DMCGALIL.CPP
//
// 	Author :  Ronnie Lim
// 	Date   :  April 2000
//
//----------------------------------------------------

#include <string.h>

#include "dmcgalil.h"

unsigned char dmcInportb (unsigned portid);
void dmcOutportb(unsigned portid, unsigned char value);

int CharAvailable( char *ch );
int CharReceive( char *ch );
int CharSend( char ch);

int address=824;
char command[120];
char response_string[120];

/*
void Interactive()
{
	char str[80];

	printf("\n\nEnter 'EXIT' to quit");
	while(1)
	{
		printf("\nDmc>");
		gets(str);
		for(int i=0; str[i]; i++)
			str[i] = toupper(str[i]);

		if(strcmp(str, "EXIT")==0)
		{
			printf("\n\nBack to main program\n\n");
			SendReceive("SH");
			return;
		}

		if(SendReceive(str)==COMMAND_OK)
			printf(GetResponse());
	}
}
*/

void SetAddress(int addr)
{
	address=addr;
}

void ClearBuffer()
{
	char ch; 		// clear buffer
	while(CharAvailable(&ch)==COMMAND_OK)
		;
		
}

//***********************************
//	check for character availability
//***********************************
int CheckResponse()
{
#define RX_READY			32		// bit 5

	// is there a byte waiting for us?
	if ( (dmcInportb(address+1) & RX_READY) == 0)
		return COMMAND_OK;
	else
		return COMMAND_NOT_READY;
}



//***********************************
//	check for character availability
//***********************************
int CharAvailable(char *ch)
{
#define RX_READY			32		// bit 5

	*ch = 0;
	// is there a byte waiting for us?
	if ( (dmcInportb(address+1) & RX_READY) == 0)
	{
		*ch = dmcInportb(address);
		return COMMAND_OK;
	}
	else
		return COMMAND_TIMEOUT;
}


//***********************************
//	A character received from DMC
//***********************************
int CharReceive(char *ch)
{
	long timeout = 20000L;
	while (--timeout > 0)
		if(CharAvailable(ch)==COMMAND_OK)
			return COMMAND_OK;

	return COMMAND_TIMEOUT;
}

//***********************************
//	Send a character to DMC
//***********************************
int CharSend(char ch)
{
#define TX_READY			16		// bit 4

	long	timeout = 20000L;

	while((dmcInportb(address+1) & TX_READY)!= 0)
		if(--timeout < 0)
			return COMMAND_TIMEOUT;

	dmcOutportb( address, ch);	// send the byte

	return COMMAND_OK;
}

//***********************************
//	Send a string to DMC
//***********************************
int SendString(char *str)
{
	int i;
	strcpy(command, str);

	for(i=0; command[i]; i++)
	{
		if( CharSend(command[i]) == COMMAND_TIMEOUT)
			return COMMAND_TIMEOUT;
	}

	// send a CR to simulate RETURN key
	if(str[strlen(command)-1]!='\r')
		if(CharSend('\r')==COMMAND_TIMEOUT)
			return COMMAND_TIMEOUT;

	return COMMAND_OK;
}

int SendReceive(char *str)
{
	int result;
	SendString(str);

	result = WaitForResponse();
	if(result == COMMAND_ERROR)
	{
		// the reason is not to use the SendReceive function recursively
		CharSend('T');
		CharSend('C');
		CharSend('1');
		CharSend('\r');

		WaitForResponse();
		return COMMAND_ERROR;
	}

	return COMMAND_OK;
}

char * GetResponse()
{
	return response_string;
}

//***********************************
//	Receive response from DMC
//***********************************
int WaitForResponse()
{
	char ch;
	int i=0;
	int result;
	response_string[0]='\0';

	do
	{
		result = CharReceive(&ch);
		if(result == COMMAND_OK)
		{
			if(ch=='?')
				return COMMAND_ERROR;
			else if(ch!=':')
				response_string[i++]=ch;
		}
		else
			return result;

	}while(ch!=':');
	response_string[i]='\0';
	return COMMAND_OK;
}


unsigned char dmcInportb (unsigned portid)
{
	return inbyte (portid);
}

void dmcOutportb(unsigned portid, unsigned char value)
{
	outbyte(portid, value);
}
