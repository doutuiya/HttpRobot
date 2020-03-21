#include "RobotService.h"
#include <TlHelp32.h>
#include "resource.h"
#include "common.h"
#include "json/json.h"
string startWechatExe(wchar_t* weChatexe);

string http_ok(string msg = "") {
	Json::Value root;
	root["code"] = 0;
	if (msg.empty() == false) {
		root["data"] = msg;
	}
	return root.toStyledString();
}

string http_error(int code = -1, string msg = "") {
	Json::Value root;
	root["code"] = code;
	if (msg.empty() == false) {
		root["data"] = msg;
	}
	return root.toStyledString();
}

string RobotService::CloseAllWx(http_message* http_message) {
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
	return http_ok();
}

string RobotService::StartWx(http_message* http_message) {
	//TODO::�ж��Ƿ��Ѿ���΢��
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Tencent\\WeChat", 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
	{
		return http_error(-1001,"ע����ʧ��");
	}
	TCHAR szProductType[MAX_PATH] = {};
	DWORD dwBufLen = MAX_PATH;
	if (RegQueryValueEx(hKey, L"InstallPath", NULL, NULL, (LPBYTE)szProductType, &dwBufLen) != ERROR_SUCCESS)
	{
		return http_error(-1002, "ע����ѯ��");
	}
	RegCloseKey(hKey);
	wchar_t weChatexe[0x500] = { 0 };
	swprintf_s(weChatexe, L"%s\\WeChat.exe", (wchar_t*)szProductType);
	return startWechatExe(weChatexe);
}

string startWechatExe(wchar_t* weChatexe)
{
	wchar_t* path = new wchar_t(MAX_PATH);
	GetCurrentDirectory(MAX_PATH, path);
	string pathStr = UnicodeToUtf8(path);
	pathStr += "\\helper.dll";
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
		return http_error(-1003, "VirtualAllocExʧ��"); 
	}
	WriteProcessMemory(pi.hProcess, Param, pathStr.c_str(), MAX_PATH, NULL);
	FARPROC address = GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "LoadLibraryA");
	HANDLE hRemote = CreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)address, Param, 0, NULL);
	ResumeThread(pi.hThread);
	return http_ok();
}