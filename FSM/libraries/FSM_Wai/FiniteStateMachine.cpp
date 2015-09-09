/* Modified version of  Alexander Brevig's FSM library
 * The Finite State Machine (FSM) class ensures the Water Quality Device is in the correct state at all times.
 * The FSM will transition the device to the correct state when events are detected 
 * Important events include : Start Up End Event (Internal), Connection Event (External), Disconnection Event (Internal), 
 * Measure Event (Internal), Measure End Event (Internal)
 * Author : Ed Storey
 * Team   : Fluid Solutions
 *
 */

#include "FiniteStateMachine.h" 

//FINITE STATE
State::State( void (*enterFunction)(),void (*startFunction)(),void (*connectionFunction)(),void (*measureFunction)(),void (*measureEndFunction)(),void (*disconnectionFunction)(), void (*actionFunction)(), void (*exitFunction)() ){
	userEnter = enterFunction;
	userStartEvent = startFunction;
	userConEvent = connectionFunction;
	userDisconEvent = disconnectionFunction;
	userMeasureEvent = measureFunction; 
	userMeasureEndEvent = measureEndFunction;  
	userAction = actionFunction;
	userExit = exitFunction;
}

//what to do when entering this state
void State::enter(){
	if (userEnter){
		userEnter();
	}
}

//what to do when the start up period has ended
void State::startEvent(){
       if(userStartEvent){
	 userStartEvent();
       }
}

//what to do when a bluetooth connection is established 
void State::connectionEvent(){
       if(userConEvent){
	 userConEvent();
       }

}

//what to do when the bluetooth connection is lost
void State::disconnectionEvent(){
 if(userDisconEvent){
	 userDisconEvent();
       }

}
//what to do when its time to measure the water quality
void State::measureEvent(){
 if(userMeasureEvent){
	 userMeasureEvent();
       }

}

//what to do when measurements have finished
void State::measureEndEvent(){
 if(userMeasureEndEvent){
	 userMeasureEndEvent();
       }

}

//what to do when this state actions
void State::action(){
	if (userAction){
		userAction();
	}
}

//what to do when exiting this state
void State::exit(){
	if (userExit){
		userExit();
	}
}
//END FINITE STATE


//FINITE STATE MACHINE
FiniteStateMachine::FiniteStateMachine(State& current){
	needToTriggerEnter = true;
	currentState = nextState = &current;
	stateChangeTime = 0;
}


FiniteStateMachine& FiniteStateMachine::startEvent() {currentState->startEvent(); return *this;}
FiniteStateMachine& FiniteStateMachine::measureEvent() {currentState->measureEvent(); return *this;}
FiniteStateMachine& FiniteStateMachine::measureEndEvent() {currentState->measureEndEvent(); return *this;}
FiniteStateMachine& FiniteStateMachine::connectionEvent() {currentState->connectionEvent(); return *this;}
FiniteStateMachine& FiniteStateMachine::disconnectionEvent() {currentState->disconnectionEvent(); return *this;}

FiniteStateMachine& FiniteStateMachine::action() {
	//simulate a transition to the first state
	//this only happens the first time action is called
	if (needToTriggerEnter) { 
		currentState->enter();
		needToTriggerEnter = false;
	} else {
		if (currentState != nextState){
			immediateTransitionTo(*nextState);
		}
		currentState->action();
	}
	return *this;
}

FiniteStateMachine& FiniteStateMachine::transitionTo(State& state){
        //millis wont increment when this function is called from an interrupt but that's ok
	nextState = &state;
	timeSinceLast = timeSinceLast + (millis() - stateChangeTime);
	stateChangeTime = millis();
	return *this;
}

FiniteStateMachine& FiniteStateMachine::immediateTransitionTo(State& state){
        //millis wont increment when this function is called from an interrupt but that's ok
	currentState->exit();
	currentState = nextState = &state;
	currentState->enter();
        timeSinceLast = timeSinceLast + (millis() - stateChangeTime);
	stateChangeTime = millis();
	return *this;
}

//return the current state
State& FiniteStateMachine::getCurrentState() {
	return *currentState;
}

//check if state is equal to the currentState
boolean FiniteStateMachine::isInState( State &state ) const {
	if (&state == currentState) {
		return true;
	} else {
		return false;
	}
}
//update measurement interval (to allow configurable change)
void FiniteStateMachine::setMeasurementInterval(unsigned long mi){
  measurementInterval = mi;
}

unsigned long  FiniteStateMachine::getMeasurementInterval()const{
 return measurementInterval;
}
unsigned long FiniteStateMachine::timeInCurrentState()const { 
return  millis() - stateChangeTime; 
}

void  FiniteStateMachine::setTimeSinceLast(unsigned long tsl){
  timeSinceLast = tsl;
}
unsigned long FiniteStateMachine::getTimeSinceLast()const {
  return timeSinceLast;
}
void FiniteStateMachine::resetTimeSinceLast() {
  timeSinceLast = 0;
}

//END FINITE STATE MACHINE
