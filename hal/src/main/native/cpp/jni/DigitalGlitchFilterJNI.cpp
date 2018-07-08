/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include <jni.h>

#include "HAL/DIO.h"
#include "HALUtil.h"
#include "edu_wpi_first_wpilibj_hal_DigitalGlitchFilterJNI.h"

using namespace frc;

/*
 * Class:     edu_wpi_first_wpilibj_hal_DigitalGlitchFilterJNI
 * Method:    createFilterForDIO
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_edu_wpi_first_wpilibj_hal_DigitalGlitchFilterJNI_createFilterForDIO
  (JNIEnv*, jclass)
{
  int32_t status = 0;

  HAL_CreateFilterForDIO(static_cast<HAL_DigitalHandle>(handle), &status);
  CheckStatus(env, status);
}

/*
 * Class:     edu_wpi_first_wpilibj_hal_DigitalGlitchFilterJNI
 * Method:    cleanFilter
 * Signature: (I)V
 */
JNIEXPORT void JNICALL
Java_edu_wpi_first_wpilibj_hal_DigitalGlitchFilterJNI_cleanFilter
  (JNIEnv*, jclass, jint handle)
{
  HAL_CleanFilter(handle);
}

/*
 * Class:     edu_wpi_first_wpilibj_hal_DigitalGlitchFilterJNI
 * Method:    setFilterPeriod
 * Signature: (II)V
 */
JNIEXPORT void JNICALL
Java_edu_wpi_first_wpilibj_hal_DigitalGlitchFilterJNI_setFilterPeriod
  (JNIEnv* env, jclass, jint filter_handle, jint fpga_cycles)
{
  int32_t status = 0;

  HAL_SetFilterPeriod(filter_handle, fpga_cycles, &status);
  CheckStatus(env, status);
}

/*
 * Class:     edu_wpi_first_wpilibj_hal_DigitalGlitchFilterJNI
 * Method:    getFilterPeriod
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL
Java_edu_wpi_first_wpilibj_hal_DigitalGlitchFilterJNI_getFilterPeriod
  (JNIEnv* env, jclass, jint filter_handle)
{
  int32_t status = 0;

  jint result = HAL_GetFilterPeriod(filter_handle, &status);
  CheckStatus(env, status);
  return result;
}
