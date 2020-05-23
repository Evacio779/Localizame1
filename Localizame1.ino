/*
 *  This sketch demonstrates how to scan WiFi networks.
 *  The API is almost the same as with the WiFi Shield library,
 *  the most obvious difference being the different file you need to include:
 */
#include <Wire.h> 
#include <HardwareSerial.h>
#include <string.h>

#include "utilities.h"
#include "sim800L.h"
#include "gps.h"

// TTGO T-Call pin definitions
#define MODEM_RST             5
#define MODEM_PWKEY           4
#define MODEM_POWER_ON        23
#define MODEM_TX              27
#define MODEM_RX              26
#define I2C_SDA               21
#define I2C_SCL               22

// GPS pin definitions
#define GPS_TX                33
#define GPS_RX                32

// Setup blink
#define BLUE_LED              13

// setup vbat pin
#define VBAT_PIN              35

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon             Serial
// Set serial for AT commands (to the module)
#define SerialAT              Serial1
#define SerialGPS             Serial2



#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

// Variables will change:
int ledState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated

Gps *p_Gps=new Gps();
sim800L *p_Sim800L = new sim800L(); 


inline void toggle(long ms){
   if(millis() - previousMillis > ms) 
   {  
       previousMillis = millis();   
       // if the LED is off turn it on and vice-versa:
       ledState = (ledState == LOW ? HIGH : LOW);
       digitalWrite(BLUE_LED, ledState);
   }
}

void setup()
{
   SerialMon.begin(115200);
   // Keep power when running from battery
   Wire.begin(I2C_SDA, I2C_SCL);
   bool isOk = setPowerBoostKeepOn(1);
   SerialMon.println(String("-->IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));

   // Set-up modem reset, enable, power pins
   pinMode(MODEM_PWKEY, OUTPUT);
   pinMode(MODEM_RST, OUTPUT);
   pinMode(MODEM_POWER_ON, OUTPUT);

   digitalWrite(MODEM_PWKEY, LOW);
   digitalWrite(MODEM_RST, HIGH);
   digitalWrite(MODEM_POWER_ON, HIGH);
   
   //
   // Set GSM module baud rate and UART pins
   SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
   p_Sim800L->Init(&SerialAT, &SerialMon); 

   if(p_Sim800L->begin() != GN_ERR_NONE)
   {
      SerialMon.println("-->SIM800L cannot begin!");
      while(1){
         toggle(500);
      }
   }
   SerialMon.println("-->Sim800 OK!");
   p_Sim800L->Ready2ReceiveSMS();

   
   //
   // Gps 
   SerialGPS.begin(4800, SERIAL_8N1,GPS_RX, GPS_TX); 
   p_Gps->Init(&SerialGPS, &SerialMon);
   

   // Setup blink
   pinMode (BLUE_LED, OUTPUT);

   Serial.println("Setup done");
   p_Sim800L->Init(); 


   // setup Vbat pin
   pinMode(VBAT_PIN, INPUT);
   // Battery Voltage
   float VBAT = (float)(analogRead(VBAT_PIN)) / 4095*2*3.3*1.1;
   /*
   The ADC value is a 12-bit number, so the maximum value is 4095 (counting from 0).
   To convert the ADC integer value to a real voltage you’ll need to divide it by the maximum value of 4095,
   then double it (note above that Adafruit halves the voltage), then multiply that by the reference voltage of the ESP32 which 
   is 3.3V and then vinally, multiply that again by the ADC Reference Voltage of 1100mV.
   */
   Serial.println("Vbat = "); Serial.print(VBAT); Serial.println(" Volts");
  
   
 
}

void loop()
{
   char ch = SerialMon.read();
   if(ch != 0xff){     
      //SerialMon.printf("->Serial Mon:%c\n",ch); 
      SerialAT.write(ch);
   }

   ch = SerialAT.read();
   if (ch != 0xff && ch != 0x00)
   {
      SerialMon.write(ch);
      p_Sim800L->atbus_rx_statemachine(ch);
      int  op = p_Sim800L->get_status();
      switch(op){
          case GN_OP_AT_IncomingSMS:
            char reply_msg[30];
            // Cambia el codigo que quieras, aqui se usa E01FF1. Asi el modem no responde
            // a cualquier mensaje entrante
            if(!strncmp(p_Sim800L->get_msg(),"E01FF1",6)){  
               strcpy(reply_msg, p_Gps->getLat());
               strcat(reply_msg,",");
               strcat(reply_msg, p_Gps->getLon());  
               p_Sim800L->SendSMS(p_Sim800L->get_phone(), reply_msg); 
            }
            // statements
            break;
          default:
            break;
      }
   }

   ch = SerialGPS.read();
   if(ch != 0xff && ch != 0x00){
          SerialMon.write(ch); 

      p_Gps->gps_rx_frame(ch);
      if(p_Gps->valid){
         //serialprintf("->%f,%f\n",p_Gps.slatitude,p_Gps.sLongitude);
      }
         
   }

   toggle(2000);
   
}
