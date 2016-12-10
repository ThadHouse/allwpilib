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
