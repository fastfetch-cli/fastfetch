extern "C" {
#include "sound.h"
#include "common/strutil.h"
}
#include <MediaAddOn.h>
#include <MediaNode.h>
#include <MediaRoster.h>
#include <ParameterWeb.h>

const char* ffDetectSound([[maybe_unused]] FFSoundOptions* options, FFlist* devices /* List of FFSoundDevice */) {
    BMediaRoster* roster = BMediaRoster::Roster();
    media_node mediaNode;
    live_node_info liveInfo;
    dormant_node_info dormantInfo;

    if (roster->GetAudioOutput(&mediaNode) != B_OK) {
        return nullptr;
    }

    FFSoundDevice* device = FF_LIST_ADD(FFSoundDevice, *devices);
    ffStrbufInit(&device->identifier);
    if (roster->GetDormantNodeFor(mediaNode, &dormantInfo) == B_OK) {
        ffStrbufAppendS(&device->identifier, dormantInfo.name);
    }
    ffStrbufInit(&device->name);
    if (roster->GetLiveNodeInfo(mediaNode, &liveInfo) == B_OK) {
        ffStrbufAppendS(&device->name, liveInfo.name);
        ffStrbufTrimRightSpace(&device->name);
    }
    ffStrbufInitStatic(&device->platformApi, "MediaKit");
    // We'll check the Mixer actually
    device->volume = 0;
    device->type = (FFSoundType) (FF_SOUND_TYPE_ACTIVE | FF_SOUND_TYPE_MAIN);

    roster->ReleaseNode(mediaNode);

    media_node mixer;
    if (roster->GetAudioMixer(&mixer) != B_OK) {
        return nullptr;
    }

    BParameterWeb* web;
    status_t status = roster->GetParameterWebFor(mixer, &web);
    roster->ReleaseNode(mixer); // the web is all we need :-)
    if (status != B_OK) {
        return nullptr;
    }

    BContinuousParameter* gain = nullptr;
    BParameter* mute = nullptr;
    BParameter* parameter;
    for (int32 index = 0; (parameter = web->ParameterAt(index)) != nullptr; index++) {
        // assume the mute preceding master gain control
        if (ffStrEquals(parameter->Kind(), B_MUTE)) {
            mute = parameter;
        }

        if (ffStrEquals(parameter->Kind(), B_MASTER_GAIN)) {
            // Can not use dynamic_cast due to fno-rtti
            // gain = dynamic_cast<BContinuousParameter *>(parameter);
            gain = (BContinuousParameter*) (parameter);
            break;
        }
    }

    if (gain == nullptr) {
        return nullptr;
    }

    bigtime_t when;
    size_t size;

    if (mute) {
        int32 isMute = false;
        size = sizeof(isMute);
        if (mute->GetValue(&isMute, &size, &when) == B_OK && isMute) {
            return nullptr;
        }
    }

    float volume = 0.0;
    size = sizeof(volume);
    if (gain->GetValue(&volume, &size, &when) == B_OK) {
        device->volume = (uint8_t) (100 * (volume - gain->MinValue()) / (gain->MaxValue() - gain->MinValue()));
    }

    return nullptr;
}
