#include "fastfetch.h"
#include "common/networking.h"

const char* ffNetworkingSendHttpRequest(FFNetworkingState* state, const char* host, const char* path, const char* headers)
{
    return "Not supported on this platform";
}

const char* ffNetworkingRecvHttpResponse(FFNetworkingState* state, FFstrbuf* buffer)
{
    return "Not supported on this platform";

}
