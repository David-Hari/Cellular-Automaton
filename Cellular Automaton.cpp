#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include <string>
#include <sstream>
#include "Cellular Automaton.h"

#define MAX_LOADSTRING 200

HINSTANCE hInst;                          // current instance
TCHAR originalTitle[MAX_LOADSTRING];      // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];      // the main window class name
HDC memoryContext;


/*
 * Interesting rules:
 *   - 18,  random
 *   - 22,  random
 *   - 30,  point
 *   - 41,  random
 *   - 45,  point, random
 *   - 54,  random
 *   - 60,  random
 *   - 75,  point, random
 *   - 110, point, random
 */
char rules[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int speeds[] = { 1, 10, 50, 100, 500, 1000, 2000 };
const int NUM_SPEEDS = (sizeof(speeds) / sizeof((speeds)[0]));



void makeBitmapBuffer(int height) {
    HBITMAP newBuffer = CreateBitmap(width, height, 1, 1, NULL);
    HDC newDC = CreateCompatibleDC(memoryContext);
    SelectObject(newDC, newBuffer);

    // If there is an existing bitmap, copy it into the new one then destroy it.
    if (!bufferBitmap) {
        // TODO: This does not seem to be working.
        BitBlt(newDC, 0, 0, width, min(height, bitmapHeight), memoryContext, 0, 0, SRCCOPY);
        DeleteObject(bufferBitmap);
        DeleteDC(memoryContext);
    }

    bufferBitmap = newBuffer;
    memoryContext = newDC;
    bitmapHeight = height;
}


void increaseSpeed() {
    speedIndex++;
    if (speedIndex >= NUM_SPEEDS) {
        speedIndex = NUM_SPEEDS - 1;
    }
    else {
        speed = speeds[speedIndex];
        setUpdateTimer();
    }
}

void decreaseSpeed() {
    speedIndex--;
    if (speedIndex < 0) {
        speedIndex = 0;
    }
    else {
        speed = speeds[speedIndex];
        setUpdateTimer();
    }
}

void setUpdateTimer() {
    if (speed > 100) {
        makeBitmapBuffer(speed / 50);              // Increase height of buffer bitmap for fast updates
        SetTimer(mainWindow, timerId, 10, NULL);   // Timer can not be less than 10ms (USER_TIMER_MINIMUM)
    }
    else {
        makeBitmapBuffer(1);
        SetTimer(mainWindow, timerId, 1000 / speed, NULL);
    }
    updateWindowTitle();
}

void updateWindowTitle() {
    std::wstring originalTitleStr(originalTitle);
    std::wostringstream stream;
    stream << originalTitleStr << L"           Rule " << ruleNumber << "    " << speed << " updates per second";
    SetWindowText(mainWindow, stream.str().c_str());
}


void setRule(int num) {
    ruleNumber = num;
    unsigned int temp = num;
    for (int i = 0; i < 8; i++) {
        rules[i] = temp & 1;
        temp = temp >> 1;
    }
    updateWindowTitle();
}

void initSimulation(bool randomStart) {
    previousRow = (char*)calloc(width + 2, sizeof(char));   // Extra space at either end
    currentRow = (char*)calloc(width, sizeof(char));

    if (randomStart) {
        for (int i = 0; i < width; i++) {
            currentRow[i] = rand() > (RAND_MAX / 2);
        }
    }
    else {
        currentRow[width / 2] = 1;
    }
}

void doSimulationStep() {
    memcpy_s(previousRow + 1, width, currentRow, width);

    // Wrap boundaries, so that left most cell takes from far right neighbour and vice versa
    previousRow[0] = currentRow[width - 1];
    previousRow[width + 1] = currentRow[0];

    for (int i = 0; i < width; i++) {
        int ruleNum = (previousRow[i] << 2) + (previousRow[i + 1] << 1) + previousRow[i + 2];
        currentRow[i] = rules[ruleNum];
        SetPixel(memoryContext, i, bitmapY, (COLORREF)(currentRow[i] * 0xFFFFFF));
    }
    bitmapY++;
}


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
            increaseSpeed();
            break;
        case ID_SLOWER:
            decreaseSpeed();
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_TIMER:
        while (bitmapY < bitmapHeight) {
            doSimulationStep();
        }
        ScrollWindowEx(mainWindow, 0, -bitmapY, NULL, NULL, NULL, NULL, SW_INVALIDATE);
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        SetBkColor(hdc, colour1);
        SetTextColor(hdc, colour0);
        BitBlt(hdc, 0, height - bitmapY, width, bitmapY, memoryContext, 0, 0, SRCCOPY);
        bitmapY = 0;
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPTSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    hInst = hInstance; // Store instance handle in our global variable
    MSG msg;
    HACCEL hAccelTable;
    WNDCLASSEX wcex;

    srand((unsigned int)time(NULL));

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, originalTitle, MAX_LOADSTRING);
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

    mainWindow = CreateWindow(szWindowClass, originalTitle, WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                            NULL, NULL, hInstance, NULL);
    HDC windowDC = GetWindowDC(mainWindow);
    memoryContext = CreateCompatibleDC(windowDC);
    HBRUSH brush = CreateSolidBrush(colour0);
    SetClassLongPtr(mainWindow, GCLP_HBRBACKGROUND, (LONG_PTR)brush);
    ShowWindow(mainWindow, SW_SHOWMAXIMIZED);
    UpdateWindow(mainWindow);

    RECT rect;
    GetClientRect(mainWindow, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    setRule(30);
    makeBitmapBuffer(1);
    initSimulation(false);
    SetTimer(mainWindow, timerId, 1000 / speed, NULL);

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
