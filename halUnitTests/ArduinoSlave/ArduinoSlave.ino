#include <Wire.h>
#include <SPI.h>

char* spiBuf;
volatile int spiCount = 0;
volatile int currentSPIMax = 0;
volatile bool spiHasFinished = false;

void setup() {
  // put your setup code here, to run once:
  Wire.begin(4);
  Wire.onReceive(receiveI2C);
  Wire.onRequest(requestI2C);
  Serial.begin(57600);

  spiBuf = malloc(16);

  // Set MISO to input
  pinMode(MISO, OUTPUT);

  // Set SPI to slave mode
  SPCR |= _BV(SPE);

  // Enable SPI interrupts
  SPI.attachInterrupt();
}

// SPI Interrupt
ISR (SPI_STC_vect)
{
  char data = SPDR;

  if (spiHasFinished) return;

   

  spiBuf[spiCount] = data;
  spiCount++;

 if (data == '\n') 
  {
    spiHasFinished = true;
    return;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (spiHasFinished)
  {
    if (spiBuf[0] == 1 && spiBuf[1] == 2)
    {
      // Transaction
      // Send response test string to Master
      char c = 0;
      for (const char * p = "Welcome Back!" ; c = *p; p++)
        SPDR=c; SPI.transfer(c);
      }
    else
    {
      // Just Read
      Serial.write(spiBuf, spiCount);
      Serial.flush();
    }
    spiCount = 0;
    spiHasFinished = 0;
  }
  delay(50);
}

void receiveI2C(int count)
{
  char* buf = new char[count];
  int i = 0;
  while (Wire.available())
  {
    char c = Wire.read();
    buf[i] = c;
    i++;
  }

  Serial.write(buf, count);
  Serial.flush();
}

void requestI2C()
{
  Wire.write("Welcome Back!");
}

