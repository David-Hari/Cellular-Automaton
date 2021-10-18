#pragma once

#include "resource.h"


HWND mainWindow = nullptr;

/* Window width and height */
int width = 0;
int height = 0;

/* Foreground (1) and background (0) colours */
COLORREF colour0 = RGB(244, 239, 223);
COLORREF colour1 = RGB(70, 58, 23);

/* Arrays to hold state */
char* previousRow = nullptr;
char* currentRow = nullptr;

/* Buffer bitmap */
HBITMAP bufferBitmap = nullptr;
void makeBitmapBuffer();

/* Update timer */
UINT_PTR timerId = 0;
unsigned int period = 50;    // Milliseconds
void increasePeriod();
void decreasePeriod();
void setUpdateTimer();

/* Simulation functions */
void setRule(int num);
void initSimulation();
void doSimulationStep();


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);