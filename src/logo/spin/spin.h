#pragma once

#include <stdbool.h>
#include <stdint.h>

// The spin logo: the builtin logo (or a text file in logo syntax) drawn as an
// extruded slab. It has no timer and reads no input of its own; the `--watch`
// loop calls `ffSpinTick` once per refresh, which is what makes it turn.

// Draws the current frame. Called from `ffLogoPrint`.
void ffSpinLogoPrint(void);

// Advances the rotation by one `--watch` refresh and returns true when the logo
// has to be drawn again, so a still logo or any other logo type costs a single
// comparison per refresh. The step is `logo.spin.speedPerFrame` degrees, or
// whatever `logo.spin.speedPerRound` makes of the time since the last frame.
bool ffSpinTick(void);
