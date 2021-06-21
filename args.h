#ifdef WIN32
#include <windows.h>
#include <shellapi.h>
#include <processenv.h>

PCHAR* CommandLineToArgvA(PCHAR CmdLine, int* _argc);
#endif
