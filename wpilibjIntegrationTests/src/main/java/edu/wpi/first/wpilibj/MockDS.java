/*----------------------------------------------------------------------------*/
/* Copyright (c) 2017 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

package edu.wpi.first.wpilibj;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.SocketException;

class MockDS {
  private Thread thread;
  private final byte[] sendData = new byte[] {};

  void start() {

    thread = new Thread(() ->
    {
      DatagramSocket socket;
      try {
        socket = new DatagramSocket();
      } catch (SocketException e1) {
        // TODO Auto-generated catch block
        e1.printStackTrace();
        return;
      }
      DatagramPacket packet = new DatagramPacket(sendData, 0, 6, new InetSocketAddress("127.0.0.1", 1110));
      while(!Thread.currentThread().isInterrupted()) {
        try {
          Thread.sleep(20);
          socket.send(packet);
        } catch (InterruptedException e) {
          Thread.currentThread().interrupt();
        } catch (IOException e) {
          // TODO Auto-generated catch block
          e.printStackTrace();
        }
      }
      socket.close();
    });
    thread.start();
  }

  void stop() {
    thread.interrupt();
    try {
      thread.join(1000);
    } catch (InterruptedException e) {
      // TODO Auto-generated catch block
      e.printStackTrace();
    }
  }
}
