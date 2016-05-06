#include <LinkedList.h>
#include <Timer.h>
#include <Value.h>

#include <TaggedItem.h>
#include <Enumerator.h>
#include <FilteredEnumerator.h>
#include <TagFilteredEnumerator.h>

#include <FSM.h>

FsmCollection root;

Timer delayTimer;
Value<Duration> delayDurationValue;


Timer sequenceTimer;
Value<Duration> sequenceDurationValue;


namespace Factor
{
   enum Type { NONE, T, EC, PH };
};

namespace Effect
{
   enum Type { NONE, UP, DOWN };
};

struct FactorEffect
{
  Factor::Type factor;
  Effect::Type effect;
  
  inline bool operator==(const FactorEffect& candidate)
  {
    return ( (factor == candidate.factor) && (effect == candidate.effect) );
  }

  inline bool operator!=(const FactorEffect& candidate)
  {
    return ( !operator==(candidate) );
  }
};

typedef TaggedItem<int, FactorEffect> IntFactorEffect;
typedef LinkedList<IntFactorEffect> IntFactorEffectLinkedList;
typedef TagFilteredEnumerator<IntFactorEffect, FactorEffect> IntFactorEffectFilteredEnumerator;

IntFactorEffectLinkedList ifeList;

/*
IntFactorEffectFilteredEnumerator ecUpEnum = IntFactorEffectFilteredEnumerator(&ifeList, (FactorEffect) { Factor::EC, Effect::UP }) ;
IntFactorEffectFilteredEnumerator ecDownEnum = IntFactorEffectFilteredEnumerator(&ifeList, (FactorEffect) { Factor::EC, Effect::DOWN }) ;
*/

typedef LinkedList<FactorEffect> FactorEffectLinkedList;
typedef Enumerator<FactorEffect> FactorEffectEnumerator;

FactorEffectLinkedList feList;
FactorEffectEnumerator feEnum = FactorEffectEnumerator(&feList) ;



class FsmDebugPrintFactorEffect : public FsmState {
  protected:
    FactorEffectEnumerator* _feEnumerator;
    
    virtual void _enterState() {
      //Serial.print(name);
      //Serial.println(" #Entered FsmDebugPrintFactorEffect::_enterState");
      
      FactorEffect fe = _feEnumerator->getCurrent();
      Serial.print(fe.factor);
      Serial.print(" ");
      Serial.println(fe.effect);
     
      _transitionAncestorToNext(1);

      //Serial.print(name);
      //Serial.println(" #Leaving FsmDebugPrintFactorEffect::_enterState");
    }
    
  public:
    FsmDebugPrintFactorEffect(FactorEffectEnumerator* feEnumerator) : _feEnumerator(feEnumerator), FsmState() { }
};

class FsmDebugPrintIntFactorEffect : public FsmState {
  protected:
    IntFactorEffectFilteredEnumerator* _ifeEnumerator;
    
    virtual void _enterState() {
      //Serial.print(name);
      //Serial.println(" #Entered FsmDebugPrintIntFactorEffect::_enterState");
      
      Serial.println(_ifeEnumerator->getCurrent().item);
      _transitionAncestorToNext(1);

      //Serial.print(name);
      //Serial.println(" #Leaving FsmDebugPrintIntFactorEffect::_enterState");

    }
    
  public:
    FsmDebugPrintIntFactorEffect(IntFactorEffectFilteredEnumerator* ifeEnumerator) : _ifeEnumerator(ifeEnumerator), FsmState() { }
};


class FsmUseIntFactorEffects : public FsmSequence {
  protected:
    IntFactorEffectFilteredEnumerator* _ifeEnumerator;
    FactorEffectEnumerator* _feEnumerator;
    
    virtual void _enterState() {
      //Serial.print(name);
      //Serial.println(" #Entered FsmUseIntFactorEffects::_enterState");

      //Serial.print(name);
      //Serial.println("  Resetting IFE Enumerator");
      
      _ifeEnumerator->reset();
      _ifeEnumerator->setFilterTag(_feEnumerator->getCurrent());

      //Serial.print(name);
      //Serial.println(" #Leaving FsmUseIntFactorEffects::_enterState");

    }
    
  public:
    FsmUseIntFactorEffects(IntFactorEffectLinkedList* ifeList, FactorEffectEnumerator* feEnumerator) : _feEnumerator(feEnumerator), FsmSequence(1) { 
      _ifeEnumerator = new IntFactorEffectFilteredEnumerator(ifeList, (FactorEffect) { Factor::NONE, Effect::NONE });
      
      addChild(new FsmFinish());
      //addChild(new FsmDebugPrint("Current IFE is.."));
      //addChild(new FsmDebugPrintIntFactorEffect(_ifeEnumerator));
      addChild(new FsmBranchOnEndOfList(_ifeEnumerator, 0));

      //addChild(new FsmDebugPrint("After MoveNext IFE is.."));
      addChild(new FsmDebugPrintIntFactorEffect(_ifeEnumerator));
    }

    ~FsmUseIntFactorEffects() {
      delete _ifeEnumerator;
    }

    
};

class FsmUseFactorEffects : public FsmSequence {
  protected:
    FactorEffectEnumerator* _feEnumerator;
    
    virtual void _enterState() {
      //Serial.print(name);
      //Serial.println(" #Entered FsmUseFactorEffects::_enterState");
      
      //Serial.print(name);
      //Serial.println("  Resetting FE Enumerator");
      
      _feEnumerator->reset();

      //Serial.print(name);
      //Serial.println(" #Leaving FsmUseFactorEffects::_enterState");

    }
    
  public:
    FsmUseFactorEffects(FactorEffectLinkedList* feList, IntFactorEffectLinkedList* ifeList) : FsmSequence(1) { 
      _feEnumerator = new FactorEffectEnumerator(feList);
      
      addChild(new FsmFinish());
      
      //addChild(new FsmDebugPrint("Current FE is.."));
      //addChild(new FsmDebugPrintFactorEffect(_feEnumerator));
      
      addChild(new FsmBranchOnEndOfList(_feEnumerator, 0));

      //addChild(new FsmDebugPrint("After MoveNext FE is.."));
      //addChild(new FsmDebugPrintFactorEffect(_feEnumerator));

      addChild(new FsmUseIntFactorEffects(ifeList, _feEnumerator));
    }

    ~FsmUseFactorEffects() {
      delete _feEnumerator;
    }
};


void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("Started");

  feList.add({ Factor::EC, Effect::UP });
  feList.add({ Factor::EC, Effect::DOWN });


  ifeList.add({ 1, { Factor::EC, Effect::UP } });
  ifeList.add({ 2, { Factor::EC, Effect::DOWN } });
  ifeList.add({ 3, { Factor::EC, Effect::UP } });
  ifeList.add({ 4, { Factor::EC, Effect::DOWN } });


  delayDurationValue.setValue(1000);
  sequenceDurationValue.setValue(5000);

  root.name = "Root";


  FsmSequence* seq0 = new FsmSequence();
  seq0->name = "Seq0";
  root.addChild(seq0);

  FsmUseFactorEffects* useFe = new FsmUseFactorEffects(&feList, &ifeList);
  useFe->name = "UseFe";
  seq0->addChild(useFe);

/*
  FsmSequence* seq1 = new FsmSequence();
  seq1->name = "Seq1";
  seq0->addChild(seq1);

  seq1->addChild(new FsmDebugPrint("Started Seq 1"));
  seq1->addChild(new FsmFinish());


  FsmSequence* seq2 = new FsmSequence();
  seq2->name = "Seq2";
  seq0->addChild(seq2);
  
  seq2->addChild(new FsmDebugPrint("Started Seq 2"));
  seq2->addChild(new FsmFinish());
*/

/*
  FsmUseIntFactorEffects* useEcUp = new FsmUseIntFactorEffects(&ifeList);
  useEcUp->name = "UseEcUp";
  seq0->addChild(useEcUp);
*/
/*
  FsmUseIntFactorEffects* useEcUp = new FsmUseIntFactorEffects(ecUpEnum);
  useEcUp->name = "UseEcUp";
  seq0->addChild(useEcUp);

  FsmUseIntFactorEffects* useEcDown = new FsmUseIntFactorEffects(ecDownEnum);
  useEcDown->name = "useEcDown";
  seq0->addChild(useEcDown);
*/


  
  
/*
  seq0->addChild(new FsmUseIntFactorEffects(ecUpEnum));
  seq0->addChild(new FsmUseIntFactorEffects(ecDownEnum));
*/

/*
  FsmSequence* seq1 = new FsmSequence(1);
  seq0->addChild(seq1);
  
  seq1->addChild(new FsmFinish());
  seq1->addChild(new FsmDebugPrint("Looping 1"));
  seq1->addChild(new FsmBranchOnEndOfList(ecUpEnum, 0));
  seq1->addChild(new FsmDebugPrintIntFactorEffect(ecUpEnum));
*/

/*
  FsmSequence* seq2 = new FsmSequence();
  seq0->addChild(seq2);

  seq2->addChild(new FsmDebugPrint("Starting 2"));
  seq2->addChild(new FsmDelay(delayDurationValue));
  seq2->addChild(new FsmDebugPrint("Ending 2"));
  seq2->addChild(new FsmFinish());
*/

/*  
  seq0->addChild(new FsmDebugPrint("Starting 1"));
  seq0->addChild(new FsmDelay(delayDurationValue));
  seq0->addChild(new FsmDebugPrint("Intermission 1"));
  seq0->addChild(new FsmDelay(delayDurationValue));
  seq0->addChild(new FsmDebugPrint("Try Ending 1"));
  seq0->addChild(new FsmWaitUntilTimerIsComplete(sequenceTimer));
  seq0->addChild(new FsmDebugPrint("Actual Ending 1"));
*/
  

}

void loop() {
  root.update();
}
