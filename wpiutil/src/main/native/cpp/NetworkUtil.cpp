/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "wpi/NetworkUtil.h"

#include <string>
#include <vector>

#include "uv.h"
#include "wpi/SmallSet.h"

namespace wpi {
std::vector<std::string> GetNetworkInterfaces() {
  wpi::SmallSet<uint64_t, 8> set;

  uv_interface_address_t* addresses;
  int count = 0;

  auto status = uv_interface_addresses(&addresses, &count);

  if (status != 0) {
    return {};
  }

  char ip[50];

  for (int i = 0; i < count; i++) {
    if (addresses[i].is_internal) continue;
    if (addresses[i].address.address4.sin_family == AF_INET) {
      // Only handle IPv4, ignore all 0's
      if (addresses[i].address.address4.sin_addr.s_addr == 0) continue;
      set.insert(addresses[i].address.address4.sin_addr.s_addr);
    }
  }

  uv_free_interface_addresses(addresses, count);

  std::vector<std::string> interfaces;
  for (uint64_t addr : set) {
#ifdef _WIN32
    InetNtop(PF_INET, &addr, ip, sizeof(ip) - 1);
#else
    inet_ntop(PF_INET, reinterpret_cast<in_addr*>(&addr), ip, sizeof(ip) - 1);
#endif
    ip[sizeof(ip) - 1] = '0';
    interfaces.emplace_back(ip);
  }
  return interfaces;
}
}  // namespace wpi
