#define COLS 3
#define ROWS 4
#define ADDRESS 3
#define TRIGGERED_PIN 2
#define DEVICE_TYPE 2
#include<SoftwareSerial.h>
#include <Lan.h>
#include <EEPROM.h>
SoftwareSerial serial(0, 1);
SoftwareSerial serial2(12, 13);
int inputPinsCount = 1;
rPin inputPins[1] = {{.pinNumber=0,.initializing=0}};
int outputPinsCount = 0;
rPin outputPins[1] = {{.pinNumber=7,.initializing=0}};
int analogPinsCount = 0;
rPin analogPins[1] = {{.pinNumber=0,.initializing=0}};
int analogTriggeredValue[] = {4};
int keyboardColPins[] = {11,10,9};
int keyboardRowPins[] = {8,7,6,5};
int insertedPin[4];
int countInsertedPin=0;
int pin[]={0,0,0,0};
unsigned long lastLogIn;
void writeLan(int byte)
{
  serial.write(byte);
}
int readLan()
{
  return serial.read();
}
int countLan()
{
  return serial.available();
}
Lan lan(ADDRESS,DEVICE_TYPE,TRIGGERED_PIN, &writeLan, &readLan, &countLan);
void loadPin()
{
  countInsertedPin=0;
  for(int i=0;i<4;++i)
  {
    pin[i]=EEPROM.read(i);
    if(pin[i]>12)
    {
      EEPROM.write(i,0);
      return loadPin();
    }
  }
}
void setup()
{
  serial2.begin(9600);
  serial.begin(9600);
  loadPin();
  for (int i = 0; i < COLS; ++i)
  {
    pinMode(keyboardColPins[i], OUTPUT);
    digitalWrite(keyboardColPins[i], HIGH);
  }
  for (int i = 0; i < ROWS; ++i)
  {
    pinMode(keyboardRowPins[i], INPUT_PULLUP);
  }
  lan.SetPins(&inputPinsCount,inputPins,&outputPinsCount,outputPins,&analogPinsCount,analogPins,analogTriggeredValue);
  lan.Register();
}
int  getLinie(int pin)
{
  return keyboardRowPins[ROWS - 1] - pin + 1;
}
int  getColoana(int pin)
{
  return keyboardColPins[COLS - 1] - pin + 1;
}
int transforLinColToNumber(int lin, int col)
{
  return ((lin - 1) * 3) + col;
}
int getTasta(int pin1, int pin2)
{
  int col, lin;
  col = getColoana(pin1);
  lin = getLinie(pin2);
  int rez = transforLinColToNumber(lin, col);
  if (rez == 11)
    return 0;
  return rez;
}
int getPressedTasta()
{
  for (int i = 0; i < COLS; ++i)
  {
    digitalWrite(keyboardColPins[i], LOW);
    for (int j = 0; j < ROWS; ++j)
    {
      int val = digitalRead(keyboardRowPins[j]);
      if (digitalRead(keyboardRowPins[j]) == 0)
      {
        digitalWrite(keyboardColPins[i], HIGH);
        return getTasta(keyboardColPins[i], keyboardRowPins[j]);
      }
    }
    digitalWrite(keyboardColPins[i], HIGH);
  }
  return -1;
}
bool isPinOk()
{
  bool ok=true;
  for(int i=0;i<4;++i)
  {
    if(pin[i]!=insertedPin[i])
    {
      ok=false;
      break;
    }
  }
  return ok;
}
void keyPressed(int number)
{
  insertedPin[countInsertedPin]=number;
  countInsertedPin++;
  if(countInsertedPin==4)
  {
    if(isPinOk())
    {
      lan.InputPinTriggered(0, 1);
      lastLogIn=millis();
      serial2.println("logged in");
    }
    else
    {
      serial2.println("bad pin");
    }
    countInsertedPin=0;
  }
}
void changePin()
{
  delay(500);
  int tasta;
  int newPin[4];
  int count=0;
  do
  {
    tasta = getPressedTasta();
    if(tasta!=-1)
    {
      newPin[count++]=tasta;
      delay(500);
    }
  }while(count<4);
  for(int i=0;i<4;++i)
  {
    EEPROM.write(i,newPin[i]);
  }
  loadPin();
}
void loop()
{
  int tasta = getPressedTasta();
  if (tasta != -1)
  {
    if(tasta==12 && millis()-lastLogIn<2000)
    {
      changePin();
    }
    else
      keyPressed(tasta);
    serial2.println(tasta) ;
    delay(500);
  }
  lan.CheckMessages();
  lan.CheckAnalogPins();
  //lan.CheckInputPins();
}
