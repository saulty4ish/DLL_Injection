// Dll_Injection.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "windows.h"
#include <iostream>
#include <cstdio>
#include <tlhelp32.h>
#include <tchar.h>
using namespace std;
wchar_t * char_to_wchar(char a[])
{
	char *CStr = a;
	size_t len = strlen(CStr) + 1;
	size_t converted = 0;
	wchar_t *WStr;
	WStr = (wchar_t*)malloc(len * sizeof(wchar_t));
	mbstowcs_s(&converted, WStr, len, CStr, _TRUNCATE);
	return WStr;
}

DWORD getid(char * processname)         //根据进程名获取pid,采用创造进程快照，遍历进程的方法/
{
	wchar_t * name = char_to_wchar(processname);
	BOOL bRet;
	PROCESSENTRY32 pe32;
	HANDLE hWnd;
	hWnd = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	pe32.dwSize = sizeof(pe32);
	bRet = Process32First(hWnd, &pe32);
	
	while (bRet)
	{
		if(lstrcmp(pe32.szExeFile,name)==0)
		{
			return pe32.th32ProcessID;
		}
		bRet = Process32Next(hWnd, &pe32);
	}
	return 0;
}

void InjectDll(DWORD pid, char * dllname)
{
	if (pid == 0 || lstrlenA(dllname) == 0)
	{
		return;
	}
	char * pFunname = "LoadLibraryA";
	HANDLE hWnd = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hWnd == NULL)
	{
		return;
	}
	//想要注入DLL文件路径长度
	int nDll = lstrlenA(dllname)+sizeof(char); 
	//开辟一块dll路径长度的储存空间
	PVOID pDLLADR = VirtualAllocEx(hWnd, NULL, nDll, MEM_COMMIT, PAGE_READWRITE); 
	if(pDLLADR == NULL)
	{
		CloseHandle(hWnd);
		return;
	}
	DWORD writenum =0;
	//将欲注入dll文件的完整路径写入目标内存
	WriteProcessMemory(hWnd,pDLLADR,dllname,nDll,&writenum); 
	//获得LoadLibraryA函数的地址
	FARPROC pFunAddr = GetProcAddress(GetModuleHandleA("kernel32.dll"),pFunname);
	//创建远程线程
	HANDLE hThread = CreateRemoteThread(hWnd, NULL, 0, (LPTHREAD_START_ROUTINE)pFunAddr, pDLLADR, 0, NULL);
	if (hThread == INVALID_HANDLE_VALUE)
	{
		cout << "Fail to create remote thread." << endl;
	}
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	CloseHandle(hWnd);
}


int main()
{
	DWORD pid=getid("notepad.exe");
	cout << "pid为："<<endl;
	wcout << pid<<endl;
	InjectDll(pid,"dll2.dll");
	system("pause");
}