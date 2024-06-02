// SlaveSwapRoles

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
int prevmillis=0;

#define CE_PIN   7
#define CSN_PIN 8

byte triggerPin = 21;
byte echoPin = 12;


const byte slaveAddress[5] = {'R','x','A','A','A'};
const byte masterAddress[5] = {'T','X','a','a','a'};

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

char dataReceived[32]; // must match dataToSend in master
int replyData; // the two values to be sent to the master
bool newData = false;

unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 100; // send once per second


void setup() {
    while(!radio.begin());
    Serial.begin(9600);
    while(!Serial);
    pinMode(A0, INPUT);
    //Serial.println("SlaveSwapRoles Starting");
    
    //radio.begin();
    radio.setDataRate( RF24_250KBPS );

    radio.openWritingPipe(masterAddress); // NB these are swapped compared to the master
    radio.openReadingPipe(1, slaveAddress);

    radio.setRetries(3,5); // delay, count
    radio.startListening();

  // Serial.println("Up and working");
   radio.stopListening();
   delay(5000);
}

//====================

void loop() {
  //getData();
  if (millis() - prevMillis >= txIntervalMillis) {
    send();     
    prevMillis = millis();
    ///Serial.println("check");
    }
    replyData=analogRead(A0);
   // Serial.println(radio.available());
}

//====================

void send() {
      radio.stopListening();
        bool rslt;
        char reply_char[32];
        itoa(replyData, reply_char, 10);         
        rslt = radio.write( &replyData, sizeof(replyData) );
        //radio.startListening();

        //Serial.print("Reply Sent ");
        //Serial.print(replyData);

        if (rslt) {
          //Serial.println("Acknowledge Received");
          //Serial.print(reply_char);
          getData();
        }
        else {
           //Serial.println("Tx failed");
        }
}

//================

void getData() {
  radio.startListening();
    if (radio.available() ) {
        //Serial.print("payloads:   ");
        //Serial.print(radio.available());
        radio.read(&dataReceived, sizeof(dataReceived) );
        //Serial.print("content line: ");
        if(int(dataReceived[0]) !=0){
           Serial.print(dataReceived);
        }
        //Serial.print("   ");
        //Serial.println(sizeof(dataReceived));
       // Serial.println("This was data I received");
        newData = true;
        //showData();
    } else{
      //Serial.println("empty");
    }
}

//================9

void showData() {
    if (newData == true) {
        Serial.print("Data received ");
        Serial.print(dataReceived);
        Serial.print("   ");
        Serial.println(millis());
        newData = false;
    }
}

//===============