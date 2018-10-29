/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#ifndef WPIUTIL_WPI_NETWORKUTIL_H_
#define WPIUTIL_WPI_NETWORKUTIL_H_

#include <string>
#include <vector>

namespace wpi {
std::vector<std::string> GetNetworkInterfaces();
}  // namespace wpi

#endif  // WPIUTIL_WPI_NETWORKUTIL_H_
