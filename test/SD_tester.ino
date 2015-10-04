  #include <SD.h>
  #include <SPI.h>

  String device_id = "fluid_sol_01";
  int sampleNumber = 0;
  unsigned long lastTime = 0;
  String values[4];
  const int chipSelect = 4; //SS on Dig. pin 4

  void setup(){
    Serial.begin(115200);
    Serial.println("setting up!");
    pinMode(10, OUTPUT);
    digitalWrite(5, HIGH); 
    if (!SD.begin(chipSelect)) {
      Serial.println("Card failed, or not present");
      // don't do anything more:
      return;
    }
    Serial.println("card initialized.");
    readAllFromSD();
  }

  void loop(){}

  void printDummyMeasurements(){
    int i;
    for(i =0; i< 4; i++){
      Serial.print("Printing element ");
      Serial.println((String)i);
      Serial.println(values[i]);
    }

  }


  void saveToSD(String sample){
    File dataFile = SD.open("data8.txt", FILE_WRITE);
   
    if (dataFile){
      Serial.println("saving to dataFile");
      dataFile.println(sample); //Sample is the string I want to write
      dataFile.close();
      // print to the serial port too:
      //Serial.println(sample);
    }  
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening data8.txt");
    } 
  }
  void readAllFromSD(){
    String temp;
    int count = 0;

    File dataFile = SD.open("data8.txt");
    if (dataFile) {
      Serial.print("data8.txt:");
      
      // read from the file until there's nothing else in it:
   
      while(dataFile.available()){

        char c = dataFile.read();

        if(c != 125){
          temp+= c;
        }
        else{
          temp+= "}";
          Serial.println(temp);
          delay(200);
          temp = "";
          count++;
        }
      }
      dataFile.close(); // close the file:
      printArrayVals();
    
      
    } 
    else{
      // if the file didn't open, print an error:
      Serial.println("error opening data8.txt");
    }
    
  }

  void printArrayVals(){
    int i;
    for(i = 0; i < 4; i++){
      Serial.println(values[i]);
    }
  }

  void buildDummyMeasurements(){ // just generate some test data to send (5 measurements)
   //Serial.println("building dummy measurements");
    int i;
    for(i = 0; i<4; i++){
      //Serial.print("adding dummy: ");
      //Serial.println((String)i);
      float t = random(10, 30);
      float c = random(500, 2500);
      String str = serialiseToJson(t, c, ms_to_min(millis() - lastTime));
      //saveToSD(str);
      values[i] = str;
      delay(200);
      lastTime = millis();
      sampleNumber++;

    }
  }

  String serialiseToJson(float temp, float cond, unsigned long t){
    // construct JSON obj for individual sample
    String sample;
    sample += "{\"DeviceID\":\"";
    sample += device_id;

    sample += "\",\n\"SampleID\":";
    sample += (String)sampleNumber;
    
    sample += ",\n\"TimeSinceLast\":";
    sample += (String)t;
    
    sample += ",\n\"Temperature\":";
    sample += (String)temp;
    
    //dtostrf(conductivity,2,2,buf);
    sample += ",\n\"Conductivity\":";
    sample += (String)cond;
    sample+="\n}";

   return sample;
  }

  unsigned long ms_to_min(unsigned long milli_seconds){
    return (milli_seconds/60000);
  }