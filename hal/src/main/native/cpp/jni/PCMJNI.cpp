#include <jni.h>

#include <cassert>

#include "HALUtil.h"

#include "edu_wpi_first_wpilibj_hal_PCMJNI.h"
#include "hal/PCM.h"
#include "hal/Ports.h"
#include "hal/cpp/Log.h"

using namespace frc;

// set the logging level
TLogLevel pcmJNILogLevel = logWARNING;

#define PCMJNI_LOG(level)     \
  if (level > pcmJNILogLevel) \
    ;                         \
  else                        \
    Log().Get(level)

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMHandle
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMHandle
  (JNIEnv * env, jclass, jint module) {
    int32_t status = 0;
    auto pcm = HAL_GetPCMHandle(module, &status);
    CheckStatusRange(env, status, 0, HAL_GetNumPCMModules(), module);
    return pcm;
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    freePCMHandle
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_freePCMHandle
  (JNIEnv * env, jclass, jint handle) {
    HAL_FreePCMHandle(handle);
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    checkPCMChannel
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_checkPCMChannel
  (JNIEnv *, jclass, jint channel) {
    return HAL_CheckPCMChannel(channel);
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    checkPCMModule
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_checkPCMModule
  (JNIEnv *, jclass, jint module) {
    return HAL_CheckPCMModule(module);
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMInputVoltage
 * Signature: (I)D
 */
JNIEXPORT jdouble JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMInputVoltage
  (JNIEnv * env, jclass, jint handle) {
    int32_t status = 0;
    auto retVal = HAL_GetPCMInputVoltage(static_cast<HAL_PCMHandle>(handle), &status);
    CheckStatus(env, status);
    return retVal;
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMSolenoidVoltage
 * Signature: (I)D
 */
JNIEXPORT jdouble JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMSolenoidVoltage
  (JNIEnv * env, jclass, jint handle) {
    int32_t status = 0;
    auto retVal = HAL_GetPCMSolenoidVoltage(static_cast<HAL_PCMHandle>(handle), &status);
    CheckStatus(env, status);
    return retVal;
  }


/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMSolenoid
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMSolenoid
  (JNIEnv * env, jclass, jint handle, jint index) {
        int32_t status = 0;
    auto retVal = HAL_GetPCMSolenoid(static_cast<HAL_PCMHandle>(handle), index, &status);
    CheckStatus(env, status);
    return retVal;
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMAllSolenoids
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMAllSolenoids
  (JNIEnv * env, jclass, jint handle) {
            int32_t status = 0;
    auto retVal = HAL_GetPCMAllSolenoids(static_cast<HAL_PCMHandle>(handle), &status);
    CheckStatus(env, status);
    return retVal;
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    setPCMSolenoid
 * Signature: (IIZ)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_setPCMSolenoid
  (JNIEnv * env, jclass, jint handle, jint index, jboolean on) {
                int32_t status = 0;
    HAL_SetPCMSolenoid(static_cast<HAL_PCMHandle>(handle), index, on, &status);
    CheckStatus(env, status);
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    setPCMAllSolenoids
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_setPCMAllSolenoids
  (JNIEnv * env, jclass, jint handle, jint values) {
                    int32_t status = 0;
    HAL_SetPCMAllSolenoids(static_cast<HAL_PCMHandle>(handle), values, &status);
    CheckStatus(env, status);
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    setPCMOneShotDuration
 * Signature: (III)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_setPCMOneShotDuration
  (JNIEnv * env, jclass, jint handle, jint index, jint durMs) {
                        int32_t status = 0;
    HAL_SetPCMOneShotDuration(static_cast<HAL_PCMHandle>(handle), index, durMs, &status);
    CheckStatus(env, status);
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    firePCMOneShot
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_firePCMOneShot
  (JNIEnv * env, jclass, jint handle, jint index) {
    int32_t status = 0;
    HAL_FirePCMOneShot(static_cast<HAL_PCMHandle>(handle), index, &status);
    CheckStatus(env, status);
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMCompressorOn
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMCompressorOn
  (JNIEnv * env, jclass, jint handle) {
            int32_t status = 0;
    auto retVal = HAL_GetPCMCompressorOn(static_cast<HAL_PCMHandle>(handle), &status);
    CheckStatus(env, status);
    return retVal;
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    setPCMCompressorClosedLoopControl
 * Signature: (IZ)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_setPCMCompressorClosedLoopControl
  (JNIEnv * env, jclass, jint handle, jboolean value) {
            int32_t status = 0;
    HAL_SetPCMCompressorClosedLoopControl(static_cast<HAL_PCMHandle>(handle), value, &status);
    CheckStatus(env, status);
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMCompressorClosedLoopControl
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMCompressorClosedLoopControl
  (JNIEnv * env, jclass, jint handle) {
            int32_t status = 0;
    auto retVal = HAL_GetPCMCompressorClosedLoopControl(static_cast<HAL_PCMHandle>(handle), &status);
    CheckStatus(env, status);
    return retVal;
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMCompressorPressureSwitch
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMCompressorPressureSwitch
  (JNIEnv * env, jclass, jint handle) {
            int32_t status = 0;
    auto retVal = HAL_GetPCMCompressorPressureSwitch(static_cast<HAL_PCMHandle>(handle), &status);
    CheckStatus(env, status);
    return retVal;
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMCompressorCurrent
 * Signature: (I)D
 */
JNIEXPORT jdouble JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMCompressorCurrent
  (JNIEnv * env, jclass, jint handle) {
            int32_t status = 0;
    auto retVal = HAL_GetPCMCompressorCurrent(static_cast<HAL_PCMHandle>(handle), &status);
    CheckStatus(env, status);
    return retVal;
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMCompressorCurrentTooHighFault
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMCompressorCurrentTooHighFault
  (JNIEnv * env, jclass, jint handle) {
            int32_t status = 0;
    auto retVal = HAL_GetPCMCompressorCurrentTooHighFault(static_cast<HAL_PCMHandle>(handle), &status);
    CheckStatus(env, status);
    return retVal;
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMCompressorCurrentTooHighStickyFault
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMCompressorCurrentTooHighStickyFault
  (JNIEnv * env, jclass, jint handle) {
            int32_t status = 0;
    auto retVal = HAL_GetPCMCompressorCurrentTooHighStickyFault(static_cast<HAL_PCMHandle>(handle), &status);
    CheckStatus(env, status);
    return retVal;
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMCompressorShortedStickyFault
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMCompressorShortedStickyFault
  (JNIEnv * env, jclass, jint handle) {
            int32_t status = 0;
    auto retVal = HAL_GetPCMCompressorShortedStickyFault(static_cast<HAL_PCMHandle>(handle), &status);
    CheckStatus(env, status);
    return retVal;
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMCompressorShortedFault
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMCompressorShortedFault
  (JNIEnv * env, jclass, jint handle) {
            int32_t status = 0;
    auto retVal = HAL_GetPCMCompressorShortedFault(static_cast<HAL_PCMHandle>(handle), &status);
    CheckStatus(env, status);
    return retVal;
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMCompressorNotConnectedStickyFault
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMCompressorNotConnectedStickyFault
  (JNIEnv * env, jclass, jint handle) {
            int32_t status = 0;
    auto retVal = HAL_GetPCMCompressorNotConnectedStickyFault(static_cast<HAL_PCMHandle>(handle), &status);
    CheckStatus(env, status);
    return retVal;
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMCompressorNotConnectedFault
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMCompressorNotConnectedFault
  (JNIEnv * env, jclass, jint handle) {
            int32_t status = 0;
    auto retVal = HAL_GetPCMCompressorNotConnectedFault(static_cast<HAL_PCMHandle>(handle), &status);
    CheckStatus(env, status);
    return retVal;
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMSolenoidBlackList
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMSolenoidBlackList
  (JNIEnv * env, jclass, jint handle) {
            int32_t status = 0;
    auto retVal = HAL_GetPCMSolenoidBlackList(static_cast<HAL_PCMHandle>(handle), &status);
    CheckStatus(env, status);
    return retVal;
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMSolenoidVoltageStickyFault
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMSolenoidVoltageStickyFault
  (JNIEnv * env, jclass, jint handle) {
            int32_t status = 0;
    auto retVal = HAL_GetPCMSolenoidVoltageStickyFault(static_cast<HAL_PCMHandle>(handle), &status);
    CheckStatus(env, status);
    return retVal;
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    getPCMSolenoidVoltageFault
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_getPCMSolenoidVoltageFault
  (JNIEnv * env, jclass, jint handle) {
            int32_t status = 0;
    auto retVal = HAL_GetPCMSolenoidVoltageFault(static_cast<HAL_PCMHandle>(handle), &status);
    CheckStatus(env, status);
    return retVal;
  }

/*
 * Class:     edu_wpi_first_wpilibj_hal_PCMJNI
 * Method:    clearAllPCMStickyFaults
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_edu_wpi_first_wpilibj_hal_PCMJNI_clearAllPCMStickyFaults
  (JNIEnv * env, jclass, jint handle) {
            int32_t status = 0;
    HAL_ClearAllPCMStickyFaults(static_cast<HAL_PCMHandle>(handle), &status);
    CheckStatus(env, status);
  }

#ifdef __cplusplus
}
#endif
