
#include "libmeshtastic.h"
#include "configuration.h"


#ifdef HAS_UDP_MULTICAST
#include "mesh/udp/UdpMulticastThread.h"
extern UdpMulticastThread *udpThread;
#endif


Router *router = NULL; // Users of router don't care what sort of subclass implements that API

const char *firmware_version = optstr(APP_VERSION_SHORT);

extern void getMacAddr(uint8_t *dmac);

MeshtasticAPI libmeshtastic;

const char *getDeviceName()
{
    uint8_t dmac[6];

    getMacAddr(dmac);

    // Meshtastic_ab3c or Shortname_abcd
    static char name[20];
    snprintf(name, sizeof(name), "%02x%02x", dmac[4], dmac[5]);
    // if the shortname exists and is NOT the new default of ab3c, use it for BLE name.
    if (strcmp(owner.short_name, name) != 0) {
        snprintf(name, sizeof(name), "%s_%02x%02x", owner.short_name, dmac[4], dmac[5]);
    } else {
        snprintf(name, sizeof(name), "Meshtastic_%02x%02x", dmac[4], dmac[5]);
    }
    return name;
}

RadioInterface *rIf = NULL;
#ifdef ARCH_PORTDUINO
RadioLibHal *RadioLibHAL = NULL;
#endif

MeshtasticAPI:: MeshtasticAPI(/* args */){

}

void MeshtasticAPI::begin(MeshService * userservice, Router * router){
    nodeDB = new NodeDB;
    service = userservice;
    service->init();
}

void MeshtasticAPI::enableUDPMulticast(){
    LOG_DEBUG("Start multicast thread");
    udpThread = new UdpMulticastThread();

    #ifdef ARCH_PORTDUINO
    // FIXME: portduino does not ever call onNetworkConnected so call it here because I don't know what happen if I call
    // onNetworkConnected there
    if (config.network.enabled_protocols & meshtastic_Config_NetworkConfig_ProtocolFlags_UDP_BROADCAST) {
        udpThread->start();
    }
    #endif

}