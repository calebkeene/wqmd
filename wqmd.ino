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

int sampleSpeedSW;//toggle switch for selected fast or slow sampling
int sampleNumber = 0;
String data[50]; //store samples
String device_id = "fluid_sol_01";
unsigned long lastSampleTime = 0;

void setup(){
	Serial.begin(115200);
	pinMode(ECsensorPin, INPUT); //conductivity sensor
	pinMode(2, INPUT); //sample speed selection switch on pin 2
}

void loop(){
	
	readSerial(); // will send data if it is being requested
	sampleSpeedSW = digitalRead(2);
	
	if(sampleSpeedSW == 0){ // slow sample rate (24 hours deployment)
		if(millis() >= (lastSampleTime + 1200000)){ // ~20 mins has passed since last sample taken
			takeSample();
		}
	}
	else{ // fast sample rate (up to ~2 hours deployment)
		if(millis() >= (lastSampleTime + 240000)){ // ~4 mins has passed since last sample taken
			takeSample();
		}
	}
}

void takeSample(){
	
	int i;
	for(i=0; i<25; i++){ //get conductivity average over 25 samples
		AnalogValueTotal += analogRead(ECsensorPin);		
	}
	AnalogAverage = AnalogValueTotal / 25;

	tempProcess(StartConvert);
	delay(800);
	temperature = tempProcess(ReadTemperature);
	averageVoltage=AnalogAverage*(float)5000/1024; //millivolt average,from 0mv to 4995mV

	//temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.0185*(fTP-25.0));
	float tempCoefficient=1.0+0.0185*(temperature-25.0);
	float voltageCoefficient=(float)averageVoltage/tempCoefficient;

	if(voltageCoefficient<=448)conductivity=6.84*voltageCoefficient-64.32;   //1ms/cm<EC<=3ms/cm
  else if(voltageCoefficient<=1457)conductivity=6.98*voltageCoefficient-127;  //3ms/cm<EC<=10ms/cm
  else conductivity=5.3*voltageCoefficient+2278;                           //10ms/cm<EC<20ms/cm
  conductivity/=1000; //convert us/cm to ms/cm
  storeSampleJSONObject(temperature, conductivity);
}

void readSerial(){
	
	char json[150]; // Allocate some space for the string
	if (Serial.available()){
		
	 	char currChar; // store current character being read
 		byte index = 0; // Index into array; where to store the character
	  unsigned long start_time = millis();

	  while(Serial.available() > 0){
	  	if(index < (sizeof(json)-1)){ // keep within array index bounds
	    	currChar = Serial.read(); // Read char
	      json[index] = currChar; 
	      index++; 
	      json[index] = '\0'; // Null terminate the string
	    }
		  // if greater than 10 seconds have passed, break out (timeout)
		  if((millis() - start_time) > 10000){
		  	break;
		  }
		}
	}
	// now parse String (char array) to JSON object
  StaticJsonBuffer<16> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);

  if (root.success()){
  	char* cmd = root["cmd"];
  	if(cmd == "RetrieveData"){
  		sendData();
  	}
  }
  else{
    Serial.println("parseObject() failed");
    return;
	}
}

void sendData(){
	int i;
	String json = "{\n\"Samples\":[";
	
	for(i=0; i<=sampleNumber; i++){
		//read samples from data array, serialise to JSON
		json += data[i];
		if(i !=sampleNumber){ //only add comma between JSON objs if not at end of data array
			json += ",\n";
		}
	}

	json += "\n]\"TimeSinceLast\":";
	json += (String)ms_to_min(millis() - lastSampleTime);
	json += "\n}";

	Serial.println(json); // send data
}

void storeSampleJSONObject(float temp, float cond){
	//char buf[10];
	String sample;
  sample += "{\"DeviceID\":\"";
  sample += device_id;

  sample += "\",\n\"SampleID\":";
  sample += (String)sampleNumber;
  
  sample += ",\n\"TimeSinceLast\":";
  sample += (String)ms_to_min(millis() - lastSampleTime);
  
  sample += ",\n\"Temperature\":";
  sample += (String)temp;
  
  //dtostrf(conductivity,2,2,buf);
  sample += ",\n\"Conductivity\":";
  sample += (String)cond;
  sample+="\n}";

  data[sampleNumber] = sample;
  lastSampleTime = millis();
	sampleNumber++; //iterate for next sample
}

unsigned long ms_to_min(unsigned long milli_seconds){
	return (milli_seconds * 6000);
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