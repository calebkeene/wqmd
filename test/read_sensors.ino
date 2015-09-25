#include <OneWire.h>
#include <SD.h>

#define StartConvert 0
#define ReadTemperature 1

byte ECsensorPin = A1;  //EC Meter analog output,pin on analog 1
byte DS18B20_Pin = 2;  //DS18B20 signal (temp sensor), pin on digital 10
byte nMOS_Pin = 5;
const int chipSelect = 8;
File dataFile;

int sampleNumber = 1;
String device_id = "fluid_sol_01";
unsigned int analogSampleInterval=25,tempSampleInterval=850; 

unsigned long lastTime = 0;
unsigned long analogValTotal = 0; // for averaging conductivity
unsigned int analogAv = 0, avVoltage=0;
float temperature, conductivity;
//Temperature chip i/o
OneWire ds(DS18B20_Pin);  // on digital pin 10

void setup(){
  Serial.begin(115200);
  for (byte thisReading = 0; thisReading < numReadings; thisReading++)
    readings[thisReading] = 0;
  tempProcess(StartConvert);
  analogSampleTime=millis();// see if the card is present and can be initialized:
  tempSampleTime=millis();
  /*
  if (SD.begin(chipSelect)){
    Serial.println("card initialized.");
  }
  else{
    Serial.println("card initialized.");
  }
  */
}

void loop(){

  // 20 mins has passed
  //if(millis() - lastTime > 1200000){
  if(millis() - lastTime > 60000){ // 1 min (for testing)
    Serial.println("one minute has passed!");
    digitalWrite(nMOS_Pin, HIGH); // turn on sensors
    String sample = takeSample();
    digitalWrite(nMOS_Pin, LOW);
    sampleNumber++;

  }
  lastTime = millis();

 
  delay(100);
}

String takeSample(){
  
  if(millis()-analogSampleTime>=analogSampleInterval)  {
    analogSampleTime=millis();
     // subtract the last reading:
    analogValTotal = analogValTotal - readings[index];
    // read from the sensor:
    readings[index] = analogRead(ECsensorPin);
    // add the reading to the total:
    analogValTotal = analogValTotal + readings[index];
    // advance to the next position in the array:
    index = index + 1;
    // if we're at the end of the array...
    if (index >= numReadings)
    // ...wrap around to the beginning:
    index = 0;
    // calculate the average:
    analogAv = analogValTotal/numReadings;
  }

  if(millis()-tempSampleTime>=tempSampleInterval){
    tempSampleTime=millis();
    temperature = TempProcess(ReadTemperature);  // read the current temperature from the  DS18B20
    TempProcess(StartConvert);                   //after the reading,start the convert for next reading
  }
  //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.0185*(fTP-25.0));
  
  //float tempCoefficient=1.0+0.0185*(temperature-25.0);   
  //float compensatedVoltage=(float)averageVoltage/tempCoefficient; 

  //get conductivity as linear function of analogue voltage
  avVoltage=analogAv*(float)5000/1024;
  conductivity = 35.813*avVoltage+148.47;

  return serialiseToJSON()
  
}

float tempProcess(bool ch){ //returns temperature in degrees C
  static byte data[12];
  static byte addr[8];
  static float tempSum;
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
    tempSum = tempRead / 16;
  }
  return tempSum;
}