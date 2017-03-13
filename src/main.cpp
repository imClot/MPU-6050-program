//A short arduino program to read the MPU-6050 accelerometer and gyro
#include <Arduino.h>
#include <Wire.h>
//MPU-6050 I2C address is 0x68

//TODO: cleanup
//Write calibration routine
//Compare raw values and fifo values 
//NOTE: BREAKOUT BOARD HAS PULL UP RESISTORS INCLUDED
const int MPUaddress = 0x68;

static const unsigned long int bitsInTwoBytes = 65536;
//Using default range of +/-2g
static const double adcUnitConversion = 4*9.8 / (65536); //adc units to ms^-2

//Stuff fOR MPUSENSOR
const uint8_t accelRegister = 0x3B;//Memory/register address of accelerometer X component
const uint8_t tempRegister = 0x41;//Memory/register address of temp 
const uint8_t gyroRegister = 0x43;//Memory/register address of gyro X component
const uint8_t gyroConfigRegister = 0x1B; //Memory/register address where you choose gyro settings
const uint8_t accelConfigRegister = 0x1C; //Memory/register where you choose the accelerometer settings

auto baudRate = 115200;

bool printConfigRegisterStatus = false;

class MPUSENSOR
{
	public:
		MPUSENSOR(int address);
		//Following values are all raw ADC values
		int16_t gyro[3]; //Rotational velocities around x,y,z axes
		int16_t accel[3]; //Array, (x,y,z)
		int16_t temp;
		//By default, the accelerometer range is +/-2g over a signed 16bit int
		//Temp: offset -521LSB, sensitivity 340LSB/degC
		//By default, the angular rot/gyro values are +/- 250deg/sec 
		bool measureAll();
		bool measureGyro();
		bool measureAccel();
		bool measureTemp();
		void initialiseScales();
	private:
		int deviceAddress;
};

void MPUSENSOR::initialiseScales()
{
	
	Wire.beginTransmission(deviceAddress);
	Wire.write(accelConfigRegister);
	Wire.write(0); //Write all zeroes to gyro config register, turns off self test and sets to lowest range
	//0x0 gives 00000000, which sets scale to +/-2g in ~+/-32000, so one g ~ 16000
	//0x8 gives 00001000, which sets scale to +/-4g in ~+/-32000, so one g ~ 8000
	Wire.endTransmission();
	
	Wire.beginTransmission(deviceAddress);
	Wire.write(gyroConfigRegister);
	Wire.write(0);
	Wire.endTransmission();
	return;
}

MPUSENSOR::MPUSENSOR(int address)
{
	this->deviceAddress = address;
}

bool MPUSENSOR::measureAccel()
{
	Wire.beginTransmission(deviceAddress);
	Wire.write(accelRegister); //Hey MPU, look at this register address
	Wire.requestFrom(deviceAddress,6,true); //Request MPU to send 6 bytes, starting from address specified earlier
	for(int i=0;i<3;i++)
	{
		int16_t mostSignificant;
		int16_t leastSignificant;
		mostSignificant = Wire.read(); //Values are 16 bits, request 8bit each time you read wire then have to use bitlogic to jam them together
		leastSignificant = Wire.read();
		this->accel[i] =  (mostSignificant << 8) | leastSignificant; //Bit shifts mostSignificant by 8 (because it's the first 8 bits of the while number) and joins on leastSignificant
	}
	Wire.endTransmission();
	return true;
}

bool MPUSENSOR::measureGyro()
{
	Wire.beginTransmission(deviceAddress);
	Wire.write(gyroRegister); //Hey MPU, look at this register address
	Wire.requestFrom(deviceAddress,6,true); //Request MPU to send 6 bytes, starting from address specified earlier
	for(int i=0;i<3;i++)
	{
		int16_t mostSignificant;
		int16_t leastSignificant;
		mostSignificant = Wire.read(); //Values are 16 bits, request 8bit each time you read wire then have to use bitlogic to jam them together
		leastSignificant = Wire.read();
		this->gyro[i] =  (mostSignificant << 8) | leastSignificant; //Bit shifts mostSignificant by 8 (because it's the first 8 bits of the while number) and joins on leastSignificant
	}
	Wire.endTransmission();
	return true;
}

bool MPUSENSOR::measureTemp()
{
	Wire.beginTransmission(deviceAddress);
	Wire.write(tempRegister); //Hey MPU, look at this register address
	Wire.requestFrom(deviceAddress,2,true); //Request MPU to send 2 bytes, starting from address specified earlier
	int16_t mostSignificant;
	int16_t leastSignificant;
	mostSignificant = Wire.read(); //Values are 16 bits, request 8bit each time you read wire then have to use bitlogic to jam them together
	leastSignificant = Wire.read();
	this->temp =  (mostSignificant << 8) | leastSignificant; //Bit shifts mostSignificant by 8 (because it's the first 8 bits of the while number) and joins on leastSignificant
	Wire.endTransmission();
	return true;
}

bool MPUSENSOR::measureAll() //Does it all in one transmission, could test if this is better but meh
{
	Wire.beginTransmission(deviceAddress);
	Wire.write(accelRegister); //Hey MPU, look at this register address
	Wire.endTransmission();
	Wire.requestFrom(deviceAddress,14,true); //Request MPU to send 14 bytes, starting from address specified earlier
	int16_t mostSignificant;
	int16_t leastSignificant;
	for(int i=0;i<3;i++)
	{	
		mostSignificant = Wire.read(); //Values are 16 bits, request 8bit each time you read wire then have to use bitlogic to jam them together
		leastSignificant = Wire.read();
		this->accel[i] =  (mostSignificant << 8) | leastSignificant; //Bit shifts mostSignificant by 8 (because it's the first 8 bits of the while number) and joins on leastSignificant
	}
	mostSignificant = Wire.read(); //Values are 16 bits, request 8bit each time you read wire then have to use bitlogic to jam them together
	leastSignificant = Wire.read();
	this->temp =  (mostSignificant << 8) | leastSignificant; //Bit shifts mostSignificant by 8 (because it's the first 8 bits of the while number) and joins on leastSignificant
	for(int i=0;i<3;i++)
	{
		mostSignificant = Wire.read(); //Values are 16 bits, request 8bit each time you read wire then have to use bitlogic to jam them together
		leastSignificant = Wire.read();
		this->gyro[i] =  (mostSignificant << 8) | leastSignificant; //Bit shifts mostSignificant by 8 (because it's the first 8 bits of the while number) and joins on leastSignificant
	}
	Wire.endTransmission();
	return true;
}

template <typename Type>
String printArray(Type array[]) //Function to take a 3 array, print out a string in the form (x,y,z)
{
	String result;
	int arrayLength = 3;
	result = "(";
	for(int i=0;i<arrayLength;i++)
	{
		result = result + String(array[i]);
		if(i != (arrayLength-1))//If last element you don't need an extra comma
		{
			result = result + ',';
		}
		else
		{
			result = result + ')';
		}
	}
	return result;
}



double arrayMagnitude(int16_t array[])
{
	double result = 0;
	int arrayLength = 3;
	for(int i=0;i<arrayLength;i++)
	{
		double value = (double)array[i];
		//Serial.println("Value is " + String(value) + " adding " + String (value * value) + " to the count");
		result = result + value * value; //Convert, because I was getting an overflow with 16bit ints
	}
	result = sqrt(result);
	return result;
}

void setup()
{
	Wire.begin(); //Start wire
	Wire.beginTransmission(MPUaddress); //Start talking to this address
	Wire.write(0x6B); //Tell MPU-6050 we want to write to this address. Check the register map, this is the PWR_MGMT_1. We want to set bit 6 to low, to get it out of sleep state
	Wire.write(0); //Should really work out the exact byte to send, but just 0 will be good enough
	Wire.endTransmission(true); // Buffer filled, send it out. Bool is to say whether to release the bus or not after transmission
	Serial.begin(baudRate);//Set up for serial communication (over USB) (read via platformio serialports monitor)
	MPUSENSOR ourSensor(MPUaddress);
	ourSensor.initialiseScales();
}

//I2C with MPU-6050: send two wire transmissions, one says "look here" and the next tells the slave (ie. MPU-6050 what to do). Eg I use wire.write(ADDRESS) 
//to say I'm going to do something with the register at that address. If I send a byte it sets the register to that value, and if I send a request it reads
//out n bytes (ie. registers?), starting from that address sent
void loop()
{
	MPUSENSOR ourSensor(MPUaddress);
	//ourSensor.measureAll();
	int numberMeasurements = 0;
	unsigned long long timeEnd;
	//Take measurements over a time period of 20ms and then average, otherwise we overload the pyserial buffer when reporting at 1kHz
	//Initialise containers
	double accel[3];
	double gyro[3];
	for(int i=0;i<3;i++)
	{
		accel[i] = 0;
		gyro[i] = 0;
	}
	double temp = 0;
	//Start measurement loop
	unsigned long long timeStart = millis();
	unsigned long timeStartMicro = micros();
	while( (millis() - timeStart) < 10) //While it's within 20 miliseconds of starting measurement
	{
		ourSensor.measureAll();
		for(int i=0;i<3;i++)
		{
			accel[i] = accel[i] + ourSensor.accel[i];
			gyro[i] = gyro[i] + ourSensor.gyro[i];
		}
		temp = temp + ourSensor.temp;
		numberMeasurements = numberMeasurements + 1;
	}
	timeEnd = millis();
	unsigned long timeEndMicro = micros();
	//Average the quantities out
	for(int i=0;i<3;i++)
	{
		accel[i] = accel[i] / numberMeasurements;
		gyro[i] = gyro[i] / numberMeasurements;
	}
	temp = temp / numberMeasurements;
	unsigned long long timeElapsed = timeEnd - timeStart;
	unsigned long timeElapsedMicro = timeEndMicro - timeStartMicro;
	//All measurements taken, prepare output
	String output;
	output = "Accel: " + String(printArray(accel));
	//output = output + " mag = " + String(arrayMagnitude(ourSensor.accel));
	double tempCorrected = double(temp) / 340 + 36.53;
	output = output + "| Temp: " + String(tempCorrected);
	output = output + "| Gyro: " + String(printArray(gyro));
	output = output + "| Time: " + String((int)timeElapsed);
	output = output + "| TimeMicro: " + String((int)timeElapsedMicro);
	output = output + "| numberMeasurements: " + String(numberMeasurements);
	Serial.println(output);
	if(printConfigRegisterStatus)
	{
		uint8_t gyroConfigResult;
		Wire.beginTransmission(MPUaddress);
		Wire.write(gyroConfigRegister); //Hey MPU, look at this register address
		Wire.endTransmission();
		Wire.requestFrom(MPUaddress,1); //Request MPU to send 2 bytes, starting from address specified earlier
		gyroConfigResult = Wire.read();
		Serial.println("gyroConfigResult is " + String(gyroConfigResult));
		
		uint8_t accelConfigResult;
		Wire.beginTransmission(MPUaddress);
		Wire.write(accelConfigRegister); //Hey MPU, look at this register address
		Wire.endTransmission();
		Wire.requestFrom(MPUaddress,1); //Request MPU to send 2 bytes, starting from address specified earlier
		accelConfigResult = Wire.read();
		Serial.println("accelConfigResult is " + String(accelConfigResult));
	}
	//delay(15);
}

