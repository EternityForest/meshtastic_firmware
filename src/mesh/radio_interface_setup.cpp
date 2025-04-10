# include "libmeshtastic.h"
#include "configuration.h"
#include "NodeDB.h"


#include "LLCC68Interface.h"
#include "LR1110Interface.h"
#include "LR1120Interface.h"
#include "LR1121Interface.h"
#include "RF95Interface.h"
#include "SX1262Interface.h"
#include "SX1268Interface.h"
#include "SX1280Interface.h"
#include "detect/LoRaRadioType.h"

#ifdef ARCH_STM32WL
#include "STM32WLE5JCInterface.h"
#endif



LockingArduinoHal* MeshtasticAPI::_getRadioHal(){

    SPISettings spiSettings(4000000, MSBFIRST, SPI_MODE0);

#if ARCH_PORTDUINO
    throw std::runtime_error("This function not used with portduino");
#elif defined(HW_SPI1_DEVICE)
    LockingArduinoHal *RadioLibHAL = new LockingArduinoHal(SPI1, spiSettings);
#else // HW_SPI1_DEVICE
    LockingArduinoHal *RadioLibHAL = new LockingArduinoHal(SPI, spiSettings);
#endif
    return RadioLibHAL;
}


void MeshtasticAPI::setupPortduinoLoRa(){
    #ifdef ARCH_PORTDUINO
    SPISettings spiSettings(settingsMap[spiSpeed], MSBFIRST, SPI_MODE0);

    const struct {
        configNames cfgName;
        std::string strName;
    } loraModules[] = {{use_rf95, "RF95"},     {use_sx1262, "sx1262"}, {use_sx1268, "sx1268"}, {use_sx1280, "sx1280"},
                       {use_lr1110, "lr1110"}, {use_lr1120, "lr1120"}, {use_lr1121, "lr1121"}, {use_llcc68, "LLCC68"}};
    // as one can't use a function pointer to the class constructor:
    auto loraModuleInterface = [](configNames cfgName, LockingArduinoHal *hal, RADIOLIB_PIN_TYPE cs, RADIOLIB_PIN_TYPE irq,
                                  RADIOLIB_PIN_TYPE rst, RADIOLIB_PIN_TYPE busy) {
        switch (cfgName) {
        case use_rf95:
            return (RadioInterface *)new RF95Interface(hal, cs, irq, rst, busy);
        case use_sx1262:
            return (RadioInterface *)new SX1262Interface(hal, cs, irq, rst, busy);
        case use_sx1268:
            return (RadioInterface *)new SX1268Interface(hal, cs, irq, rst, busy);
        case use_sx1280:
            return (RadioInterface *)new SX1280Interface(hal, cs, irq, rst, busy);
        case use_lr1110:
            return (RadioInterface *)new LR1110Interface(hal, cs, irq, rst, busy);
        case use_lr1120:
            return (RadioInterface *)new LR1120Interface(hal, cs, irq, rst, busy);
        case use_lr1121:
            return (RadioInterface *)new LR1121Interface(hal, cs, irq, rst, busy);
        case use_llcc68:
            return (RadioInterface *)new LLCC68Interface(hal, cs, irq, rst, busy);
        default:
            assert(0); // shouldn't happen
            return (RadioInterface *)nullptr;
        }
    };
    for (auto &loraModule : loraModules) {
        if (settingsMap[loraModule.cfgName] && !rIf) {
            LOG_DEBUG("Activate %s radio on SPI port %s", loraModule.strName.c_str(), settingsStrings[spidev].c_str());
            if (settingsStrings[spidev] == "ch341") {
                RadioLibHAL = ch341Hal;
            } else {
                RadioLibHAL = new LockingArduinoHal(SPI, spiSettings);
            }
            rIf = loraModuleInterface(loraModule.cfgName, (LockingArduinoHal *)RadioLibHAL, settingsMap[cs_pin],
                                      settingsMap[irq_pin], settingsMap[reset_pin], settingsMap[busy_pin]);
            if (!rIf->init()) {
                LOG_WARN("No %s radio", loraModule.strName.c_str());
                delete rIf;
                rIf = NULL;
                exit(EXIT_FAILURE);
            } else {
                LOG_INFO("%s init success", loraModule.strName.c_str());
            }
        }
    }
    #else
    throw std::runtime_error("This is not a portduino build");
    #endif
}