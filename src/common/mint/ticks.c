#include <mint/sysvars.h>
#include <mint/osbind.h>

double mint_ticks_ms(void)
{
    long oldmode = Super(0L);
    double ticks = *_hz_200 * 5.;
    Super(oldmode);

    return ticks;
}
