#include <SoftwareSerial.h>
const int SSERIAL_RX_PIN = 10;   //Soft Serial Receive pin
const int SSERIAL_TX_PIN = 11;   //Soft Serial Transmit pin
const int SSERIAL_CTRL_PIN = 3;  //RS485 Direction control
const int RS485_TRANSMIT = HIGH;
const int RS485_RECEIVE = LOW;

const int BTN_START_PIN = 4;         // Button Start motor
const int BTN_STOP_PIN = 5;          // Button Stop motor
const int BTN_RPM_INCREASE_PIN = 7;  // Button Increase RPM
const int BTN_RPM_DECREASE_PIN = 6;  // Button Decrease RPM

SoftwareSerial RS485Serial(SSERIAL_RX_PIN, SSERIAL_TX_PIN);  // RX, TX

int byteReceived;

int rpmSpeed = 1000;
bool canPress = true;

void setup() {
  Serial.begin(9600);

  pinMode(BTN_START_PIN, INPUT);
  pinMode(BTN_STOP_PIN, INPUT);
  pinMode(BTN_RPM_INCREASE_PIN, INPUT);
  pinMode(BTN_RPM_DECREASE_PIN, INPUT);

  pinMode(SSERIAL_CTRL_PIN, OUTPUT);

  digitalWrite(SSERIAL_CTRL_PIN, RS485_RECEIVE);  // Put RS485 in receive mode

  RS485Serial.begin(115200);  // Start the RS485 soft serial port
}
//===============================================================================
//  Main
//===============================================================================
void loop() {

  int btnMotorStart = digitalRead(BTN_START_PIN);
  int btnMotorStop = digitalRead(BTN_STOP_PIN);
  int btnRpmIncrease = digitalRead(BTN_RPM_INCREASE_PIN);
  int btnRpmDecrease = digitalRead(BTN_RPM_DECREASE_PIN);

  if (btnMotorStart == HIGH && canPress) {
    canPress = false;
    Serial.println("PRESS START");
    const uint8_t command[] = { 0x01, 0x10, 0x01, 0x31, 0x00, 0x01, 0x02, 0x00, 0x0F };
    send_frame(command, sizeof(command));
  }

  if (btnMotorStop == HIGH && canPress) {
    canPress = false;
    Serial.println("PRESS STOP");
    const uint8_t command[] = { 0x01, 0x10, 0x01, 0x31, 0x00, 0x01, 0x02, 0x00, 0x0E };
    send_frame(command, sizeof(command));
  }

  if (btnRpmIncrease == HIGH && canPress) {
    canPress = false;
    rpmSpeed += 100;

    if (rpmSpeed > 6000) {
      rpmSpeed = 6000;
    }

    Serial.print("SET RPM: ");
    Serial.println(rpmSpeed);
    setRPM();
  }

  if (btnRpmDecrease == HIGH && canPress) {
    canPress = false;
    rpmSpeed -= 100;

    if (rpmSpeed < 1000) {
      rpmSpeed = 1000;
    }

    Serial.print("SET RPM: ");
    Serial.println(rpmSpeed);
    setRPM();
  }

  if (btnMotorStart == LOW && btnMotorStop == LOW && btnRpmIncrease == LOW && btnRpmDecrease == LOW) {
    canPress = true;
  }

  if (RS485Serial.available()) {
    byteReceived = RS485Serial.read();
    Serial.write(byteReceived);
    delay(10);
  }
}

void setRPM() {
  uint8_t command[9];
  command[0] = { 0x01 };
  command[1] = { 0x10 };
  command[2] = { 0x00 };
  command[3] = { 0x6A };
  command[4] = { 0x00 };
  command[5] = { 0x01 };
  command[6] = { 0x02 };
  command[7] = { rpmSpeed >> 8 };
  command[8] = { rpmSpeed };

  send_frame(command, sizeof(command));

  Serial.println("SAVE PARAMETARS");
  const uint8_t commandForSave[] = { 0x01, 0x10, 0x01, 0x31, 0x00, 0x01, 0x02, 0x00, 0x23 };
  send_frame(commandForSave, sizeof(commandForSave));
}

uint16_t crc16(const uint8_t *buf, uint8_t len) {
  uint16_t crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos];
    for (int i = 8; i != 0; i--) {
      if ((crc & 0x0001) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

uint16_t send_frame(const uint8_t *buf, uint8_t len) {
  digitalWrite(SSERIAL_CTRL_PIN, RS485_TRANSMIT);

  // compute CRC
  int16_t crc = crc16(buf, len);
  // send frame body and CRC
  RS485Serial.write(buf, len);
  RS485Serial.write(crc & 0xff);
  RS485Serial.write(crc >> 8 & 0xff);

  delay(1);
  digitalWrite(SSERIAL_CTRL_PIN, RS485_RECEIVE);
}
