#include <HX711_ADC.h>
#include <SoftwareSerial.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

//HX711 constructor (dout pin, sck pin):
HX711_ADC LoadCell(3, 2);
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
SoftwareSerial bluetooth(7, 6); // RX, TX

long t;
float previ=0;
int j=0;

void setup() {
  
  float calValue; // calibration value
  calValue = 19.29; // uncomment this if you want to set this value in the sketch

  lcd.init();                      // initialize the lcd 
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Configuration");
  lcd.setCursor(0,1);
  lcd.print("please wait");
  
  Serial.begin(9600); delay(10);
  Serial.println("Starting...");
  LoadCell.begin();
  long stabilisingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilising time
  LoadCell.start(stabilisingtime);
  if(LoadCell.getTareTimeoutFlag()) {
    Serial.println("Tare timeout, check MCU>HX711 wiring and pin designations");
  }
  else {
    LoadCell.setCalFactor(calValue); // set calibration value (float)
    Serial.println("Startup + tare is complete");
  }
  
  bluetooth.begin(9600); //connecting bluetooth
  while (bluetooth.available() == 0) {
    ; // wait for device to connect
  }
  lcd.setCursor(0,0);
  lcd.print("   Connected ");
  lcd.setCursor(0,1);
  lcd.print("  Ready to use");
}

void loop() {
  //update() should be called at least as often as HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS
  //use of delay in sketch will reduce effective sample rate (be carefull with use of delay() in the loop)
  LoadCell.update();
  

  //get smoothed value from data set
  if (millis() > t + 500) {
    float i = LoadCell.getData();
    Serial.print("val: ");
    Serial.println(i);
    if (i-previ <= 100 && i-previ >= (-100) && i>100)//check const measure heavier than 100g
    {
      j++;
      if (j==6)
      {
        Serial.print("Const measure: ");
        Serial.println(i);
        lcd.clear();
        lcd.setCursor(3,0);
        lcd.print(i/1000);
        lcd.print(" KG");
        bluetooth.print(i/1000);
        bluetooth.println(" KG");
        j=0;
      }
    }
    else
    {
      j=0;
    }
    previ=i;
    t = millis();
  }

  //receive from serial terminal
  if (Serial.available() > 0) {
    float i;
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

  //check if last tare operation is complete
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }

}
