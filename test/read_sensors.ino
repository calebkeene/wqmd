#include <ArduinoJson.h>
#include <OneWire.h>

#define StartConvert 0
#define ReadTemperature 1


byte ECsensorPin = A1;  //EC Meter analog output,pin on analog 1
byte DS18B20_Pin = 10; //DS18B20 signal (temp sensor), pin on digital 10

unsigned long AnalogValueTotal = 0; // for averaging conductivity
unsigned int AnalogAverage = 0, averageVoltage=0;
float temperature, conductivity;
//Temperature chip i/o
OneWire ds(DS18B20_Pin);  // on digital pin 10

void setup(){
  Serial.begin(115200);
  pinMode(ECsensorPin, INPUT); //conductivity sensor
  pinMode(DS18B20_Pin, INPUT); // Dallas onewire temp sensor
}

void loop(){

	takeSample();

	Serial.print("temp: ");
	Serial.println(temperature);
	Serial.print("cond: ");
	Serial.println(conductivity);
}

void takeSample(){
  
  int i;
  for(i=0; i<25; i++){ //get conductivity average over 25 samples
    AnalogValueTotal += analogRead(ECsensorPin);    
  }
  AnalogAverage = AnalogValueTotal / 25;

  tempProcess(StartConvert);
  delay(800); // need to wait at least 750ms after conversion before reading temp (ensures accuracy)
  
  temperature = tempProcess(ReadTemperature);
  averageVoltage=AnalogAverage*(float)5000/1024; //millivolt average,from 0mv to 4995mV

  //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.0185*(fTP-25.0));
  float tempCoefficient=1.0+0.0185*(temperature-25.0);
  float voltageCoefficient=(float)averageVoltage/tempCoefficient;

  if(voltageCoefficient <= 448)conductivity=6.84*voltageCoefficient-64.32;//1ms/cm<EC<=3ms/cm
  else if(voltageCoefficient <= 1457)conductivity=6.98*voltageCoefficient-127;//3ms/cm<EC<=10ms/cm
  else conductivity = 5.3*voltageCoefficient+2278;//10ms/cm<EC<20ms/cm
  conductivity/=1000; //convert us/cm to ms/cm
 
}

float tempProcess(bool ch){ //returns temperature in degrees C
  static byte data[12];
  static byte addr[8];
  static float temperatureSum;
  if(!ch){
    if ( !ds.search(addr)) {
      Serial.println("no more sensors on chain, reset search");
      ds.reset_search();
      return 0;
    }      
    if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC not valid");
      return 0;
    }        
    if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device not recognized");
      return 0;
    }      
    ds.reset();
    ds.select(addr);
    ds.write(0x44,1); // start conversion, with parasite power on at the end
  }
  else{  
    byte present = ds.reset();
    ds.select(addr);    
    ds.write(0xBE); // Read Scratchpad            
    for (int i = 0; i < 9; i++) { // need 9 bytes
      data[i] = ds.read();
    }         
    ds.reset_search();           
    byte MSB = data[1];
    byte LSB = data[0];        
    float tempRead = ((MSB << 8) | LSB); //using twos complement
    temperatureSum = tempRead / 16;
  }
  return temperatureSum;  
}