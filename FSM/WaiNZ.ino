#include <FiniteStateMachine.h>

#include <FiniteStateMachine.h>
//#include <FSM_States.h>
#include <LowPower.h>


//define the finite state machine & its states
State start = State(startEnter,startStartEnd,startConnection,startMeasure,startMeasureEnd,startDisconnection,startAction,startExit);
State energySaving = State(energySavingEnter,energySavingStartEnd,energySavingConnection,energySavingMeasure,energySavingMeasureEnd,energySavingDisconnection,energySavingAction,energySavingExit);
State measure = State(measureEnter,measureStartEnd,measureConnection,measureMeasure,measureMeasureEnd,measureDisconnection,measureAction,measureExit);                           
State paired = State(pairedEnter,pairedStartEnd,pairedConnection,pairedMeasure,pairedMeasureEnd,pairedDisconnection,pairedAction,pairedExit);
State shut_down = State(shutdownEnter,shutdownStartEnd, shutdownConnection,shutdownMeasure,shutdownMeasureEnd,shutdownDisconnection, shutdownAction,shutdownExit);
//initialise FSM to Start Mode
FSM fsm = FSM(start);

 
//External Interrupt Service Routines
//Interrupt Service Routine to process a bluetooth connection event (using the serial connection, on digital pin 2 (RX))
void connection()
{
  //prevent further interrupts being sent on pin2 
  //detachInterrupt(1);
  //FSM CONNECTION EVENT transition to paired state but dont log it here. Don't use serial in the ISR thread.
  fsm.connectionEvent();
}   


  
 void setup() {
 
  //define the connection interrupt [interrupt 0 == Digital Pin 2 (Serial RX)]
  EIFR = 1;  // clear flag for interrupt 0. any queued interrupts are removed
  attachInterrupt(0, connection, FALLING);

  //initiate the serial connection
  Serial.begin(115200);
  
 }

void loop() {
 
  //let the state do what it needs to do (measure/transmit)
  fsm.action();
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
    Serial.println("Called startatartend");
    fsm.immediateTransitionTo(energySaving);   
}
void energySavingStartEnd(){Serial.println("Called startatartend2");}
void pairedStartEnd(){Serial.println("Called startatartend3");}
void measureStartEnd(){Serial.println("Called startatartend4");}
void shutdownStartEnd(){Serial.println("Called startatartend5");}
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
void pairedMeasure(){}
void measureMeasure(){}
void shutdownMeasure(){}
//State Measure End Event Functions
void startMeasureEnd(){}
void energySavingMeasureEnd(){}
void pairedMeasureEnd(){}
void measureMeasureEnd(){
 //measurements have finished transition back to energy saving mode
 // fsm.resetTimeSinceLast();
  fsm.transitionTo(energySaving);  
}
void shutdownMeasureEnd(){}

//State Action Functions
void startAction(){
  //temp instead of using the timer to transition state 
  //- just transition straight to energy saving mode
 //FSM START EVENT transition to energy saving state
 Serial.println("Start->EnergySaving");
 fsm.startEvent();
}

void energySavingAction(){
  //go into energy saving mode
  unsigned long timeSinceLast = fsm.getTimeSinceLast();
  unsigned long measurementInterval = fsm.getMeasurementInterval();
  //to add : wait here for 'measurement interval' seconds
  Serial.println("Debug:EnergySaving PowerDown 24s");
  delay(300);
  Serial.println(millis());
  //still to config low power settings
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  Serial.println(millis());
  fsm.setTimeSinceLast(fsm.getTimeSinceLast() + 8000);    
  Serial.println("EnergySaving->Measure");
  //FSM EVENT - transition to measure state
  fsm.measureEvent();
}
void pairedAction(){
 
  //check serial connection, parse incoming data, return measurements 
  //temp - finished transmitting, just go back to energysaving state
   Serial.println("Paired->EnergySaving");
  //FSM DISCONNECTION EVENT - transition to measure state
  fsm.disconnectionEvent(); 
}

void measureAction(){
  //take a couple of measurements for each sensor. Average the results  
  //simulate measurement code delay
  delay(2000);
  Serial.println("Measure->EnergySaving");
  //FSM MEASURE EVENT - transition to energysaving state
  fsm.measureEndEvent();
}
void shutdownAction(){
 //write to log. Flash LED.... 
 }


