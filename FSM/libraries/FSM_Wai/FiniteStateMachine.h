/* Modified version of  Alexander Brevig's FSM library
 * The Finite State Machine (FSM) class ensures the Water Quality Device is in the correct state at all times.
 * The FSM will transition the device to the correct state when events are detected 
 * Important events include : Start Up End Event (Internal), Connection Event (External), Disconnection Event (Internal), 
 * Measure Event (Internal), Measure End Event (Internal)
 * Author : Ed Storey
 * Team   : Fluid Solutions
 *
 */


#ifndef FINITESTATEMACHINE_H
#define FINITESTATEMACHINE_H

#include <Arduino.h>


#define NO_ENTER (0)
#define NO_ACTION (0)
#define NO_EXIT (0)

#define FSM FiniteStateMachine

//define the functionality of the states
class State {
	public:
  State( void (*enterFunction)(), void (*startFunction)(), void (*connectionFunction)(),void (*measureFunction)(),void (*measureEndFunction)(),void (*disconnectionFunction)(), void (*actionFunction)(), void (*exitFunction)() );
			
                void enter();
		void startEvent();
		void connectionEvent();
		void disconnectionEvent();
		void measureEvent();
		void measureEndEvent();
		void action();
		void exit();
	private:
       
		void (*userEnter)();
		void (*userAction)();
		void (*userExit)();
		void (*userStartEvent)();
		void (*userConEvent)();
		void (*userDisconEvent)();
		void (*userMeasureEvent)();
		void (*userMeasureEndEvent)();

};

//define the finite state machine functionality
class FiniteStateMachine {
	public:
		FiniteStateMachine(State& current);
		FiniteStateMachine& startEvent();
		FiniteStateMachine& connectionEvent();
		FiniteStateMachine& disconnectionEvent();
		FiniteStateMachine& measureEvent();
		FiniteStateMachine& measureEndEvent();
		FiniteStateMachine& action();
		FiniteStateMachine& transitionTo( State& state );
		FiniteStateMachine& immediateTransitionTo( State& state );
		
		State& getCurrentState();
		bool isInState( State &state ) const;
		unsigned long getMeasurementInterval() const;
		void setMeasurementInterval(unsigned long mi);
		unsigned long timeInCurrentState() const;
                void setTimeSinceLast(unsigned long tsl);
		unsigned long getTimeSinceLast() const;
		void resetTimeSinceLast();
		unsigned long getWakeUpTime() const;

		void setWakeUpTime(unsigned long wakeUp);
		
	private:
		bool 	needToTriggerEnter;
		State* 	currentState;
		State* 	nextState;
		unsigned long stateChangeTime = 0;
		unsigned long timeSinceLast = 0;
		unsigned long wakeUpTime = 0;
		unsigned long measurementInterval = 0;
};

#endif
