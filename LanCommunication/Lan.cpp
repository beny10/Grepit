#include "Lan.h"
Lan::Lan(int address, int deviceType, int triggerPin, void(*writeFunct)(int), int(*readFunct)(), int(*countFunct)())
{
	_lanComm = new LanCommunication(triggerPin, writeFunct, readFunct, countFunct);
	_address = address;
	_deviceType = deviceType;
}

Lan::~Lan()
{
}

void Lan::SetPins(int *inputPinsCount, int *inputPins, int *outputPinsCount, int *outputPins, int *analogPinsCount, int *analogPins, int *analogTriggeredValue)
{
	_inputPinsCount = inputPinsCount;
	_inputPins = inputPins;
	_outputPins = outputPins;
	_outputPinsCount = outputPinsCount;
	_analogPins = analogPins;
	_analogPinsCount = analogPinsCount;
	_analogTriggeredValue = analogTriggeredValue;
	_isAnalogTriggered = (int*)malloc(*_analogPinsCount * sizeof(int));
	for (int i = 0; i < *_analogPinsCount; i++)
	{
		_isAnalogTriggered[i] = 1;
	}
	for (int i = 0; i < *_inputPinsCount; ++i)
	{
		pinMode(_inputPins[i], INPUT_PULLUP);
	}
	for (int i = 0; i < *_outputPinsCount; ++i)
	{
		pinMode(_outputPins[i], OUTPUT);
	}
}

void Lan::Register()
{
	int data[6] = { MASTER_ADDRESS, 0, _address, _deviceType, 0 };
	_lanComm->SendCommand(data);
	//toAddress,typeOfResponse,address,pinNumber,pinType(0-input,1-output,2-analog)
	//Serial.println(*_outputPinsCount);
	for (int i = 0; i < *_outputPinsCount; ++i)
	{
		data[0] = MASTER_ADDRESS;
		data[1] = 1;
		data[2] = _address;
		data[3] = _outputPins[i];
		data[4] = 1;
		data[5] = 0;
		_lanComm->SendCommand(data);
	}
	for (int i = 0; i < *_analogPinsCount; ++i)
	{
		data[0] = MASTER_ADDRESS;
		data[1] = 1;
		data[2] = _address;
		data[3] = _analogPins[i];
		data[4] = 2;
		data[5] = 0;
		_lanComm->SendCommand(data);
	}
	//toAddress,typeOfResponse,address,pinNumber,pinType(0-input,1-output,2-analog)
	for (int i = 0; i < *_inputPinsCount; ++i)
	{
		data[0] = MASTER_ADDRESS;
		data[1] = 1;
		data[2] = _address;
		data[3] = _inputPins[i];
		data[4] = 0;
		data[5] = 0;
		_lanComm->SendCommand(data);
	}
	data[0] = MASTER_ADDRESS;
	data[1] = 4;
	data[2] = _address;
	data[3] = 0;
	data[4] = 0;
	data[5] = 0;
	_lanComm->SendCommand(data);
}
void Lan::CheckMessages()
{
	if (_lanComm->IsCommandAvailable())
	{
		if (_lanComm->ReadCommand())
		{
			int *bytes = _lanComm->GetLastCommand();
			if (bytes[0] == _address)
			{
				Serial.println("int2");
				Serial.print(bytes[0]);
				Serial.print(bytes[1]);
				Serial.print(bytes[2]);
				Serial.println(bytes[3]);
				switch (bytes[1])
				{
				case 0:

					switch (bytes[3])
					{
					case 0:
						digitalWrite(bytes[2], LOW);
						break;
					case 1:
						digitalWrite(bytes[2], HIGH);
						break;
					case 2:
						int status = digitalRead(bytes[2]);
						if (status == 1)
							status = 0;
						else
							status = 1;
						digitalWrite(bytes[2], status);
						break;
					}
					break;
				case 1:
					if (_address != 0)
						delay(10);
					Register();
					break;
				case 2:
					for (int i = 0; i < *_analogPinsCount; ++i)
					{
						if (_analogPins[i] == bytes[2])
						{
							_analogTriggeredValue[i] = bytes[3];
							Serial.println("limit setted");
							Serial.println(bytes[3]);
						}
					}
					break;
				}
			}
		}
	}
}
void Lan::CheckAnalogPins()
{
	for (int i = 0; i < *_analogPinsCount; ++i)
	{
		int value = map(analogRead(A0 - _analogPins[i]), 0, 1024, 0, 9);
		/*Serial.print(value);
		Serial.print(" ");
		Serial.print(isAnalogTriggered[i]);
		Serial.print(" ");
		Serial.println(analogTriggeredValue[i]);*/
		if (value>_analogTriggeredValue[i] && _isAnalogTriggered[i] == 0)
		{
			int data[6] = { MASTER_ADDRESS, 2, _address, _analogPins[i], value, 0 };
			_lanComm->SendCommand(data);
			_isAnalogTriggered[i] = 1;
			Serial.println("anal pin trig");
			Serial.println(value);
		}
		else if (value <= _analogTriggeredValue[i] && _isAnalogTriggered[i] == 1)
		{
			int data[6] = { MASTER_ADDRESS, 2, _address, _analogPins[i], value, 1 };
			_lanComm->SendCommand(data);
			_isAnalogTriggered[i] = 0;
			Serial.println("agb");
		}
	}
}
void Lan::CheckInputPins()
{
	for (int i = 0; i < *_inputPinsCount; ++i)
	{
		int value = digitalRead(_inputPins[i]);
		if (value == 0)
		{
			//masterAddress,respondType,fromAddress,pinNumber,value
			int data[6] = { MASTER_ADDRESS, 2, _address, _inputPins[i], value, 0 };
			_lanComm->SendCommand(data);
			delay(500);
		}
	}
}