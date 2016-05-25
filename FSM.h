/** @file FSM.h 
  *  Copyright (c) 2016 Ozbotics 
  *  Distributed under the MIT license (see LICENSE)
  */ 
#ifndef _FSM_H
 #define _FSM_H

#include <LinkedList.h>
#include <Timer.h>
#include <Value.h>
#include <ValueExpr.h>
#include <Condition.h>
#include <Enumerator.h>


//#define DEBUG_TRACE


class FsmState;
class FsmCollection;
class FsmSequence;

/**
 * The base of all Finite State Machines (FSM)
 *
 * Requires that subclasses implement update()
 * Provides ability to attach to a parent FSM Collection
 */
class FsmUpdatable {
  protected:
    FsmCollection* _parent;  /**< protected variable  _parent Pointer to parent FSM (if any) */ 
    
  public:
  
   /**
    * Constructor
    */
    FsmUpdatable() { }  
    
   /**
    * require update method
    */
    virtual void update()=0;
    
   /**
    * over-ride this to define what happens when the FSM is forced to exit
    *  by default does nothing
    */
    virtual void forceExit() {}
    
   /**
    * attach the parent FSM
    *
    * @param parent Pointer to parent FSM
    */
    virtual void setParent(FsmCollection* parent) {
      _parent = parent;
    }
};

/* --------------------------------------------------------------------------------------- */

/**
 * The base of all FSM States
 *
 * Derived classes may specify behaviour for
 *  enterState
 *  updateState
 *  exitState
 *
 * Provides ability to request that the Parent FSM (or Ancestor) transition to another State
 */
class FsmState : public FsmUpdatable {
  protected:
    bool _entered;  /**< protected variable  _entered Is Entering State */ 
    bool _leaving;  /**< protected variable  _leaving Is Leaving State */ 

   /**
    * over-ride this to define what happens when the State is Entered
    *  by default does nothing
    *  (this is where to do start-up behaviour, eg; turn on a devices
    */
    virtual void _enterState() { }
    
   /**
    * over-ride this to define what happens when the State is Update
    *  by default does nothing 
    *  (this is where to make decisions about leaving the state)
    */
    virtual void _updateState() { }
    
   /**
    * over-ride this to define what happens when the State is Exited
    *  by default does nothing
    *  (this is where to do shut-up behaviour, eg; turn off a devices
    */
    virtual void _exitState() { }

   /**
    * handle leaving the state.
    *  calls user defined _exitState()
    */
    virtual void _leaveState() {
      _exitState();
      
      _leaving = false;
      _entered = false;
    }
    
   /**
    * make request to leave state by requesting that an Ancestor (usually the Parent) move its focus to the next state
    *  called from within _updateState()
    *
    * For example, Parent FSM 'Container', contains States A, B, C
    *  When A::_updateState() calls _transitionAncestorToNext(1)
    *   Container, currently focused on state A, will transition to state B
    *   Along the way, A::_exitState will be called (giving it the chance to stop devices and such)
    *   Then, as the transistion is made, B::_enterState is called (giving it the chance to start devices and such)
    * 
    * @param depth The number of ancestor hops (parent=1)
    */
    void _transitionAncestorToNext(byte depth);

   /**
    * make request to leave state by requesting that an Ancestor (usually the Parent) move its focus to the previous state
    *  called from within _updateState()
    *
    * @param depth The number of ancestor hops (parent=1)
    */
    void _transitionAncestorToPrevious(byte depth);
    
   /**
    * make request to leave state by requesting that an Ancestor (usually the Parent) move its focus to the next state
    *  called from within _updateState()
    *
    * @param childInd The target state identifier
    * @param depth The number of ancestor hops (parent=1)
    */
    void _transitionAncestorTo(byte childInd, byte depth);
    
   /**
    * make request to leave state by requesting that an Ancestor (usually the Parent) move its focus to its first state
    *  called from within _updateState()
    *
    * @param depth The number of ancestor hops (parent=1)
    */
    void _transitionAncestorToStart(byte depth);
    
   /**
    * request leave state
    */
    void _markAsLeaving();
    
  public:
   /**
    * Constructor
    */
    FsmState() : _entered(false), _leaving(false), FsmUpdatable() { }
    
   /**
    * Implement the update Interface
    *  on update, call _enterState(), _updateState() & _exitState (via _leaveState()) as required
    */
    virtual void update();

   /**
    * Implement the forceExit Interface
    *  on forceExit, call _exitState (via _leaveState())
    */
    virtual void forceExit();

   /**
    * attach the Parent FSM
    */
    virtual void setParent(FsmCollection* parent) {
      FsmUpdatable::setParent(parent);
    }

};

/* --------------------------------------------------------------------------------------- */

/**
 * The base of all FSM Collections
 *
 * Provides a means to group FSMs into a Collection
 * All child States are considered to be active and consequently execute 'simultaneously'
 */
class FsmCollection : public FsmState {
  protected:
    LinkedList<FsmUpdatable*> _children;  /**< protected variable  _children List of pointers to child FSMs */ 

   /**
    * update all child States
    */
    virtual void _updateState();

   /**
    * overr-ride leaveState
    *  do normal_leaveState and also force children (And descendants) to exit
    */
    virtual void _leaveState() {
      FsmState::_leaveState();
      
      _forceDescendantsToExit();
    }
    
   /**
    * request that all children forceExit
    */
    virtual void _forceDescendantsToExit();
  
  public: 
  
   /**
    * Constructor
    */
    FsmCollection() : FsmState() { }
    
   /**
    * Destructor
    */
    ~FsmCollection() {
      for (byte i=0; i<_children.size(); i++) {
        delete _children.get(i);
      }
    }
    
   /**
    * add a child State
    *
    * @param child Pointer to an FSM
    */
    byte addChild(FsmUpdatable* child);
    
};

/* --------------------------------------------------------------------------------------- */

/**
 * The base of all FSM Sequences
 *
 * Provides a means to group FSMs into an ordered Sequence
 * Only one child State is considered to be active at any one. 
 * The other child States are inactive nad nust wait until they are transitioned to
 */
class FsmSequence : public FsmCollection {
  protected:
    byte _currentChildInd;             /**< protected variable  _currentChildInd Index of the currently selected state */
    byte _startChildInd;               /**< protected variable  _startChildInd Index of the start state */
    
   /**
    * over-ride _enterState
    *  do debug tracing
    */
    virtual void _enterState();
    
   /**
    * over-ride _enterState
    *  update the focused State
    */
    virtual void _updateState();
    
   /**
    * over-ride _exitState
    *  do debug tracing
    */
    virtual void _exitState();
    
   /**
    * Transition to specified State
    *
    * @param childInd The index of the requested state
    */
    void _transitionTo(byte childInd);
    
   /**
    * Transition to the next State
    */
    void _transitionToNext();
    
   /**
    * Transition to the previous State
    */
    void _transitionToPrevious();
    
   /**
    * Transition to the the start State
    */
    void _transitionToStart();
    
   /**
    * request that focused child state forceExit
    */
    virtual void _forceDescendantsToExit();


  public:
   /**
    * Constructor
    *
    * @param startChildInd The index of the Start Child State, defaults to 0
    */
    FsmSequence(byte startChildInd) : _startChildInd(startChildInd), _currentChildInd(startChildInd), FsmCollection() { }
    FsmSequence() : FsmSequence(0) { }

   /**
    * make request to leave state by requesting that an Ancestor (usually the Parent) move its focus to the next state
    *  called from within _updateState()
    *
    * @param depth The number of ancestor hops (parent=1)
    */
    void transitionAncestorToNext(byte depth);
    
   /**
    * make request to leave state by requesting that an Ancestor (usually the Parent) move its focus to the previous state
    *  called from within _updateState()
    *
    * @param depth The number of ancestor hops (parent=1)
    */
    void transitionAncestorToPrevious(byte depth);

   /**
    * make request to leave state by requesting that an Ancestor (usually the Parent) move its focus to the next state
    *  called from within _updateState()
    *
    * @param childInd The is of the target state
    * @param depth The number of ancestor hops (parent=1)
    */
    void transitionAncestorTo(byte childInd, byte depth);

   /**
    * make request to leave state by requesting that an Ancestor (usually the Parent) move its focus to its first state
    *  called from within _updateState()
    *
    * @param depth The number of ancestor hops (parent=1)
    */
    void transitionAncestorToStart(byte depth);
    
   /**
    * over-ride forceExit to reset focus to start State
    */
    virtual void forceExit();
};

/* --------------------------------------------------------------------------------------- */

/**
 * Choose between two states based on a Condition (Value<bool>)
 *
 * When the Condition changes, the selected state is forced to exit the other state becomes focused
 */
class FsmSelectStateFromCondition : public FsmSequence {
  protected:
    Value<bool>* _value;  /**< protected variable _value Pointer to the Condition (Value<bool>) */ 
    bool _oldValue;       /**< protected variable _oldValue last value */ 
    
   /**
    * over-ride _enterState to select initial focused state
    */
    virtual void _enterState();
    
   /**
    * over-ride _updateState to force transition when value changes
    */
    virtual void _updateState();
    
  public:
  
   /**
    * Constructor
    *
    * @param value Pointer to the Condition (Value<bool>) 
    */
    FsmSelectStateFromCondition(Value<bool>* value) : _value(value), FsmSequence() {}
};

/* --------------------------------------------------------------------------------------- */

/**
 * Remain in this State for the specified duration 
 *
 * When the duration has passed, transition to the next state
 */
class FsmDelay : public FsmState {
  protected:
    Timer* _timer;                    /**< protected variable  _timer Timer used to countdown duration */
    Value<Duration>* _durationValue;  /**< protected variable _value Pointer to the duration Value (Value<Duration>) */
    
   /**
    * over-ride _enterState to start the timer (using the duration specified by _durationValue 
    */
    virtual void _enterState();
    
   /**
    * over-ride _updateState to request transitioon when the timer is complete
    */
    virtual void _updateState();
    
  public:
  
   /**
    * Constructor
    *
    * @param durationValue Pointer to the duration Value
    */
    FsmDelay(Value<Duration>* durationValue) : _durationValue(durationValue), FsmState() {
      _timer = new Timer();  
    }
    
   /**
    * Destructor
    */
    ~FsmDelay() {
      delete _timer;
    }
};

/* --------------------------------------------------------------------------------------- */

/**
 * Start the provided timer
 *
 * Start the timer, then progress to next state. 
 * Another state ('FsmWaitUntilTimerIsComplete') will monitor the timer.
 */
class FsmStartTimer : public FsmState {
  protected:
    Timer* _timer;                   /**< protected variable  _parent Pointer to the Timer */ 
    Value<Duration>* _durationValue; /**< protected variable _value Pointer to the duration Value (Value<Duration>) */
    
   /**
    * over-ride _enterState to start the timer (using the duration specified by _durationValue, 
    *   then request transitioon to next state
    */
    virtual void _enterState();
    
  public:
   /**
    * Constructor
    *
    * @param timer Pointer to the Timer
    * @param durationValue Pointer to the duration Value
    */
    FsmStartTimer(Timer* timer, Value<Duration>* durationValue) : _timer(timer), _durationValue(durationValue), FsmState() {}
};

/* --------------------------------------------------------------------------------------- */

/**
 * Wait for the provided timer to complete
 *
 * Works in conjunction with 'FsmStartTimer'
 * Transitions to the next state when the timer is complete
 */
class FsmWaitUntilTimerIsComplete : public FsmState {
  protected:
    Timer* _timer;  /**< protected variable  _timer Pointer to the Timer */ 
    
   /**
    * over-ride _updateState to request transitioon when the timer is complete
    */
    virtual void _updateState();
    
  public:
   /**
    * Constructor
    *
    * @param timer Pointer to the Timer
    */
    FsmWaitUntilTimerIsComplete(Timer* timer) : _timer(timer), FsmState() {}

};

/* --------------------------------------------------------------------------------------- */

/**
 * Marks the end of a Sequence.
 *
 * Causes the parent FSM to become reset
 *  and the parent's parent FSM to transition to the next state
 */
class FsmFinish : public FsmState {
  protected:
   /**
    * over-ride _updateState to request transitioon to start and ancestor transition to next
    */
    virtual void _enterState();
    
  public:
   /**
    * Constructor
    */
    FsmFinish() : FsmState() {}

};

/* --------------------------------------------------------------------------------------- */

/**
 * If possible, increment the provided Enumerator, otherwise Transition to the specified state
 *
 * FSMs that handle lists of 'things' can use FsmBranchOnEndOfList to increment the Enumerator
 *  then progress to the next state (presumably to do something with the Enumerator's current item)
 *  after possibly a number of States, the parent Sequence will loop around and this state will be encountered again
 *  Ultimately the Enumerator will reach the end of it's list, 
 *   at which point this state will request a transition to the specified state
 */
class FsmBranchOnEndOfList : public FsmState {
  protected:
    EnumeratorBase* _enumerator;  /**< protected variable _enumerator The Enumerator used to traverse a List */
    byte _branchInd;              /**< protected variable _branchInd The state to transition to at end of list */
    
   /**
    * over-ride _updateState to iterate enumerator and request a transition when necessary
    */
    virtual void _enterState();
    
  public:
   /**
    * Constructor
    *
    * @param enumerator The Enumerator used to traverse a List
    * @param branchInd The state to transition to at end of list 
    */  
    FsmBranchOnEndOfList(EnumeratorBase* enumerator, byte branchInd=0) : _enumerator(enumerator), _branchInd(branchInd), FsmState() {}
};

/* --------------------------------------------------------------------------------------- */

/**
 * If possible, increment the provided Enumerator, otherwise Finish (see FsmFinish)
 *
 * Same as FsmBranchOnEndOfList, however rather than branching the Sequence simply finishes
 */
class FsmFinishOnEndOfList : public FsmState {
  protected:
    EnumeratorBase* _enumerator;  /**< protected variable _enumerator The Enumerator used to traverse a List */
    
   /**
    * over-ride _updateState to iterate enumerator and finish when necessary
    */
    virtual void _enterState();
    
  public:
   /**
    * Constructor
    *
    * @param enumerator The Enumerator used to traverse a List
    * @param branchInd The id of the state to branch to
    */  
    FsmFinishOnEndOfList(EnumeratorBase* enumerator, byte branchInd=0) : _enumerator(enumerator), FsmState() {}
};

/* --------------------------------------------------------------------------------------- */

/**
 * If possible, increment the provided Enumerator, otherwise Transition to the specified state
 *
 * FSMs that need to vary their behaviour based on a Condition can use 'FsmBranchOnConditionFalse'
 *  It can cause a Sequence to take a different path through its child states based on a Condition Value<bool>)
 */
class FsmBranchOnConditionFalse : public FsmState {
  protected:
    Condition** _condition;  /**< protected variable _condition Pointer to Pointer to Condition */
    byte _branchInd;         /**< protected variable _branchInd The state to transition to at end of list */
    
   /**
    * over-ride _enterState to evaluate the condition and transition to next or branch to the specified state
    */
    virtual void _enterState();
    
  public:
   /**
    * Constructor
    *
    * @todo - is there a better way to do this than using 'Pointer to Pointer to Condition'
    *   note: it is this way because I needed to change the Condition at runtime
    *
    * @param condition Pointer to Pointer to Condition used to decide on wether to branch
    * @param branchInd The state to transition when the condition evaluates to false
    */
    FsmBranchOnConditionFalse(Condition** condition, byte branchInd=0) : _condition(condition), _branchInd(branchInd), FsmState() {}
};

/* --------------------------------------------------------------------------------------- */

/**
 * Update the provided Value to be the value of the evaluated Condition
 *
 * FSMs that need to expose information to other processes can use 'FsmAssignConditionToValue'
 *  to update a Value<bool> with the value of the Condition at the time that the State was active
 *
 * @todo - make equivalent 'FsmAssignValueExpressionToValue'
 */
class FsmAssignConditionToValue : public FsmState {
  protected:
    Condition* _condition;  /**< protected variable _condition Pointer to Condition */
    Value<bool>* _value;    /**< protected variable _value Pointer to the output Value (Value<bool>) */
    
   /**
    * over-ride _enterState to assign the value of the condition to the Value, then transition to next
    */
    virtual void _enterState();
    
  public:
  /**
    * Constructor
    *
    * @param condition Pointer to Pointer to Condition used to decide on wether to branch
    * @param value The Value that will be set to the value of the Condition
    */
    FsmAssignConditionToValue(Condition* condition, Value<bool>* value) : _condition(condition), _value(value), FsmState() {}
};

/* --------------------------------------------------------------------------------------- */

/**
 * Output a Debug Message to Serial
 *
 * FSMs that need to provide diagnostic output can use 'FsmAssignConditionToValue'
 *  to send a literal charcter string to Serial
 */
class FsmDebugPrint : public FsmState {
  protected:
    String* _msg;  /**< protected variable _msg Pointer to message String */
    
   /**
    * over-ride _enterState to display the message then request transition to next
    */
    virtual void _enterState() {
      Serial.println(*_msg);
      _transitionAncestorToNext(1);
    }
    
  public:
   /**
    * Constructor
    *
    * @param msg Literal character string
    */
    FsmDebugPrint(char* msg) : FsmState() {
      _msg = new String(msg);
    }
    
   /**
    * Destructor
    */
    ~FsmDebugPrint(){
      delete _msg;
    }
};

/* --------------------------------------------------------------------------------------- */

/**
 * Idle
 *
 * FSMs that need to perform particular behavior when a condition is met, may need to use 'FsmIdle'
 *  to do *nothing* when the condition is not met.
 *
 * FsmIdle has no way to initiate a Transition. It will only transition aay when forced to by a ancestot transition.
 * FsmIdle(s) are typically used with 'FsmSelectStateFromCondition'
 */
class FsmIdle : public FsmState {
    
  public:
   /**
    * Constructor
    */
    FsmIdle() : FsmState() {}
};

/* --------------------------------------------------------------------------------------- */

/**
 * DebugState
 *
 * FSMs that need to provide diagnostic output about state changes forced externally can use 'FsmDebugState'
 *  to send messages to Serial on Enter & Exit of this DebugState 
 *
 * FsmDebugState(s) are typically used with 'FsmSelectStateFromCondition'
 */
class FsmDebugState : public FsmState {
  protected:
    String* _msg;  /**< protected variable _msg Pointer to message String */
  
  public:
   /**
    * Constructor
    *
    * @param msg Literal character string
    */
    FsmDebugState(char* msg) : FsmState() {
      _msg = new String(msg);
    }
    
   /**
    * over-ride _enterState to display debout output on enter state
    */
    virtual void _enterState() {
      Serial.print(F("Entering "));
      Serial.println(*_msg);
    }

   /**
    * over-ride _enterState to display debout output on update state
    */
    virtual void _updateState() {
      //Serial.print(F("Updating "));
      //Serial.println(*_msg);
    }
    
   /**
    * over-ride _exitState to display debout output on exit state
    */
    virtual void _exitState() {
      Serial.print(F("Exiting "));
      Serial.println(*_msg);
    }
    
   /**
    * Destructor
    */
    ~FsmDebugState(){
      delete _msg;
    }
  
};


#endif  // _FSM_H