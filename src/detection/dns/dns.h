#pragma once

#include "fastfetch.h"
#include "modules/dns/option.h"

const char* ffDetectDNS(FFDNSOptions* options, FFlist* results /* list of FFstrbuf */);
