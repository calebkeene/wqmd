#include <OneWire.h>
#include <SD.h>
#include <SPI.h>

#define StartConvert 0
#define ReadTemperature 1

byte ECsensorPin = A1;  //EC Meter analog output,pin on analog 1
byte DS18B20_Pin = 2;  //DS18B20 signal (temp sensor), pin on digital 2
byte nMOS_Pin = 5; // FET for disconnecting sensors in between samples
const int chipSelect = 4; // for SD

File dataFile;
String currFileName;
//int fileIndex = 1;
int hasData = 0; // flag to set when first writing data to SD
int cmd = 0;
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
  pinMode(10, OUTPUT); // need this to be set to output for SD module
  
  for(byte thisReading = 0; thisReading < numReadings; thisReading++)
    readings[thisReading] = 0;
  tempProcess(StartConvert);
  analogSampleTime=millis();// see if the card is present and can be initialised:
  tempSampleTime=millis();

  currFileName = "data_1";

  if (SD.begin(chipSelect)){
    Serial.println("SD initialised");
  }
  else{
    Serial.println("SD initialisation failed");
  }
}

void loop(){
  //if phone is paired over bluetooth, get command from App
  if(Serial.available()){
    readSerial();
  }
  //if 20 minutes has elapsed since last sample, take a new one
  if(millis() - lastTime > 1200000){
    digitalWrite(nMOS_Pin, HIGH); // turn on sensors
    String newSample = takeSample();
    digitalWrite(nMOS_Pin, LOW); // turn off sensors
    saveToSD(newSample); // store sample on SD card
  }
}

void readSerial(){
  
  unsigned long start_time = millis();
  while(Serial.available() > 0){
      cmd += Serial.read(); // Read char, add to ASCII sum
    // if greater than 10 seconds have passed, break out (timeout)
    if((millis() - start_time) > 10000){
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
    if(hasData == 1){
      Serial.println("{\"status\":\"ready\"}");
    }
    else{
      Serial.println("{\"status\":\"nodata\"}");
    }
    cmd = 0;
  }
}

void saveToSD(String sample){
  File dataFile = SD.open(currFile, FILE_WRITE);
 
  if (dataFile){
    if(hasData == 0){ hasData = 1;} 
    Serial.println("saving to dataFile");
    dataFile.println(sample); // write sample to file
    dataFile.close();
  }  
  else {
    // if the file isn't open, print error
    String errMsg = "error opening " + currFileName;
    Serial.println(errMsg);
  } 
}

void sendSingleSample(){
    Serial.println("[databegin]");
    Serial.println("{\"status\":\"ready\"}");

    digitalWrite(nMOS_Pin, HIGH); 
    takeSample();
    digitalWrite(nMOS_Pin, LOW);

    String sample = serialiseToJson(temperature, conductivity, ms_to_min(millis() - lastTime));
    Serial.println(sample);
    Serial.println("[dataend]");
    lastTime = millis(); // update lasTime for next sample
  }
}

void sendAllSamples(){

  String temp;
  int count = 0;
  File dataFile = SD.open(currFileName);
  
  if (dataFile){ // file opened successfully
    if(hasData == 1){ // the file has had sample(s) written to it
      Serial.println("[databegin] {");
      Serial.println("\"Status\": \"ready\",");
      Serial.println("\"samples\": [");
      // read all samples off File
      while(dataFile.available()){
        
        char c = dataFile.read();
        if(c != 125){
          temp+= c;
        }
        else{
          temp+= "}";
          Serial.println(temp);
          delay(200); // delay to avoid phone JSON buffer overflow
          temp = "";
          count++;
        }
      }
      dataFile.close(); // close the file:

        String endTime = "\n\"TimeSinceLast\":";
      endTime += (String)(ms_to_min(millis()-lastTime));
      Serial.println(endTime);
      Serial.println("} [dataend]");
    }
    else{Serial.println("\"Error\": \"nodata\"");} //file hasn't had samples written to it
    
  } 
  else{
    // if the file didn't open, print an error:
    Serial.println("\"Error\": \"could not read SD\"");
  }
}

String takeSample(){
  //these timing constraints should always be true since we're waiting 20 mins,
  // but keep them in anyway for the time being
  if(millis()-analogSampleTime >= analogSampleInterval)  {
    analogSampleTime=millis();
     // subtract the last reading:
    analogValTotal = analogValTotal - readings[index];
    // read from the sensor:
    readings[index] = analogRead(ECsensorPin);
    // add the reading to the total:
    analogValTotal = analogValTotal + readings[index];
    // advance to the next position in the array:
    index = index + 1;
    // at end of the array
    if (index >= numReadings)
    // ...wrap around to the beginning:
    index = 0;
    // calculate the average:
    analogAv = analogValTotal/numReadings;
  }

  if(millis()-tempSampleTime >= tempSampleInterval){
    tempSampleTime=millis();
    temperature = TempProcess(ReadTemperature);  // read the current temperature from the  DS18B20
    TempProcess(StartConvert);                   //after the reading,start the convert for next reading
  }
  avVoltage=analogAv*(float)5000/1024;
  conductivity = 35.813*avVoltage+148.47; //linear function obtained from measurements
  timeSinceLast = ms_to_min(millis()- lastTime);

  return serialiseToJson(temperature, conductivity, timeSinceLast);
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
  sample+="\n}";

  lastTime = millis();
  sampleNumber++;

  return Sample;
}

unsigned long ms_to_min(unsigned long ms){
  return ms/60000;
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