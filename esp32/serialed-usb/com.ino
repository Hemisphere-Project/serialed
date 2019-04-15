
#define BYTE_START_FRAME 253
#define BYTE_STOP_FRAME 254
#define BYTE_MSG_END 255

int readSize = 0;

unsigned long lastPing = 0;

void com_init() {
  Serial.begin(BAUDRATE);
  Serial.print("\n");
  Serial.print("READY\n");
}


bool com_loop() {

  // PING
  if ((millis() - lastPing) > 1000) {
    Serial.print("WAIT\n");
    lastPing = millis();
  }

  // RECEIVE
  readSize = Serial.readBytesUntil(BYTE_MSG_END, framebuffer, BUFFERSIZE);
  if (readSize == 0) {
    Serial.print("WAIT\n");
    return false;
  }
  lastPing = millis();

  // ECHO
  // Serial.print("GOT "+String(readSize)+" bytes: ");
  // for (int k=0; k<(readSize); k+=1) {
  //   Serial.print(framebuffer[k]);
  //   Serial.print(" ");
  // }
  // Serial.print("\n");

  // CONFIRM INTEGRITY
  // Byte 0     = BYTE_START_FRAME
  // Byte N     = BYTE_STOP_FRAME
  if (framebuffer[0] == BYTE_START_FRAME) {
    if (framebuffer[readSize-1] == BYTE_STOP_FRAME) {
      if (readSize-2 == FRAMESIZE) {

          // DRAW
          Serial.print("DRAW "+String(newFrame)+"\n");
          return true;
      }
      else Serial.print("ERROR wrong frame length, expected "+String(FRAMESIZE)+" received "+String(readSize-2)+"\n");
    }
    else Serial.print("ERROR frame incomplete\n");
  }
  else Serial.print("ERROR first byte is invalid\n");

  return false;
}
