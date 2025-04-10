#include "configuration.h"

#if !MESHTASTIC_EXCLUDE_GPS
#include "GPS.h"
#endif

#include "../concurrency/Periodic.h"
#include "BluetoothCommon.h" // needed for updateBatteryLevel, FIXME, eventually when we pull mesh out into a lib we shouldn't be whacking bluetooth from here
#include "MeshService.h"
#include "NodeDB.h"
#include "PowerFSM.h"
#include "RTC.h"
#include "TypeConversions.h"
#include "mesh-pb-constants.h"
#include "meshUtils.h"
#include "modules/NodeInfoModule.h"
#include "modules/PositionModule.h"
#include "power.h"
#include <assert.h>
#include <string>

#if ARCH_PORTDUINO
#include "PortduinoGlue.h"
#endif

void AppMeshService::init()
{
#if HAS_GPS
    if (gps)
        gpsObserver.observe(&gps->newStatus);
#endif
}

int AppMeshService::handleFromRadio(const meshtastic_MeshPacket *mp)
{
    powerFSM.trigger(EVENT_PACKET_FOR_PHONE); // Possibly keep the node from sleeping

    nodeDB->updateFrom(*mp); // update our DB state based off sniffing every RX packet from the radio
    bool isPreferredRebroadcaster =
        IS_ONE_OF(config.device.role, meshtastic_Config_DeviceConfig_Role_ROUTER, meshtastic_Config_DeviceConfig_Role_REPEATER);
    if (mp->which_payload_variant == meshtastic_MeshPacket_decoded_tag &&
        mp->decoded.portnum == meshtastic_PortNum_TELEMETRY_APP && mp->decoded.request_id > 0) {
        LOG_DEBUG("Received telemetry response. Skip sending our NodeInfo"); //  because this potentially a Repeater which will
                                                                             //  ignore our request for its NodeInfo
    } else if (mp->which_payload_variant == meshtastic_MeshPacket_decoded_tag && !nodeDB->getMeshNode(mp->from)->has_user &&
               nodeInfoModule && !isPreferredRebroadcaster && !nodeDB->isFull()) {
        if (airTime->isTxAllowedChannelUtil(true)) {
            // Hops used by the request. If somebody in between running modified firmware modified it, ignore it
            auto hopStart = mp->hop_start;
            auto hopLimit = mp->hop_limit;
            uint8_t hopsUsed = hopStart < hopLimit ? config.lora.hop_limit : hopStart - hopLimit;
            if (hopsUsed > config.lora.hop_limit + 2) {
                LOG_DEBUG("Skip send NodeInfo: %d hops away is too far away", hopsUsed);
            } else {
                LOG_INFO("Heard new node on ch. %d, send NodeInfo and ask for response", mp->channel);
                nodeInfoModule->sendOurNodeInfo(mp->from, true, mp->channel);
            }
        } else {
            LOG_DEBUG("Skip sending NodeInfo > 25%% ch. util");
        }
    }

    printPacket("Forwarding to phone", mp);
    sendToPhone(packetPool.allocCopy(*mp));

    return 0;
}