#include<DHT.h>
#include <Adafruit_BMP085.h>
#include<SoftwareSerial.h>

#define dhtPin 4
#define dhtType DHT11

#define smokeIn 2

DHT dht(dhtPin, dhtType);
SoftwareSerial gsm(16,17); //Rx, Tx
Adafruit_BMP085 bmp;

float temp;
byte humidity; 
float pressure; 
float alt;
String msgrcv;
int smokeLevel;
byte smokePercent;

void setup() {
  // put your setup code here, to run once:


  Serial.begin(9600);
  dht.begin();
  bmp.begin(); 
  gsm.begin(9600);
  pinMode(smokeIn, INPUT);

  Serial.println("Initializing Netwrok...");

  gsm.println("AT");
  checkSerial();

  gsm.println("AT+CSQ");
  checkSerial();

  gsm.println("AT+CCID");
  checkSerial();

  gsm.println("AT+CREG?");
  checkSerial();

  gsm.println("AT+CBC");
  checkSerial();

  gsm.println("AT+COPS?");
  checkSerial();

  gsm.println("AT+CMGF=1"); // Initializes the text mode
  checkSerial();

  gsm.println("AT+CNMI=2,2,0,0,0"); // Decides how newly arrived messages will be handled
  checkSerial();
  

}

void loop() {
  // put your main code here, to run repeatedly:
  
  temp = dht.readTemperature(); 
  humidity = dht.readHumidity(); 
  //pressure = ((bmp.readPressure())/101325)*760;
  pressure = bmp.readPressure();
  pressure=(pressure/101325)*760 ; 
  alt = bmp.readAltitude(); 
  smokeLevel = analogRead(smokeIn);
  smokePercent = map(smokeLevel, 300,3000,0,100);

//  Serial.print("Temperature is " );
//  Serial.print(temp);
//  Serial.write(227);
//  Serial.println("C");
//
//  Serial.print("Humidity is " );
//  Serial.print(humidity);
//  Serial.println("%");
//
//  Serial.print("Pressure is " );
//  Serial.print(pressure);
//  Serial.println("mmHg");
//
//  Serial.print("Altitude is " );
//  Serial.print(alt);
//  Serial.println("meter");
//
//  Serial.print("Smoke Level: " );
//  Serial.println(smokeLevel);
//
//  Serial.print("Smoke Percent: " );
//  Serial.print(smokePercent);
//  Serial.println("%");

  if ((gsm.available())) {

    msgrcv = gsm.readString();
    checkSerial();

    if (msgrcv.indexOf("data") >= 0) {

      String msg = "Air Temperature: " + String(temp) + " *C\n"
                   "Humidity: " + String(humidity) + "%\n"
                   "Air Pressure: " + String(pressure) + " mmHg\n"
                   "Altitude: " + String(alt) + "m\n"
                   "Smoke Level: " + String(smokePercent) + "%" ;
      sms(msg);

    }

  }

}

void checkSerial() {

  delay(500); // Used to ensure enough lag time between the at commands

  //  while (Serial.available())
  //    gsm.write(Serial.read());

  while (gsm.available())
    Serial.write(gsm.read());

}

void sms(String MSG)  {

  /*gsm.println("ATD+8801988448287;"); // Dials the Destination Number ***Make Call Prior to Sending SMS
  checkSerial();

  delay(15000);

  gsm.println("ATH"); // Hangs up the call after 20 Seconds
  checkSerial();*/

  gsm.println("AT+CMGF=1"); // Initialize the text mode
  checkSerial();

  gsm.println("AT+CMGS=\"+8801988448287\""); // Set Destination Phone Number
  checkSerial();

  gsm.println(MSG); // Set Message Content
  checkSerial();

  gsm.write(26);

}
