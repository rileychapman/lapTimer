#include <LiquidCrystal.h>
#include <SdFat.h>
const int chipSelect = 10;
SdFat sd;
SdFile lapTimes;
int lapNumber = 0;
LiquidCrystal lcd(7,6,5,4,3,2);
const int buttonPin = 8;
const int receiverPin = 9;
int buttonState = 1;
int receiverState = 1;
int firstLapStarted = 0;
int driverNumber = 1;
float startLap = 0.0;

void setup() {
  lcd.begin(16,2); // initize an lcd with specefied demensions (in characters)
  lcd.print("Start"); //print start in upper left
  lcd.setCursor(0,1); // mv cursor to second line
  lcd.print("Driver: "); //print driver on second line
  lcd.print(driverNumber);
  sd.begin(chipSelect, SPI_HALF_SPEED); //make sure the SD card in plugged in
  if(!lapTimes.open("lapTimes.txt", O_RDWR | O_CREAT | O_AT_END)) { //open the file
    lcd.setCursor(8,1);
    lcd.print("SD Error"); //does not throw an error, it just warns the driver
  }
  pinMode(buttonPin, INPUT); 
  lapTimes.println("-----New Driver-----"); //write to file that the diver has changed
  lapTimes.close(); //close the file
}

void loop() {
  float time = micros()/1000000.00 - startLap; //time since the last signal received
  buttonState = digitalRead(buttonPin); // check the state of the button and receiver
  receiverState = digitalRead(receiverPin);
  if (lapNumber > 0) { 
  lcd.setCursor(0,1);
  lcd.print(time,2);
  }
  if (receiverState == LOW || buttonState == LOW) {
    if(!lapTimes.open("lapTimes.txt", O_RDWR | O_CREAT | O_AT_END)) {
      lcd.setCursor(8,1);
      lcd.print("SD Error"); 
    }
    if (lapNumber == 0) {
      startLap = micros()/1000000.00;
      lapNumber += 1;
    }
    else {
      startLap = micros()/1000000.00;
      lapTimes.println(get_ready_to_print(lapNumber,time));
      lapNumber += 1;
    } 
    lapTimes.close();
    delay(3000);
    lcd.setCursor(0,0);
    lcd.print("Lap ");
    lcd.print(lapNumber);
     
  }
}

String get_ready_to_print(int lapNumber,float time) {
  String one = "Lap: ";
      String two = String(lapNumber);
      String three = " Time: ";
      char buffer[5];
      String four = dtostrf(time,1,4,buffer);
      String five = String(one+two+three+four);
      return five;
}

