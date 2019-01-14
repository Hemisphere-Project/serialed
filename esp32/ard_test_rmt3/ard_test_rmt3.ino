#include <Arduino.h>

#define BAUDRATE 12000000

int readSize = 0;
unsigned long lastPing = 0;

void setup()
{
  // disable BT
  btStop();

  // UPLINK
  Serial.begin(BAUDRATE);
  Serial.print("\n");

  // READY
  Serial.print("READY\n");
}

#define SIZE 2048
byte chunk[SIZE];
int panelCounter = 0;
bool panelRecv = false;

void loop()
{

  // Ping
  if ((millis() - lastPing) > 1000) {
    Serial.print("WAIT\n");
    lastPing = millis();
  }

  // Uplink RECV
  readSize = Serial.readBytesUntil(255, chunk, SIZE);
  if (readSize == 0) return;
  // else Serial.print("GOT "+String(readSize)+" last: "+String(chunk[readSize-1])+"\n");
  lastPing = millis();

  // PANEL START
  if (!panelRecv && readSize >= 3 && chunk[0] == 250 && chunk[2] == 1) panelRecv = true;

  // PANEL STOP
  else if (panelRecv && chunk[readSize-1] == 253) {
    panelCounter+=1;
    panelRecv = false;
  }
  
  // DRAW
  else if (chunk[readSize-1] == 254) {
    Serial.print("DRAW "+String(panelCounter)+"\n");
    panelCounter=0;
  }


}
