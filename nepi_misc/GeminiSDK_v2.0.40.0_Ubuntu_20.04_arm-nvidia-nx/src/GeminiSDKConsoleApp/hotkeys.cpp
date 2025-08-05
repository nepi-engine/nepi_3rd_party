// HotKeys.cpp : Defines the entry point for the application.
//
#include <windows.h>

// Global Variables:
HINSTANCE hInst;                                // current instance
char szTitle[] = "ConsoleDataLogger";           // The title bar text
char szWindowClass[] = "WinDataLogger";        // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
HANDLE hThread = NULL;

#ifdef _DATA_LOGGER_
void onRangeUp();
void onRangeDown();
void onRange( double range );
void onSetFrameRateInterval( unsigned short int msInterval );
void onStartStoplog( bool recSTart );
#endif

DWORD WINAPI CaptureHotKey();

int RegisterForHotKeys()
{
    // Create thread 1.
    hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE )&CaptureHotKey, 0, 0, NULL);

    return 0;
}

DWORD WINAPI CaptureHotKey()
{
    HINSTANCE hInstance = (HINSTANCE)GetCurrentProcess();//GetModuleHandle(NULL);

    // Initialize global strings
    if (!MyRegisterClass(hInstance))
    {
        return FALSE;
    }

    // Perform application initialization:
    if (!InitInstance (hInstance, SW_HIDE))
    {
        return FALSE;
    }

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (msg.message == WM_QUIT)
        {
            CloseHandle( hThread );
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    memset(&wcex, 0, sizeof(wcex));
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    //wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HOTKEYS));
    //wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    //wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_HOTKEYS);
    wcex.lpszClassName  = szWindowClass;
    //wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      0, 0, 20, 20, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_HOTKEY:
    {
        switch (lParam)
        {
#ifdef _DATA_LOGGER_
        case 0x00430002:
            onRangeDown();
            break;
        case 0x00440002:
            onRangeUp();
            break;
        case 0x004C0002:
            onStartStoplog( wParam ? true : false);
            break;
        case 0xC01D0001:
            onRange( (double)wParam );
            break;
        case 0xC0170001:
            onSetFrameRateInterval( (unsigned short)wParam );
            break;
#endif
        default:
            break;
        }
        return true;
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
