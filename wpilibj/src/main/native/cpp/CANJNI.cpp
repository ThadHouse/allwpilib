/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2016. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include <assert.h>
#include <jni.h>

#ifdef CONFIG_ATHENA
#include "FRC_NetworkCommunication/CANSessionMux.h"
#endif
#include "HAL/cpp/Log.h"
#include "HALUtil.h"
#include "edu_wpi_first_wpilibj_can_CANJNI.h"
#include "llvm/SmallString.h"
#include "llvm/raw_ostream.h"
#include "support/jni_util.h"

using namespace frc;
using namespace wpi::java;

// set the logging level
// TLogLevel canJNILogLevel = logDEBUG;
TLogLevel canJNILogLevel = logERROR;

#define CANJNI_LOG(level)     \
  if (level > canJNILogLevel) \
    ;                         \
  else                        \
  Log().Get(level)

extern "C" {

/*
 * Class:     edu_wpi_first_wpilibj_can_CANJNI
 * Method:    FRCNetCommCANSessionMuxSendMessage
 * Signature: (I[BI)V
 */
JNIEXPORT void JNICALL
Java_edu_wpi_first_wpilibj_can_CANJNI_FRCNetCommCANSessionMuxSendMessage(
    JNIEnv *env, jclass, jint messageID, jbyteArray data, jint periodMs) {
#ifdef CONFIG_ATHENA
  CANJNI_LOG(logDEBUG)
      << "Calling CANJNI FRCNetCommCANSessionMuxSendMessage";

  JByteArrayRef dataArray{env, data};

  const uint8_t *dataBuffer = reinterpret_cast<const uint8_t*>(dataArray.array().data());
  uint8_t dataSize = dataArray.array().size();

  CANJNI_LOG(logDEBUG) << "Message ID ";
  CANJNI_LOG(logDEBUG).write_hex(messageID);

  if (logDEBUG <= canJNILogLevel) {
    if (dataBuffer) {
      llvm::SmallString<128> buf;
      llvm::raw_svector_ostream str(buf);
      for (int32_t i = 0; i < dataSize; i++) {
        str.write_hex(dataBuffer[i]) << ' ';
      }

      Log().Get(logDEBUG) << "Data: " << str.str();
    } else {
      CANJNI_LOG(logDEBUG) << "Data: null";
    }
  }

  CANJNI_LOG(logDEBUG) << "Period: " << periodMs;

  int32_t status = 0;
  FRC_NetworkCommunication_CANSessionMux_sendMessage(
      messageID, dataBuffer, dataSize, periodMs, &status);

  CANJNI_LOG(logDEBUG) << "Status: " << status;
  CheckCANStatus(env, status, messageID);
#else
  // Noop on other platforms
#endif
}

/*
 * Class:     edu_wpi_first_wpilibj_can_CANJNI
 * Method:    FRCNetCommCANSessionMuxReceiveMessage
 * Signature: (Ljava/nio/IntBuffer;ILjava/nio/ByteBuffer;)[B
 */
JNIEXPORT jbyteArray JNICALL
Java_edu_wpi_first_wpilibj_can_CANJNI_FRCNetCommCANSessionMuxReceiveMessage(
    JNIEnv *env, jclass, jobject messageID, jint messageIDMask,
    jobject timeStamp) {
#ifdef CONFIG_ATHENA
  CANJNI_LOG(logDEBUG)
      << "Calling CANJNI FRCNetCommCANSessionMuxReceiveMessage";

  uint32_t *messageIDPtr = (uint32_t *)env->GetDirectBufferAddress(messageID);
  uint32_t *timeStampPtr = (uint32_t *)env->GetDirectBufferAddress(timeStamp);

  uint8_t dataSize = 0;
  uint8_t buffer[8];

  int32_t status = 0;
  FRC_NetworkCommunication_CANSessionMux_receiveMessage(
      messageIDPtr, messageIDMask, buffer, &dataSize, timeStampPtr, &status);

  CANJNI_LOG(logDEBUG) << "Message ID ";
  CANJNI_LOG(logDEBUG).write_hex(*messageIDPtr);

  if (logDEBUG <= canJNILogLevel) {
    llvm::SmallString<128> buf;
    llvm::raw_svector_ostream str(buf);

    for (int32_t i = 0; i < dataSize; i++) {
      // Pad one-digit data with a zero
      if (buffer[i] <= 16) {
        str << '0';
      }

      str.write_hex(buffer[i]) << ' ';
    }

    Log().Get(logDEBUG) << "Data: " << str.str();
  }

  CANJNI_LOG(logDEBUG) << "Timestamp: " << *timeStampPtr;
  CANJNI_LOG(logDEBUG) << "Status: " << status;

  if (!CheckCANStatus(env, status, *messageIDPtr)) return nullptr;
  return MakeJByteArray(env, llvm::StringRef{reinterpret_cast<const char*>(buffer), 
                        static_cast<size_t>(dataSize)});
#else
  // Noop on other platforms. Return nullptr
  return nullptr;
#endif
}

}  // extern "C"
