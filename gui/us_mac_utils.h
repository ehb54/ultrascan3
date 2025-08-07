#ifndef MAC_UTILS_H
#define MAC_UTILS_H

#ifndef ulong
#define ulong unsigned long int
#endif

char* GetLastError(void);
int GetErrorCode(void);
int ShowInterpNames(char*, int);
int SendCommand(ulong, char*, char*);
int SendQuery(ulong, char*, char*, char*, int);

#endif
