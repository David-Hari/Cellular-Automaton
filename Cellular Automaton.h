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
HBITMAP bufferBitmap;
int bitmapHeight = 1;
int bitmapY = 0;
void makeBitmapBuffer(int height);

/* Update timer */
UINT_PTR timerId = 1;
unsigned int speed = 10;    // Updates per second
int speedIndex = 1;
void increaseSpeed();
void decreaseSpeed();
void setUpdateTimer();

/* Simulation functions */
int ruleNumber;
void setRule(int num);
void initSimulation();
void doSimulationStep();

void updateWindowTitle();


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);