#include "common/FFstrbuf.h"
#include "common/textModifier.h"

#include <stdio.h>
#include <stdlib.h>

bool ffCPUQualcommSnapdragonToName(FFstrbuf* name, const char* model);


static void verifyName(const char* model, bool expectedResult, const char* expected) {
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateS("unchanged");
    const bool returned = ffCPUQualcommSnapdragonToName(&result, model);

    if (returned != expectedResult || !ffStrbufEqualS(&result, expected)) {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR
            "ffCPUQualcommSnapdragonToName(\"%s\"): expected %s \"%s\", got %s \"%s\"\n"
            FASTFETCH_TEXT_MODIFIER_RESET,
            model, expectedResult ? "true" : "false", expected,
            returned ? "true" : "false", result.chars);
        exit(1);
    }
}

#define VERIFY_NAME(model, expectedResult, expected) \
    verifyName((model), (expectedResult), (expected))

int main(void) {
    // Existing X1 and direct X2 model names
    VERIFY_NAME("x1e80100", true, "Qualcomm Snapdragon X Elite X1E-80-100");
    VERIFY_NAME("x1p42100", true, "Qualcomm Snapdragon X Plus X1P-42-100");
    VERIFY_NAME("x2e88100", true, "Qualcomm Snapdragon X2 Elite X2E-88-100");
    VERIFY_NAME("x2e90100", true, "Qualcomm Snapdragon X2 Elite X2E-90-100");
    VERIFY_NAME("x2e94100", true, "Qualcomm Snapdragon X2 Elite Extreme X2E-94-100");
    VERIFY_NAME("x2e96100", true, "Qualcomm Snapdragon X2 Elite Extreme X2E-96-100");
    VERIFY_NAME("x2p42100", true, "Qualcomm Snapdragon X2 Plus X2P-42-100");

    // X2 platform codenames identify the marketing family, not an exact SKU
    VERIFY_NAME("glymur", true, "Qualcomm Snapdragon X2 Elite Extreme");
    VERIFY_NAME("mahua", true, "Qualcomm Snapdragon X2 Elite");
    VERIFY_NAME("kalambo", true, "Qualcomm Snapdragon X2 Plus");

    // Other Qualcomm platform names are handled by the caller
    VERIFY_NAME("sc8280", false, "unchanged");

    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
    return 0;
}
