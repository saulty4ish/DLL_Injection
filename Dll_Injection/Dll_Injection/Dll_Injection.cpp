// Dll_Injection.cpp : �������̨Ӧ�ó������ڵ㡣
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

DWORD getid(char * processname)         //���ݽ�������ȡpid,���ô�����̿��գ��������̵ķ���/
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
	//��Ҫע��DLL�ļ�·������
	int nDll = lstrlenA(dllname)+sizeof(char); 
	//����һ��dll·�����ȵĴ���ռ�
	PVOID pDLLADR = VirtualAllocEx(hWnd, NULL, nDll, MEM_COMMIT, PAGE_READWRITE); 
	if(pDLLADR == NULL)
	{
		CloseHandle(hWnd);
		return;
	}
	DWORD writenum =0;
	//����ע��dll�ļ�������·��д��Ŀ���ڴ�
	WriteProcessMemory(hWnd,pDLLADR,dllname,nDll,&writenum); 
	//���LoadLibraryA�����ĵ�ַ
	FARPROC pFunAddr = GetProcAddress(GetModuleHandleA("kernel32.dll"),pFunname);
	//����Զ���߳�
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
	cout << "pidΪ��"<<endl;
	wcout << pid<<endl;
	InjectDll(pid,"dll2.dll");
	system("pause");
}