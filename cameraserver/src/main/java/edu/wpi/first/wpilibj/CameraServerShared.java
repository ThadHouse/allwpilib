/*----------------------------------------------------------------------------*/
/* Copyright (c) 2018 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

package edu.wpi.first.wpilibj;

import edu.wpi.first.wpilibj.sharedinterfaces.DriverStationErrorFunc;
import edu.wpi.first.wpilibj.sharedinterfaces.ReportFunc;
import edu.wpi.first.wpilibj.sharedinterfaces.RobotMainThreadIdFunc;

public class CameraServerShared {
  private static CameraServerShared cameraServerShared = null;

  /**
   * get the CameraServerShared object.
   */
  public static synchronized CameraServerShared getCameraServerShared() {
    if (cameraServerShared == null) {
      cameraServerShared = new CameraServerShared();
      cameraServerShared.m_reportAxisCamera = (int id) -> { };
      cameraServerShared.m_reportUsbCamera = (int id) -> { };
      cameraServerShared.m_reportVideoServer = (int id) -> { };
      cameraServerShared.m_reportDriverStationError = (String err) -> { };
      cameraServerShared.m_robotMainThreadId = () -> {
        return null;
      };
    }
    return cameraServerShared;
  }

  /**
   * set the CameraServerShared object.
   */
  public static synchronized void setCameraServerShared(CameraServerShared shared) {
    cameraServerShared = shared;
  }

  private ReportFunc m_reportUsbCamera;
  private ReportFunc m_reportAxisCamera;
  private ReportFunc m_reportVideoServer;
  private DriverStationErrorFunc m_reportDriverStationError;
  private RobotMainThreadIdFunc m_robotMainThreadId;

  /**
   * get the main thread id func.
   *
   * @return the robotMainThreadId
   */
  public RobotMainThreadIdFunc getRobotMainThreadId() {
    return m_robotMainThreadId;
  }

  /**
   * set the main thread id func.
   *
   * @param robotMainThreadId the robotMainThreadId to set
   */
  public void setRobotMainThreadId(RobotMainThreadIdFunc robotMainThreadId) {
    this.m_robotMainThreadId = robotMainThreadId;
  }

  /**
   * get the reportDriverStationError func.
   *
   * @return the reportDriverStationError
   */
  public DriverStationErrorFunc getReportDriverStationError() {
    return m_reportDriverStationError;
  }

  /**
   * set the reportDriverStationError func.
   *
   * @param reportDriverStationError the reportDriverStationError to set
   */
  public void setReportDriverStationError(DriverStationErrorFunc reportDriverStationError) {
    this.m_reportDriverStationError = reportDriverStationError;
  }

  /**
   * get the reportVideoServer func.
   *
   * @return the reportVideoServer
   */
  public ReportFunc getReportVideoServer() {
    return m_reportVideoServer;
  }

  /**
   * set the reportVideoServer func.
   *
   * @param reportVideoServer the reportVideoServer to set
   */
  public void setReportVideoServer(ReportFunc reportVideoServer) {
    this.m_reportVideoServer = reportVideoServer;
  }

  /**
   * get the reportAxisCamera func.
   *
   * @return the reportAxisCamera
   */
  public ReportFunc getReportAxisCamera() {
    return m_reportAxisCamera;
  }

  /**
   * set the reportAxisCamera func.
   *
   * @param reportAxisCamera the reportAxisCamera to set
   */
  public void setReportAxisCamera(ReportFunc reportAxisCamera) {
    this.m_reportAxisCamera = reportAxisCamera;
  }

  /**
   * get the reportUsbCamera func.
   *
   * @return the reportUsbCamera
   */
  public ReportFunc getReportUsbCamera() {
    return m_reportUsbCamera;
  }

  /**
   * set the reportUsbCamera func.
   *
   * @param reportUsbCamera the reportUsbCamera to set
   */
  public void setReportUsbCamera(ReportFunc reportUsbCamera) {
    this.m_reportUsbCamera = reportUsbCamera;
  }
}
