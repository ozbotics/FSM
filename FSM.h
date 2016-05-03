/*
	FSM.h - Finite State Machine Package

	Created by Jon van Noort, August 2015.
  
  Defines classes:
    FsmUpdatable
    FsmCollection
    FsmSequence
    FsmState
    FsmDelay
    FsmStartTimer
    FsmWaitUntilTimerIsComplete
    FsmFinish
    FsmBranchOnEndOfList
    FsmDebugPrint
  
*/

//#define DEBUG_TRACE

#ifndef _FSM_H
 #define _FSM_H

#include <LinkedList.h>
#include <Timer.h>
#include <Value.h>
#include <ValueExpr.h>
#include <Condition.h>
#include <Enumerator.h>

typedef unsigned long Duration;


class FsmState;
class FsmCollection;
class FsmSequence;

class FsmUpdatable {
  protected:
    FsmCollection* _parent;
    
  public: 
    FsmUpdatable() { }  
    virtual void update()=0;
    
    virtual void forceExit() {}
    
    virtual void setParent(FsmCollection* parent) {
      _parent = parent;
    }
};

class FsmState : public FsmUpdatable {
  protected:
    bool _entered;
    bool _leaving;

    virtual void _enterState() { }
    
    virtual void _updateState() { }
    
    virtual void _exitState() { }

    virtual void _leaveState() {
      _exitState();
      
      _leaving = false;
      _entered = false;
    }
    
    void _transitionAncestorTo(byte childInd, byte depth);
    void _transitionAncestorToNext(byte depth);
    void _transitionAncestorToPrevious(byte depth);
    void _transitionAncestorToStart(byte depth);
    
    void _markAsLeaving();
    
  public: 
    FsmState() : _entered(false), _leaving(false), FsmUpdatable() { }
    
    virtual void update();

    virtual void forceExit();

    virtual void setParent(FsmSequence* parent) {
      FsmUpdatable::setParent((FsmCollection*) parent);
    }

};

class FsmCollection : public FsmState {
  protected:
    LinkedList<FsmUpdatable*> _children;

    virtual void _updateState();

    virtual void _leaveState() {
      FsmState::_leaveState();
      
      _forceDescendantsToExit();
      ///// _updateState();
    }
    
    virtual void _forceDescendantsToExit();
  
  public: 
    FsmCollection() : FsmState() { }
    
    ~FsmCollection() {
      for (byte i=0; i<_children.size(); i++) {
        delete _children.get(i);
      }
    }
    
    byte addChild(FsmUpdatable* child);
    
};

class FsmSequence : public FsmCollection {
  protected:
    byte _currentChildInd;
    byte _startChildInd;
    
    virtual void _enterState();
    virtual void _updateState();
    virtual void _exitState();
    
    void _transitionTo(byte childInd);
    void _transitionToNext();
    void _transitionToPrevious();
    void _transitionToStart();
    
    virtual void _forceDescendantsToExit();


  public:
    FsmSequence(byte startChildInd) : _startChildInd(startChildInd), _currentChildInd(startChildInd), FsmCollection() { }
    FsmSequence() : FsmSequence(0) { }

    void transitionAncestorTo(byte childInd, byte depth);
    void transitionAncestorToNext(byte depth);
    void transitionAncestorToPrevious(byte depth);
    void transitionAncestorToStart(byte depth);
    
    virtual void forceExit();
};

class FsmSelectStateFromCondition : public FsmSequence {
  protected:
    Value<bool>* _value;
    bool _oldValue;
    
    virtual void _enterState();
    virtual void _updateState();
    
  public:
    FsmSelectStateFromCondition(Value<bool>* value) : _value(value), FsmSequence() {}
};
class FsmDelay : public FsmState {
  protected:
    Timer* _timer;
    Value<Duration>* _durationValue;
    
    virtual void _enterState();
    virtual void _updateState();
    
  public:
    FsmDelay(Value<Duration>* durationValue) : _durationValue(durationValue), FsmState() {
      _timer = new Timer();  
    }
    
    ~FsmDelay() {
      delete _timer;
    }
};

class FsmStartTimer : public FsmState {
  protected:
    Timer* _timer;
    Value<Duration>* _durationValue;
    
    virtual void _enterState();
    
  public:
    FsmStartTimer(Timer* timer, Value<Duration>* durationValue) : _timer(timer), _durationValue(durationValue), FsmState() {}
};

class FsmWaitUntilTimerIsComplete : public FsmState {
  protected:
    Timer* _timer;
    
    virtual void _updateState();
    
  public:
    FsmWaitUntilTimerIsComplete(Timer* timer) : _timer(timer), FsmState() {}

};

class FsmFinish : public FsmState {
  protected:
    virtual void _enterState();
    
  public:
    FsmFinish() : FsmState() {}

};

class FsmBranchOnEndOfList : public FsmState {
  protected:
    EnumeratorBase* _enumerator;
    byte _branchInd;
    
    virtual void _enterState();
    
  public:
    FsmBranchOnEndOfList(EnumeratorBase* enumerator, byte branchInd=0) : _enumerator(enumerator), _branchInd(branchInd), FsmState() {}
};

class FsmFinishOnEndOfList : public FsmState {
  protected:
    EnumeratorBase* _enumerator;
    
    virtual void _enterState();
    
  public:
    FsmFinishOnEndOfList(EnumeratorBase* enumerator, byte branchInd=0) : _enumerator(enumerator), FsmState() {}
};


class FsmBranchOnConditionFalse : public FsmState {
  protected:
    Condition** _condition;
    byte _branchInd;
    
    virtual void _enterState();
    
  public:
    FsmBranchOnConditionFalse(Condition** condition, byte branchInd=0) : _condition(condition), _branchInd(branchInd), FsmState() {}
};


class FsmAssignConditionToValue : public FsmState {
  protected:
    Condition* _condition;
    Value<bool>* _value;
    
    virtual void _enterState();
    
  public:
    FsmAssignConditionToValue(Condition* condition, Value<bool>* value) : _condition(condition), _value(value), FsmState() {}
};


class FsmDebugPrint : public FsmState {
  protected:
    String* _msg;
    virtual void _enterState() {
      Serial.println(*_msg);
      _transitionAncestorToNext(1);
    }
    
  public:
    FsmDebugPrint(char* msg) : FsmState() {
      _msg = new String(msg);
    }
    
    ~FsmDebugPrint(){
      delete _msg;
    }
};

class FsmIdle : public FsmState {
    
  public:
    FsmIdle() : FsmState() {}
};


class FsmDebugState : public FsmState {
  protected:
    String* _msg;
  
  public:
    FsmDebugState(char* msg) : FsmState() {
      _msg = new String(msg);
    }
    
    virtual void _enterState() {
      Serial.print(F("Entering "));
      Serial.println(*_msg);
    }

    virtual void _updateState() {
      //Serial.print(F("Updating "));
      //Serial.println(*_msg);
    }
    
    virtual void _exitState() {
      Serial.print(F("Exiting "));
      Serial.println(*_msg);
    }
    
    ~FsmDebugState(){
      delete _msg;
    }
  
};


#endif  // _FSM_H