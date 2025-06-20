#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define CE_PIN   9
#define CSN_PIN 10
#define xAxis 14
#define yAxis 15

const byte diachi[6] = "12345";

RF24 radio(CE_PIN, CSN_PIN);
unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 100;

int joystick[6];

int buttonUp    = 2;
int buttonRight = 3;
int buttonDown  = 4;
int buttonLeft  = 5;

void setup() {
  Serial.begin(9600);
  radio.begin();

  if (!radio.begin()) {
    Serial.println("Module không khởi động được...!!");
    while (1) {}
  } else {
    Serial.println("Module đã khởi động được...!!");
  }

  radio.openWritingPipe(diachi);
  radio.setPALevel(RF24_PA_MIN);
  radio.setChannel(80);
  radio.setDataRate(RF24_1MBPS);
  radio.stopListening();

  if (!radio.available()) {
    Serial.println("Chưa kết nối được với RX...!!");
    Serial.print("CHỜ KẾT NỐI.......");
  } else {
    Serial.println("Đã kết nối RX");
  }

  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonRight, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);
  pinMode(buttonLeft, INPUT_PULLUP);

  digitalWrite(buttonUp, LOW);
  digitalWrite(buttonRight, LOW);
  digitalWrite(buttonDown, LOW);
  digitalWrite(buttonLeft, LOW);
}

void loop() {
  currentMillis = millis();
  if (currentMillis - prevMillis >= txIntervalMillis) {
    send();
    prevMillis = millis();
  }

  joystick[0] = analogRead(xAxis);
  joystick[1] = analogRead(yAxis);
  joystick[2] = digitalRead(buttonUp);
  joystick[3] = digitalRead(buttonRight);
  joystick[4] = digitalRead(buttonDown);
  joystick[5] = digitalRead(buttonLeft);
}

void send() {
  radio.write(&joystick, sizeof(joystick));

  Serial.print("X = ");
  Serial.print(analogRead(xAxis));
  Serial.print(" Y = ");
  Serial.print(analogRead(yAxis));
  Serial.print(" Up = ");
  Serial.print(digitalRead(buttonUp));
  Serial.print(" Right = ");
  Serial.print(digitalRead(buttonRight));
  Serial.print(" Down = ");
  Serial.print(digitalRead(buttonDown));
  Serial.print(" Left = ");
  Serial.println(digitalRead(buttonLeft));
}
