#pragma once

#include <wpi/SmallVector.h>
#include <wpi/Twine.h>
#include <string>

namespace frc {
  std::string GetFilePath(const wpi::Twine& path) {
    wpi::SmallVector<char, 128> tmp;
    auto pathRef = path.toStringRef(tmp);
    if (pathRef.empty()) {
      return "";
    }
    #ifdef __FRC_ROBORIO__
      if (pathRef[0] == '/') {
        return pathRef.str();
      }
      auto combinedPath = wpi::Twine("/home/lvuser/") + path;
      return combinedPath.str();
    #else
      if (pathRef.startswith_lower("/home/lvuser/")) {
        return pathRef.drop_front(13);
      } else {
        return pathRef.str();
      }
    #endif
  }
}
