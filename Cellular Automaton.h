#pragma once

#include "resource.h"


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void initSimulation(int width);
void doSimulationStep();