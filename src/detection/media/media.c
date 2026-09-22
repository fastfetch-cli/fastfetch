#include "media.h"
#include "common/FFcache.h"
#include "common/io.h"

void ffDetectMediaImpl(FFMediaResult* media, bool saveCover);

static FFMediaResult result;

// The cover file of this run, if there is one. It is tracked here rather than in `result` because the
// cached result is dropped between `--dynamic-interval` rounds, and `result.cover` — the only other
// copy of the path — is destroyed with it. The file itself has to outlive that, since the
// `kitty-direct` logo type hands its path to the terminal, so it is kept for the whole run and
// removed at exit.
static FFstrbuf coverFile;
static bool coverFileRegistered = false;

static void removeMediaCoverFile(void) {
    if (coverFile.length > 0) {
        ffRemoveFile(coverFile.chars);
        ffStrbufDestroy(&coverFile);
    }
}

static void destroyMediaResult(void* storage) {
    FFMediaResult* media = storage;

    ffStrbufDestroy(&media->error);
    ffStrbufDestroy(&media->playerId);
    ffStrbufDestroy(&media->player);
    ffStrbufDestroy(&media->song);
    ffStrbufDestroy(&media->artist);
    ffStrbufDestroy(&media->album);
    ffStrbufDestroy(&media->url);
    ffStrbufDestroy(&media->status);
    ffStrbufDestroy(&media->cover);
    media->length = 0;
    media->position = 0;
    media->removeCoverAfterUse = false;
}

// `saveCover` asks for a side effect and is not part of the cache identity, so the entry cannot be
// initialized through `init(void*)`: the first caller of a generation decides, and the callers that
// follow reuse its result, as they did when the result was cached for the whole run.
static FFcacheEntry ffCacheEntryMedia = {
    .name = "media",
    .storage = &result,
    .destroy = destroyMediaResult,
};

const FFMediaResult* ffDetectMedia(bool saveCover) {
    if (ffCacheBeginInit(&ffCacheEntryMedia)) {
        ffStrbufInit(&result.error);
        ffStrbufInit(&result.playerId);
        ffStrbufInit(&result.player);
        ffStrbufInit(&result.song);
        ffStrbufInit(&result.artist);
        ffStrbufInit(&result.album);
        ffStrbufInit(&result.url);
        ffStrbufInit(&result.status);
        ffStrbufInit(&result.cover);
        result.length = 0;
        result.position = 0;
        result.removeCoverAfterUse = false;
        ffDetectMediaImpl(&result, saveCover);

        if (result.song.length == 0 && result.error.length == 0) {
            ffStrbufAppendS(&result.error, "No media found");
        }
        ffStrbufTrimRightSpace(&result.song);
        ffStrbufTrimRightSpace(&result.artist);
        ffStrbufTrimRightSpace(&result.album);
        ffStrbufTrimRightSpace(&result.player);

        if (saveCover && result.removeCoverAfterUse) {
            // A new round can produce a new cover while the previous file is still on disk, and
            // `coverFile` is about to forget its path. Remove it here, otherwise only the last one
            // would ever be cleaned up by `atexit`.
            if (coverFile.length > 0 && !ffStrbufEqual(&coverFile, &result.cover)) {
                ffRemoveFile(coverFile.chars);
            }
            ffStrbufSet(&coverFile, &result.cover);
            if (!coverFileRegistered) {
                coverFileRegistered = true;
                atexit(removeMediaCoverFile);
            }
        }
    }

    return &result;
}
