#pragma once

#include <stdint.h>

void Wait(double seconds);

void FillCANBuffer(uint8_t* sendArray, uint8_t* data, int arbId, int count);
