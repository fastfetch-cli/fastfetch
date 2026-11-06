#include "logo/spin/spin.h"

#include "common/io.h"
#include "common/strutil.h"
#include "common/time.h"
#include "logo/logo.h"
#include "logo/spin/render.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// Degrees per frame when neither speed option is set.
#define SPIN_DEFAULT_DEGREES_PER_FRAME 10.0f

// One step of the turn. `logo.spin.speedPerRound` wins over
// `logo.spin.speedPerFrame` when both are set; 0 means unset for either.
static float spinStepDegrees(void) {
    const FFOptionsLogoSpin* spin = &instance.config.logo.spin;
    if (spin->speedPerRound != 0.0f) {
        // Seconds per revolution: follow the clock instead of counting frames,
        // so the turn keeps its pace even when a refresh takes longer than the
        // interval asked for.
        FFLogoSpinState* state = &instance.state.logoSpin;
        const double now = ffTimeGetTick();
        const double elapsedMs = now - state->lastDrawMs;
        state->lastDrawMs = now;
        return (float) (360.0 / (double) spin->speedPerRound * elapsedMs / 1000.0);
    }
    if (spin->speedPerFrame != 0.0f) {
        return spin->speedPerFrame;
    }
    return SPIN_DEFAULT_DEGREES_PER_FRAME;
}

static bool spinIsAuto(const FFstrbuf* source) {
    return source->length == 0 || ffStrbufIgnCaseEqualS(source, "auto");
}

static const FFlogo* spinBuiltin(const FFstrbuf* source, FFLogoSize size) {
    return spinIsAuto(source) ? ffLogoGetBuiltinDetected(size) : ffLogoGetBuiltinForName(source, size);
}

static bool spinLoadFile(const FFstrbuf* source, FFstrbuf* text) {
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();
    if (ffPathExpandEnv(source->chars, &path) && ffAppendFileBuffer(path.chars, text)) {
        return text->length > 0;
    }
    return ffAppendFileBuffer(source->chars, text) && text->length > 0;
}

void ffSpinLogoPrint(void) {
    FFOptionsLogo* options = &instance.config.logo;
    FFLogoSpinState* state = &instance.state.logoSpin;
    instance.state.logoWidth = 0;
    instance.state.logoHeight = 0;

    if (!state->started) {
        // The first frame starts where `logo.spin.angle` says, and every
        // `--watch` refresh turns it by one step.
        state->started = true;
        state->degrees = fmodf(options->spin.angle, 360.0f);
        if (state->degrees < 0.0f) {
            state->degrees += 360.0f;
        }
    }
    const float spinAngle = state->degrees * (float) M_PI / 180.0f;

    // The source is the detected OS (`auto`), a builtin logo name, or a file
    // in fastfetch's logo syntax, like the other text logo types.
    FF_STRBUF_AUTO_DESTROY fileText = ffStrbufCreate();
    const FFlogo* logo = spinBuiltin(&options->source, FF_LOGO_SIZE_NORMAL);
    if (!logo) {
        logo = spinBuiltin(&options->source, FF_LOGO_SIZE_UNKNOWN);
    }
    if (!logo && !spinLoadFile(&options->source, &fileText)) {
        if (instance.config.display.showErrors) {
            fprintf(stderr, "Logo (spin): no builtin logo or readable file named %s\n", options->source.chars);
        }
    }

    // `logo.color.N` on top of the logo's own colors; a file logo borrows the
    // detected logo's palette, as the other text logo types do.
    ffLogoApplyColors(logo ? logo : ffLogoGetBuiltinDetected(FF_LOGO_SIZE_NORMAL), true);

    FFSpinArt art = {};
    if (!ffSpinArtParse(&art, logo ? logo->lines : fileText.chars, options->colors, FASTFETCH_LOGO_MAX_COLORS)) {
        ffSpinArtDestroy(&art); // parse may have allocated before it gave up
        return;
    }

    const float tilt = options->spin.tilt * (float) M_PI / 180.0f;
    uint32_t width = art.width;
    uint32_t height = art.height;
    // With `--watch` the frame has to stay the same size from the first draw on,
    // or the information column would move when the logo starts turning.
    const bool animated = instance.state.dynamicInterval > 0;
    if (animated || spinAngle != 0.0f || tilt != 0.0f) {
        // A turned slab sticks out of the art's box, so the frame has to make
        // room for its widest angle or the logo would be clipped.
        ffSpinFrameSize(&art, tilt, &width, &height);
    }

    FFSpinFrame frame = {};
    if (!ffSpinFrameInit(&frame, width, height)) {
        ffSpinArtDestroy(&art);
        return;
    }

    if (spinAngle == 0.0f && tilt == 0.0f) {
        ffSpinFramePutArt(&frame, &art); // face on: exactly the original art
    } else {
        ffSpinRenderArt(&art, spinAngle, tilt, &frame);
    }

    // Hand the frame to the upstream logo pipeline: it pads, positions and
    // measures the art, and its line cache is what puts the modules next to
    // the logo. `false`: the frame already carries its own SGR sequences.
    FF_STRBUF_AUTO_DESTROY ansi = ffStrbufCreateA(width * height * 4);
    ffSpinFrameToAnsi(&frame, &art, !instance.config.display.pipe, instance.config.display.brightColor, !animated, &ansi);
    // The serialiser ends every row with a newline; the last one would add an
    // empty line to the logo and change the output from the other logo types.
    if (ansi.length > 0 && ansi.chars[ansi.length - 1] == '\n') {
        ansi.chars[--ansi.length] = '\0';
    }

    // A resize changes the frame size, and the cells outside the new frame
    // would keep showing the previous one.
    if (state->printed && (width != state->printedWidth || height != state->printedHeight)) {
        fputs("\e[2J\e[H", stdout);
    }
    // `ffLogoPrintChars` writes the art with write(2) for `--logo-position top`,
    // while the caller's cursor moves go through stdio; flush so the art lands
    // where the cursor was just moved to.
    fflush(stdout);
    ffLogoPrintChars(ansi.chars, false);

    ffSpinFrameDestroy(&frame);
    ffSpinArtDestroy(&art);

    state->printed = true;
    state->printedWidth = width;
    state->printedHeight = height;
    state->lastDrawMs = ffTimeGetTick(); // start measuring the next step from now
}

bool ffSpinTick(void) {
    FFLogoSpinState* state = &instance.state.logoSpin;
    if (!state->printed) {
        return false;
    }

    const float step = spinStepDegrees();
    if (step == 0.0f) {
        return false; // a logo that does not turn does not have to be redrawn
    }

    // The angle is wrapped instead of growing, or a long `--watch` session
    // would lose precision in the sine and cosine.
    float degrees = state->degrees + step;
    state->degrees = degrees - floorf(degrees / 360.0f) * 360.0f;
    return true;
}
