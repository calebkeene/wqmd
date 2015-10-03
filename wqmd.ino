#include <OneWire.h>
#include <SD.h>
#include <SPI.h>

#define StartConvert 0
#define ReadTemperature 1

byte ECsensorPin = A1;  //EC Meter analog output,pin on analog 1
byte DS18B20_Pin = 2;  //DS18B20 signal (temp sensor), pin on digital 2
byte nMOS_Pin = 5; // FET for disconnecting sensors in between samples
const int chipSelect = 4; // for SD
const byte numReadings = 25;

int hasData = 0; // flag to set when first writing data to SD
int cmd = 0;
int sampleNumber = 1;
String device_id = "fluid_sol_01";

unsigned long lastTime = 0;
unsigned long lastSampleTime = 0;
float temperature, conductivity;
//Temperature chip i/o
OneWire ds(DS18B20_Pin);

void setup(){
  Serial.begin(115200);
  pinMode(10, OUTPUT); // need this to be set to output for SD module
  
  pinMode(nMOS_Pin, OUTPUT);
  digitalWrite(nMOS_Pin, HIGH);
 
  if (SD.begin(chipSelect)){
    Serial.println("{\"status\":\"ready\"}");
  }
  
  else{ 
    Serial.println("{\"status\":\"fatal\"}"); 
  }
}

void loop(){
  
  //if phone is paired over bluetooth, get command from App
  if(Serial.available()){
    readSerial();
  }
  //if 20 minutes has elapsed since last sample, take a new one
  //if((millis() - lastTime) > 1200000) {
  
  // 60 seconds for testing
  if((millis() - lastTime) > 30000){
    tempProcess(StartConvert);   
    delay(2000); // give conversion time
    String newSample = takeSample();
    saveToSD(newSample); // store sample on SD card
    lastTime = millis();
  }
}

void readSerial(){
  
  unsigned long startTime = millis();
  while(Serial.available() > 0){
      cmd += Serial.read(); // Read char, add to ASCII sum
    // if greater than 10 seconds have passed, break out (timeout)
    if((millis() - startTime) > 10000){
      break;
    }
  }
  
  if(cmd == 416){// Test
    //take single sample, return it
    sendSingleSample();
    cmd = 0;
  }
  else if(cmd == 1216){// RetrieveData
    sendAllSamples();
    cmd = 0;
  }
  else if(cmd == 644){// Status
    Serial.println("[databegin]");
    if(hasData == 1){
      Serial.println("{\"status\":\"ready\"}");
    }
    else{
      Serial.println("{\"status\":\"nodata\"}");
    }
    Serial.println("[dataend]");
    cmd = 0;
  }
}

void saveToSD(String sample){
  File dataFile = SD.open("data.txt", FILE_WRITE);
 
  if (dataFile){
    if(hasData == 0){ hasData = 1;} 
    dataFile.println(sample); // write sample to file
    dataFile.close();
  }
}

void sendSingleSample(){
  
  Serial.println("[databegin]"); // for v1.3 API
  if(hasData == 1){  
    Serial.println("{\"status\":\"ready\"}");

    tempProcess(StartConvert);   
    delay(2000);
    
    String newSample = takeSample();
    Serial.println(newSample);
  }
  else{ 
    Serial.println("\"Status\": \"nodata\"");//file hasn't had samples written to it
  }
  Serial.println("[dataend]");
}

void sendAllSamples(){

  String currSample;
  int count = 0;
  File dataFile = SD.open("data.txt");
  
  Serial.println("[databegin]");
  if (dataFile){ // file opened successfully
    if(hasData == 1){ // the file has had sample(s) written to it
      Serial.println("{");
      Serial.println("\"Status\": \"ready\",");
      Serial.println("\"samples\": [");
      // read all samples off File
      while(dataFile.available()){
        
        char c = dataFile.read();
        if(c != 125){ // 125 = }
          currSample+= c;
        }
        else{
          currSample += "}";
          Serial.println(currSample);
          delay(200); // delay to avoid phone JSON buffer overflow
          currSample= ""; //reset temporary string for next sample
          count++;
        }
      }
      dataFile.close(); // close the file:

      String endTime = "] \n\"TimeSinceLast\": ";
      endTime += (String)(ms_to_min(millis()-lastTime));
      Serial.println(endTime);
      Serial.println("}");
    }
    else{ 
      Serial.println("\"Status\": \"nodata\"");//file hasn't had samples written to it
    }
  } 
  else{
    // if the file didn't open, fatalerror:
    Serial.println("\"Status\": \"fatal\"");
  }
  Serial.println("[dataend]");
}
String takeSample(){
  int i;
  unsigned int analogValTotal = 0;
  // calculate average analogue voltage (over 25 samples)
  for(i=0; i<numReadings; i++){
    analogValTotal += analogRead(ECsensorPin);
  }
  unsigned int analogAv = analogValTotal/numReadings;
  temperature = tempProcess(ReadTemperature);  // read the current temperature from the  DS18B20
  unsigned int avVoltage=analogAv*(float)5000/1024;

  float TempCoefficient=1.0+0.0185*(temperature-25.0); //temp compensation (needs adjusting)
  // calibration was done ~ approx 20 degrees C
  float compensatedVoltage = aVVoltage/TempCoefficient;

  //linear function obtained from measurements, (Cond/AVoltage) valid from ~200uS/cm to 1.4mS/cm
  conductivity = 7.1905*compensatedVoltage+162.41; 
  
  lastSampleTime = ms_to_min(millis() - lastTime);
  return serialiseToJson(temperature, conductivity, lastSampleTime);
}

String serialiseToJson(float temp, float cond, unsigned long timeSinceLast){
  // construct JSON obj for individual sample
  String sample;

  sample += "{\"DeviceID\":\"";
  sample += device_id;

  sample += "\",\n\"SampleID\":";
  sample += (String)sampleNumber;

  sample += ",\n\"TimeSinceLast\":";
  sample += (String)timeSinceLast;

  sample += ",\n\"Temperature\":";
  sample += (String)temp;

  sample += ",\n\"Conductivity\":";
  sample += (String)cond;
  //sample+="\n}";

  //lastTime = millis();
  sampleNumber++;
  return sample;
}

unsigned long ms_to_min(unsigned long ms){
  return (unsigned long)(ms/(float)60000);
}

float tempProcess(bool ch){ //returns temperature in degrees C
  static byte data[12];
  static byte addr[8];
  static float tempSum;
  if(!ch){
    if ( !ds.search(addr)) {
      ds.reset_search();
      return 0;
    }      
    if ( OneWire::crc8( addr, 7) != addr[7]) {
      return 0;
    }        
    if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.println("\"Status\": \"temp\"");
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