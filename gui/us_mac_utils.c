/*
 * This set of utility routines handles IPC on MacOS systems.
 * They are based on the x11win.c source in RasMol.
 *
 * The primary public functions it provides are:
 *
 *   ShowInterpNames( ... )   - get the list of interpreters
 *   SendCommand( ... )       - send a command to an interpreter
 *   SendQuery( ... )         - send a command and get the response
 *   GetLastError( ... )      - get last IPC error text
 *   GetErrorCode( ... )      - get last IPC error code
 *
 * +++ Presently, the Mac version is simply a copy of the X11
 *     version, using X11 protocols on MacOS +++
 *
 */

#include "us_mac_utils.h"
#include "us_x11_utils.c"
