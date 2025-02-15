extern "C"
{
#include "sound.h"
}
#include <MediaAddOn.h>
#include <MediaNode.h>
#include <MediaRoster.h>
#include <ParameterWeb.h>

const char* ffDetectSound(FFlist* devices /* List of FFSoundDevice */)
{
    BMediaRoster* roster = BMediaRoster::Roster();
    media_node mediaNode;
    live_node_info liveInfo;
    dormant_node_info dormantInfo;
    status_t status;

    if (roster->GetAudioOutput(&mediaNode) != B_OK)
        return NULL;

    FFSoundDevice* device = (FFSoundDevice*)ffListAdd(devices);
    if (roster->GetDormantNodeFor(mediaNode, &dormantInfo) == B_OK)
    {
        ffStrbufInitS(&device->identifier, dormantInfo.name);
    }
    if (roster->GetLiveNodeInfo(mediaNode, &liveInfo) == B_OK)
    {
        ffStrbufInitS(&device->name, liveInfo.name);
        ffStrbufTrimRightSpace(&device->name);
    }
    ffStrbufInitF(&device->platformApi, "%s", "MediaKit");
    // We'll check the Mixer actually
    device->volume = (uint8_t) 100;
    device->active = true;
    device->main = true;

    roster->ReleaseNode(mediaNode);

    media_node mixer;
    status = roster->GetAudioMixer(&mixer);
    if (status != B_OK)
        return NULL;

    BParameterWeb *web;
    status = roster->GetParameterWebFor(mixer, &web);
    roster->ReleaseNode(mixer); // the web is all we need :-)
    if (status != B_OK)
        return NULL;

    BContinuousParameter *gain = NULL;
    BParameter *mute = NULL;
    BParameter *parameter;
    for (int32 index = 0; (parameter = web->ParameterAt(index)) != NULL; index++) {
        // assume the mute preceding master gain control
        if (!strcmp(parameter->Kind(), B_MUTE))
            mute = parameter;

        if (!strcmp(parameter->Kind(), B_MASTER_GAIN)) {
            // Can not use dynamic_cast due to fno-rtti
            //gain = dynamic_cast<BContinuousParameter *>(parameter);
            gain = (BContinuousParameter *)(parameter);
            break;
        }
    }

    if (gain == NULL)
        return NULL;

    float volume = 0.0;
    bigtime_t when;
    size_t size = sizeof(volume);
    gain->GetValue(&volume, &size, &when);

    device->volume = (uint8_t) (100 * (volume - gain->MinValue()) / (gain->MaxValue() - gain->MinValue()));

    return NULL;
}
