#pragma once
#include "RadioInterface.h"
#include "RadioLibInterface.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "generated/meshtastic/mesh.pb.h"


#define MESHTASTIC_EXCLUDE_MQTT 1


class  MeshtasticAPI
{
private:
    /* data */
    LockingArduinoHal* _getRadioHal();

public:
    // Set up the pins before calling any of the LoRa chip setup functions
    RADIOLIB_PIN_TYPE cs_pin=1000;
    RADIOLIB_PIN_TYPE irq_pin=1000;
    RADIOLIB_PIN_TYPE rst_pin=1000;
    RADIOLIB_PIN_TYPE busy_pin=1000;


     MeshtasticAPI(/* args */);
    ~ MeshtasticAPI();

    void setNodeDB(NodeDB * db){
        nodeDB = db;};
    void begin(MeshService * service, Router * router);

    // Call AFTER calling one of the LoRa chip setup functions
    void setRegion(meshtastic_Config_LoRaConfig_RegionCode region);

    void enableUDPMulticast();


    void setupPortduinoLoRa();


    void useSTM32WLx();
};



 MeshtasticAPI::~ MeshtasticAPI()
{
}


extern MeshtasticAPI libmeshtastic;