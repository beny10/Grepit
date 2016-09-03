//relay module
#include<SoftwareSerial.h>
SoftwareSerial serial(0,1);
int y[] = {9, 5, 6, 8};
int x[5];
int address = 2;
int triggerPin = A3;
int masterAddress = 0;
//type(0-master,1-relay,2-keyboard,3-network)
int deviceType = 1;
int inputPinsCount=1;
int inputPins[1]={8};
int outputPinsCount=1;
int outputPins[1]={9};
void registerInLan()
{
  //toAddress,typeOfResponse(0-register,1-PinRegister),fromAddress,type(0-master,1-relay,2-keyboard,3-network)
  int data[6] = {masterAddress, 0, address, deviceType,0};
  sendCommandViaMax(data);
  //toAddress,typeOfResponse,address,pinNumber,pinType(0-input,1-output,2-analog)
  for(int i=0;i<outputPinsCount;++i)
  {
    data[0]=masterAddress;
    data[1]=1;
    data[2]=address;
    data[3]=outputPins[i];
    data[4]=1;
    data[5]=0;
    sendCommandViaMax(data);
  }
  //toAddress,typeOfResponse,address,pinNumber,pinType(0-input,1-output,2-analog)
  for(int i=0;i<inputPinsCount;++i)
  {
    data[0]=masterAddress;
    data[1]=1;
    data[2]=address;
    data[3]=inputPins[i];
    data[4]=0;
    data[5]=0;
    sendCommandViaMax(data);
  }
  data[0] = masterAddress;
  data[1] = 4;
  data[2] = address;
  data[3] = 0;
  data[4] = 0;
  data[5]=0;
  sendCommandViaMax(data);
}
void setup()
{
  serial.begin(9600);
  pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW);
  registerInLan();
  for(int i=0;i<inputPinsCount;++i)
  {
    pinMode(inputPins[i],INPUT_PULLUP);
  }
  for(int i=0;i<outputPinsCount;++i)
  {
    pinMode(outputPins[i],OUTPUT);
  }
}
void sendCommandViaMax(int bytes[])
{
  digitalWrite(triggerPin, HIGH);
  delay(100);
  serial.write(y[0] + 48);
  serial.write(y[1] + 48);
  serial.write(y[2] + 48);
  serial.write(y[3] + 48);
  for (int i = 0; i < 6; ++i)
  {
    serial.write(48 + bytes[i]);
  }
  digitalWrite(triggerPin, LOW);
  delay(1);
}
void sendOneByteViaMax(int address, int byte)
{
  x[0] = address;
  x[1] = byte;
  x[2] = 0;
  x[3] = 0;
  x[4] = 0;
  x[5]=0;
  sendCommandViaMax(x);
}
void checkMax()
{
  if (serial.available() > 9)
  {
    
    while (serial.available())
    {
      for (int i = 0; i < 3; ++i)
      {
        x[i] = x[i + 1];
      }
      x[3] = serial.read() - 48;
      bool isOk = true;
      for (int i = 0; i < 4; ++i)
      {
        if (x[i] != y[i])
        {
          isOk = false;
        }
      }
      if (isOk)
      {
        for (int i = 0; i < 6; ++i)
        {
          x[i] = serial.read() - 48;
        }
        if (x[0] == address)
        {
          switch(x[1])
          {
            case 0:
              switch(x[3])
              {
                case 0:
                  digitalWrite(x[2],LOW);
                  break;
                case 1:
                  digitalWrite(x[2],HIGH);
                  break;
                case 2:
                  int status=digitalRead(x[2]);
                  if(status==1)
                    status=0;
                  else
                    status=1;
                  digitalWrite(x[2],status);
                  break;
              }
              break;
            case 1:
              if(address!=0)
                registerInLan();
              break;
          }
        }
      }
    }
  }
}
void loop() {
  checkMax();
  for(int i=0;i<inputPinsCount;++i)
  {
    int value=digitalRead(inputPins[i]);
    if(value==0)
    {
      //masterAddress,respondType,fromAddress,pinNumber,value
      int data[6] = {masterAddress, 2, address, inputPins[i],value,0};
      sendCommandViaMax(data);
      delay(500);
    }
  }
}
