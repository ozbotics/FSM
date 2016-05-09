#include <FSM.h>

void FsmState::_markAsLeaving() {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmState::_markAsLeaving"));
#endif

  _leaving = true;
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmState::_markAsLeaving"));
#endif
}

void FsmState::_transitionAncestorTo(byte childInd, byte depth) {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmState::_transitionAncestorTo"));
#endif

  if (_parent) {
    if (depth == 1) { 
      _markAsLeaving(); 
    }
     
    ((FsmSequence*)_parent)->transitionAncestorTo(childInd, depth - 1);
  }
  else {
    //Serial.println(F("no ancestor"));
  }
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmState::_transitionAncestorTo"));
#endif
}

void FsmState::_transitionAncestorToNext(byte depth) {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmState::_transitionAncestorToNext"));
#endif

  if (_parent) {
    if (depth == 1) { 
      _markAsLeaving(); 
    }
    
    ((FsmSequence*)_parent)->transitionAncestorToNext(depth - 1);
  }
  else {
    //Serial.println(F("no ancestor"));
  }
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmState::_transitionAncestorToNext"));
#endif
}

void FsmState::_transitionAncestorToPrevious(byte depth) {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmState::_transitionAncestorToPrevious"));
#endif

  if (_parent) {
    if (depth == 1) { 
      _markAsLeaving(); 
    }
    
    ((FsmSequence*)_parent)->transitionAncestorToPrevious(depth - 1);
  }
  else {
    //Serial.println(F("no ancestor"));
  }

#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmState::_transitionAncestorToPrevious"));
#endif
}

void FsmState::_transitionAncestorToStart(byte depth) {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmState::_transitionAncestorToStart"));
#endif

  if (_parent) {
    if (depth == 1) { 
      _markAsLeaving(); 
    }
    
    ((FsmSequence*)_parent)->transitionAncestorToStart(depth - 1);
  }
  else {
    //Serial.println(F("no ancestor"));
  }

#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmState::_transitionAncestorToStart"));
#endif
}

void FsmState::update() { 
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmState::update"));
#endif
  
  if (!_entered) {
    _entered = true;
    _enterState();
  }
  
  _updateState();
  
  if (_leaving) {
    _leaveState();
  }
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmState::update"));
#endif
}

void FsmState::forceExit() {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmState::forceExit"));
#endif
  
  //_markAsLeaving();
  _leaveState();
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmState::forceExit"));
#endif
}

void FsmCollection::_updateState() { 
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmCollection::_updateState"));
#endif

  for (byte i=0; i<_children.size(); i++) {
  //  Serial.print("  Updating child "));
  //  Serial.println(i);
    _children.get(i)->update();
  }
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmCollection::_updateState"));
#endif
}

void FsmCollection::_forceDescendantsToExit() { 
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmCollection::_forceDescendatsToExit"));
#endif

  for (byte i=0; i<_children.size(); i++) {
    _children.get(i)->forceExit();
  }
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmCollection::_forceDescendatsToExit"));
#endif
}

    
    
byte FsmCollection::addChild(FsmUpdatable* child) {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmCollection::addChild"));
#endif

  child->setParent(this);
  _children.add(child);
  
  return _children.size() - 1;
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmCollection::addChild"));
#endif
}

void FsmSequence::_enterState() { 
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmSequence::_enterState"));
#endif
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmSequence::_enterState"));
#endif
}

void FsmSequence::_updateState() { 
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmSequence::_updateState"));
#endif

  _children.get(_currentChildInd)->update();
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmSequence::_updateState"));
#endif
}

void FsmSequence::_exitState() { 
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmSequence::_exitState"));
#endif

#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmSequence::_exitState"));
#endif
}


void FsmSequence::_transitionTo(byte childInd) {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmSequence::_transitionTo"));
#endif
  
  _currentChildInd = childInd;
  
  if (_currentChildInd >= _children.size()) {
    //_currentChildInd = 0;
    _currentChildInd = _startChildInd;
  }
  
  if (_currentChildInd < 0) {
    _currentChildInd = _children.size() - 1;
  }

#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmSequence::_transitionTo"));
#endif
}

void FsmSequence::_transitionToNext() {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmSequence::_transitionToNext"));
#endif
  
  _currentChildInd++;
  
  _transitionTo(_currentChildInd);
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmSequence::_transitionToNext"));
#endif
}

void FsmSequence::_transitionToPrevious() {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmSequence::_transitionToPrevious"));
#endif

  _currentChildInd--;
  
  _transitionTo(_currentChildInd);

#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmSequence::_transitionToPrevious"));
#endif
}


void FsmSequence::_transitionToStart() {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmSequence::_transitionToStart"));
#endif

  _currentChildInd = _startChildInd;
  _transitionTo(_currentChildInd);
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmSequence::_transitionToStart"));
#endif
}

void FsmSequence::_forceDescendantsToExit() { 
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmSequence::_forceDescendatsToExit"));
#endif

  _children.get(_currentChildInd)->forceExit();
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmSequence::_forceDescendatsToExit"));
#endif
}

void FsmSequence::transitionAncestorTo(byte childInd, byte depth) {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmSequence::transitionAncestorTo"));
#endif

  if (depth == 0)  {
    _transitionTo(childInd);
  } 
  else {
    if (_parent) {
      if (depth == 1) { 
        _markAsLeaving(); 
      }
      
      ((FsmSequence*)_parent)->transitionAncestorTo(childInd, depth - 1);
    }
  }
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmSequence::transitionAncestorTo"));
#endif
}

void FsmSequence::transitionAncestorToNext(byte depth) {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmSequence::transitionAncestorToNext"));
#endif

  if (depth == 0)  {
    _transitionToNext();
  } 
  else {
    if (_parent) {
      if (depth == 1) { 
        _markAsLeaving(); 
      }

      ((FsmSequence*)_parent)->transitionAncestorToNext(depth - 1);
    }
  }
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmSequence::transitionAncestorToNext"));
#endif
}

void FsmSequence::transitionAncestorToPrevious(byte depth) {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmSequence::transitionAncestorToPrevious"));
#endif

  if (depth == 0)  {
    _transitionToPrevious();
  } 
  else {
    if (_parent) {
      if (depth == 1) { 
        _markAsLeaving(); 
      }

      ((FsmSequence*)_parent)->transitionAncestorToPrevious(depth - 1);
    }
  }
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmSequence::transitionAncestorToPrevious"));
#endif
}

void FsmSequence::transitionAncestorToStart(byte depth) {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmSequence::transitionAncestorToStart"));
#endif

  if (depth == 0)  {
    _transitionToStart();
  } 
  else {
    if (_parent) {
      if (depth == 1) { 
        _markAsLeaving(); 
      }

      ((FsmSequence*)_parent)->transitionAncestorToPrevious(depth - 1);
    }
  }
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmSequence::transitionAncestorToStart"));
#endif
}

void FsmSequence::forceExit() {
  #ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmSequence::forceExit"));
#endif
  
  FsmState::forceExit();
  _transitionToStart();    
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmSequence::forceExit"));
#endif
}



void FsmSelectStateFromCondition::_enterState() {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmSelectStateFromCondition::_enterState"));
#endif
  
  _oldValue = _value->getValue();
  _transitionTo((byte) _value->getValue());
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmSelectStateFromCondition::_enterState"));
#endif
}

void FsmSelectStateFromCondition::_updateState() {
  #ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmSelectStateFromCondition::_updateState"));
#endif
  
  if (_oldValue != _value->getValue()) {
    _oldValue = _value->getValue();
    _forceDescendantsToExit();
    _transitionTo((byte) _value->getValue());
  }
  
  FsmSequence::_updateState();
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmSelectStateFromCondition::_updateState"));
#endif
}


void FsmDelay::_enterState() {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmDelay::_enterState"));
#endif

  _timer->start(_durationValue->getValue());
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmDelay::_enterState"));
#endif
}

void FsmDelay::_updateState() {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmDelay::_updateState"));
#endif

  if (_timer->isComplete()) {
    _transitionAncestorToNext(1);
  }
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmDelay::_updateState"));
#endif
}

void FsmStartTimer::_enterState() {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmStartTimer::_enterState"));
#endif

  _timer->start(_durationValue->getValue());
  _transitionAncestorToNext(1);
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmStartTimer::_enterState"));
#endif

}

void FsmWaitUntilTimerIsComplete::_updateState() {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmWaitUntilTimerIsComplete::_updateState"));
#endif

  if (_timer->isComplete()) {
    _transitionAncestorToNext(1);
  }
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmWaitUntilTimerIsComplete::_updateState"));
#endif
}

void FsmFinish::_enterState() {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmFinish::_enterState"));
#endif

  //Serial.print(F("Finishing "));
  //Serial.println(_parent->name);

  _transitionAncestorToStart(1);
  _transitionAncestorToNext(2);

#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmFinish::_enterState"));
#endif
}

void FsmBranchOnEndOfList::_enterState() {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmBranchOnEndOfList::_enterState"));
#endif

  if (_enumerator->moveNext()) {
    _transitionAncestorToNext(1);
  }
  else {
    _transitionAncestorTo(_branchInd, 1);
  }
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmBranchOnEndOfList::_enterState"));
#endif
}

void FsmFinishOnEndOfList::_enterState() {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmFinishOnEndOfList::_enterState"));
#endif

  if (_enumerator->moveNext()) {
    _transitionAncestorToNext(1);
  }
  else {
    _transitionAncestorToStart(1);
    _transitionAncestorToNext(2);
  }
  
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmFinishOnEndOfList::_enterState"));
#endif
}

void FsmBranchOnConditionFalse::_enterState() {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmBranchOnConditionFalse::_enterState"));
#endif
  
  if ((*_condition)->getValue()) {
    //Serial.println(F("Condition is True - Doing Actions"));
    _transitionAncestorToNext(1);
  }
  else {
    //Serial.println(F("Condition is False - Skipping Actions"));
    _transitionAncestorTo(_branchInd, 1);
  }

#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmBranchOnConditionFalse::_enterState"));
#endif
}


void FsmAssignConditionToValue::_enterState() {
#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Entered FsmAssignConditionToValue::_enterState"));
#endif
  
  //Serial.print(F("FsmAssignConditionToValue, value is "));
  //Serial.println(_condition->getValue());
  
  _value->setValue(_condition->getValue());
  _transitionAncestorToNext(1);

#ifdef DEBUG_TRACE
  //Serial.print(name);
  Serial.println(F(" #Leaving FsmAssignConditionToValue::_enterState"));
#endif
}
