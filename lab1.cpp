#ifndef UNICODE
#define UNICODE
#endif 


#include <windows.h>
#include <string>
#include <vector>


#define START_PROCESS_NOTEPAD 1
#define START_PROCESS_PAINT 2
#define CLOSE_MESSAGE 3
#define UPDATE_STATUS 4
#define TIMER 5

typedef struct {
    PROCESS_INFORMATION PI;
    std::wstring name;
    bool running;
    bool statusChanged;
}ProcessInfo;

typedef struct {
    DWORD processId;
    HWND hwnd;
}WindowInfo;


HWND hListBox;
std::vector<ProcessInfo> procInfo;


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void InitializeComponent(HWND hwnd);

void StartProcess(const std::wstring& appName);

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

void SendCloseMessage();

void UpdateProcessStatus();

void ShowError(const std::wstring& msg);


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"Process Manager";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Process Manager",
        WS_OVERLAPPEDWINDOW,

        CW_USEDEFAULT, 
        CW_USEDEFAULT, 
        650,           
        500,

        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };

    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        InitializeComponent(hwnd);
        break;
    }
    case WM_COMMAND:
        switch (wParam)
        {
        case START_PROCESS_NOTEPAD:
        {
            StartProcess(L"notepad.exe");
            break;
        }
        case START_PROCESS_PAINT:
        {
            StartProcess(L"mspaint.exe");
            break;
        }
        case CLOSE_MESSAGE:
        {
            SendCloseMessage();
            break;
        }

        }
        case WM_TIMER:
        {
            UpdateProcessStatus();
            break;
        }

        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void InitializeComponent(HWND hwnd)
{
    CreateWindow(
        L"BUTTON",
        L"Start Notepad Process",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10,
        10,
        180,
        40,
        hwnd,
        (HMENU)START_PROCESS_NOTEPAD,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL);

    CreateWindow(
        L"BUTTON",
        L"Start Paint Process",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        200,
        10,
        150,
        40,
        hwnd,
        (HMENU)START_PROCESS_PAINT,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL);

    CreateWindow(
        L"BUTTON",
        L"Send Close Message",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        380,
        10,
        200,
        40,
        hwnd,
        (HMENU)CLOSE_MESSAGE,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL);

    hListBox = CreateWindow(
        L"LISTBOX",
        NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_AUTOVSCROLL,
        10,
        60,
        300,
        400,
        hwnd,
        NULL,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL);

    SetTimer(hwnd, TIMER, 100, NULL);

    SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)L"id         file name         status");

}

void StartProcess(const std::wstring& appName)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::wstring commandLine = appName;

    if (CreateProcess(NULL, &commandLine[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        ProcessInfo pInfo = { pi, appName, TRUE };
        procInfo.push_back(pInfo);
        std::wstring msg = std::to_wstring(pInfo.PI.dwProcessId) + L"  " + appName + L"  running";
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)msg.c_str());
    }
    else
    {
        ShowError(L"Failed to start Process");
    }
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    DWORD windowProcessId;
    GetWindowThreadProcessId(hwnd, &windowProcessId);

    WindowInfo* pInfo = reinterpret_cast<WindowInfo*>(lParam);
    if (windowProcessId == pInfo->processId) {
        pInfo->hwnd = hwnd;
        return FALSE;
    }

    return TRUE;
}

void SendCloseMessage()
{
    short index = SendMessage(hListBox, LB_GETCURSEL, 0, 0) - 1;


    if (index != LB_ERR && index < procInfo.size())
    {
        WindowInfo info = { procInfo[index].PI.dwProcessId, NULL };
        EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&info));
        if (info.hwnd)
        {
            SendMessage(info.hwnd, WM_CLOSE, 0, 0);
            procInfo[index].running = FALSE;

        }
        else
        {
            ShowError(L"Process is terminated");
        }
    }
}


void UpdateProcessStatus()
{
    bool shouldUpdateList = false; 

    for (auto& pi : procInfo)
    {
        DWORD waitResult = WaitForSingleObject(pi.PI.hProcess, 0);

        if (waitResult == WAIT_TIMEOUT && !pi.running)
        {
            pi.running = true;
            pi.statusChanged = true; 
            shouldUpdateList = true;
        }
        else if (waitResult == WAIT_OBJECT_0 && pi.running)
        {
            pi.running = false; 
            pi.statusChanged = true; 
            shouldUpdateList = true; 
        }
    }

    if (shouldUpdateList)
    {
        SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)L"id         file name         status");

        for (const auto& pi : procInfo)
        {
            std::wstring msg = std::to_wstring(pi.PI.dwProcessId) + L"  " + pi.name + L"  " + (pi.running ? L"running" : L"terminated");
            SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)msg.c_str());
        }
    }
}


void ShowError(const std::wstring& msg)
{
    MessageBox(NULL, msg.c_str(), L"Error", MB_OK | MB_ICONERROR);
}

