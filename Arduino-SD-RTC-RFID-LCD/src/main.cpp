#include <Arduino.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 6, 2, 5, 3, 4);

#include <MFRC522.h> // for the RFID
#include <SPI.h> // for the RFID and SD card module
#include <SD.h> // for the SD card
#include <RTClib.h> // for the RTC
#include <string.h>


// define pins for RFID
#define CS_RFID 10
#define RST_RFID 9

// define select pin for SD card module
#define CS_SD A0 

// Create a file to store the data
File myFile;
char *fileName = (char*)malloc(13*sizeof(char));

// Instance of the class for RFID
MFRC522 rfid(CS_RFID, RST_RFID);
DateTime now; 

// Variable to hold the tag's UID
char* uidChar = (char*)malloc(12*sizeof(char));

// Instance of the class for RTC
RTC_DS1307 rtc;

// General Variable 
int contLoop = 10;
char tmp[5] = "";



boolean readRFID() {
  Serial.print("RFID.getID()==========");
  if ( !rfid.PICC_ReadCardSerial()){
    Serial.println("FAIL_READ");
    return false;
  }
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  if (
    piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println("FAIL_COMP");
    return false;
  }
  uidChar[0] = '\0';
  for (int i = 0; i < 4; i++) {
    sprintf(tmp, "%02X",rfid.uid.uidByte[i]);
    strcat(uidChar,tmp);
    if(i!=3){
      strcat(uidChar,":");
    }
  }
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  Serial.println("DONE");
  Serial.print("RFID.read()=== ");
  Serial.println(uidChar);
  return true;
}

boolean check_and_save(){
  digitalWrite(CS_SD, LOW);
  Serial.print("SDC.name_by_id()======");
  myFile = SD.open("user.csv", FILE_READ);
  delay(100);
  if (myFile.available()){
    if (myFile.find(uidChar)){
      myFile.read();
      lcd.setCursor(0, 0);
      lcd.print("BEM VINDO");
      lcd.setCursor(0, 1);
      lcd.print(myFile.readStringUntil('\n'));
      myFile.close();
      Serial.println("FIND");
      sprintf(fileName, "%02d%02d%04d.csv",now.day(),now.month(),now.year()); 
      Serial.print(fileName);
      Serial.print(".save()===");
      myFile = SD.open(fileName, FILE_WRITE);
      delay(200);
      if (myFile.availableForWrite()){
        sprintf(tmp,"%02d:",now.hour());
        myFile.print(tmp);
        sprintf(tmp,"%02d:",now.minute());
        myFile.print(tmp);
        sprintf(tmp,"%02d;",now.second());
        myFile.print(tmp);
        myFile.println(uidChar);
        myFile.close();
        digitalWrite(CS_SD,HIGH);
        Serial.println("DONE");
        return true;  
      }else{
        Serial.println("FAIL_OPE"); 
      }
    }else{    
      Serial.println("MISS_USE");
    }
  }else{
    Serial.println("FAIL_OPE");
  }
  lcd.setCursor(0, 0);
  lcd.print("ACESSO NEGADO");
  myFile.close();
  digitalWrite(CS_SD,HIGH);
  return false;
}


void setup() { 
  // Init Serial port
  Serial.begin(9600);
  Serial.println("============BOOT==========");
  lcd.begin(16,2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");

  // Init SPI bus
  SPI.begin(); 
  // Init MFRC522 
  rfid.PCD_Init(); 

  // Setup for the SD card
  Serial.print("SDC.setup()===========");
  pinMode(CS_SD, OUTPUT);
  digitalWrite(CS_SD, LOW);
  lcd.setCursor(0, 1);
  lcd.print("TEST SDCard... ");
  delay(500);
  if(!SD.begin(CS_SD)) {
    Serial.println("FAIL_BEG");
    lcd.print("FAIL");
    while(1);
  }
  Serial.println("DONE");
  lcd.print("DONE");
  digitalWrite(CS_SD,HIGH);
  delay(500);
  
  // Setup for the RTC
  Serial.print("RTC.setup()===========");
  lcd.setCursor(0, 1);
  lcd.print("                    ");  
  lcd.setCursor(0, 1);
  lcd.print("TEST RELOGIO.. ");  
  delay(500);
  if(!rtc.begin()) {
    Serial.println("FAIL");
    lcd.print("FAIL");
    while(1);
  }
  if(!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.print("*"); 
  }
  Serial.println("DONE");
  lcd.print("DONE");
  delay(500);
  

   // Setup for the RFID
  Serial.print("RFID.setup()==========");
  lcd.setCursor(0, 1);
  lcd.print("                    ");  
  lcd.setCursor(0, 1);
  lcd.print("TEST RFID..... ");  
  delay(500);
  if (rfid.PCD_DumpVersion() < 0){
    Serial.println("FAIL");
    lcd.print("FAIL");
    while(1);
  }
  Serial.println("DONE");
  lcd.print("DONE");
  delay(500);
  

  Serial.println("============END===========");
  lcd.clear();
  lcd.print("Aproxime o cartao.");
}

void loop() {
  // Atualizando o Relogio no Display
  
  if (contLoop > 7){
    now = rtc.now(); 
    lcd.setCursor(0, 1);
    sprintf(tmp,"%02d",now.hour());
    lcd.print(tmp);
    lcd.print(":");
    sprintf(tmp,"%02d",now.minute());
    lcd.print(tmp);
    lcd.print(":");
    sprintf(tmp,"%02d",now.second());
    lcd.print(tmp);
    lcd.print("  ");
    sprintf(tmp,"%02d",now.day());
    lcd.print(tmp);
    lcd.print("/");
    sprintf(tmp,"%02d",now.month());
    lcd.print(tmp);
    lcd.print("/");
    sprintf(tmp,"%04d",now.year());
    lcd.print(tmp);
    contLoop = 0;
  }
  
  // Verificando novo cartao
  if(rfid.PICC_IsNewCardPresent()) {
    if (readRFID()){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Buscando...");
      check_and_save();
      delay(3000);
      lcd.clear();
      lcd.print("Aproxime o cartao.");
      contLoop = 10;
    }  
  }
  delay(100);
  contLoop ++;
}
