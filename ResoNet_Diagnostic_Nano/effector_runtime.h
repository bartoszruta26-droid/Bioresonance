#ifndef RESONET_DIAGNOSTIC_EFFECTOR_RUNTIME_H
#define RESONET_DIAGNOSTIC_EFFECTOR_RUNTIME_H
#include <stdint.h>
namespace ResoNetDiagnostic {
static const uint8_t EFFECTOR_QUEUE_CAPACITY = 8;
static const uint8_t EFFECTOR_CHANNEL_COUNT = 3;
enum EffectorId : uint8_t { EFFECTOR_IR_LED=0, EFFECTOR_PIEZO=1, EFFECTOR_VIBRATOR=2 };
enum EventType : uint8_t { EVENT_CONFIGURE_PWM=1, EVENT_START=2, EVENT_STOP=3 };
struct EffectorEvent { EventType type; EffectorId effector; uint16_t frequency_hz; uint8_t duty_percent; uint16_t duration_ms; };
struct EffectorState { uint16_t frequency_hz; uint8_t duty_percent; uint16_t duration_ms; uint16_t started_at_ms; bool requested_on; };
struct PwmProfile { uint16_t frequency_hz; uint8_t duty_percent; uint16_t duration_ms; };
class EffectorRuntime {
public:
 EffectorRuntime():head_(0),tail_(0),count_(0),overflowed_(false){clearStates();}
 static bool validEffector(EffectorId id){return static_cast<uint8_t>(id)<EFFECTOR_CHANNEL_COUNT;}
 static bool validProfile(const PwmProfile& p){return p.frequency_hz>0 && p.duty_percent<=100 && p.duration_ms>0;}
 bool enqueue(const EffectorEvent& e){
  if(!validEffector(e.effector))return false;
  if(e.type==EVENT_CONFIGURE_PWM || e.type==EVENT_START){const PwmProfile p={e.frequency_hz,e.duty_percent,e.duration_ms};if(!validProfile(p))return false;}
  else if(e.type!=EVENT_STOP)return false;
  if(count_>=EFFECTOR_QUEUE_CAPACITY){overflowed_=true;return false;}
  uint8_t* r=queue_[tail_];
  r[0]=static_cast<uint8_t>((static_cast<uint8_t>(e.type)<<2)|static_cast<uint8_t>(e.effector));
  r[1]=static_cast<uint8_t>(e.frequency_hz);r[2]=static_cast<uint8_t>(e.frequency_hz>>8);
  r[3]=e.duty_percent;r[4]=static_cast<uint8_t>(e.duration_ms);r[5]=static_cast<uint8_t>(e.duration_ms>>8);
  tail_=static_cast<uint8_t>((tail_+1U)%EFFECTOR_QUEUE_CAPACITY);++count_;return true;
 }
 bool processOne(uint32_t now_ms){
  if(!count_)return false;
  const uint8_t* r=queue_[head_];const uint8_t control=r[0],type=static_cast<uint8_t>(control>>2),id=static_cast<uint8_t>(control&3U);
  const uint16_t hz=static_cast<uint16_t>(r[1])|(static_cast<uint16_t>(r[2])<<8);
  const uint16_t duration=static_cast<uint16_t>(r[4])|(static_cast<uint16_t>(r[5])<<8);
  head_=static_cast<uint8_t>((head_+1U)%EFFECTOR_QUEUE_CAPACITY);--count_;
  EffectorState& s=states_[id];
  if(type==EVENT_STOP){s.requested_on=false;s.started_at_ms=0;}
  else{s.frequency_hz=hz;s.duty_percent=r[3];s.duration_ms=duration;s.requested_on=(type==EVENT_START);s.started_at_ms=s.requested_on?static_cast<uint16_t>(now_ms):0;}
  return true;
 }
 void tick(uint32_t now_ms){
  const uint16_t now16=static_cast<uint16_t>(now_ms);
  for(uint8_t i=0;i<EFFECTOR_CHANNEL_COUNT;++i){EffectorState& s=states_[i];if(s.requested_on && static_cast<uint16_t>(now16-s.started_at_ms)>=s.duration_ms){s.requested_on=false;s.started_at_ms=0;}}
 }
 const EffectorState* state(EffectorId id)const{return validEffector(id)?&states_[static_cast<uint8_t>(id)]:0;}
 uint8_t queued()const{return count_;} bool overflowed()const{return overflowed_;}
 void clearQueue(){head_=tail_=count_=0;} void resetOverflowFlag(){overflowed_=false;}
 void clearModel(){clearQueue();clearStates();}
private:
 void clearStates(){for(uint8_t i=0;i<EFFECTOR_CHANNEL_COUNT;++i){states_[i].frequency_hz=0;states_[i].duty_percent=0;states_[i].duration_ms=0;states_[i].started_at_ms=0;states_[i].requested_on=false;}}
 // 6 bytes per event: packed control, frequency (2), duty, duration (2).
 uint8_t queue_[EFFECTOR_QUEUE_CAPACITY][6];EffectorState states_[EFFECTOR_CHANNEL_COUNT];
 uint8_t head_,tail_,count_;bool overflowed_;
};
}
#endif
