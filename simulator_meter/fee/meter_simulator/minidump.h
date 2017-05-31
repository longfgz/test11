#ifndef MINIDUMP_H
#define MINIDUMP_H

#ifdef WIN32

#include <windows.h>

#include <imagehlp.h>

#include <stdlib.h>

#include <tchar.h>

#pragma comment(lib, "dbghelp.lib")



inline BOOL IsDataSectionNeeded(const WCHAR* pModuleName)
{

	if(pModuleName == 0)
	{

		return FALSE;

	}



	WCHAR szFileName[_MAX_FNAME] = L"";

	_wsplitpath(pModuleName, NULL, NULL, szFileName, NULL);

	if(_wcsicmp(szFileName, L"ntdll") == 0)

		return TRUE;



	return FALSE; 

}



inline BOOL CALLBACK MiniDumpCallback(PVOID                            pParam, 
									  const PMINIDUMP_CALLBACK_INPUT   pInput, 
									  PMINIDUMP_CALLBACK_OUTPUT        pOutput)
{

	if(pInput == 0 || pOutput == 0)

		return FALSE;



	switch(pInput->CallbackType)

	{

	case ModuleCallback: 

		if(pOutput->ModuleWriteFlags & ModuleWriteDataSeg) 



			if(!IsDataSectionNeeded(pInput->Module.FullPath)) 



				pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg); 



	case IncludeModuleCallback:

	case IncludeThreadCallback:

	case ThreadCallback:

	case ThreadExCallback:

		return TRUE;



	default:;

	}



	return FALSE;

}



//创建Dump文件

inline void CreateMiniDump(EXCEPTION_POINTERS* pep, LPCTSTR strFileName)
{

	HANDLE hFile = CreateFile(strFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))

	{

		MINIDUMP_EXCEPTION_INFORMATION mdei;

		mdei.ThreadId           = GetCurrentThreadId();

		mdei.ExceptionPointers  = pep;

		mdei.ClientPointers     = FALSE;

		MINIDUMP_CALLBACK_INFORMATION mci;

		mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;

		mci.CallbackParam       = 0;

		MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)0x0000ffff;

		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &mdei, NULL, &mci);



		CloseHandle(hFile); 

	}

}



LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{

	return NULL;

}

void WriteJump(void* pAddress, ULONG_PTR JumpTo)  
{  
	DWORD dwOldProtect;  
	VirtualProtect(pAddress, 14, PAGE_READWRITE, &dwOldProtect);  

	BYTE *pDst = (BYTE *)pAddress;  

	ULONG_PTR dis = std::max(JumpTo, (ULONG_PTR)pAddress) - std::min(JumpTo, (ULONG_PTR)pAddress);  
	if (dis <= (ULONG_PTR) 0x7FFF0000)  
	{  
		*(pDst++) = 0xE9;  
		DWORD dwRelAddr = (DWORD)(JumpTo - (ULONG_PTR)pAddress) - 5;  
		memcpy(pDst, &dwRelAddr, sizeof(DWORD));  
	}  
	else  
	{  
#ifndef _WIN64   
		*(pDst++) = 0xFF;  
		*(pDst++) = 0x25;  
		*((DWORD *)pDst) = (DWORD)(((ULONG_PTR)pDst) + sizeof(DWORD));  
		pDst += sizeof(DWORD);  
		*((ULONG_PTR *)pDst) = JumpTo;  
#else
		*(pDst++) = 0xFF;  
		*(pDst++) = 0x25;  
		*((DWORD*)pDst) = 0;  
		pDst += sizeof(DWORD);  
		*((ULONG_PTR *)pDst) = JumpTo;  
#endif   
	}  

	DWORD dwNotUsed;  
	VirtualProtect(pAddress, 14, dwOldProtect, &dwNotUsed);  
} 

BOOL PreventSetUnhandledExceptionFilter()
{
	HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
	if (hKernel32 ==   NULL)
		return FALSE;


	void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
	if(pOrgEntry == NULL)
		return FALSE;

	WriteJump(pOrgEntry, (ULONG_PTR)MyDummySetUnhandledExceptionFilter);
	return TRUE;
}





LONG WINAPI UnhandledExceptionFilterEx(struct _EXCEPTION_POINTERS *pException)
{

	TCHAR szMbsFile[MAX_PATH] = { 0 };

	::GetModuleFileName(NULL, szMbsFile, MAX_PATH);

	TCHAR* pFind = _tcsrchr(szMbsFile, '\\');

	if(pFind)

	{

		*(pFind+1) = 0;

		_tcscat(szMbsFile, _T("CreateMiniDump.dmp"));

		CreateMiniDump(pException,szMbsFile);

	}





	// TODO: MiniDumpWriteDump

	FatalAppExit(-1,  _T("Fatal Error"));

	return EXCEPTION_CONTINUE_SEARCH;

}

#endif//WIN32

//运行异常处理

void RunCrashHandler()
{
#ifdef WIN32
	SetUnhandledExceptionFilter(UnhandledExceptionFilterEx);

	PreventSetUnhandledExceptionFilter();
#endif
}

#endif//MINIDUMP_H