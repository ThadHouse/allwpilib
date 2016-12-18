#include <mcp_can.h>
#include <SPI.h>
#include <Wire.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];


volatile unsigned char solenoids[64];
volatile unsigned char compressor[64];

volatile bool requestedPcmReset[64];
volatile bool requestedPdpReset[64];
volatile bool requestedPdpEnergyReset[64];



#define CAN0_INT 7                              // Set INT to pin 2

  

MCP_CAN CAN0(10);     // Set CS to pin 10

void setup()
{


   Wire.begin(4);
  Wire.onReceive(receiveI2C);
  Wire.onRequest(requestI2C);

    Serial.begin(57600);
  Serial1.begin(57600);

  for (int i = 0; i < 64; i++)
  {
    solenoids[i] = 0;
    compressor[i] = 0;
    requestedPcmReset[i] = 0;
    requestedPdpReset[i] = 0;
    requestedPdpEnergyReset[i] = 0;
  }

  //while (!Serial);

  CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ);

  CAN0.setMode(MCP_NORMAL);  

  pinMode(CAN0_INT, INPUT);
}

void loop()
{
  if(!digitalRead(CAN0_INT))                     
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);   

    if (((rxId & 0x1FFFFFFF) & 0x09041C00) == 0x09041C00)
    {
      // Got a Solenoid Message

      int packet = rxId & 0xC0;
      int canID = rxId & 0x3F;
      //Serial.println(rxId);
      //Serial.println(canID);

      switch(packet)
      {
        case 0x00:
          // Control1 
          // Byte 2 solenoid bits
          // Byte 3 compressor
          // byte 4 oneshot
          // byte 5 oneshot 2
          sprintf(msgString, "PCM ID: %d SD: 0x%.2X CP 0x%.2X ", canID, rxBuf[2], rxBuf[3]);
          solenoids[canID] = rxBuf[2];
          compressor[canID] = rxBuf[3];
          Serial.println(msgString);
          break;
        case 0x40:
          // Control2
          // ResetStickyFaults
          requestedPcmReset[canID] = true;
          sprintf(msgString, "PCM ID: %d requested reset", canID);
          Serial.println(msgString);
          break;
        case 0x80:
          // Control3
          // OneShot data
          break;
      }
    }
    else if (((rxId & 0x1FFFFFFF) & 0x08041C00) == 0x08041C00)
    {
      // Got a PDP message

      // Only 1 packet exists

      int canID = rxId & 0x3F;
      
      if (rxBuf[0] == 0x80)
      {
        // Reset Sticky Faults
        requestedPdpReset[canID] = true;
          sprintf(msgString, "PDP ID: %d requested reset", canID);
          Serial.println(msgString);
      }
      else if (rxBuf[0] == 0x40)
      {
        // Reset Energy
        requestedPdpEnergyReset[canID] = true;
          sprintf(msgString, "PDP ID: %d requested energy reset", canID);
          Serial.println(msgString);
      }
    }
    return;
  }
}

volatile char i2cRecBuf[128];
volatile int lastCount = 0;

void receiveI2C(int count)
{
  Serial.println("Received I2C");
  int i = 0;
  while (Wire.available())
  {
    char c = Wire.read();
    i2cRecBuf[i] = c;
    i++;
  }
  lastCount = count;

  if (i2cRecBuf[0] == '9') // Requested from I2C Loop Test
  {
    Serial1.write((char*)i2cRecBuf, count);
    Serial1.flush();
  }
  else if (i2cRecBuf[0] == '1')
  {
    // 1 means requesting latest solenoid buffer
    // 2nd byte is solenoid ID
    i2cRecBuf[2] = solenoids[i2cRecBuf[1]];
    lastCount = 3;
    Serial.println("Writing Solenoid Buffer");
  }
  else if (i2cRecBuf[0] == '2')
  {
    // 1 means requesting latest compressor buffer
    // 2nd byte is solenoid ID
    i2cRecBuf[2] = compressor[i2cRecBuf[1]];
    lastCount = 3;
  }
  else if (i2cRecBuf[0] == '3')
  {
    // 1 means requesting if pcm reset
    // 2nd byte is solenoid ID
    i2cRecBuf[2] = requestedPcmReset[i2cRecBuf[1]];
    requestedPcmReset[i2cRecBuf[1]] = false;
    lastCount = 3;
  }

  else if (i2cRecBuf[0] == '4')
  {
    // 1 means requesting if pdp reset
    // 2nd byte is solenoid ID
    i2cRecBuf[2] = requestedPdpReset[i2cRecBuf[1]];
    requestedPdpReset[i2cRecBuf[1]] = false;
    lastCount = 3;
  }

  else if (i2cRecBuf[0] == '5')
  {
    // 1 means requesting if pdp energy reset
    // 2nd byte is solenoid ID
    i2cRecBuf[2] = requestedPdpEnergyReset[i2cRecBuf[1]];
    requestedPdpEnergyReset[i2cRecBuf[1]] = false;
    lastCount = 3;
  }
  else if (i2cRecBuf[0] = 'T')
  {
    // Write buffer out to CAN
    // First 4 bytes are ID
    INT32U id = 0;
    memcpy(&id, &(i2cRecBuf[2]), 4);
    Serial.println(id, 1);
    CAN0.sendMsgBuf(id, count - 6, &(i2cRecBuf[6]));
  }
}

void requestI2C()
{
  Wire.write((char*)i2cRecBuf, lastCount);
}
