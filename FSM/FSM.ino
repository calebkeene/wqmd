#include <ArduinoJson.h>
#include <FiniteStateMachine.h>
#include <OneWire.h>
#include <LowPower.h>
#include <avr/power.h> 
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <EEPROM.h>

#define StartConvert 0
#define ReadTemperature 1
byte ECsensorPin = A1;  //EC Meter analog output,pin on analog 1
byte DS18B20_Pin = 10; //DS18B20 signal (temp sensor), pin on digital 10
byte mosFETPin = 5;  //mosFET power saving attached on digital pin 8

unsigned long AnalogValueTotal = 0; // for averaging conductivity
unsigned int AnalogAverage = 0, averageVoltage=0;
float temperature, conductivity;
//Temperature chip i/o
OneWire ds(DS18B20_Pin);  // on digital pin 10

int bluetoothSW;//toggle switch
int sampleNumber = 0;


String data[50]; //store samples
String device_id = "fluid_sol_01";
unsigned long lastSampleTime = 0;



//define the finite state machine & its states
State start = State(startEnter,startStartEnd,startConnection,startMeasure,startMeasureEnd,startDisconnection,startAction,startExit);
State energySaving = State(energySavingEnter,energySavingStartEnd,energySavingConnection,energySavingMeasure,energySavingMeasureEnd,energySavingDisconnection,energySavingAction,energySavingExit);
State measure = State(measureEnter,measureStartEnd,measureConnection,measureMeasure,measureMeasureEnd,measureDisconnection,measureAction,measureExit);                           
State paired = State(pairedEnter,pairedStartEnd,pairedConnection,pairedMeasure,pairedMeasureEnd,pairedDisconnection,pairedAction,pairedExit);
State shut_down = State(shutdownEnter,shutdownStartEnd, shutdownConnection,shutdownMeasure,shutdownMeasureEnd,shutdownDisconnection, shutdownAction,shutdownExit);
//initialise FSM to Start Mode
FSM fsm = FSM(start);
volatile int isr_triggered = 0;
 
//External Interrupt Service Routines
//Interrupt Service Routine to process a bluetooth connection event (using the serial connection, on digital pin 2 (RX))
void isr()
{
 
 
}   

 void setup() {
//mosFET enable pin
 pinMode(mosFETPin, OUTPUT);
//temp set measurement interval to 120 seconds
 fsm.setMeasurementInterval(585000);
  //initiate the serial connection
  Serial.begin(115200);
  
 }

void loop() {
 
  //let the state do what it needs to do (measure/transmit)
  fsm.action();
  //interrupts not working poll instead
  bluetoothSW = digitalRead(2);
  if(bluetoothSW == 1){
    fsm.connectionEvent();
  }
}





/* These methods are defined for each state in the finite state machine. There are five states, Start Up, Energy Saving, Measure,Paired, and Shutdown. 
 * Each method identifies what should be done when a specific event is received in that state.
 * Method Naming Convention : [state][event] (Enter and Exit functions are not events. Simply for performing neccessary actions exiting or entering a state)
 */
//State Enter Functions
void startEnter(){
  //read start time from non volatile memory (period of time we stay in the start state
  //create startEnd interrupt to tell us when we need to change state - this timer will be disabled in the ISR when time is reached
 //now back to what we are supposed to be doing.. 
 //run system check - is everything ok, if not write error to sd log and shutdown
 //fsm.immediateTransitionTo(shut_down); 
 //grab device id, sampling rate, sample id etc from non-volatile memory
}

void energySavingEnter(){ }
void pairedEnter(){}
void measureEnter(){}
void shutdownEnter(){}
//State Exit Functions
void startExit(){}
void energySavingExit(){}
void pairedExit(){}
void measureExit(){}
void shutdownExit(){}
//State Start Finished Event Functions
void startStartEnd(){
    //start up timed out. move to energy saving state
    fsm.immediateTransitionTo(energySaving);   
}
void energySavingStartEnd(){}
void pairedStartEnd(){}
void measureStartEnd(){}
void shutdownStartEnd(){}
//State Connection Event Functions
void startConnection(){
  fsm.immediateTransitionTo(paired);   
}
void energySavingConnection(){
   fsm.immediateTransitionTo(paired);
}
void pairedConnection(){
 //already connected do nothing on connection event   
}
void measureConnection(){
  //on connection event stop measurement
  //and connect?
  fsm.immediateTransitionTo(paired);
}
void shutdownConnection(){}
//State Disconnection Event Functions
void startDisconnection(){/*log error - unexpected event*/}
void energySavingDisconnection(){/*log error - unexpected event*/}
void pairedDisconnection(){
  fsm.immediateTransitionTo(energySaving);
}
void measureDisconnection(){/*log error - unexpected event*/}
void shutdownDisconnection(){/*log error - unexpected event*/}
//State Measure Event Functions
void startMeasure(){}
void energySavingMeasure(){
  
  fsm.transitionTo(measure);  
}
void pairedMeasure(){
 
  //APIv2 has test cmd requesting measurement, don't transition from paired
  takeSample(); 
  fsm.measureEndEvent();
}
void measureMeasure(){}
void shutdownMeasure(){}
//State Measure End Event Functions
void startMeasureEnd(){}
void energySavingMeasureEnd(){}
void pairedMeasureEnd(){
  //reset time
    fsm.resetTimeSinceLast(); 
  }
void measureMeasureEnd(){
 //measurements have finished transition back to energy saving mode

    //reset time
    fsm.resetTimeSinceLast();
  fsm.transitionTo(energySaving);  
}
void shutdownMeasureEnd(){}

//State Action Functions
void startAction(){
  //temp instead of using the timer to transition state 
  //- just transition straight to energy saving mode
 //FSM START EVENT transition to energy saving state
 //Serial.println("Start->EnergySaving");
 fsm.startEvent();
}

void energySavingAction(){
  //go into energy saving mode
  unsigned long timeSinceLast = fsm.getTimeSinceLast();
  unsigned long measurementInterval = fsm.getMeasurementInterval();
  unsigned long wut =  fsm.getWakeUpTime();
 
  //test code will need to be updated to add : wait here for 'measurement interval' seconds
  //write HIGH to mosFet pin to power down sensors and sd card
  digitalWrite(mosFETPin, HIGH); 
 
  //hack as interrupts aren't working - sleep for a short time but dont oversleep
  //still to config low power settings
  period_t sleep_time;
  int addTime = 0;
  if((timeSinceLast+8000)<measurementInterval){
    sleep_time = SLEEP_8S;
    addTime=8000;
 
  }
  else if ((timeSinceLast+4000)<measurementInterval){
    sleep_time= SLEEP_4S;
    addTime=4000;
 
  }
   else if ((timeSinceLast+2000)<measurementInterval){
    sleep_time= SLEEP_2S;
    addTime=2000;
 
  } 
    else if ((timeSinceLast+1000)<measurementInterval){
    sleep_time= SLEEP_1S;
    addTime=1000;  
 
  }else if ((timeSinceLast+500)<measurementInterval){
    sleep_time= SLEEP_500MS;
    addTime=500;  
  }else if ((timeSinceLast+250)<measurementInterval){
    sleep_time= SLEEP_250MS;
    addTime=250;  
  }else if ((timeSinceLast+120)<measurementInterval){
    sleep_time= SLEEP_120MS;
    addTime=120;  
 
  }
  unsigned long timeAwake = millis() - wut;
  unsigned long curTsl = timeSinceLast + timeAwake;
 float tsl = (float) curTsl/1000;
 
  //if we need to sleep save time details as timer shuts off
  if(addTime>0){   
    eepromWriteFloat(0,tsl);
    LowPower.powerDown(sleep_time, ADC_OFF, BOD_OFF);
    fsm.setWakeUpTime(millis());
    tsl = eepromReadFloat(0);
  }

  
  fsm.setTimeSinceLast(curTsl+addTime);
 

  //if time to measure FSM EVENT - transition to measure state
  //turn the sensors and sd card mback on
  if(fsm.getTimeSinceLast()>measurementInterval){
    Serial.println("EnergySaving->Measure");
    //reset eeprom to 0
   // eepromWriteFloat(0,0.0);
    //turn the sensors back on
    digitalWrite(mosFETPin, LOW); 
    fsm.measureEvent();
  }
}


void pairedAction(){
 
  //check serial connection, parse incoming data, return measurements 
  readSerial(); // will send data if it is being requested
  //temp - finished transmitting, just go back to energysaving state 
   Serial.println("Connected..Paired->EnergySaving");   
  //FSM DISCONNECTION EVENT - transition to energy saving state
  fsm.disconnectionEvent(); 
}

void measureAction(){
  //take a measurement and store it  
  takeSample();

  Serial.println("Measure->EnergySaving");
  //FSM MEASURE EVENT - transition to energy saving state
  fsm.measureEndEvent();
}
void shutdownAction(){
 //write to log. Flash LED.... 
 }


//-----------Caleb's -  General Methods to measure and to transmit data -----------------

void takeSample(){
  Serial.print("taking sample");
  Serial.print(analogRead(ECsensorPin));
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
  // Serialise to measurement to JSON
  serialiseToJson(temperature, conductivity);
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
        index++; // add to String
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

  if (!root.success()){
    //Serial.println("parseObject() failed");
    return;
  }

  const char* cmd = root["cmd"].asString(); //explicit cast on the end
  if(strcmp(cmd, "RetrieveData") == 0){ //if command from mobile app is RetrieveData
    if(isData() == 1){ //if there are any measurements to send
      sendData();
    }
  }
}

int isData(){
  // just check if there is at least 1 measurement
  if(sizeof(data[0]) > 1) return 1;
  else return 0;
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
  json += (String)ms_to_min(fsm.getTimeSinceLast());
  json += "\n}";
  
  //issue with this printing at the moment
  // undefined reference to `Print::print(String const&)'
  Serial.print(json); // send data
}

void serialiseToJson(float temp, float cond){
  // construct JSON obj for individual sample
  String sample;
  sample += "{\"DeviceID\":\"";
  sample += device_id;

  sample += "\",\n\"SampleID\":";
  sample += (String)sampleNumber;
  
  sample += ",\n\"TimeSinceLast\":";
  sample += (String)ms_to_min(fsm.getTimeSinceLast());
  
  sample += ",\n\"Temperature\":";
  sample += (String)temp;
  
  //dtostrf(conductivity,2,2,buf);
  sample += ",\n\"Conductivity\":";
  sample += (String)cond;
  sample+="\n}";

  data[sampleNumber] = sample;
 // lastSampleTime = millis();
  sampleNumber++; //iterate for next sample
}

unsigned long ms_to_min(unsigned long milli_seconds){
  return (milli_seconds * 60000);
}

float tempProcess(bool ch){ //returns temperature in degrees C
  static byte data[12];
  static byte addr[8];
  static float temperatureSum;
  if(!ch){
    if ( !ds.search(addr)) {
      //Serial.println("no more sensors on chain, reset search");
      ds.reset_search();
      return 0;
    }      
    if ( OneWire::crc8( addr, 7) != addr[7]) {
      //Serial.println("CRC not valid");
      return 0;
    }        
    if ( addr[0] != 0x10 && addr[0] != 0x28) {
      //Serial.print("Device not recognized");
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
//----------------EEPROM access-----------------------------
float eepromReadFloat(int address){
   union u_tag {
     byte b[4];
     float fval;
   } u;   
   u.b[0] = EEPROM.read(address);
   u.b[1] = EEPROM.read(address+1);
   u.b[2] = EEPROM.read(address+2);
   u.b[3] = EEPROM.read(address+3);
   return u.fval;
}
 
void eepromWriteFloat(int address, float value){
   union u_tag {
     byte b[4];
     float fval;
   } u;
   u.fval=value;
 
   EEPROM.write(address  , u.b[0]);
   EEPROM.write(address+1, u.b[1]);
   EEPROM.write(address+2, u.b[2]);
   EEPROM.write(address+3, u.b[3]);
}
 



