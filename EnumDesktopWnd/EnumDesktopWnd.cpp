// EnumDesktopWnd.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include <CommCtrl.h>
#include <assert.h>

void GetDesktopIconInfo(HWND hListView);

int main()
{
    //获取桌面窗口句柄，顶级窗口，也称Root窗口
    HWND hDesktop0 = GetDesktopWindow();
    if (hDesktop0 != NULL)
    {
        _tprintf(_T("Desktop window: 0x%p\n"), hDesktop0);

        //获取类名
        TCHAR szClassName[MAX_PATH] = { 0 };
        GetClassName(hDesktop0, szClassName, MAX_PATH);
        _tprintf(_T("Desktop class: %s\n"), szClassName);  //#32769

        //获取标题
        TCHAR szText[MAX_PATH] = { 0 };
        GetWindowText(hDesktop0, szText, MAX_PATH);
        _tprintf(_T("Desktop title: %s\n"), szText);
    }

    //获取桌面窗口句柄，非顶级窗口
    HWND hDesktop = FindWindow(TEXT("Progman"), TEXT("Program Manager"));
    if (hDesktop != NULL)
    {
        _tprintf(_T("Desktop window: 0x%p\n"), hDesktop);

        //获取父窗口，not work
        HWND hParent = GetParent(hDesktop);
        _tprintf(_T("Parent window: 0x%p\n"), hParent);
        if (hParent == NULL)
        {
            _tprintf(_T("GetParent fail: %d\n"), GetLastError());
        }

        //获取父窗口，work，也就是顶级窗口GetDesktopWindow()
        HWND hParent2 = GetAncestor(hDesktop, GA_PARENT);
        _tprintf(_T("Parent window 2: 0x%p\n"), hParent2);
        
        assert(hParent2 == hDesktop0);

        //获取子窗口SHELLDLL_DefView
        HWND hShellDLL_DefView = FindWindowEx(hDesktop, NULL, TEXT("SHELLDLL_DefView"), NULL);
        if (hShellDLL_DefView != NULL)
        {
            _tprintf(_T("SHELLDLL_DefView window: 0x%p\n"), hShellDLL_DefView);
            //获取子窗口SysListView32
            HWND hSysListView32 = FindWindowEx(hShellDLL_DefView, NULL, TEXT("SysListView32"), NULL);
            if (hSysListView32 != NULL)
            {
                _tprintf(_T("SysListView32 window: 0x%p\n"), hSysListView32);
                //枚举子窗口
                HWND hChild = FindWindowEx(hSysListView32, NULL, NULL, NULL);
                int nCount = 0;
                while (hChild != NULL)
                {
                    //获取窗口类名，SysHeader32
                    TCHAR szClassName[MAX_PATH] = { 0 };
                    GetClassName(hChild, szClassName, MAX_PATH);
                    _tprintf(_T("Child %d Class: %s\n"), nCount, szClassName);

                    //获取窗口标题
                    TCHAR szText[MAX_PATH] = { 0 };
                    GetWindowText(hChild, szText, MAX_PATH);
                    _tprintf(_T("Child %d Title: %s\n"), nCount, szText);

                    nCount++;
                    hChild = FindWindowEx(hSysListView32, hChild, NULL, NULL);
                }

                GetDesktopIconInfo(hSysListView32);
            }
            else
            {
                _tprintf(_T("SysListView32 window not found!\n"));
            }
        }
        else
        {
            _tprintf(_T("SHELLDLL_DefView window not found!\n"));
        }
    }
    else
    {
        _tprintf(_T("Desktop window not found!\n"));
    }
}

void GetDesktopIconInfo(HWND hListView)
{
    setlocale(LC_ALL, "chs");  //设置中文字符集，否则中文乱码

    //获取窗口所属进程ID
    DWORD dwProcessId = 0;
    GetWindowThreadProcessId(hListView, &dwProcessId);
    _tprintf(_T("Process ID: %d\n"), dwProcessId);

    //打开进程，进行读写
    HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION
        , FALSE, dwProcessId);

    if (hProcess != NULL)
    {
        //获取桌面图标数量
        int nCount = ListView_GetItemCount(hListView);
        _tprintf(_T("Desktop icon count: %d\n"), nCount);

        LVITEM* pItem = NULL;
        TCHAR* pszText = NULL;

        //在目标进程中分配内存
        pItem = (LVITEM*)VirtualAllocEx(hProcess, NULL, sizeof(LVITEM), MEM_COMMIT, PAGE_READWRITE);
        pszText = (TCHAR*)VirtualAllocEx(hProcess, NULL, MAX_PATH * sizeof(TCHAR), MEM_COMMIT, PAGE_READWRITE);
        if (pItem != NULL && pszText != NULL)
        {
            //获取桌面图标名称
            for (int i = 0; i < nCount; i++)
            {
                LVITEM lvItem = { 0 };
                lvItem.mask = LVIF_TEXT;
                lvItem.iSubItem = 0;
                lvItem.pszText = pszText;  //指向目标进程的内存
                lvItem.cchTextMax = MAX_PATH;
                lvItem.iItem = i;

                SIZE_T nSize = 0;
                WriteProcessMemory(hProcess, pItem, &lvItem, sizeof(LVITEM), &nSize);

                //不要使用ListView_GetItemText
                SendMessage(hListView, LVM_GETITEMTEXT, i, (LPARAM)pItem);

                TCHAR szText[MAX_PATH] = { 0 };
                ReadProcessMemory(hProcess, pszText, szText, MAX_PATH * sizeof(TCHAR), &nSize);
                _tprintf(_T("Desktop title %d: %s\n"), i, szText);
            }
        }
        else
        {
            _tprintf(_T("VirtualAllocEx fail: %d\n"), GetLastError());
        }

        if (pItem != NULL)
        {
            VirtualFreeEx(hProcess, pItem, 0, MEM_RELEASE);
        }

        if (pszText != NULL)
        {
            VirtualFreeEx(hProcess, pszText, 0, MEM_RELEASE);
        }

        CloseHandle(hProcess);
    }
    else
    {
        _tprintf(_T("OpenProcess fail: %d\n"), GetLastError());
    }
}
