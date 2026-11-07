#ifndef RESONET_DIAGNOSTIC_EFFECTOR_GPIO_H
#define RESONET_DIAGNOSTIC_EFFECTOR_GPIO_H
#include <stdint.h>
#include "effector_runtime.h"
namespace ResoNetDiagnostic {
static const uint8_t EFFECTOR_PIN_UNASSIGNED=0xFF;
static const uint8_t ARDUINO_NANO_MAX_DIGITAL_PIN=19;
struct EffectorPinMap { uint8_t pin[EFFECTOR_CHANNEL_COUNT]; EffectorPinMap():pin{EFFECTOR_PIN_UNASSIGNED,EFFECTOR_PIN_UNASSIGNED,EFFECTOR_PIN_UNASSIGNED}{} };
struct EffectorGpioIo { void (*configure_output)(uint8_t,void*);void (*write_level)(uint8_t,bool,void*);void* context; };
class EffectorGpioDriver {
public:
 EffectorGpioDriver():io_{0,0,0},ready_(false){for(uint8_t i=0;i<EFFECTOR_CHANNEL_COUNT;++i)pins_[i]=EFFECTOR_PIN_UNASSIGNED;}
 static bool validPinMap(const EffectorPinMap& m){for(uint8_t i=0;i<EFFECTOR_CHANNEL_COUNT;++i){if(m.pin[i]>ARDUINO_NANO_MAX_DIGITAL_PIN)return false;for(uint8_t j=static_cast<uint8_t>(i+1);j<EFFECTOR_CHANNEL_COUNT;++j)if(m.pin[i]==m.pin[j])return false;}return true;}
 bool begin(const EffectorPinMap& m,const EffectorGpioIo& io){shutdown();if(!validPinMap(m)||!io.configure_output||!io.write_level)return false;io_=io;for(uint8_t i=0;i<EFFECTOR_CHANNEL_COUNT;++i){pins_[i]=m.pin[i];io_.configure_output(pins_[i],io_.context);io_.write_level(pins_[i],false,io_.context);}ready_=true;return true;}
 bool setEnabled(EffectorId id,bool enabled){if(!ready_||!EffectorRuntime::validEffector(id))return false;io_.write_level(pins_[static_cast<uint8_t>(id)],enabled,io_.context);return true;}
 void shutdown(){if(ready_)for(uint8_t i=0;i<EFFECTOR_CHANNEL_COUNT;++i)io_.write_level(pins_[i],false,io_.context);ready_=false;io_.configure_output=0;io_.write_level=0;io_.context=0;for(uint8_t i=0;i<EFFECTOR_CHANNEL_COUNT;++i)pins_[i]=EFFECTOR_PIN_UNASSIGNED;}
 bool ready()const{return ready_;}
private:uint8_t pins_[EFFECTOR_CHANNEL_COUNT];EffectorGpioIo io_;bool ready_;
};
}
#endif
