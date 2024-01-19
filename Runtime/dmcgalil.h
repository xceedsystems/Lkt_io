void Interactive();
int SendReceive( char str[] );
int SendString(char *str);
char * GetResponse();
int WaitForResponse();
void ClearBuffer();
void SetAddress(int addr);
int CheckResponse();

#define COMMAND_OK			-1
#define COMMAND_ERROR		-2
#define COMMAND_NOT_READY	-3
#define COMMAND_TIMEOUT		-4
