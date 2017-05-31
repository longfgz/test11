#include "meter_simulator_server.h"
#include <util/public.h>
#include "minidump.h"

meter_simulator_server server_;

#if defined(WIN32)
#include <windows.h>
BOOL WINAPI cmd_handler(DWORD ctrl_type)
{
	switch (ctrl_type)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		{
			server_.exit();
			return TRUE;
		}
		break;
	default:
		return FALSE;
	}
}
#endif

#ifdef NDEBUG 
#pragma   comment(linker, "/subsystem:windows /entry:mainCRTStartup" )
#endif

int main(int argc, char *argv[])
{
	RunCrashHandler();
#if defined(WIN32)
	char   TheTitle[255];
	GetConsoleTitle(TheTitle,255);
	HWND hWnd = FindWindow(NULL,TheTitle);
	if (hWnd)
	{
		HMENU hMenu = GetSystemMenu(hWnd,FALSE);
#if defined (NDEBUG)
		DeleteMenu(hMenu,SC_CLOSE,MF_BYCOMMAND);
#endif;
	}
	strcat(TheTitle,VERSION);
	//strcat(TheTitle,"(请使用CTRL+C组合键关闭程序)");
	SetConsoleTitle(TheTitle);
	SetConsoleCtrlHandler(cmd_handler, TRUE);
#endif


	server_.run();

	return 0;
}
