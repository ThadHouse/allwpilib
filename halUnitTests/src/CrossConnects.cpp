#include "CrossConnects.h"

std::vector<CrossConnectPair> PWM_DIO_CrossConnects {
  {0, 0},
  {1, 1},
  {10, 11},
  {11, 10},
  {12, 13},
  {13, 12},
};

std::vector<CrossConnectPair> DIO_DIO_CrossConnects {
  {10, 11},
  {11, 10},
  {12, 13},
  {13, 12},
};

std::vector<std::pair<int32_t, HAL_SerialPort>> I2C_Serial_CrossConnects{
    // Onboard, Top USB
    {0, HAL_SerialPort_USB1},
    // MXP, Bottom USB
    //{1, HAL_SerialPort_USB2},
};

std::vector<std::pair<int32_t, HAL_SerialPort>> SPI_Serial_CrossConnects {
  // Onboard CS0, Top USB
  {0, HAL_SerialPort_USB1},
  // MXP, Bottom USB
  //{4, HAL_SerialPort_USB2}, 
};
