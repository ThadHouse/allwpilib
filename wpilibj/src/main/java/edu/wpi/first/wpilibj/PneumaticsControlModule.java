package edu.wpi.first.wpilibj;

import edu.wpi.first.wpilibj.hal.HAL;
import edu.wpi.first.wpilibj.hal.PCMJNI;
import edu.wpi.first.wpilibj.hal.FRCNetComm.tResourceType;

public class PneumaticsControlModule implements AutoCloseable {
  private int m_handle;

  public PneumaticsControlModule() {
    this(SensorUtil.getDefaultSolenoidModule());
  }

  public PneumaticsControlModule(final int module) {
    SensorUtil.checkSolenoidModule(module);
    m_handle = PCMJNI.getPCMHandle(module);

    HAL.report(tResourceType.kResourceType_PCM, module);
  }

  public void close() {
    if(m_handle != 0) {
      PCMJNI.freePCMHandle(m_handle);
    }
  }

  /**
   * Read all 8 solenoids from the module used by this solenoid as a single byte.
   *
   * @return The current value of all 8 solenoids on this module.
   */
  public int getAll() {
    return PCMJNI.getPCMAllSolenoids(m_handle);
  }

  /**
   * Reads complete solenoid blacklist for all 8 solenoids as a single byte. If a solenoid is
   * shorted, it is added to the blacklist and disabled until power cycle, or until faults are
   * cleared.
   *
   * @return The solenoid blacklist of all 8 solenoids on the module.
   * @see #clearAllPCMStickyFaults()
   */
  public int getPCMSolenoidBlackList() {
    return PCMJNI.getPCMSolenoidBlackList(m_handle);
  }

  /**
   * If true, the common highside solenoid voltage rail is too low, most likely a solenoid channel
   * is shorted.
   *
   * @return true if PCM sticky fault is set
   */
  public boolean getPCMSolenoidVoltageStickyFault() {
    return PCMJNI.getPCMSolenoidVoltageStickyFault(m_handle);
  }

  /**
   * The common highside solenoid voltage rail is too low, most likely a solenoid channel is
   * shorted.
   *
   * @return true if PCM is in fault state.
   */
  public boolean getPCMSolenoidVoltageFault() {
    return PCMJNI.getPCMSolenoidVoltageFault(m_handle);
  }

  /**
   * Clear ALL sticky faults inside PCM that Compressor is wired to.
   *
   * <p>If a sticky fault is set, then it will be persistently cleared. Compressor drive maybe
   * momentarily disable while flags are being cleared. Care should be taken to not call this too
   * frequently, otherwise normal compressor functionality may be prevented.
   *
   * <p>If no sticky faults are set then this call will have no effect.
   */
  public void clearAllPCMStickyFaults() {
    PCMJNI.clearAllPCMStickyFaults(m_handle);
  }
}
