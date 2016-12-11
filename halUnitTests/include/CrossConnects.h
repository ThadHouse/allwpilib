#pragma once

#include <vector>
#include <utility>

#include "HAL/HAL.h"

typedef std::pair<int, int> CrossConnectPair;

extern std::vector<CrossConnectPair> PWM_DIO_CrossConnects;

extern std::vector<CrossConnectPair> DIO_DIO_CrossConnects;

extern std::vector<std::pair<int32_t, HAL_SerialPort>> I2C_Serial_CrossConnects;

extern std::vector<std::pair<int32_t, HAL_SerialPort>> SPI_Serial_CrossConnects;
