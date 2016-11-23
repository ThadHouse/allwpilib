/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2016. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include "HAL/SerialPort.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <vector>

#include "HAL/Errors.h"
#include "HAL/cpp/priority_mutex.h"
#include "llvm/SmallString.h"
#include "llvm/SmallVector.h"
#include "visa/visa.h"

constexpr const char* OnboardResource = "ASRL1::INSTR";
constexpr const char* MxpResource = "ASRL2::INSTR";

static int32_t resourceManagerHandle;

static priority_mutex portMutex;
static HAL_SerialPort portHandles[4];

static std::string m_usbNames[2]{"", ""};

static void CoiteratedSort(
    const llvm::SmallVectorImpl<llvm::SmallString<16>>& coiteratedArray,
    const llvm::SmallVectorImpl<llvm::SmallString<16>>& arrayToSort,
    llvm::SmallVectorImpl<llvm::SmallString<16>>& coiteratedArrayOut,
    llvm::SmallVectorImpl<llvm::SmallString<16>>& sortedArray) {
  sortedArray.clear();
  sortedArray = arrayToSort;
  std::sort(sortedArray.begin(), sortedArray.end(),
            [](llvm::SmallVectorImpl<char>& lhs,
               llvm::SmallVectorImpl<char>& rhs) -> int {
              llvm::StringRef lhsRef(lhs.begin(), lhs.size());
              llvm::StringRef rhsRef(rhs.begin(), rhs.size());
              return lhsRef.compare(rhsRef);
            });

  coiteratedArrayOut.clear();
  for (auto& str : sortedArray) {
    for (size_t i = 0; i < arrayToSort.size(); i++) {
      if (llvm::StringRef{arrayToSort[i].begin(), arrayToSort[i].size()}.equals(
              llvm::StringRef{str.begin(), str.size()})) {
        coiteratedArrayOut.push_back(coiteratedArray[i]);
        break;
      }
    }
  }
}

static void SerialQueryHubPaths(
    llvm::SmallVectorImpl<llvm::SmallString<16>>& visaResource,
    llvm::SmallVectorImpl<llvm::SmallString<16>>& osResource,
    llvm::SmallVectorImpl<llvm::SmallString<16>>& hubPath, int32_t* status) {
  // Arrays to hold data before sorting.
  // Note we will most likely have at max 2 instances, and the longest string
  // is around 12, so these should never touch the heap;
  llvm::SmallVector<llvm::SmallString<16>, 4> hubLocationToSort;
  llvm::SmallVector<llvm::SmallString<16>, 4> osResourceToSortArray;
  llvm::SmallVector<llvm::SmallString<16>, 4> visaResourceToSortArray;

  // VISA resource matching string
  const char* str = "?*";
  // Items needed for VISA
  ViUInt32 retCnt = 0;
  ViFindList viList = 0;
  ViChar desc[VI_FIND_BUFLEN];
  *status = viFindRsrc(resourceManagerHandle, const_cast<char*>(str), &viList,
                       &retCnt, desc);

  if (*status < 0) {
    // Handle the bad status elsewhere
    // Note let positive statii (warnings) continue
    return;
  }
  // Status might be positive, so reset it to 0
  *status = 0;

  // Storage buffers for Visa calls and system exec calls
  char osName[256];
  char execBuffer[128];

  // Regex to filter out the device name
  // Will be index 1 on success
  std::regex devMatch("[^\\s]+\\s+\\(\\/dev\\/([^\\s]+)\\)");

  // Loop through all returned VISA objects.
  // Increment the internal VISA ptr every loop
  for (size_t i = 0; i < retCnt; i++, viFindNext(viList, desc)) {
    // Ignore any matches to the 2 onboard ports
    if (std::strcmp(OnboardResource, desc) == 0 ||
        std::strcmp(MxpResource, desc) == 0) {
      continue;
    }

    // Open the resource, grab its interface name, and close it.
    ViSession vSession;
    *status = viOpen(resourceManagerHandle, desc, VI_NULL, VI_NULL, &vSession);
    if (*status < 0) return;
    *status = 0;

    *status = viGetAttribute(vSession, VI_ATTR_INTF_INST_NAME, &osName);
    // Ignore an error here, as we want to close the session on an error
    // Use a seperate close variable so we can check
    ViStatus closeStatus = viClose(vSession);
    if (*status < 0) return;
    if (closeStatus < 0) return;
    *status = 0;

    std::smatch regexMatcher;
    auto regexVal =
        std::regex_match(std::string(osName), regexMatcher, devMatch);

    if (regexVal > 0 && regexMatcher.size() > 1) {
      // Store our match string, as output was ocassionally being corrupted
      // before being accessed
      auto matchString = regexMatcher[1].str();

      // Run find using pipe to get a list of system accessors
      llvm::SmallString<128> val(
          "sh -c \"find /sys/devices/soc0 | grep amba | grep usb | grep ");
      val += matchString;
      val += "\"";

      // Pipe code found on StackOverflow
      // http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix

      // Using std::string because this is guarenteed to be large
      std::string output = "";

      std::shared_ptr<FILE> pipe(popen(val.c_str(), "r"), pclose);
      // Just check the next item on a pipe failure
      if (!pipe) continue;
      while (!feof(pipe.get())) {
        if (std::fgets(execBuffer, 128, pipe.get()) != 0) output += execBuffer;
      }

      if (!output.empty()) {
        llvm::SmallVector<llvm::StringRef, 16> pathSplitVec;
        // Split output by line, grab first line, and split it into
        // individual directories
        llvm::StringRef{output}.split('\n').first.split(pathSplitVec, '/', -1,
                                                        false);

        // Find each individual item index

        const char* usb1 = "usb1";
        const char* tty = "tty";

        int findusb = -1;
        int findtty = -1;
        int findregex = -1;
        for (size_t i = 0; i < pathSplitVec.size(); i++) {
          if (findusb == -1 && pathSplitVec[i].equals(usb1)) {
            findusb = i;
          }
          if (findtty == -1 && pathSplitVec[i].equals(tty)) {
            findtty = i;
          }
          if (findregex == -1 && pathSplitVec[i].equals(matchString)) {
            findregex = i;
          }
        }

        // Get the index for our device
        int hubIndex = findtty;
        if (findtty == -1) hubIndex = findregex;

        int devStart = findusb + 1;

        if (hubIndex < devStart) continue;

        // Add our devices to our list
        hubLocationToSort.push_back(
            llvm::StringRef{pathSplitVec[hubIndex - 2]});
        visaResourceToSortArray.push_back(llvm::StringRef{desc});
        osResourceToSortArray.push_back(llvm::StringRef{matchString});
      }
    }
  }

  CoiteratedSort(visaResourceToSortArray, hubLocationToSort, visaResource,
                 hubPath);
  CoiteratedSort(osResourceToSortArray, hubLocationToSort, osResource, hubPath);
}

static std::string GetUSBSerialPortName(HAL_SerialPort port, int32_t* status) {
  llvm::SmallVector<llvm::SmallString<16>, 4> visaResource;
  llvm::SmallVector<llvm::SmallString<16>, 4> osResource;
  llvm::SmallVector<llvm::SmallString<16>, 4> hubPath;

  SerialQueryHubPaths(visaResource, osResource, hubPath, status);

  // If paths are empty or status error, return error
  if (*status != 0 || visaResource.empty() || osResource.empty() ||
      hubPath.empty()) {
    *status = HAL_SERIAL_PORT_NOT_FOUND;
    return "";
  }

  std::string portString = m_usbNames[port - 2];

  llvm::SmallVector<int32_t, 4> indices;

  // If port has not been assigned, find the one to assign
  if (portString.empty()) {
    for (size_t i = 0; i < 2; i++) {
      // Remove all used ports
      auto idx = std::find(hubPath.begin(), hubPath.end(), m_usbNames[i]);
      if (idx != hubPath.end()) {
        // found
        hubPath.erase(idx);
      }
      if (m_usbNames[i] == "") {
        indices.push_back(i);
      }
    }

    int32_t idx = -1;
    for (size_t i = 0; i < indices.size(); i++) {
      if (indices[i] == port - 2) {
        idx = i;
        break;
      }
    }

    if (idx == -1) {
      *status = HAL_SERIAL_PORT_NOT_FOUND;
      return "";
    }

    if (idx >= static_cast<int32_t>(hubPath.size())) {
      *status = HAL_SERIAL_PORT_NOT_FOUND;
      return "";
    }

    portString = hubPath[idx].str();
    m_usbNames[port - 2] = portString;
  }

  int visaIndex = -1;

  for (size_t i = 0; i < hubPath.size(); i++) {
    if (hubPath[i].equals(portString)) {
      visaIndex = i;
      break;
    }
  }

  if (visaIndex == -1) {
    *status = HAL_SERIAL_PORT_NOT_FOUND;
    return "";
    // Error
  } else {
    return visaResource[visaIndex].str();
  }
}

extern "C" {

void HAL_InitializeSerialPort(HAL_SerialPort port, int32_t* status) {
  std::string portName;

  if (resourceManagerHandle == 0)
    viOpenDefaultRM(reinterpret_cast<ViSession*>(&resourceManagerHandle));

  if (port == 0)
    portName = OnboardResource;
  else if (port == 1)
    portName = MxpResource;
  else
    portName = GetUSBSerialPortName(port, status);

  if (*status < 0) {
    return;
  }

  *status = viOpen(resourceManagerHandle, const_cast<char*>(portName.c_str()),
                   VI_NULL, VI_NULL,
                   reinterpret_cast<ViSession*>(&portHandles[port]));
  if (*status > 0) *status = 0;
}

void HAL_SetSerialBaudRate(HAL_SerialPort port, int32_t baud, int32_t* status) {
  *status = viSetAttribute(portHandles[port], VI_ATTR_ASRL_BAUD, baud);
  if (*status > 0) *status = 0;
}

void HAL_SetSerialDataBits(HAL_SerialPort port, int32_t bits, int32_t* status) {
  *status = viSetAttribute(portHandles[port], VI_ATTR_ASRL_DATA_BITS, bits);
  if (*status > 0) *status = 0;
}

void HAL_SetSerialParity(HAL_SerialPort port, int32_t parity, int32_t* status) {
  *status = viSetAttribute(portHandles[port], VI_ATTR_ASRL_PARITY, parity);
  if (*status > 0) *status = 0;
}

void HAL_SetSerialStopBits(HAL_SerialPort port, int32_t stopBits,
                           int32_t* status) {
  *status = viSetAttribute(portHandles[port], VI_ATTR_ASRL_STOP_BITS, stopBits);
  if (*status > 0) *status = 0;
}

void HAL_SetSerialWriteMode(HAL_SerialPort port, int32_t mode,
                            int32_t* status) {
  *status = viSetAttribute(portHandles[port], VI_ATTR_WR_BUF_OPER_MODE, mode);
  if (*status > 0) *status = 0;
}

void HAL_SetSerialFlowControl(HAL_SerialPort port, int32_t flow,
                              int32_t* status) {
  *status = viSetAttribute(portHandles[port], VI_ATTR_ASRL_FLOW_CNTRL, flow);
  if (*status > 0) *status = 0;
}

void HAL_SetSerialTimeout(HAL_SerialPort port, double timeout,
                          int32_t* status) {
  *status = viSetAttribute(portHandles[port], VI_ATTR_TMO_VALUE,
                           static_cast<uint32_t>(timeout * 1e3));
  if (*status > 0) *status = 0;
}

void HAL_EnableSerialTermination(HAL_SerialPort port, char terminator,
                                 int32_t* status) {
  viSetAttribute(portHandles[port], VI_ATTR_TERMCHAR_EN, VI_TRUE);
  viSetAttribute(portHandles[port], VI_ATTR_TERMCHAR, terminator);
  *status = viSetAttribute(portHandles[port], VI_ATTR_ASRL_END_IN,
                           VI_ASRL_END_TERMCHAR);
  if (*status > 0) *status = 0;
}

void HAL_DisableSerialTermination(HAL_SerialPort port, int32_t* status) {
  viSetAttribute(portHandles[port], VI_ATTR_TERMCHAR_EN, VI_FALSE);
  *status =
      viSetAttribute(portHandles[port], VI_ATTR_ASRL_END_IN, VI_ASRL_END_NONE);
  if (*status > 0) *status = 0;
}

void HAL_SetSerialReadBufferSize(HAL_SerialPort port, int32_t size,
                                 int32_t* status) {
  *status = viSetBuf(portHandles[port], VI_READ_BUF, size);
  if (*status > 0) *status = 0;
}

void HAL_SetSerialWriteBufferSize(HAL_SerialPort port, int32_t size,
                                  int32_t* status) {
  *status = viSetBuf(portHandles[port], VI_WRITE_BUF, size);
  if (*status > 0) *status = 0;
}

int32_t HAL_GetSerialBytesReceived(HAL_SerialPort port, int32_t* status) {
  int32_t bytes = 0;

  *status = viGetAttribute(portHandles[port], VI_ATTR_ASRL_AVAIL_NUM, &bytes);
  if (*status > 0) *status = 0;
  return bytes;
}

int32_t HAL_ReadSerial(HAL_SerialPort port, char* buffer, int32_t count,
                       int32_t* status) {
  uint32_t retCount = 0;

  *status =
      viRead(portHandles[port], (ViPBuf)buffer, count, (ViPUInt32)&retCount);

  if (*status == VI_ERROR_IO || *status == VI_ERROR_ASRL_OVERRUN ||
      *status == VI_ERROR_ASRL_FRAMING || *status == VI_ERROR_ASRL_PARITY) {
    int32_t localStatus = 0;
    HAL_ClearSerial(port, &localStatus);
  }

  if (*status == VI_ERROR_TMO || *status > 0) *status = 0;
  return static_cast<int32_t>(retCount);
}

int32_t HAL_WriteSerial(HAL_SerialPort port, const char* buffer, int32_t count,
                        int32_t* status) {
  uint32_t retCount = 0;

  *status =
      viWrite(portHandles[port], (ViPBuf)buffer, count, (ViPUInt32)&retCount);

  if (*status > 0) *status = 0;
  return static_cast<int32_t>(retCount);
}

void HAL_FlushSerial(HAL_SerialPort port, int32_t* status) {
  *status = viFlush(portHandles[port], VI_WRITE_BUF);
  if (*status > 0) *status = 0;
}

void HAL_ClearSerial(HAL_SerialPort port, int32_t* status) {
  *status = viClear(portHandles[port]);
  if (*status > 0) *status = 0;
}

void HAL_CloseSerial(HAL_SerialPort port, int32_t* status) {
  *status = viClose(portHandles[port]);
  if (*status > 0) *status = 0;
}

}  // extern "C"
