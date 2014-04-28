
// Arduino sketch for the lap timer build for the REVO electic go-kart
// Author: Riley Chapman

#include <LiquidCrystal.h>
#include <SdFat.h>
const int chipSelect = 10;
SdFat sd;
SdFile lapTimes;
int lapNumber = 0;
LiquidCrystal lcd(7,6,5,4,3,2);
const int buttonPin = 8;
const int receiverPin = 9;
int buttonState = HIGH;
int lastButtonState = HIGH;
int firstLapStarted = LOW;
int driverNumber = 1;
float startLap = 0.0;
float lastReceiverTrigger = 10000000; //bigger than necessary so it will trigegr the first time
int receiverTriggerDelay = 3000; // milliseconds (3 seconds) the minnimum lap time
long lastDebounceTime = 0;
long debounceDelay = 50; // increase if output flickers
long displayLapDelay = 30000; //microseconds 
int lapNumberToDisplay, lapNumberDisplayed = 0;
int highpulse = 0, lowpulse = 0, token = 0;
#define maxHighPulse 2000 //time till reset tokens in the event of a false positive
#define maxLowPulse 28 //the maximum pulse count we need
#define minLowPulse 18 //the minimum pulse count we need
#define resolution 20 //take a reading from the receiver every 20 microseconds



void setup() {
  Serial.begin(9600);
  Serial.println("startup");
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
  
  //The Button Interface
  int buttonReading = digitalRead(buttonPin); // check the state of the button
  if (buttonReading != lastButtonState) lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > debounceDelay) {
   //if the reading had been there for long enough 
    if (buttonReading != lastButtonState && buttonReading == LOW) {
        //if the button was just pressed
        buttonState = LOW; // change the state for one loop
        Serial.print('button pressed, button state changed to LOW');
    } 
  }
  
  // The Receiver Interface
  int receiverState = digitalRead(receiverPin);
  if (receiverState == HIGH){ //no beam is being received
    highpulse++;
    delayMicroseconds(resolution); //wait some time
  }
  if (receiverState == LOW) { //a beam is being received
    lowpulse++;
    delayMicroseconds(resolution); //wait some time
  }
  if (highpulse > maxHighPulse) { // we have an incorrect signal
    token = 0; //clear the tokens
    highpulse = 0; //reset the highpulse
  }
  if (lowpulse > minLowPulse && lowpulse < maxLowPulse){
    //we have the pulse length we expected
    token ++; //we need 3 tokens to trigger
    lastReceiverTrigger = millis();
    Serial.println("3 tokens");
  }
  
  // If we just crossed the finish line
  if (token == 3) {
    if(!lapTimes.open("lapTimes.txt", O_RDWR | O_CREAT | O_AT_END)) {
      lcd.setCursor(8,1);
      lcd.print("SD Error"); 
    }
    if (lapNumber == 0) {
      // If we corssed the start line
      startLap = micros()/1000000.00;
      lapNumber += 1;
      lapTimes.println(String("Driver Number: " + String(driverNumber)));
    }
    else {
      startLap = micros()/1000000.00;
      lapTimes.println(get_ready_to_print(lapNumber,time));
      lapNumber += 1;
    } 
    lapTimes.close(); 
    token = 0;
  }
  
  // Update the Time on screen
  if (lapNumber > 0 && (((millis()- lastReceiverTrigger) < displayLapDelay) || lapNumber == 1)) { 
    //constantly update the time 
    // unless were displaying the time at the end of a lap *
    // *with the exeption of the start
    lcd.setCursor(0,1);
    lcd.print(time,2);
  }
  
  // Update the Lap Number on screen
  if (( millis()- lastReceiverTrigger) < displayLapDelay) lapNumberToDisplay = lapNumber-1;
  else lapNumberToDisplay = lapNumber;
  if (lapNumberToDisplay != lapNumberDisplayed && lapNumber > 0) {
    lcd.setCursor(0,0);
    lcd.print("Lap ");
    lcd.print(lapNumberToDisplay); //update the display only if necessary
    lapNumberDisplayed = lapNumberToDisplay;
  }
    
  //Driver select
  if (lapNumber == 0){
    if (buttonState == LOW) driverNumber ++;
    lcd.setCursor(0,1); // mv cursor to second line
    lcd.print("Driver: "); //print driver on second line
    lcd.print(driverNumber);
  }
  
  lastButtonState = buttonReading;
  buttonState = HIGH; //Only let the states be triggered for one loop
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

