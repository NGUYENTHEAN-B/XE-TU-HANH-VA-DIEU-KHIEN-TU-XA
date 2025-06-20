//RX - NRF24
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN 8
#define CSN_PIN 9

const byte diachi[6] = "12345"; //Mảng kí tự dạng chuỗi có 6 kí tự
char nhan[10]; //30 là số kí tự trong mảng

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

//SCR
#include <Adafruit_TCS34725.h>
#include <Wire.h>
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_101MS, TCS34725_GAIN_4X);

//SERVOR
#include <Servo.h>
Servo myservo;  // create servo object to control a servo

int pos = 0;    // variable to store the servo position
int dongcoservo=23;      // chân Orange của Servo.

//SRF05
const int trig = 46; //Khai báo chân trig nối với chân số 8 là hằng số
const int echo = 47; //Khai báo chân echo nối với chân số 7 là hằng số

unsigned long thoigian; //Thời gian là kiểu số nguyên
int khoangcach; //Khoảng cách là kiểu số nguyên
int gioihan = 30;
int khoangcachtrai,khoangcachphai;

//MOTOR - L298N
int in1 = 4;
int in2 = 5;
int in3 = 6;
int in4 = 7;
int ena = 2;
int enb = 3;
int i;
int nspeed =90;
int rotaspeed = 150;


//TRACKING LINE - TCRT
int s1 = 10; //Cảm biến nối chân số 2 Arduino
int s2 = 11;
int s3 = 12;
int s4 = 13;
int s5 = 22;
int giatri1, giatri2, giatri3, giatri4, giatri5, gtcongtac, gthongngoai;

byte SpMax = 150;
int demcross = 0;
byte previousStatus = 0;
byte recentStatus  = 0;
bool newData_LINE = 0;


//JOYSTICK
int joystick[6];  // 6 element array holding Joystick readings
int speedRight = 0;
int speedLeft = 0;
int  xAxis, yAxis;
// the four button variables from joystick
int buttonUp;
int buttonRight;
int buttonDown;
int buttonLeft;
bool newData = 0;

//BUTTON CHANGE COLOR
const int buttonRed_Pin = 30;     // Nút đỏ
const int buttonGreen_Pin = 32;   // Nút xanh
const int buttonYellow_Pin = 34;  // Nút vàng
unsigned long lastButtonPress[3] = {0, 0, 0}; // Mảng cho Red, Green, Yellow
const unsigned long DEBOUNCE_DELAY = 5; // Giảm thời gian chống dội xuống 50ms

//SWITCH CHANGE MODE
const int mode_Pin = 26;

//LCD 
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

String selectedColor = "CHUA CHON";
String selectedMode = "CHUA CHON";
String selectedStatus = "KIEM TRA";
String lastDisplayedMode = "";
String lastDisplayedColor = "";

// Timing variables
unsigned long lastLCDUpdate = 0;
const unsigned long LCD_UPDATE_INTERVAL = 500;
unsigned long lastSensorRead = 0;
const unsigned long SENSOR_READ_INTERVAL = 20;
unsigned long lastServoMove = 0;
const unsigned long SERVO_MOVE_INTERVAL = 500;

void setup()
{
  Serial.begin(115200);
//CHECK STATUS
  //NRF24
      Serial.println("Đang kiểm tra module Module Điều khiển từ xa");

      if (!radio.begin()) 
    {
      Serial.println("Module không khởi động được...!!");
      selectedStatus = "LOI NRF24";
    }
      else{
      Serial.println("Module đã khởi động được...!!");
      }
    if (!radio.available())
    {
      Serial.print("CHỜ KẾT NỐI.......");
      // selectedStatus = "CHO KET NOI RX";

    }  
    else
    {    Serial.println("Đã kết nối RX");
    }

  //SCR
  Serial.println("Đang kiểm tra module Cảm biến màu");
  if (tcs.begin()) {
    Serial.println("Cảm biến màu hoạt động");
  } else {
    Serial.println("Cảm biến màu không kết nối...");
    selectedStatus = "LOI SCR";
  }

  //CA HAI
  if (radio.begin() && tcs.begin() ) {
    selectedStatus = "THANH CONG";
  } else if (!radio.begin() || !tcs.begin() ) {
    selectedStatus = "KHONG THANH CONG";
  }


//NRF
  radio.begin();
  
  radio.openReadingPipe(1, diachi);
  radio.setPALevel(RF24_PA_MIN);
  radio.setChannel(80);
  radio.setDataRate(RF24_1MBPS);
  radio.startListening();

//MOTOR - L298N
  pinMode (ena, OUTPUT);
  pinMode (in1, OUTPUT);
  pinMode (in2, OUTPUT);
  pinMode (in3, OUTPUT);
  pinMode (in4, OUTPUT);
  pinMode (enb, OUTPUT);

//TRACKING LINE - TCRT 
  pinMode(s1, INPUT); //Cảm biến nhận tín hiệu
  pinMode(s2, INPUT);
  pinMode(s3, INPUT);
  pinMode(s4, INPUT);
  pinMode(s5, INPUT);

//DETECT AVOID - SRF
  pinMode(trig, OUTPUT); //Chân trig xuất tín hiệu
  pinMode(echo, INPUT); //Chân echo nhận tín hiệu

//SRF05
  pinMode(trig, OUTPUT); //Chân trig xuất tín hiệu
  pinMode(echo, INPUT); //Chân echo nhận tín hiệu

//SERVOR
  myservo.attach(23);  // attaches the servo on pin 9 to the servo object
  myservo.write(90);    

//BUTTON CHANGE COLOR
  pinMode(buttonRed_Pin, INPUT_PULLUP);
  pinMode(buttonGreen_Pin, INPUT_PULLUP);
  pinMode(buttonYellow_Pin, INPUT_PULLUP);

//SWITCH CHANGE MOD
  pinMode(mode_Pin, INPUT);

//LCD
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("DANG KIEM TRA:");
  lcd.setCursor(0, 1);
  lcd.print(selectedStatus);

}

void loop() {
  int modeState = digitalRead(mode_Pin);

  // Ưu tiên kiểm tra và cập nhật chế độ ngay lập tức
  if (modeState == LOW) {
    selectedMode = "THU CONG";
    getData();
    if (newData) showData();
  } else {
    selectedMode = "TU DONG";
    setColor(); // Chỉ gọi setColor khi ở chế độ tự động
    if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL) {
      doline();
      if (newData_LINE) showdoline();
      cambienmau();
      tranhvatcan();
      lastSensorRead = millis();
    }
  }

  // Cập nhật LCD ngay khi selectedMode hoặc selectedColor thay đổi
  if (lastDisplayedMode != selectedMode || modeState == LOW) { // Cập nhật chế độ và xóa màu khi thủ công
    showMode();
    lastDisplayedMode = selectedMode;
  }
  if (lastDisplayedColor != selectedColor && modeState == HIGH) { // Chỉ hiển thị màu khi ở chế độ tự động
    showColor();
    lastDisplayedColor = selectedColor;
  }
  // Giữ LCD_UPDATE_INTERVAL cho các cập nhật khác (nếu cần)
  if (millis() - lastLCDUpdate >= LCD_UPDATE_INTERVAL) {
    lastLCDUpdate = millis();
  }
}

//RECEIVE DATA FROM TX - JOYSTICK
void getData(){
  if ( radio.available() )
  {

      radio.read(&joystick, sizeof(joystick) );
      xAxis = joystick[0];
      yAxis = joystick[1];
      
      // the four button variables from joystick array
      int buttonUp    = joystick[2];
      int buttonRight = joystick[3];
      int buttonDown  = joystick[4];
      int buttonLeft  = joystick[5];
      
      newData = true;
      //THẲNG PHẢI
      if ((buttonRight == LOW && buttonUp == LOW) ||(buttonRight == LOW && yAxis > 550)){
      digitalWrite (in1, HIGH);digitalWrite (in3, LOW);
      digitalWrite (in2, LOW);digitalWrite (in4, LOW);
      analogWrite (ena, nspeed);
      analogWrite (enb, nspeed);
      }
      //THẲNG TRÁI
      else if ((buttonLeft == LOW && buttonUp == LOW) || (buttonLeft == LOW && yAxis > 550)){
      digitalWrite (in1, LOW);digitalWrite (in3, HIGH);
      digitalWrite (in2, LOW);digitalWrite (in4, LOW);
      analogWrite (ena, nspeed);
      analogWrite (enb, nspeed);
      }
      //LÙI PHẢI
      else if ((buttonRight == LOW && buttonDown == LOW) || (buttonRight == LOW && yAxis < 470)){
      digitalWrite (in1, LOW);digitalWrite (in3, LOW);
      digitalWrite (in2, HIGH);digitalWrite (in4, LOW);
      analogWrite (ena, nspeed);
      analogWrite (enb, nspeed);
      }
      //LÙI TRÁI
      else if ((buttonLeft == LOW && buttonDown == LOW) || (buttonLeft == LOW && yAxis < 470)){
      digitalWrite (in1, LOW);digitalWrite (in3, LOW);
      digitalWrite (in2, LOW);digitalWrite (in4, HIGH);
      analogWrite (ena, nspeed);
      analogWrite (enb, nspeed);
      }
      //TIẾN
      else if (buttonUp == LOW || yAxis > 550){
      tien();
      }
      //LÙI
      else if (buttonDown == LOW || yAxis < 470){
      lui();
      }
      //PHẢI
      else if (buttonRight == LOW || xAxis > 550){
      phai();
      }
      //TRÁI
      else if (buttonLeft == LOW || xAxis < 470){
      trai();
      }
      else{
      dung();
      }

}
} 
//SHOW DATA FROM TX -JOYSTICK
void showData (){
    if (newData == true) {
      Serial.print("X = ");
      Serial.print(xAxis);
      Serial.print(" Y = ");  
      Serial.print(yAxis);
      Serial.print(" Up = ");
      Serial.print(joystick[2]);
      Serial.print(" Right = ");  
      Serial.print(joystick[3]);
      Serial.print(" Down = ");
      Serial.print(joystick[4]);
      Serial.print(" Left = ");  
      Serial.println(joystick[5]); 
      newData = false;
    }

}


//SHOW DATA FROM TRACKING LINE - TCRT
void showdoline()
{
  
  giatri1 = digitalRead(s1); //Đọc giá trị cảm biến s1 và gán vào biến giatri1
  giatri2 = digitalRead(s2);
  giatri3 = digitalRead(s3);
  giatri4 = digitalRead(s4);
  giatri5 = digitalRead(s5);
  Serial.print("S1: ");
  Serial.println(giatri1);Serial.print("   ");
  Serial.print("S2: ");
  Serial.println(giatri2);Serial.print("   ");
  Serial.print("S3: ");
  Serial.println(giatri3);Serial.print("   ");
  Serial.print("S4: ");
  Serial.println(giatri4);Serial.print("   ");
  Serial.print("S5: ");
  Serial.println(giatri5);Serial.print("   ");
  delay (1000);

}

//RECEIVE DAT FROM TRACKING LINE - TCRT 
void doline()
{
  giatri1 = digitalRead(s1); //Đọc giá trị cảm biến s1 và gán vào biến giatri1
  giatri2 = digitalRead(s2);
  giatri3 = digitalRead(s3);
  giatri4 = digitalRead(s4);
  giatri5 = digitalRead(s5);

  if (giatri2 == 1 && giatri3 == 0 && giatri4 ==1)
  {
    tien();
  }
  else if (giatri2 == 0 && giatri3 == 1 && giatri4 ==1)
  {
    trai();
    if (giatri1 == 0){
      trai();
    }
  }
  else if (giatri2 == 1 && giatri3 == 1 && giatri4 ==0)
  {
    phai();
    if (giatri5 == 0){
      phai();
    }

  }
  else
  {
    tien();
    static unsigned long lastStopTime = 0;
    if (millis() - lastStopTime >= 50) {
      dung();
      lastStopTime = millis();
    }
  }
}

//ACTION DETECT AVOID - SRF
void tranhvatcan ()
{
  khoangcach=0;
  dokhoangcach();
  if(khoangcach>gioihan||khoangcach==0) 
  {
    dokhoangcach();
    //KHONG CO VAT CAN PHIA TRUOC 
    if(khoangcach>gioihan||khoangcach==0)  
      {
        Serial.println("Không có vật cản phía trước");
        Serial.println("Đi thẳng");
        delay(500);
        //DI tien
        tien();
      }
  }
  else
  {
      
      dung();
      quaycbsangtrai();
      
      khoangcachtrai=khoangcach;
      Serial.print("Khoảng cách trái: ");
      Serial.print(khoangcachtrai);
      Serial.println("cm");

      quaycbsangphai();
      khoangcachphai=khoangcach;
      Serial.print("Khoảng cách phải: ");
      Serial.print(khoangcachphai);
      Serial.println("cm");

      //VAT CAN PHIA TRUOC GAN HON 10 
      if(khoangcach<10)
      {
        Serial.println("Có vật cản phía trước quá gần");
        Serial.println("Đi lùi ");
        lui();
        static unsigned long lastReverseTime = 0;
        if (millis() - lastReverseTime >= 1000) {
          lastReverseTime = millis();
        }

        quaycbsangtrai();
      
        khoangcachtrai=khoangcach;
        Serial.print("Khoảng cách trái: ");
        Serial.print(khoangcachtrai);
        Serial.println("cm");

        quaycbsangphai();
        khoangcachphai=khoangcach;
        Serial.print("Khoảng cách phải: ");
        Serial.print(khoangcachphai);
        Serial.println("cm");

        //VAT CAN BEN TRAI
        if(khoangcachphai>khoangcachtrai) 
        {
          Serial.println("Có vật cản bên trái");

          Serial.println("Sang phải ");
          phai();
          static unsigned long lastTurnTime = 0;
          if (millis() - lastTurnTime >= 500) {
            lastTurnTime = millis();
          }
          Serial.println("Đi thẳng");
          tien();
          if (millis() - lastTurnTime >= 2500) {
            lastTurnTime = millis();
          }
          Serial.println("Sang trái ");
          trai();
          if (millis() - lastTurnTime >= 3000) {
            lastTurnTime = millis();
          }
          Serial.println("Đi thẳng");
          tien();
          if (millis() - lastTurnTime >= 3000) {
            lastTurnTime = millis();
          }
          Serial.println("Sang trái ");
          trai();
          if (millis() - lastTurnTime >= 3500) {
            lastTurnTime = millis();
          }          
          Serial.println("Đi thẳng");
          tien();
          if (millis() - lastTurnTime >= 3500) {
            lastTurnTime = millis();
          }          
          Serial.println("Sang phải ");
          phai();
        }
        //VAT CAN BEN PHAI 
        if(khoangcachphai<khoangcachtrai) 
        {
          Serial.println("Có vật cản bên phải");

          Serial.println("Sang trái ");
          trai();
          static unsigned long lastTurnTime2 = 0;
          if (millis() - lastTurnTime2 >= 500) {
            lastTurnTime2 = millis();
          }

          Serial.println("Đi thẳng");
          tien();
          if (millis() - lastTurnTime2 >= 2500) {
            lastTurnTime2 = millis();
          }

          Serial.println("Sang phải ");
          phai();
          if (millis() - lastTurnTime2 >= 3000) {
            lastTurnTime2 = millis();
          }

          Serial.println("Đi thẳng");
          tien();
          if (millis() - lastTurnTime2 >= 4000) {
            lastTurnTime2 = millis();
          }

          Serial.println("Sang phải ");
          phai();
          if (millis() - lastTurnTime2 >= 4500) {
            lastTurnTime2 = millis();
          }

          Serial.println("Đi thẳng");
          tien();
          if (millis() - lastTurnTime2 >= 4500) {
            lastTurnTime2 = millis();
          }

          Serial.println("Sang trái ");
          trai();
          if (millis() - lastTurnTime2 >= 5000) {
            lastTurnTime2 = millis();
          }

        }
      }
      else
      {
        //VAT CAN BEN TRAI
        if(khoangcachphai>khoangcachtrai) 
        {
          Serial.println("Có vật cản bên trái");

          Serial.println("Sang phải ");
          phai();
          static unsigned long lastTurnTime3 = 0;
          if (millis() - lastTurnTime3 >= 500) {
            lastTurnTime3 = millis();
          }  

          Serial.println("Đi thẳng");
          tien();
          if (millis() - lastTurnTime3 >= 2500) {
            lastTurnTime3 = millis();
          }
          
          Serial.println("Sang trái ");
          trai();
          if (millis() - lastTurnTime3 >= 3000) {
            lastTurnTime3 = millis();
          }

          Serial.println("Đi thẳng");
          tien();
          if (millis() - lastTurnTime3 >= 3000) {
            lastTurnTime3 = millis();
          }

          Serial.println("Sang trái ");
          trai();
          if (millis() - lastTurnTime3 >= 3500) {
            lastTurnTime3 = millis();
          }

          Serial.println("Đi thẳng");
          tien();
          if (millis() - lastTurnTime3 >= 3500) {
            lastTurnTime3 = millis();
          }

          Serial.println("Sang phải ");
          phai();
          if (millis() - lastTurnTime3 >= 4000) {
            lastTurnTime3 = millis();
          }

        }
        //VAT CAN BEN PHAI 
        if(khoangcachphai<khoangcachtrai) 
        {
          Serial.println("Có vật cản bên phải");
          
          Serial.println("Sang trái ");
          trai();
          static unsigned long lastTurnTime4 = 0;
          if (millis() - lastTurnTime4 >= 500) {
            lastTurnTime4 = millis();
          }
          
          Serial.println("Đi thẳng");
          tien();
          if (millis() - lastTurnTime4 >= 2500) {
            lastTurnTime4 = millis();
          }
          
          Serial.println("Sang phải ");
          phai();
          if (millis() - lastTurnTime4 >= 3000) {
            lastTurnTime4 = millis();
          }

          Serial.println("Đi thẳng");
          tien();
          if (millis() - lastTurnTime4 >= 3000) {
            lastTurnTime4 = millis();
          }

          Serial.println("Sang phải ");
          phai();
          if (millis() - lastTurnTime4 >= 3500) {
            lastTurnTime4 = millis();
          }

          Serial.println("Đi thẳng");
          tien();
          if (millis() - lastTurnTime4 >= 3500) {
            lastTurnTime4 = millis();
          }

          Serial.println("Sang trái ");
          trai();
          if (millis() - lastTurnTime4 >= 4000) {
            lastTurnTime4 = millis();
          }

        }
      }

  } 
}

//CHECK DETECT AVOID - SRF
void dokhoangcach()
{
  //Phát xung từ chân trig, có độ rộng là 10ms
  digitalWrite(trig,0); //Tắt chân trig
  delayMicroseconds(2); 
  digitalWrite(trig,1); //bật chân trig để phát xung
  delayMicroseconds(10); //Xung có độ rộng là 10 microsecond
  digitalWrite(trig,0);

  //Chân echo sẽ nhận xung phản xạ lại
  //Và đo độ rộng xung cao ở chân echo
  thoigian = pulseIn (echo, HIGH);
  
  //Tính khoảng cách đến vật thể (Đơn vị đo là cm)
  //Tốc độ của âm thanh trong không khí là 340 m/s, tương đương 29,412 microSeconds/cm,
  //Do thời gian được tính từ lúc phát tín hiệu tới khi sóng âm phản xạ lại,
  //vì vậy phải chia cho 2
  khoangcach = int (thoigian / 2 / 29.412); 

  //In lên Serial kết quả
  Serial.print("Khoảng cách: ");
  Serial.print(khoangcach);
  Serial.println("cm");
  delay(500);
}

//DETECT COLOR
void cambienmau()
{
  
    uint16_t r, g, b, c, colorTemp, lux;
     
    tcs.getRawData(&r, &g, &b, &c);
    
    colorTemp = tcs.calculateColorTemperature(r, g, b); 
    
    lux = tcs.calculateLux(r, g, b); 
     
    Serial.print("Color Temp: "); Serial.print(colorTemp); Serial.print(" K - ");
    Serial.print("Lux: "); Serial.print(lux); Serial.print(" - ");
    Serial.print("Red: "); Serial.print(r); Serial.print(" ");
    Serial.print("Green: "); Serial.print(g); Serial.print(" ");
    Serial.print("Blue: "); Serial.print(b); Serial.print(" ");
    Serial.print("Clear: "); Serial.print(c); Serial.print(" ");
    Serial.println(" ");
    
    if(c>r && c>g && c>b && lux>500) //Không có màu
    {
      Serial.println(" Màu sắc hiện tại là: KHÔNG CÓ MÀU");
      Serial.println(" ");
      doline();
    }
    if(r>g && r>b && r<c && lux>10 && lux<100) //Màu đỏ
    {
        Serial.println(" Màu sắc hiện tại là: MÀU ĐỎ");
        Serial.println(" ");  
      if (selectedColor == "DO") {
        dung(); // Dừng khi màu đỏ được chọn và phát hiện
      } else {
        doline(); // Tiếp tục nếu không khớp
      }
    }
    else if(g>r && g>b && g<c && lux>180 && lux<350) //Màu lục
     {
      Serial.println(" Màu sắc hiện tại là: MÀU LỤC");
      Serial.println(" ");         
      if (selectedColor == "LUC") {
        dung(); // Dừng khi màu đỏ được chọn và phát hiện
      } else {
        doline(); // Tiếp tục nếu không khớp
      }
     }
     else if(b>r && b>g && b<c && lux>100 && lux<200) //Màu xanh biển
     {
      Serial.println(" Màu sắc hiện tại là: MÀU XANH BIỂN");
      Serial.println(" ");        
      if (selectedColor == "XANH") {
        dung(); // Dừng khi màu đỏ được chọn và phát hiện
      } else {
        doline(); // Tiếp tục nếu không khớp
      }
     }
     else {
        doline();
     }    
}

//ACTION
void tien()
{
  analogWrite (ena, nspeed);
  analogWrite (enb, nspeed);
  digitalWrite (in2, HIGH);digitalWrite (in4, HIGH);
  digitalWrite (in1, LOW);digitalWrite (in3, LOW);
}

void lui()
{
  analogWrite (ena, nspeed);
  analogWrite (enb, nspeed);
  digitalWrite (in1, HIGH);digitalWrite (in3, HIGH);
  digitalWrite (in2, LOW);digitalWrite (in4, LOW);
}

void trai()
{
  analogWrite (ena, rotaspeed);
  analogWrite (enb, rotaspeed);
  digitalWrite (in1, LOW);digitalWrite (in3, LOW);
  digitalWrite (in2, HIGH);digitalWrite (in4, LOW);
}

void phai()
{
  analogWrite (ena, rotaspeed);
  analogWrite (enb, rotaspeed);
  digitalWrite (in1, LOW);digitalWrite (in3, LOW);
  digitalWrite (in2, LOW);digitalWrite (in4, HIGH);
}

void dung()
{
  digitalWrite (in1, LOW);digitalWrite (in3, LOW);
  digitalWrite (in2, LOW);digitalWrite (in4, LOW);

}


//QUAY SERVO SANG TRAI
void quaycbsangtrai()
{
    myservo.write(180);              // tell servo to go to position in variable 'pos'
    delay(1000);
    dokhoangcach();
    myservo.write(90);              // tell servo to go to position in variable 'pos'  
}

//QUAY SERVO SANG PHAI
void quaycbsangphai()
{
    myservo.write(0);              // tell servo to go to position in variable 'pos'
    delay(1000);
    dokhoangcach();
    myservo.write(90);              // tell servo to go to position in variable 'pos'
}

//QUAY SERVO VE GIUA
void resetservo()
{
   myservo.write(90);
}

//LCD _ HIEN MAU 
void showColor() {
  lcd.setCursor(0, 1);
  lcd.print("MAU:            "); // Ghi đè với khoảng trắng để xóa dữ liệu cũ
  lcd.setCursor(5, 1);
  lcd.print(selectedColor);
}

// Định nghĩa hàm showMode
void showMode() {
  lcd.setCursor(0, 0);
  lcd.print("CHE DO:         "); // Ghi đè với khoảng trắng để xóa dữ liệu cũ
  lcd.setCursor(8, 0);
  lcd.print(selectedMode);
  if (selectedMode == "THU CONG") {
    lcd.setCursor(0, 1);
    lcd.print("                "); // Xóa dòng thứ hai khi ở chế độ thủ công
  }
}

//BUTTON_CHON MAU
void setColor() {
  if (digitalRead(buttonRed_Pin) == LOW && millis() - lastButtonPress[0] >= DEBOUNCE_DELAY) {
    selectedColor = "DO";
    lastButtonPress[0] = millis();
  }
  if (digitalRead(buttonGreen_Pin) == LOW && millis() - lastButtonPress[1] >= DEBOUNCE_DELAY) {
    selectedColor = "XANH";
    lastButtonPress[1] = millis();
  }
  if (digitalRead(buttonYellow_Pin) == LOW && millis() - lastButtonPress[2] >= DEBOUNCE_DELAY) {
    selectedColor = "LUC";
    lastButtonPress[2] = millis();
  }
}

