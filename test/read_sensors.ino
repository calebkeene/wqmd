#include <ArduinoJson.h>
#include <OneWire.h>
#include <SD.h>

#define StartConvert 0
#define ReadTemperature 1


byte ECsensorPin = A1;  //EC Meter analog output,pin on analog 1
byte DS18B20_Pin = 10; //DS18B20 signal (temp sensor), pin on digital 10
const int chipSelect = 8;
File dataFile;

int sampleNumber = 1;
String device_id = "fluid_sol_01";


unsigned long lastTime = 0;
unsigned long AnalogValueTotal = 0; // for averaging conductivity
unsigned int AnalogAverage = 0, averageVoltage=0;
float temperature, conductivity;
//Temperature chip i/o
OneWire ds(DS18B20_Pin);  // on digital pin 10

void setup(){
  Serial.begin(9600);
  pinMode(ECsensorPin, INPUT); //conductivity sensor

  pinMode(DS18B20_Pin, INPUT); // Dallas onewire temp sensor
  pinMode(chipSelect, OUTPUT);

// see if the card is present and can be initialized:
  if (SD.begin(chipSelect)){
    Serial.println("card initialized.");
  }
  else{
    Serial.println("card initialized.");
  }
}

void loop(){
	Serial.println("iterating");

    takeSample();
  
    writeJSONtoSD(temperature, conductivity, millis()-lastTime);
    Serial.println("returned from writing to SD");
    //dataFile.println(str);
    //dataFile.close();
    // print to the serial port too:
    
    // for next iteration
    lastTime = millis();
    sampleNumber++;
  // if the file isn't open, pop up an error:
 
  delay(10000);
}

void writeJSONtoSD(float temp, float cond, unsigned long time){
  // construct JSON obj for individual sample
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  
  String sample;
  sample += "{\"DeviceID\":\"";
  sample += device_id;

  sample += "\",\n\"SampleID\":";
  sample += (String)sampleNumber;
  
  sample += ",\n\"TimeSinceLast\":";
  sample += (String)time;
  
  sample += ",\n\"Temperature\":";
  sample += (String)temp;
  
  //dtostrf(conductivity,2,2,buf);
  sample += ",\n\"Conductivity\":";
  sample += (String)cond;
  sample+="\n}";

  if(dataFile){
    dataFile.println(sample);
    dataFile.close();
    Serial.println("saved sample to SD:");
    Serial.println(sample);

  }
  else{
    Serial.println("error opening datalog.txt");
  } 
}

void takeSample(){
  Serial.println("taking sample");
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
  Serial.println("tempProcess");
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