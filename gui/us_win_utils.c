#include "us_win_utils.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

char* LastErrorMsg(void) {
  char* bufPtr = NULL;
  DWORD err = GetLastError();
  FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, err, 0, (LPSTR)&bufPtr, 0, NULL);

  return bufPtr;
}
int GetErrorCode(void) {
  DWORD err = GetLastError();
  return (int)err;
}
int ShowInterpNames(char* inames, int mxchar) {
  int stat = (mxchar >= 0) ? 0 : 1;
  inames[0] = '\0';
  return stat;
}
int SendCommand(unsigned long int iid, char* interp, char* command) {
  int stat = (iid > 0 && interp != NULL && command != NULL) ? 0 : 1;
  return stat;
}
int SendQuery(unsigned long int iid, char* interp, char* command, char* retval,
              int mxch) {
  int stat = (iid > 0 && interp != NULL && command != NULL) ? 0 : 1;
  stat = (retval != NULL && mxch > 0) ? stat : 2;
  return stat;
}
