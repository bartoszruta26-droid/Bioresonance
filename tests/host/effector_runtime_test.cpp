#include "../../ResoNet_Diagnostic_Nano/effector_runtime.h"
#include "../../ResoNet_Diagnostic_Nano/effector_gpio.h"
#include <assert.h>
using namespace ResoNetDiagnostic;
static EffectorEvent event(EventType t,EffectorId id,uint16_t hz=1000,uint8_t duty=25,uint16_t ms=500){EffectorEvent e={t,id,hz,duty,ms};return e;}
struct Fake {uint8_t pins[3];bool levels[3];uint8_t configured,writes;};
static void configure(uint8_t p,void*c){Fake*f=static_cast<Fake*>(c);f->pins[f->configured++]=p;}
static void writePin(uint8_t p,bool high,void*c){Fake*f=static_cast<Fake*>(c);for(uint8_t i=0;i<3;++i)if(f->pins[i]==p)f->levels[i]=high;++f->writes;}
int main(){
 assert(EffectorRuntime::validProfile(PwmProfile{1,0,1}));assert(EffectorRuntime::validProfile(PwmProfile{65535,100,65535}));
 assert(!EffectorRuntime::validProfile(PwmProfile{0,20,100}));assert(!EffectorRuntime::validProfile(PwmProfile{1000,101,100}));assert(!EffectorRuntime::validProfile(PwmProfile{1000,20,0}));
 EffectorRuntime r;assert(r.queued()==0);assert(r.enqueue(event(EVENT_CONFIGURE_PWM,EFFECTOR_IR_LED)));assert(r.processOne(10));
 const EffectorState* ir=r.state(EFFECTOR_IR_LED);assert(ir&&ir->frequency_hz==1000&&ir->duty_percent==25&&!ir->requested_on);
 assert(r.enqueue(event(EVENT_START,EFFECTOR_PIEZO,2200,40,250)));assert(r.processOne(100));assert(r.state(EFFECTOR_PIEZO)->requested_on);
 r.tick(349);assert(r.state(EFFECTOR_PIEZO)->requested_on);r.tick(350);assert(!r.state(EFFECTOR_PIEZO)->requested_on);
 assert(r.enqueue(event(EVENT_START,EFFECTOR_VIBRATOR,1000,50,20)));assert(r.processOne(65530UL));r.tick(13UL);assert(r.state(EFFECTOR_VIBRATOR)->requested_on);r.tick(14UL);assert(!r.state(EFFECTOR_VIBRATOR)->requested_on);
 assert(r.enqueue(event(EVENT_START,EFFECTOR_PIEZO,1000,25,500)));assert(r.processOne(1000));assert(r.enqueue(event(EVENT_STOP,EFFECTOR_PIEZO)));assert(r.processOne(1001));r.tick(2000);assert(!r.state(EFFECTOR_PIEZO)->requested_on);
 assert(!r.enqueue(event(EVENT_START,EFFECTOR_VIBRATOR,0,20,100)));assert(!r.enqueue(event(EVENT_START,EFFECTOR_VIBRATOR,1000,101,100)));assert(!r.enqueue(event(EVENT_START,EFFECTOR_VIBRATOR,1000,20,0)));
 assert(!r.enqueue(event(static_cast<EventType>(0),EFFECTOR_IR_LED)));assert(!r.enqueue(event(EVENT_STOP,static_cast<EffectorId>(3))));
 for(uint8_t i=0;i<EFFECTOR_QUEUE_CAPACITY;++i)assert(r.enqueue(event(EVENT_STOP,EFFECTOR_IR_LED)));
 assert(!r.enqueue(event(EVENT_STOP,EFFECTOR_IR_LED))&&r.overflowed());while(r.processOne(0)){}assert(r.queued()==0);
 r.resetOverflowFlag();assert(!r.overflowed());r.clearModel();assert(!r.state(EFFECTOR_IR_LED)->requested_on&&r.state(EFFECTOR_IR_LED)->frequency_hz==0);
 // Full-range duration boundary and repeated ring-buffer wraparound.
 assert(r.enqueue(event(EVENT_START,EFFECTOR_IR_LED,500,50,65535)));
 assert(r.processOne(1000));
 r.tick(66534UL);assert(r.state(EFFECTOR_IR_LED)->requested_on);
 r.tick(66535UL);assert(!r.state(EFFECTOR_IR_LED)->requested_on);
 for(uint8_t n=0;n<24;++n){
  const EffectorId id=static_cast<EffectorId>(n%EFFECTOR_CHANNEL_COUNT);
  assert(r.enqueue(event(EVENT_CONFIGURE_PWM,id,static_cast<uint16_t>(100+n),10,10)));
  assert(r.processOne(static_cast<uint32_t>(n)));
  assert(r.state(id)->frequency_hz==static_cast<uint16_t>(100+n));
 }

 EffectorPinMap map;assert(!EffectorGpioDriver::validPinMap(map));map.pin[0]=2;map.pin[1]=3;map.pin[2]=4;assert(EffectorGpioDriver::validPinMap(map));map.pin[2]=3;assert(!EffectorGpioDriver::validPinMap(map));map.pin[2]=20;assert(!EffectorGpioDriver::validPinMap(map));
 Fake f={{EFFECTOR_PIN_UNASSIGNED,EFFECTOR_PIN_UNASSIGNED,EFFECTOR_PIN_UNASSIGNED},{true,true,true},0,0};EffectorGpioIo io={configure,writePin,&f};EffectorGpioDriver gpio;
 assert(!gpio.begin(map,io));map.pin[2]=4;assert(gpio.begin(map,io));assert(gpio.ready()&&f.configured==3);assert(!f.levels[0]&&!f.levels[1]&&!f.levels[2]);
 assert(gpio.setEnabled(EFFECTOR_IR_LED,true)&&f.levels[0]);assert(!gpio.setEnabled(static_cast<EffectorId>(3),true));gpio.shutdown();assert(!gpio.ready()&&!f.levels[0]&&!f.levels[1]&&!f.levels[2]);assert(!gpio.setEnabled(EFFECTOR_IR_LED,true));
 return 0;
}
