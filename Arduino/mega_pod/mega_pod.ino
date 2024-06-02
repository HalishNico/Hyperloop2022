// MasterSwapRoles

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <VescUart.h>
#include "HCSR04.h"
#include "DFRobot_BMI160.h"

#define CE_PIN   7
#define CSN_PIN 8

byte triggerPin = 3;
byte* echoPin = new byte[2]{12, 13};

const byte slaveAddress[5] = {'R','x','A','A','A'};
const byte masterAddress[5] = {'T','X','a','a','a'};

VescUart VESCUART;

float value;
float dutyValue=510;
float maxDuty=0.80;
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio
int counter=0;
char dataToSend[30];
char txNum = '0';
int dataReceived; // to hold the data from the slave - must match replyData[] in the slave
bool newData = false;

unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 100; // send once per second

int start_time;
int now_time;
DFRobot_BMI160 bmi160;
const int8_t i2c_addr = 0x69;

//============

void setup() {
    Serial.begin(115200);
    while(!Serial);
    Serial.println("setup");
    //HCSR04.begin(triggerPin, echoPin);

    while(!radio.begin());
    Serial.println("MasterSwapRoles Starting");
    radio.setDataRate( RF24_250KBPS );
    

    radio.openWritingPipe(slaveAddress);
    radio.openReadingPipe(1, masterAddress);

    radio.setRetries(3,5); // delay, count
    prevMillis = millis(); // set clock
    Serial3.begin(115200);
   VESCUART.setSerialPort(&Serial3);
    newData = false;
    radio.startListening();
  //init the hardware bmin160  
  if (bmi160.softReset() != BMI160_OK){
    Serial.println("reset false");
    while(1);
  }
  //set and init the bmi160 i2c address
  if (bmi160.I2cInit(i2c_addr) != BMI160_OK){
    Serial.println("init false");
    while(1);
  }
  strcat(dataToSend, "n");
  start_time = millis();
  radio.startListening();
  Serial.println("up and working");
}

//=============

void loop() {   
    getData();
    //strcat(dataToSend, "n");
    value=float(dataReceived);
    if (value < 500)
  {
      dutyValue = (value/500.0);
      dutyValue = (dutyValue / (1/maxDuty)) - maxDuty;
  }
  //FORWARD DIRECTION
  else if (value > 520)
  {
      value = value - 520;
      dutyValue = (value / (1023.0-520.0))/(1/maxDuty);
  }
  //STATIONARY
  else
  {
      dutyValue = 0.0;    
  }
//Serial.println(dutyValue);
 VESCUART.setDuty(dutyValue);
  if ( VESCUART.getVescValues() ) {
    /* Serial.println("");
    Serial.print("RPM: ");
    */
    float rpm = VESCUART.data.rpm/7.0;
    char rotation[8];
    dtostrf(rpm, 6, 2, rotation);
    strcat(dataToSend, rotation);
    strcat(dataToSend, "m");
    }
    /*
    
    Serial.print("\t");
    Serial.print("VOLTAGE: ");
    Serial.print(VESCUART.data.inpVoltage);
    Serial.print("\t");
    Serial.print("AMP HOURS: ");*/
    /*Serial.print("\t");
    Serial.print("Tachometer: ");
    Serial.print(VESCUART.data.tachometerAbs);
    Serial.print("\t");
    Serial.print("Duty Value: ");
    Serial.println(dutyValue);

  }
  else
  {
    Serial.println("Failed to get data!");
  }*/

  int16_t accelGyro[3]={0}; 
  //get both accel and gyro data from bmi160
  //parameter accelGyro is the pointer to store the data
  int rslt = bmi160.getAccelData(accelGyro);
  float plane_acc;
  if(rslt == 0){
      float accX = accelGyro[0]/16384.0;
      float accY = accelGyro[1]/16384.0;
      float accZ = accelGyro[2]/16284.0;
      plane_acc = sqrt(accY*accY + accX*accX)*9.81;
      int length = 4;
      char char_acc[length];
      dtostrf(plane_acc, 2, 2, char_acc);
      strcat(dataToSend, char_acc);
      strcat(dataToSend, "m");
      char char_us[6];
      dtostrf(accZ, 4,2, char_us);
      strcat(dataToSend, char_us);
      strcat(dataToSend, "m");      
     // Serial.print("Acceleration :   ");
      //Serial.print(char_acc);
      //Serial.print(" also ");
      //Serial.print(dataToSend);
      //Serial.print("        Size:");
      //Serial.println(sizeof(char_acc));
  }else{
    Serial.println("err");
  }
  
  //Serial.println(dataToSend);
  //Serial.print("Ultrasound :  ");
 // double* ultraSound = HCSR04.measureDistanceMm();
  //Serial.println(ultraSound[0]);
  //Serial.println(ultraSound[1]);
  if (millis() - prevMillis >= txIntervalMillis) {
    now_time= millis();
    char time[6];
    itoa(now_time, time, 10);
    strcat(dataToSend, time);    
    strcat(dataToSend, "n");
    send();
    prevMillis = millis();
  }
  dataToSend[0] = '\0';
}

//====================

void send() {
        radio.stopListening();
            bool rslt;
            rslt = radio.write(&dataToSend, sizeof(dataToSend) );
            Serial.println(dataToSend);
        //radio.startListening();
        if (rslt) {
            Serial.println("  Acknowledge received");
            Serial.print("Number of Failures  :  ");
            Serial.println(counter);
            counter=0;
            updateMessage();
            newData =false;
        }
        else {
           // Serial.println("  Tx failed");
            counter++;
        }
        radio.startListening();
}

//================

void getData(){
    radio.startListening();
    if ( radio.available() ) {
    radio.read( &dataReceived, sizeof(dataReceived) );
    newData = true;
    Serial.println(dataReceived);
    Serial.println("This was data I received");
    //currentMillis = millis();
    }
}

//================

void updateMessage() {
        // so you can see that new data is being sent
    txNum += 1;
    if (txNum > '9') {
        txNum = '0';
    }
}