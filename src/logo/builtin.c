#include "logo.h"
#include "logo_builtin.h"
#include "common/color.h"

const FFlogo ffLogoUnknown = {
    .names = { "unknown" },
    .lines = FASTFETCH_DATATEXT_LOGO_UNKNOWN,
    .colors = {
        FF_COLOR_FG_DEFAULT,
    },
};

#include "./ascii/a.inc"
#include "./ascii/b.inc"
#include "./ascii/c.inc"
#include "./ascii/d.inc"
#include "./ascii/e.inc"
#include "./ascii/f.inc"
#include "./ascii/g.inc"
#include "./ascii/h.inc"
#include "./ascii/i.inc"
#include "./ascii/j.inc"
#include "./ascii/k.inc"
#include "./ascii/l.inc"
#include "./ascii/m.inc"
#include "./ascii/n.inc"
#include "./ascii/o.inc"
#include "./ascii/p.inc"
#include "./ascii/q.inc"
#include "./ascii/r.inc"
#include "./ascii/s.inc"
#include "./ascii/t.inc"
#include "./ascii/u.inc"
#include "./ascii/v.inc"
#include "./ascii/w.inc"
#include "./ascii/x.inc"
#include "./ascii/y.inc"
#include "./ascii/z.inc"

const FFlogo* ffLogoBuiltins[] = {
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
};
