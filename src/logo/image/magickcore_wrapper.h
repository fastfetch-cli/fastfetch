#pragma once

// ImageMagick-7/MagickCore/pixel-accessor.h:518:46: warning: implicit conversion from 'QuantumAny' (aka 'unsigned long long') to 'double' may lose precision
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-int-float-conversion"

#ifdef FF_HAVE_IMAGEMAGICK7
    #include <MagickCore/MagickCore.h>
#elif defined(FF_HAVE_IMAGEMAGICK6)
    #include <magick/MagickCore.h>
#endif

#pragma GCC diagnostic pop
