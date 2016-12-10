#pragma once

#include <vector>
#include <utility>

typedef std::pair<int, int> CrossConnectPair;

std::vector<CrossConnectPair> PWM_DIO_CrossConnects {
  {0, 0},
  {1, 1},
  {10, 11},
  {11, 10},
  {12, 13},
  {13, 12},
};