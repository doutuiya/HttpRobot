#include <Windows.h>
#include "common.h"
#include "offset.h"
#include "resource.h"
#include "InlineHook.h"

/**
 * ����QQ��50728123
 * ����Ⱥ��810420984
 * ����΢�ţ�codeByDog
 * ��ȡ΢�ŵ�¼״̬
 * @param
 * @return int
 */
int isLogin()
{
    return (int)*(int*)(getWeChatWinAddr() + ISLOGIN);
}

void LogoutWeChat()
{
    HWND dbSelect = GetDlgItem(getGlobalHwnd(), DATABASE_SELECT);
    for (auto& db : getDbHandleList())
    {
        SendMessage(dbSelect, CB_DELETESTRING, 0, (LPARAM)db.path);
    }
    clearDbHandleList();
    DWORD dwCallAddress = getWeChatWinAddr() + LOGOUT;
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)dwCallAddress, 0, NULL, 0);
    if (hThread != 0)
    {
        CloseHandle(hThread);
    }
}

VOID GotoQrCode()
{
    DWORD dwCallAddr1 = getWeChatWinAddr() + GOTOQRCODE1;
    DWORD dwCallAddr2 = getWeChatWinAddr() + GOTOQRCODE2;

    __asm {
        call dwCallAddr1;
        mov ecx, eax;
        call dwCallAddr2;
    }

}

/**
 * ����QQ��50728123
 * ����Ⱥ��810420984
 * ����΢�ţ�codeByDog
 * �̳߳�����ȡ��¼״̬
 * @param  HWND hwnd
 * @return void
 */
void getLoginStatus()
{
    if (isLogin() == 0)
    {
        char qrcodeStr[500] = { 0 };
        sprintf_s(qrcodeStr, "��ά���ַ��http://weixin.qq.com/x/%s", (char*)*((DWORD*)(getWeChatWinAddr() + LOGINQRCODESTR)));
        SetDlgItemTextA(getGlobalHwnd(), MY_INFO_TEXT, qrcodeStr);
        SetDlgItemText(getGlobalHwnd(), LOGIN_STATUS, L"δ��¼");
    }
    else
    {
        SetDlgItemText(getGlobalHwnd(), LOGIN_STATUS, L"�ѵ�¼");
    }
    Sleep(100);
    HANDLE lThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)getLoginStatus, NULL, NULL, 0);
    if (lThread != 0) {
        CloseHandle(lThread);
    }
}