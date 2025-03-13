#include<DHT.h>
#include <Adafruit_BMP085.h>
#include<SoftwareSerial.h>
#include<ESP_Mail_Client.h>
#include "arduino_secrets.h"
#include "thingProperties.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

// Initialize Telegram BOT
#define BOTtoken "7230981972:AAEFo2FxODCeX03-a0aqH33Ka8xSZwcum3I"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "5029073982"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

#define dhtPin 4
#define dhtType DHT11

int smokeIn = 33;

DHT dht(dhtPin, dhtType);
SoftwareSerial gsm(16,17); //Rx, Tx
Adafruit_BMP085 bmp;

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
float temp;
byte humidity; 
float pressure; 
float alt;
String msgrcv;
int smokeLevel;
int smokePercent;

// Email Credinetials

#define SMTP_server "smtp.gmail.com"

#define SMTP_Port 465

#define sender_email "rehanaakterrumi71@gmail.com"

#define sender_password "sjuhbwjneoglsaab"

#define Recipient_email "hoquerafi727@gmail.com"

#define Recipient_name "THR"

SMTPSession smtp;

//Telegram Rx Msg config
// Handle what happens when you receive new messages

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

  
    if (text == "/data") {
      bot.sendMessage(chat_id, "Processing Weather Information...", "");
      String msg = "Air Temperature: " + String(temp) + " *C\n"
                   "Humidity: " + String(humidity) + "%\n"
                   "Air Pressure: " + String(pressure) + " mmHg\n"
                   "Altitude: " + String(alt) + "m\n"
                   "Smoke Level: " + String(smokePercent) + "%" ;
      //sms(msg);
      //sendmail(msg);
      bot.sendMessage(CHAT_ID, msg, "");
    }
    
  }
}


void setup() {
  // put your setup code here, to run once:


  Serial.begin(9600);
  // Defined in thingProperties.h
  initProperties();
  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  
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

  bot.sendMessage(CHAT_ID, "ESP32 Board Booted", "");


}

void loop() {
  // put your main code here, to run repeatedly:

  ArduinoCloud.update();
  temp = dht.readTemperature(); 
  humidity = dht.readHumidity(); 
  //pressure = ((bmp.readPressure())/101325)*760;
  pressure = bmp.readPressure();
  pressure=(pressure/101325)*760 ; 
  alt = bmp.readAltitude(); 
  smokeLevel = analogRead(smokeIn);
  smokePercent = map(smokeLevel, 0,3000,0,100);

  temperature1 = temp; 
  humidity1=humidity; 
  pressure1= pressure;
  gas1 = smokePercent;

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
  Serial.print("Smoke Level: " );
  Serial.println(smokeLevel);

  Serial.print("Smoke Percent: " );
  Serial.print(smokePercent);
  Serial.println("%");

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

 //Config Tg Rx Msg Interval

 if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

}

void onAlert1Change()  {
  // Add your code here to act upon Alert1 change

  if(alert1) {

    String msg = "Air Temperature: " + String(temp) + " *C\n"
                   "Humidity: " + String(humidity) + "%\n"
                   "Air Pressure: " + String(pressure) + " mmHg\n"
                   "Altitude: " + String(alt) + "m\n"
                   "Smoke Level: " + String(smokePercent) + "%" ;
      sms(msg);
      sendmail(msg);
      bot.sendMessage(CHAT_ID, msg, "");
    
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

void sendmail(String msg) {

  smtp.debug(1);

  ESP_Mail_Session session;

  session.server.host_name = SMTP_server ;

  session.server.port = SMTP_Port;

  session.login.email = sender_email;

  session.login.password = sender_password;

  session.login.user_domain = "";

  /* Declare the message class */

  SMTP_Message message;

  message.sender.name = "WEATHER STATION";

  message.sender.email = sender_email;

  message.subject = "Alert From Weather Station";

  message.addRecipient(Recipient_name, Recipient_email);


  //Send simple text message

  String textMsg = msg; // "Door has been opened. Thank You."

  message.text.content = textMsg.c_str();

  message.text.charSet = "us-ascii";

  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  if (!smtp.connect(&session))

    return;

  if (!MailClient.sendMail(&smtp, &message))

    Serial.println("Error sending Email, " + smtp.errorReason());
}
