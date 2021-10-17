#pragma once

#include "resource.h"


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void setRule(int num);
void initSimulation();
void doSimulationStep();