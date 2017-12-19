#include <SoftwareSerial.h>
#include <FPC1020.h>

extern unsigned char l_ucFPID;
extern unsigned char rBuf[192];  //Receive return data
#define sw_serial_rx_pin 6 //  Connect this pin to TX on the FPC1020
#define sw_serial_tx_pin 7 //  Connect this pin to RX on the FPC1020

SoftwareSerial swSerial(sw_serial_rx_pin, sw_serial_tx_pin); // Fingerprint serial (RX, TX)
FPC1020 Finger(&swSerial);

unsigned int User_ID = 0;
unsigned char incomingNub;
unsigned int  matchUserID = 0;
unsigned char rtf = 0;
volatile int stat = 0;
int MODE = 0, i;
unsigned int timei;
unsigned int timen;
//Assign unique ID

void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT); // IRQ
  pinMode(13, OUTPUT);
  Serial.println("Fingerprint Test !"); 
  attachInterrupt(0, read_pin, CHANGE);
  digitalWrite(13, LOW);
}

void loop() {
  unsigned int User_ID = 0;
  unsigned char incomingNub;
  unsigned int  matchUserID = 0;
  unsigned char rtf = 0;
  //Wait for button interrupt
  //Decide button interrupt for lock mode
  if(MODE > 0){
    timei = millis();
    Serial.println(MODE);
    delay(2000);
    if(MODE == 2){
      //Bluetooth mode (Indicate connection)
      Serial.println("Bluetooth Mode");
      while(swSerial.available()<0){
        digitalWrite(13, HIGH);
        delay(1000);
        digitalWrite(13, LOW);
        delay(1000);
      }
      //Wait for connection byte (Indicate active connection)
      Serial.println("Bluetooth Connected");
      digitalWrite(13, HIGH);
      //Push all data
      //Wait for command
        //Check inactive time exceed 2 min
        timeCheck();
        //Register user
        //Remove user
        //Format lock
        //Disconnect or Inactive to 2 min (Return to standby mode)
    }else{
      //Authorization Mode 
      Serial.println("Authorization Mode");
      //Authorize user (Indicate ready) - Limit to 3 tries before reset to standby or 30 seconds
      for(i=0; i<2; i++){
        digitalWrite(13, HIGH);
        if(Auth()){        
          //if True unlock (Indicate unlock)
          digitalWrite(13, HIGH);
          break;
        }else{
          //else lock (Indicate fail)
          digitalWrite(13, HIGH);
        }
        digitalWrite(13, LOW);
        delay(500);
      }
    }   
  }
  //Reset system 
  digitalWrite(13, LOW);
  MODE = 0;
}

//Authenticating User
void Auth(){
  Serial.println("Match Fingerprint, please put your finger on the Sensor.");
       
  if( Finger.Search()){
    Serial.print("Success, your User ID is: "); 
    Serial.println( l_ucFPID, DEC);
    return 1;
  }else {
    Serial.println("Failed, please try again.");
    return 0;
  }                 
  delay(1000);
}

//Adding New User
void New(){
  User_ID = 0;
              
  Serial.println("Please input the new user ID (0 ~ 99).");
  while(Serial.available()<=0);
  delay(100);
  incomingNub = Serial.available();
  for(char i=incomingNub; i>=1; i--){
    User_ID = User_ID + (Serial.read()-0x30)*pow(10,(i-1));
  }
              
  Serial.println("Add Fingerprint, please put your finger on the Fingerprint Sensor.");
  rtf = Finger.Enroll(User_ID);
              
  if(rtf == TRUE) { 
    Serial.print("Success, your User ID is: "); 
    Serial.println( User_ID , DEC);
  }else if (rtf == FALSE) {
    Serial.println("Failed, please try again.");
  }else if( rtf == ACK_USER_OCCUPIED){
    Serial.println("Failed, this User ID alread exsits.");
  }else if( rtf == ACK_USER_EXIST){
    Serial.println("Failed, this fingerprint alread exsits.");
  }
  delay(2000);
}

//Display Data
void Display(){
  if(Finger.PrintUserID()){
    Serial.print("Number of Fingerprint User is:"); 
    unsigned char UserNumb;
    UserNumb = (l_ucFPID-2)/3;
            
    Serial.println(UserNumb,DEC);
    Serial.println("Print all the User ID:"); 
                 
    for(char i = 0; i < UserNumb; i++){
      Serial.println(rBuf[12+i*3],DEC);
    }
  }else {
    Serial.println("Print User ID Fail!");
  }
  delay(1000);
}

//Delete User
void Delete(){
  User_ID = 0;
  
  Serial.println("Please input the user ID(0 ~ 99) you want to delecte.");
  while(Serial.available()<=0);
  delay(100);
  incomingNub = Serial.available();
  for(char i=incomingNub; i>=1; i--){
    User_ID = User_ID + (Serial.read()-0x30)*pow(10,(i-1));
  }
             
  if(Finger.Delete(User_ID)) {
    Serial.println("Delete Fingerprint User Success!"); 
  }else{
    Serial.println("Delete Fingerprint User Fail!");
  }
  delay(1000);
}

//Delete All User
void Format(){
  unsigned char DeleteFlag = 0;
                
  Serial.println("Delete All Users, Y/N ?");
  for(unsigned char i=200; i>0; i--)//wait response info
  {
    delay(20);
    if(Serial.available()>0){
      DeleteFlag = Serial.read();
      break;
    }
  }
                
  if(DeleteFlag == 'Y'||'y'){
    if(Finger.Clear()){
      Serial.println("Delete All Fingerprint User Success!");
    }else{
      Serial.println("Delete All Fingerprint User Fail!");
    }
  }
  delay(500);
}

//Read interrupt
void read_pin(){
  MODE++;
  Serial.println(MODE);
}

void timeCheck(){
  timen = millis();
}

