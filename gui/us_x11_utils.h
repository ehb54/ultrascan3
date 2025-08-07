#ifndef X11_UTILS_H
#define X11_UTILS_H

char* GetLastError(void);
int GetErrorCode(void);
int ShowInterpNames(char*, int);
int SendCommand(ulong, char*, char*);
int SendQuery(ulong, char*, char*, char*, int);

#endif
