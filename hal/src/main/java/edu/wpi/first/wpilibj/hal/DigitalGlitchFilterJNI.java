/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

package edu.wpi.first.wpilibj.hal;

public class DigitalGlitchFilterJNI extends JNIWrapper {
  public static native int createFilterForDIO(int digitalPortHandle);

  public static native void cleanFilter(int filterHandle);

  public static native void setFilterPeriod(int filterHandle, int fpgaCycles);

  public static native int getFilterPeriod(int filterHandle);
}
