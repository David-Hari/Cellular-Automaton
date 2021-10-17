#include "stdafx.h"
#include "Cellular Automaton.h"

#define MAX_LOADSTRING 200

// Global Variables:
HINSTANCE hInst;                                // current instance
TCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND mainWindow = nullptr;
UINT_PTR timerId = 0;
int period = 100;        // Milliseconds
char* previousRow = nullptr;
char* currentRow = nullptr;
int width = 0;
int height = 0;

int yPos = 0;

/*
 * Interesting rules:
 *   - 18, random
 *   - 22, random
 *   - 30, point
 *   - 41, random
 *   - 45, point, random
 *   - 54, random
 *   - 60, random
 *   - 75, point, random
 */
char rules[8] = {
    /* 000 */ 0,
    /* 001 */ 1,
    /* 010 */ 1,
    /* 011 */ 1,
    /* 100 */ 1,
    /* 101 */ 0,
    /* 110 */ 0,
    /* 111 */ 0,
};


// Processes messages for the main window.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message) {
        case WM_COMMAND:
            wmId = LOWORD(wParam);
            wmEvent = HIWORD(wParam);
            // Parse the menu selections:
            switch (wmId) {
                case ID_FASTER:
                    period -= 10;
                    if (period < 10) { period = 10; }
                    SetTimer(mainWindow, timerId, period, NULL);
                    break;
                case ID_SLOWER:
                    period += 10;
                    if (period > 1000) { period = 1000; }
                    SetTimer(mainWindow, timerId, period, NULL);
                    break;
                case ID_SAVE:
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_TIMER:
            doSimulationStep();
            InvalidateRect(mainWindow, NULL, FALSE);
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            for (int i = 0; i < width; i++) {
                RECT rect;
                rect.left = i;
                rect.top = yPos;
                rect.right = i + 1;
                rect.bottom = yPos + 1;
                FillRect(hdc, &rect, (HBRUSH)((currentRow[i] == 1 ? COLOR_WINDOWTEXT : COLOR_WINDOW) + 1));
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

void setRule(int num) {
    //for (int i = 0; i < 8; i++) {
    //  set rules[i] if that bit in num is set
    //}
}

void initSimulation() {
    previousRow = (char*)calloc(width + 2, sizeof(char));   // Extra space at either end
    currentRow = (char*)calloc(width, sizeof(char));

    currentRow[width / 2] = 1;
}

void doSimulationStep() {
    memcpy_s(previousRow + 1, width, currentRow, width);

    for (int i = 0; i < width; i++) {
        int ruleNum = (previousRow[i] << 2) + (previousRow[i + 1] << 1) + previousRow[i + 2];
        currentRow[i] = rules[ruleNum];
    }

    yPos++;
}


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPTSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    hInst = hInstance; // Store instance handle in our global variable
    MSG msg;
    HACCEL hAccelTable;
    WNDCLASSEX wcex;

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_WINDOW, szWindowClass, MAX_LOADSTRING);

    // Register window class
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOW));
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName  = MAKEINTRESOURCE(IDC_WINDOW);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm       = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    RegisterClassEx(&wcex);

    mainWindow = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                            NULL, NULL, hInstance, NULL);
    ShowWindow(mainWindow, SW_SHOWMAXIMIZED);
    UpdateWindow(mainWindow);

    RECT rect;
    GetClientRect(mainWindow, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    initSimulation();
    timerId = SetTimer(mainWindow, 0, period, NULL);

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOW));

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}
