#pragma once
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <TlHelp32.h>
#include <direct.h>
#include <fstream>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"
#include "RobotService.h"
#include "json/json.h"
#include "HttpServer.h"
using namespace std;

INT_PTR CALLBACK wechatRobot(
	HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
);
VOID startWechat(HWND hDlg);
BOOL CloseWeChat();
VOID startWechatExe(wchar_t* weChatexe, HWND hDlg);
void InitServer();

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	Json::Value root;

	//���������壬��ֹ������࿪
	HANDLE hThread = ::CreateMutex(NULL, FALSE, L"_CodeByDog_Wechat_");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		if (hThread != 0)
		{
			CloseHandle(hThread);
		}
		return 0;
	}
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)InitServer, NULL, 0, NULL);
	//��Ⱦ����
	DialogBox(hInstance, MAKEINTRESOURCE(ROBOT_MAIN), NULL, &wechatRobot);
	return 0;
}

void InitServer()
{
	auto http_server = shared_ptr<HttpServer>(new HttpServer);
	http_server->Init("7999");
	http_server->AddHandler("/api/start_wx", RobotService::StartWx);
	http_server->AddHandler("/api/close_wx", RobotService::CloseAllWx);
	http_server->Start();
}


INT_PTR CALLBACK wechatRobot(
	HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
	{
		SetDlgItemTextA(hDlg, WECHAT_PATH, "���ջ�����ǰֵ����ע����в�ѯ΢�Ű�װ·��");
		char WorkPath[0x1000] = { 0 };
		sprintf_s(WorkPath, "%s\\*.dll", _getcwd(NULL, 0));
		long handle;
		struct _finddata_t fileinfo;
		handle = _findfirst(WorkPath, &fileinfo);
		if (handle != -1)
		{
			do
			{
				HWND dllSelect = GetDlgItem(hDlg, INJECT_DLL);
				SendMessageA(dllSelect, CB_ADDSTRING, 0, (LPARAM)fileinfo.name);
				SendMessageA(dllSelect, CB_SELECTSTRING, 0, (LPARAM)fileinfo.name);

			} while (!_findnext(handle, &fileinfo));
			_findclose(handle);
		}
		return (INT_PTR)TRUE;
	}

    case WM_COMMAND:
	{
		switch (wParam)
		{
		case START_WECHAT:
			startWechat(hDlg);
			break;
		case CLOSE_WECHAT:
			CloseWeChat();
			break;
		default:
			break;
		}
		break;
	}
    case WM_CLOSE:
        EndDialog(hDlg, 0);
        break;
    }
    return (INT_PTR)FALSE;
}


VOID startWechat(HWND hDlg)
{
	wchar_t exePath[0x500] = { 0 };
	GetDlgItemText(hDlg, WECHAT_PATH, exePath, 500);
	if (_waccess(exePath, 0) != -1)
	{
		startWechatExe(exePath, hDlg);
		return;
	}

	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Tencent\\WeChat", 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "ע����ʧ��", "����", 0);
		return;
	}
	TCHAR szProductType[MAX_PATH] = {};
	DWORD dwBufLen = MAX_PATH;
	if (RegQueryValueEx(hKey, L"InstallPath", NULL, NULL, (LPBYTE)szProductType, &dwBufLen) != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "ע����ѯʧ��", "����", 0);
		return;
	}

	RegCloseKey(hKey);
	wchar_t weChatexe[0x500] = { 0 };
	swprintf_s(weChatexe, L"%s\\WeChat.exe", (wchar_t*)szProductType);
	startWechatExe(weChatexe, hDlg);
	return;
}

VOID startWechatExe(wchar_t* weChatexe,HWND hDlg)
{
	char DllName[0x500] = { 0 };
	GetDlgItemTextA(hDlg, INJECT_DLL, DllName, 500);
	char DllPath[0x500] = { 0 };
	sprintf_s(DllPath, "%s\\%s", _getcwd(NULL, 0), DllName);
	if (_access(DllPath, 0) == -1)
	{
		MessageBoxA(NULL, DllPath, "DLL������", 0);
		return;
	}

	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;//SW_SHOW
	CreateProcess(NULL, weChatexe, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);

	LPVOID Param = VirtualAllocEx(pi.hProcess, NULL, MAX_PATH, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (Param == 0)
	{
		MessageBoxA(NULL, "VirtualAllocExʧ��", "����", 0);
		return;
	}
	WriteProcessMemory(pi.hProcess, Param, DllPath, MAX_PATH, NULL);

	FARPROC address = GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "LoadLibraryA");
	HANDLE hRemote = CreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)address, Param, 0, NULL);
	ResumeThread(pi.hThread);
}

BOOL CloseWeChat()
{
	HANDLE ProcesssAll = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32 proessInfo = { 0 };
	proessInfo.dwSize = sizeof(PROCESSENTRY32);
	do
	{
		if (wcscmp(L"WeChat.exe", proessInfo.szExeFile) == 0) {
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, proessInfo.th32ProcessID);
			if (hProcess != NULL) {
				TerminateProcess(hProcess, 0);
			}
		}
	} while (Process32Next(ProcesssAll, &proessInfo));
	return TRUE;
}