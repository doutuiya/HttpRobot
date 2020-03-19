#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "resource.h"
#include "userInfo.h"
#include "Login.h"
#include "InlineHook.h"
#include "common.h"
#include <CommCtrl.h>
#include "SendMsg.h"
#include <Shlwapi.h>
#include "Sql.h"
#include "AiFun.h"
#include "chatroom.h"
#pragma comment(lib,"Shlwapi.lib")
using namespace std;

//----------���� Start----------
BOOL ShowDialog(HMODULE hModule);
INT_PTR CALLBACK DialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

//----------���� Over----------

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        HANDLE lThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShowDialog, hModule, NULL, 0);
        if (lThread != 0) {
            CloseHandle(lThread);
        }
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

/**
 * ����QQ��50728123
 * ����Ⱥ��810420984
 * ����΢�ţ�codeByDog
 * �߳��������ڣ���ֹ΢�ſ���
 * @param _In_ HMODULE hModule
 * @return DWORD
 */
BOOL ShowDialog(HMODULE hModule)
{
    DialogBox(hModule, MAKEINTRESOURCE(DEBUG_MAIN), NULL, &DialogProc);
    return TRUE;
}

/**
 * ����QQ��50728123
 * ����Ⱥ��810420984
 * ����΢�ţ�codeByDog
 * ��ʾ����
 * @param _In_ HWND   hwndDlg,_In_ UINT   uMsg,_In_ WPARAM wParam,_In_ LPARAM lParam
 * @return INT_PTR
 */

INT_PTR CALLBACK DialogProc(
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
        setGlobalHwnd(hDlg);
        SetDlgItemText(hDlg, SQL_TEXT, L"select * from sqlite_master");
        SetDlgItemText(hDlg, SQL_RESULT, L"hookд��ɹ����ѿ�ʼ�������ݿ���");
        SetDlgItemText(hDlg, RECEIVE_WXID_TEXT, L"filehelper");
        //��ʼ����Ϣ����list
        LV_COLUMN msgPcol = { 0 };
        LPCWSTR msgTitle[] = { L"����",L"self",L"��Դ",L"������", L"����" };
        int msgCx[] = { 40,40,80,80,200 };
        msgPcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
        msgPcol.fmt = LVCFMT_LEFT;
        for (unsigned int i = 0; i < size(msgTitle); i++) {
            msgPcol.pszText = (LPWSTR)msgTitle[i];
            msgPcol.cx = msgCx[i];
            ListView_InsertColumn(GetDlgItem(hDlg, RECIEVE_MSG_LIST), i, &msgPcol);
        }
        LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(RECIEVE_MSG_LIST);

        //��ʼ�������б�list
        LV_COLUMN friendPcol = { 0 };
        LPCWSTR friendTitle[] = { L"wxid",L"�˺�",L"�ǳ�",L"��ע",L"ͷ��" };
        int friendCx[] = { 80,80,80,80,80 };
        friendPcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
        friendPcol.fmt = LVCFMT_LEFT;
        for (unsigned int i = 0; i < size(friendTitle); i++) {
            friendPcol.pszText = (LPWSTR)friendTitle[i];
            friendPcol.cx = friendCx[i];
            ListView_InsertColumn(GetDlgItem(hDlg, FRIEND_LIST), i, &friendPcol);
        }

        HANDLE lThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)getLoginStatus, NULL, NULL, 0);
        if (lThread != 0) {
            CloseHandle(lThread);
        }

        HANDLE hookThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)inLineHook, NULL, NULL, 0);
        if (hookThread != 0) {
            CloseHandle(hookThread);
        }
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
    {
        wchar_t RecverWxid[0x500] = { 0 };
        GetDlgItemText(hDlg, RECEIVE_WXID_TEXT, RecverWxid, 500);
        wchar_t RecverContent[0x2000] = { 0 };
        GetDlgItemText(hDlg, RECEIVE_CONTENT_TEXT, RecverContent, 2000);
        switch (wParam)
        {
        case GOTO_QRCODE:
        {
            GotoQrCode();
            break;
        }
        case GET_MY_INFO://��ȡ������Ϣ
        {
            Information* myInfo = getMyInfo();
            wchar_t str[0x1000] = { 0 };
            swprintf_s(str,
                L"΢��ID��%s\r\n�˺ţ�%s\r\n�ǳƣ�%s\r\n�豸��%s\r\n�ֻ��ţ�%s\r\n���䣺%s\r\n�Ա�%d\r\n������%s\r\nʡ�ݣ�%s\r\n���У�%s\r\nǩ����%s\r\nͷ��%s",
                myInfo->wxid,
                myInfo->account,
                myInfo->nickname,
                myInfo->device,
                myInfo->phone,
                myInfo->email,
                myInfo->sex,
                myInfo->nation,
                myInfo->province,
                myInfo->city,
                myInfo->signName,
                myInfo->bigHeader);
            SetDlgItemText(hDlg, MY_INFO_TEXT, str);
            break;
        }
        case GET_CONTACT_LIST://��ȡ��ϵ��
            getContactList();
            break;
        case RUN_SQL_BTN://ִ��SQL
        {
            SetDlgItemTextA(getGlobalHwnd(), SQL_RESULT, "���ݿ��ѯ��...");
            char* sqlErrmsg = NULL;
            char dbName[0x100] = { 0 };
            GetDlgItemTextA(hDlg, DATABASE_SELECT, dbName, 100);
            char sql[0x1000] = { 0 };
            GetDlgItemTextA(hDlg, SQL_TEXT, sql, 1000);
            runSql(dbName, sql, runSqlCallBack, sqlErrmsg);
            break;
        }
        case SEND_TEXT_BTN://�����ı�
        {
            SendTextMsg(RecverWxid, RecverContent);
            break;
        }
        case SEND_IMG_BTN://����ͼƬ
        {
            SendImageMsg(RecverWxid, RecverContent);
            break;
        }
        case SEND_ATTACH_BTN://���͸���
        {
            SendAttachMsg(RecverWxid, RecverContent);
            break;
        }
        case SEND_USERCARD_BTN://���ͺ�����Ƭ
        {
            //<?xml version="1.0"?><msg bigheadimgurl="http://wx.qlogo.cn/mmhead/ver_1/LiccciblITlIKGcj01r8wFVxqC0IdPPoicpHqFtCjAc2EJ2wyaX4kvhuKCuicGmX6ZaEZzUHiaiahoVquNOO2r1eHAdoJsMam0iatVJOV9X84J7Z3c/0" smallheadimgurl="http://wx.qlogo.cn/mmhead/ver_1/LiccciblITlIKGcj01r8wFVxqC0IdPPoicpHqFtCjAc2EJ2wyaX4kvhuKCuicGmX6ZaEZzUHiaiahoVquNOO2r1eHAdoJsMam0iatVJOV9X84J7Z3c/132" username="������д΢��id" nickname="111111" fullpy="" shortpy="" alias="" imagestatus="0" scene="17" province="" city="" sign="" sex="1" certflag="0" certinfo="" brandIconUrl="" brandHomeUrl="" brandSubscriptConfigUrl= "" brandFlags="0" regionCode="" />
            SendXmlCard(RecverWxid, RecverContent);
            break;
        }
        case SEND_URL_BTN://��������
        {
            wchar_t fromWxid[0x100];
           // swprintf_s(fromWxid, L"%s", L"΢��ID");
            SendXmlArticle(RecverWxid, fromWxid, RecverContent);
            break;
        }
        case ADD_NEW_FRIEND://��Ӻ���
            AddUserFromWxid(RecverWxid, RecverContent);
            break;
        case CLEAR_FANS://�޺����
            MessageBoxA(NULL,"������δ����","��ʾ",0);
            break;
        case LOGOUT_ACCOUNT://�˳���¼
            LogoutWeChat();
            break;
        case DELETE_FRIEND://ɾ������
            DeleteFriend(RecverWxid);
            break;
        case SEND_CHATROOM_AT_MSG://����Ⱥ@��Ϣ
        {
            wchar_t atWxid[0x100] = L"΢��ID";
            wchar_t nickname[0x100] = L"������Ĺ�";
            SendChatroomAtMsg(RecverWxid, atWxid, nickname, RecverContent);
            break;
        }
        case ADD_CHATROMM_USER://���Ⱥ��Ա
            AddChatroomMember(RecverWxid, RecverContent);
            break;
        case SET_CHATROOM_NAME://����Ⱥ������
            SetChatoomName(RecverWxid, RecverContent);
            break;
        case SET_CHATROOM_ANNOUNCEMENT://����Ⱥ����
            SetChatroomAnnouncement(RecverWxid, RecverContent);
            break;
        case DELETE_CHATROOM_USER://ɾ��Ⱥ��Ա
            DeleteChatroomMember(RecverWxid, RecverContent);
            break;
        case GET_CHATROOM_USER:
        {
            GetChatroomUser(RecverWxid);
            break;
        }
        case QUIT_CHATROOM://�˳�Ⱥ��
            QuitChatRoom(RecverWxid);
            break;
        case AUTO_COLLECT_MONEY://�Զ��տ�
        {
            char str[0x100] = { 0 };
            GetDlgItemTextA(hDlg, AUTO_COLLECT_MONEY, str, 100);
            SetDlgItemTextA(hDlg, AUTO_COLLECT_MONEY, StrStrA(str, "����") ? "�Զ��տ�" : "�ѿ����Զ��տ�");
            break;
        }
        case AUTO_AGREE_NEW_FRIEND_APPLY://�Զ�ͬ���������
        {
            char str[0x100] = { 0 };
            GetDlgItemTextA(hDlg, AUTO_AGREE_NEW_FRIEND_APPLY, str, 100);
            SetDlgItemTextA(hDlg, AUTO_AGREE_NEW_FRIEND_APPLY, StrStrA(str, "����") ? "�Զ�ͨ����������" : "�ѿ����Զ�ͨ����������");
            break;
        }
        case AUTO_DOWNLOAD_IMAGE://�Զ���������ͼƬ
        {
            char str[0x100] = { 0 };
            GetDlgItemTextA(hDlg, AUTO_DOWNLOAD_IMAGE, str, 100);
            SetDlgItemTextA(hDlg, AUTO_DOWNLOAD_IMAGE, StrStrA(str, "����") ? "��������ͼƬ" : "�ѿ�����������ͼƬ");
            break;
        }
        case AUTO_FOLLOW_GH://�Զ���ע���ں���Ƭ
        {
            char str[0x100] = { 0 };
            GetDlgItemTextA(hDlg, AUTO_FOLLOW_GH, str, 100);
            SetDlgItemTextA(hDlg, AUTO_FOLLOW_GH, StrStrA(str, "����") ? "��ע���ں���Ƭ" : "�ѿ�����ע���ں���Ƭ");
            break;
        }
        case AUTO_JOIN_GROUP://�Զ�ͬ��Ҫ����Ⱥ
        {
            char str[0x100] = { 0 };
            GetDlgItemTextA(hDlg, AUTO_JOIN_GROUP, str, 100);
            SetDlgItemTextA(hDlg, AUTO_JOIN_GROUP, StrStrA(str, "����") ? "ͬ���Ⱥ����" : "�ѿ����Զ���Ⱥ����");
            break;
        }
        case AUTO_ADD_USER_CARD://�Զ�ͬ��Ҫ����Ⱥ
        {
            char str[0x100] = { 0 };
            GetDlgItemTextA(hDlg, AUTO_ADD_USER_CARD, str, 100);
            SetDlgItemTextA(hDlg, AUTO_ADD_USER_CARD, StrStrA(str, "����") ? "����û���Ƭ" : "�ѿ����Զ������Ƭ����");
            break;
        }
        default:
            break; 
        }
        break;
    }
    case WM_CLOSE:
        break;
    }
    return (INT_PTR)FALSE;
}