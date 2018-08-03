package edu.wpi.first.wpilibj.hal;

@SuppressWarnings("AbbreviationAsWordInName")
public class PCMJNI extends JNIWrapper {
  public static native int getPCMHandle(int module);

  public static native void freePCMHandle(int handle);

  public static native boolean checkPCMChannel(int channel);

  public static native boolean checkPCMModule(int module);

  public static native double getPCMInputVoltage(int handle);

  public static native double getPCMSolenoidVoltage(int handle);

  public static native boolean getPCMSolenoid(int handle, int index);

  public static native int getPCMAllSolenoids(int handle);

  public static native void setPCMSolenoid(int handle, int index, boolean value);

  public static native void setPCMAllSolenoids(int handle, int values);

  public static native void setPCMOneShotDuration(int handle, int index, int durMs);

  public static native void firePCMOneShot(int handle, int index);

  public static native boolean getPCMCompressorOn(int handle);

  public static native void setPCMCompressorClosedLoopControl(int handle, boolean value);

  public static native boolean getPCMCompressorClosedLoopControl(int handle);

  public static native boolean getPCMCompressorPressureSwitch(int handle);

  public static native double getPCMCompressorCurrent(int handle);

  public static native boolean getPCMCompressorCurrentTooHighFault(int handle);

  public static native boolean getPCMCompressorCurrentTooHighStickyFault(int handle);

  public static native boolean getPCMCompressorShortedStickyFault(int handle);

  public static native boolean getPCMCompressorShortedFault(int handle);

  public static native boolean getPCMCompressorNotConnectedStickyFault(int handle);

  public static native boolean getPCMCompressorNotConnectedFault(int handle);

  public static native int getPCMSolenoidBlackList(int handle);

  public static native boolean getPCMSolenoidVoltageStickyFault(int handle);

  public static native boolean getPCMSolenoidVoltageFault(int handle);

  public static native void clearAllPCMStickyFaults(int handle);
}
