
#include <LiquidCrystal.h> // includes the LiquidCrystal Library 
#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>
#define buzzer 8
#define trigPin 9
#define echoPin 10
#define redLED 11
#define greenLED 12
long duration;
int distance, initialDistance, currentDistance, i;
int screenOffMsg =0;
String password="1234";
String tempPassword;
boolean activated = false; // State of the alarm
boolean isActivated;
boolean rfid = true;
boolean activateAlarm = false;
boolean alarmActivated = false;
boolean enteredPassword; // State of the entered password to stop the alarm
boolean passChangeMode = false;
boolean passChanged = false;
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keypressed;
//define the cymbols on the buttons of the keypads
char keyMap[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {14, 15, 16, 17}; //Row pinouts of the keypad
byte colPins[COLS] = {18, 19, 20, 21}; //Column pinouts of the keypad
//String tagUID = "99 01 D3 7E";  // String to store UID of tag
byte readCard[4];   // Stores scanned ID read from RFID Module
byte tagUID[4]={0x99,0x01,0xD3,0x7E};  
uint8_t successRead;    // Variable integer to keep if we have Successful Read from Reader 
boolean distanceActive;
MFRC522 mfrc522(53,5); // MFRC522 mfrc522(SS_PIN, RST_PIN)


Keypad myKeypad = Keypad( makeKeymap(keyMap), rowPins, colPins, ROWS, COLS); 
LiquidCrystal lcd(13, 2, 4, 3, 6, 7); // Creates an LC object. Parameters: (rs, enable, d4, d5, d6, d7) 

uint8_t getID();
long getDistance();

void tag();
void enterPassword();
bool isMaster();
void setup() {
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  lcd.begin(16,2); 
  pinMode(buzzer, OUTPUT); // Set buzzer as an output
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(redLED, OUTPUT); // Sets the redLED pin as output
  pinMode(greenLED, OUTPUT);// Sets the greenLED pin as output
  Serial.begin(9600);

}
void loop() {

  if (activateAlarm) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, HIGH); 
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Alarm will be");
    lcd.setCursor(0,1);
    lcd.print("activated in");
   
    int countdown = 9; // 9 seconds count down before activating the alarm
    while (countdown != 0) {
      lcd.setCursor(13,1);
      lcd.print(countdown);
      countdown--;
      tone(buzzer, 700, 100);
      delay(500);
    }
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Alarm Activated!");
    Serial.println("Alarm Activated!");
    initialDistance = getDistance();
    Serial.print("initialDistance = ");
    Serial.println(initialDistance);
    activateAlarm = false;
    alarmActivated = true;
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW); 
  }
  if (alarmActivated == true){
      currentDistance = getDistance() + 10;
      Serial.print("currentDistance = ");
      Serial.println(currentDistance);
      if ( currentDistance < initialDistance) {
        tone(buzzer, 1000); // Send 1KHz sound signal 
        digitalWrite(greenLED, LOW);
        digitalWrite(redLED, HIGH);
        
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("C - Use TAG");
        lcd.setCursor(0,1);
        lcd.print("D - Use PASS");
        distanceActive=1;
      }
      if(distanceActive==1){
        keypressed = myKeypad.getKey();
        if(keypressed == 'C'){
          tag();
        }
        else if(keypressed == 'D'){
          enterPassword();
        } 
     }
    }
  if (!alarmActivated) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, HIGH); 
    if (screenOffMsg == 0 ){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("A - Activate");
      lcd.setCursor(0,1);
      lcd.print("B - Change Pass");
      screenOffMsg = 1;
    }
    keypressed = myKeypad.getKey();
     if (keypressed =='A'){        //If A is pressed, activate the alarm
      tone(buzzer, 1000, 200);
      activateAlarm = true;            
    }
    else if (keypressed =='B') {
      lcd.clear();
      int i=1;
      tone(buzzer, 2000, 100);
      tempPassword = "";
      lcd.setCursor(0,0);
      lcd.print("Current Password");
      lcd.setCursor(0,1);
      lcd.print(">");
      passChangeMode = true;
      passChanged = true;   
      while(passChanged) {      
      keypressed = myKeypad.getKey();
      if (keypressed != NO_KEY){
        if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
            keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
            keypressed == '8' || keypressed == '9' ) {
         tempPassword += keypressed;
         lcd.setCursor(i,1);
         lcd.print("*");
         i++;
         tone(buzzer, 2000, 100);
        }
      }
      if (i > 5 || keypressed == '#') {
        tempPassword = "";
        i=1;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Current Password");
        lcd.setCursor(0,1);
        lcd.print(">"); 
      }
      if ( keypressed == '*') {
        i=1;
        tone(buzzer, 2000, 100);
        if (password == tempPassword) {
          tempPassword="";
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Set New Password");
          lcd.setCursor(0,1);
          lcd.print(">");
          while(passChangeMode) {
            keypressed = myKeypad.getKey();
            if (keypressed != NO_KEY){
              if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
                  keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
                  keypressed == '8' || keypressed == '9' ) {
                tempPassword += keypressed;
                lcd.setCursor(i,1);
                lcd.print("*");
                i++;
                tone(buzzer, 2000, 100);
              }
            }
            if (i > 5 || keypressed == '#') {
              tempPassword = "";
              i=1;
              tone(buzzer, 2000, 100);
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Set New Password");
              lcd.setCursor(0,1);
              lcd.print(">");
            }
            if ( keypressed == '*') {
              i=1;
              tone(buzzer, 2000, 100);
              password = tempPassword;
              passChangeMode = false;
              passChanged = false;
              screenOffMsg = 0;
            }            
          }
        }
      }
    }
   }
 }
}
void enterPassword() {
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, HIGH);  
  int k=5;
  tempPassword = "";
  activated = true;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" *** ALARM *** ");
  lcd.setCursor(0,1);
  lcd.print("Pass>");
      while(activated) {
      keypressed = myKeypad.getKey();
      if (keypressed != NO_KEY){
        if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' ||
            keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' ||
            keypressed == '8' || keypressed == '9' ) {
          tempPassword += keypressed;
          lcd.setCursor(k,1);
          lcd.print("*");
          k++;
        }
      }
      if (k > 9 || keypressed == '#') {
        tempPassword = "";
        k=5;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(" *** ALARM *** ");
        lcd.setCursor(0,1);
        lcd.print("Pass>");
      }
      if ( keypressed == '*') {
        if ( tempPassword == password ) {
          activated = false;
          alarmActivated = false;
          distanceActive=0;
          noTone(buzzer);
          screenOffMsg = 0;
          digitalWrite(greenLED, HIGH);
          digitalWrite(redLED, LOW);  
        }
        else if (tempPassword != password) {
          lcd.setCursor(0,1);
          lcd.print("Wrong! Try Again");
          digitalWrite(greenLED, LOW); 
          digitalWrite(redLED, HIGH); 
          delay(1000);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(" *** ALARM *** ");
          lcd.setCursor(0,1);
          lcd.print("Pass>");
        }
      }    
    }
}
void tag(){
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, LOW);  
  int k=5;
  tempPassword = "";
  activated = true;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" *** ALARM *** ");
  lcd.setCursor(0,1);
  lcd.print("Scan your TAG");
 // Serial.println("Scan your TAG");
  delay(100);
  
  while(activated == true)
  {
    //  Serial.println("Scaning...");
      successRead = getID();            // sets successRead to 1 when we get read from reader otherwise 0
      
      //Checking the card
    if(successRead==1){
     
      if (isMaster(readCard)) { 
      
        // If UID of tag is matched
        lcd.clear();
        lcd.print("Tag Matched");
       // Serial.println("Tag Matched");
        noTone(buzzer);
        screenOffMsg = 0;
        digitalWrite(greenLED, HIGH);
        delay(1000);
        digitalWrite(greenLED, LOW);
        activated = false;
        alarmActivated = false;
        distanceActive=0;
        
      }
      else 
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Wrong Tag Shown");
      //  Serial.println("Wrong Tag Shown");
        lcd.setCursor(0, 1);
        lcd.print("Access Denied");
        digitalWrite(redLED, HIGH);
        delay(1000); 
        digitalWrite(redLED, LOW);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(" Scan Your Tag ");
        activated = true;
        alarmActivated = true;
      }
    }
  }
 }
uint8_t getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue   
      activated = true;
      return 0;
  }
  
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
     activated = true;    
     return 0;
  }
 
//  Serial.println(F("Scanned PICC's UID:"));
  for ( uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
  //  Serial.print(readCard[i], HEX);
  }
 // Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}



bool checkTwo ( byte a[], byte b[] ) {   
  for ( uint8_t k = 0; k < 4; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] ) {     // IF a != b then false, because: one fails, all fail
       return false;
    }
  }
  return true;  
}



// Check to see if the ID passed is the master programing card
bool isMaster( byte test[] ) {
  return checkTwo(test, tagUID);
}
  
// Custom function for the Ultrasonic sensor
long getDistance(){
  //int i=10;
  
  //while( i<=10 ) {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration*0.034/2;
  //sumDistance += distance;
  //}
  //int averageDistance= sumDistance/10;
  return distance;
}
