extern "C"
{
#include "sound.h"
}
#include <MediaRoster.h>

const char* ffDetectSound(FF_MAYBE_UNUSED FFlist* devices /* List of FFSoundDevice */)
{
    BMediaRoster* roster = BMediaRoster::Roster();
    media_node mediaNode;

    roster->GetAudioOutput(&mediaNode);

    int32 mediaOutputCount = 0;
    roster->GetAllOutputsFor(mediaNode, NULL, 0, &mediaOutputCount);
    if (mediaOutputCount == 0)
        return NULL;

    // TODO: Implement the rest of the function

    return "Not supported on this platform";
}
