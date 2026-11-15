#include "logo/logo.h"
#include "common/ffdata.h"
#include "common/init.h"
#include "common/textModifier.h"
#include "fastfetch.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

noreturn static void testFailed(const char* expression, int lineNo) {
    fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] %s\n" FASTFETCH_TEXT_MODIFIER_RESET, lineNo, expression);
    exit(1);
}

noreturn static void testFailedEnum(FFLogoPosition expected, FFLogoPosition actual, const char* what, int lineNo) {
    fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] %s: expected %u, got %u\n" FASTFETCH_TEXT_MODIFIER_RESET, lineNo, what, (unsigned) expected, (unsigned) actual);
    exit(1);
}

#define VERIFY(expression) \
    if (!(expression)) testFailed(#expression, __LINE__)

#define VERIFY_POSITION(expected, actual, what) \
    if ((expected) != (actual)) testFailedEnum((expected), (actual), (what), __LINE__)

static void parseLogoJson(FFOptionsLogo* options, const char* jsonText, const char** errorOut, FFstrbuf* keyOut) {
    ffStrbufClear(keyOut);
    yyjson_doc* doc = yyjson_read(jsonText, strlen(jsonText), 0);
    VERIFY(doc != nullptr);

    yyjson_val* pkey = nullptr;
    *errorOut = ffOptionsParseLogoJsonConfig(options, yyjson_doc_get_root(doc), &pkey);
    if (pkey != nullptr && yyjson_is_str(pkey)) {
        ffStrbufSetJsonVal(keyOut, pkey);
    }

    yyjson_doc_free(doc);
}

static void buildNestedFallbackJson(uint32_t depth, FFstrbuf* out) {
    ffStrbufSetStatic(out, "{\"logo\":{\"source\":\"0\"");
    for (uint32_t i = 0; i < depth; ++i) {
        ffStrbufAppendS(out, ",\"fallback\":{\"source\":\"x\"");
    }
    for (uint32_t i = 0; i < depth; ++i) {
        ffStrbufAppendC(out, '}');
    }
    ffStrbufAppendS(out, "}}");
}

static void resetLogoTestInstance(void) {
    ffLogoPrintRemaining();
    ffOptionsDestroyLogo(&instance.config.logo);
    ffOptionsInitLogo(&instance.config.logo);
    instance.config.display.pipe = true;
    instance.config.display.showErrors = false;
    ffStrbufClear(&instance.config.display.colorKeys);
    ffStrbufClear(&instance.config.display.colorTitle);
    instance.state.logoWidth = 0;
    instance.state.logoHeight = 0;
    instance.state.keysHeight = 0;
    instance.state.logoSpin = (FFLogoSpinState) {};
}

static void testJsonFallbackParsing(void) {
    FFOptionsLogo opts;
    FF_STRBUF_AUTO_DESTROY keyOut = ffStrbufCreate();
    const char* error = nullptr;

    ffOptionsInitLogo(&opts);
    parseLogoJson(&opts,
        "{\"logo\":{\"type\":\"kitty\",\"source\":\"missing.png\",\"width\":10,"
        "\"fallback\":{\"type\":\"file\",\"source\":\"fallback.txt\",\"padding\":{\"right\":2},\"fallback\":\"none\"}}}",
        &error,
        &keyOut);
    VERIFY(error == nullptr);
    VERIFY(opts.type == FF_LOGO_TYPE_IMAGE_KITTY);
    VERIFY(ffStrbufEqualS(&opts.source, "missing.png"));
    VERIFY(opts.width == 10);
    VERIFY(opts.fallback != nullptr);
    VERIFY(opts.fallback->type == FF_LOGO_TYPE_FILE);
    VERIFY(ffStrbufEqualS(&opts.fallback->source, "fallback.txt"));
    VERIFY(opts.fallback->paddingRight == 2);
    VERIFY(opts.fallback->fallback != nullptr);
    VERIFY(opts.fallback->fallback->type == FF_LOGO_TYPE_NONE);
    VERIFY(opts.fallback->fallback->paddingTop == 0 && opts.fallback->fallback->paddingBottom == 0);
    VERIFY(opts.fallback->fallback->paddingLeft == 0 && opts.fallback->fallback->paddingRight == 0);
    VERIFY(opts.fallback->fallback->fallback == nullptr);
    ffOptionsDestroyLogo(&opts);

    ffOptionsInitLogo(&opts);
    parseLogoJson(&opts, "{\"logo\":{\"source\":\"x\",\"fallback\":\"arch\"}}", &error, &keyOut);
    VERIFY(error == nullptr);
    VERIFY(opts.fallback != nullptr);
    VERIFY(opts.fallback->type == FF_LOGO_TYPE_AUTO);
    VERIFY(ffStrbufEqualS(&opts.fallback->source, "arch"));
    ffOptionsDestroyLogo(&opts);

    ffOptionsInitLogo(&opts);
    parseLogoJson(&opts, "{\"logo\":{\"source\":\"x\",\"fallback\":\"small\"}}", &error, &keyOut);
    VERIFY(error == nullptr);
    VERIFY(opts.fallback != nullptr);
    VERIFY(opts.fallback->type == FF_LOGO_TYPE_SMALL);
    VERIFY(opts.fallback->source.length == 0);
    ffOptionsDestroyLogo(&opts);

    ffOptionsInitLogo(&opts);
    parseLogoJson(&opts, "{\"logo\":{\"source\":\"x\",\"fallback\":null}}", &error, &keyOut);
    VERIFY(error == nullptr);
    VERIFY(opts.fallback != nullptr);
    VERIFY(opts.fallback->type == FF_LOGO_TYPE_NONE);
    VERIFY(opts.fallback->paddingRight == 0);
    ffOptionsDestroyLogo(&opts);

    ffOptionsInitLogo(&opts);
    parseLogoJson(&opts, "{\"logo\":{\"fallback\":123}}", &error, &keyOut);
    VERIFY(error != nullptr);
    VERIFY(ffStrbufEqualS(&keyOut, "fallback"));
    ffOptionsDestroyLogo(&opts);

    ffOptionsInitLogo(&opts);
    parseLogoJson(&opts, "{\"logo\":{\"fallback\":{\"unknownKey\":true}}}", &error, &keyOut);
    VERIFY(error != nullptr);
    VERIFY(ffStrbufEqualS(&keyOut, "unknownKey"));
    ffOptionsDestroyLogo(&opts);

    FF_STRBUF_AUTO_DESTROY deepJson = ffStrbufCreate();
    buildNestedFallbackJson(16, &deepJson);
    ffOptionsInitLogo(&opts);
    parseLogoJson(&opts, deepJson.chars, &error, &keyOut);
    VERIFY(error == nullptr);
    ffOptionsDestroyLogo(&opts);

    buildNestedFallbackJson(17, &deepJson);
    ffOptionsInitLogo(&opts);
    parseLogoJson(&opts, deepJson.chars, &error, &keyOut);
    VERIFY(error != nullptr);
    VERIFY(ffStrbufEqualS(&keyOut, "fallback"));
    ffOptionsDestroyLogo(&opts);

#if !FF_HAVE_IMAGE_LOGO
    ffOptionsInitLogo(&opts);
    parseLogoJson(&opts, "{\"logo\":{\"type\":\"kitty\",\"source\":\"a.png\"}}", &error, &keyOut);
    VERIFY(error != nullptr);
    ffOptionsDestroyLogo(&opts);

    ffOptionsInitLogo(&opts);
    parseLogoJson(&opts, "{\"logo\":{\"type\":\"kitty\",\"source\":\"a.png\",\"fallback\":\"none\"}}", &error, &keyOut);
    VERIFY(error == nullptr);
    VERIFY(opts.type == FF_LOGO_TYPE_IMAGE_KITTY);
    VERIFY(opts.fallback != nullptr && opts.fallback->type == FF_LOGO_TYPE_NONE);
    ffOptionsDestroyLogo(&opts);
#endif

#if !FF_HAVE_CHAFA
    ffOptionsInitLogo(&opts);
    parseLogoJson(&opts, "{\"logo\":{\"type\":\"data\",\"source\":\"X\",\"chafa\":{\"fgOnly\":true}}}", &error, &keyOut);
    VERIFY(error != nullptr);
    ffOptionsDestroyLogo(&opts);

    ffOptionsInitLogo(&opts);
    parseLogoJson(&opts, "{\"logo\":{\"type\":\"kitty\",\"source\":\"a.png\",\"chafa\":{\"fgOnly\":true},\"fallback\":\"none\"}}", &error, &keyOut);
    VERIFY(error == nullptr);
    ffOptionsDestroyLogo(&opts);

    ffOptionsInitLogo(&opts);
    parseLogoJson(&opts, "{\"logo\":{\"type\":\"kitty\",\"source\":\"a.png\",\"chafa\":42,\"fallback\":\"none\"}}", &error, &keyOut);
    VERIFY(error != nullptr);
    VERIFY(ffStrbufEqualS(&keyOut, "chafa"));
    ffOptionsDestroyLogo(&opts);
#endif
}

static void testCliFallbackParsing(void) {
    FFOptionsLogo opts;
    ffOptionsInitLogo(&opts);

    VERIFY(ffOptionsParseLogoCommandLine(&opts, "--logo-fallback", "arch"));
    VERIFY(opts.fallback != nullptr);
    VERIFY(opts.fallback->type == FF_LOGO_TYPE_AUTO);
    VERIFY(ffStrbufEqualS(&opts.fallback->source, "arch"));

    VERIFY(ffOptionsParseLogoCommandLine(&opts, "--logo-fallback", "small"));
    VERIFY(opts.fallback != nullptr);
    VERIFY(opts.fallback->type == FF_LOGO_TYPE_SMALL);
    VERIFY(opts.fallback->source.length == 0);

    VERIFY(ffOptionsParseLogoCommandLine(&opts, "--logo-fallback", "none"));
    VERIFY(opts.fallback != nullptr);
    VERIFY(opts.fallback->type == FF_LOGO_TYPE_NONE);
    VERIFY(opts.fallback->paddingTop == 0 && opts.fallback->paddingBottom == 0);
    VERIFY(opts.fallback->paddingLeft == 0 && opts.fallback->paddingRight == 0);

    ffOptionsDestroyLogo(&opts);
}

static void testGenerateLogoJsonConfig(void) {
    FFOptionsLogo opts;
    FF_STRBUF_AUTO_DESTROY keyOut = ffStrbufCreate();
    const char* error = nullptr;

    ffOptionsInitLogo(&opts);
    parseLogoJson(&opts,
        "{\"logo\":{\"type\":\"kitty\",\"source\":\"missing.png\","
        "\"fallback\":{\"type\":\"builtin\",\"source\":\"arch\",\"fallback\":null}}}",
        &error,
        &keyOut);
    VERIFY(error == nullptr);

    yyjson_mut_doc* mutDoc = yyjson_mut_doc_new(nullptr);
    VERIFY(mutDoc != nullptr);
    yyjson_mut_doc_set_root(mutDoc, yyjson_mut_obj(mutDoc));
    FFdata data = { .resultDoc = mutDoc };
    ffOptionsGenerateLogoJsonConfig(&data, &opts);

    yyjson_mut_val* logoVal = yyjson_mut_obj_get(mutDoc->root, "logo");
    VERIFY(logoVal != nullptr);
    const char* typeStr = yyjson_mut_get_str(yyjson_mut_obj_get(logoVal, "type"));
    const char* sourceStr = yyjson_mut_get_str(yyjson_mut_obj_get(logoVal, "source"));
    VERIFY(typeStr != nullptr && strcmp(typeStr, "kitty") == 0);
    VERIFY(sourceStr != nullptr && strcmp(sourceStr, "missing.png") == 0);

    yyjson_mut_val* fbVal = yyjson_mut_obj_get(logoVal, "fallback");
    VERIFY(fbVal != nullptr);
    const char* fbTypeStr = yyjson_mut_get_str(yyjson_mut_obj_get(fbVal, "type"));
    const char* fbSourceStr = yyjson_mut_get_str(yyjson_mut_obj_get(fbVal, "source"));
    VERIFY(fbTypeStr != nullptr && strcmp(fbTypeStr, "builtin") == 0);
    VERIFY(fbSourceStr != nullptr && strcmp(fbSourceStr, "arch") == 0);

    yyjson_mut_val* fb2Val = yyjson_mut_obj_get(fbVal, "fallback");
    VERIFY(fb2Val != nullptr && yyjson_mut_is_null(fb2Val));

    yyjson_mut_doc_free(mutDoc);
    ffOptionsDestroyLogo(&opts);

    ffOptionsInitLogo(&opts);
    mutDoc = yyjson_mut_doc_new(nullptr);
    VERIFY(mutDoc != nullptr);
    yyjson_mut_doc_set_root(mutDoc, yyjson_mut_obj(mutDoc));
    data = (FFdata) { .resultDoc = mutDoc };
    ffOptionsGenerateLogoJsonConfig(&data, &opts);
    logoVal = yyjson_mut_obj_get(mutDoc->root, "logo");
    VERIFY(logoVal != nullptr && yyjson_mut_obj_get(logoVal, "fallback") == nullptr);
    yyjson_mut_doc_free(mutDoc);
    ffOptionsDestroyLogo(&opts);
}

static void testActivateAndPrintFallback(void) {
    FFOptionsLogo opts;
    FF_STRBUF_AUTO_DESTROY keyOut = ffStrbufCreate();
    const char* error = nullptr;

    ffOptionsInitLogo(&opts);
    VERIFY(!ffOptionsActivateLogoFallback(&opts));
    parseLogoJson(&opts,
        "{\"logo\":{\"source\":\"primary\",\"padding\":{\"right\":9},"
        "\"fallback\":{\"type\":\"builtin\",\"source\":\"arch\",\"padding\":{\"right\":2},\"fallback\":\"none\"}}}",
        &error,
        &keyOut);
    VERIFY(error == nullptr);
    VERIFY(ffOptionsActivateLogoFallback(&opts));
    VERIFY(opts.type == FF_LOGO_TYPE_BUILTIN);
    VERIFY(ffStrbufEqualS(&opts.source, "arch"));
    VERIFY(opts.paddingRight == 2);
    VERIFY(opts.fallback != nullptr);
    VERIFY(ffOptionsActivateLogoFallback(&opts));
    VERIFY(opts.type == FF_LOGO_TYPE_NONE);
    VERIFY(opts.fallback == nullptr);
    VERIFY(!ffOptionsActivateLogoFallback(&opts));
    ffOptionsDestroyLogo(&opts);

    resetLogoTestInstance();
    parseLogoJson(&instance.config.logo, "{\"logo\":{\"type\":\"data\",\"source\":\"OK_PRIMARY\\nL2\",\"fallback\":\"none\"}}", &error, &keyOut);
    VERIFY(error == nullptr);
    ffLogoPrint();
    VERIFY(instance.config.logo.fallback != nullptr);
    VERIFY(instance.state.logoWidth == 10 + 4);
    VERIFY(instance.state.logoHeight == 1);

    resetLogoTestInstance();
    parseLogoJson(&instance.config.logo,
        "{\"logo\":{\"type\":\"file\",\"source\":\"/nonexistent/fastfetch-missing.txt\","
        "\"fallback\":{\"type\":\"data\",\"source\":\"FALLBACK\\nLINE2\",\"padding\":{\"right\":1}}}}",
        &error,
        &keyOut);
    VERIFY(error == nullptr);
    ffLogoPrint();
    VERIFY(instance.config.logo.fallback == nullptr);
    VERIFY(ffStrbufEqualS(&instance.config.logo.source, "FALLBACK\nLINE2"));
    VERIFY(instance.state.logoWidth == 8 + 1);
    VERIFY(instance.state.logoHeight == 1);

    resetLogoTestInstance();
    parseLogoJson(&instance.config.logo,
        "{\"logo\":{\"type\":\"spin\",\"source\":\"/nonexistent/fastfetch-missing-spin.txt\","
        "\"fallback\":{\"type\":\"kitty\",\"source\":\"missing.png\","
        "\"fallback\":{\"type\":\"data\",\"source\":\"CHAINED\\nNEXT\"}}}}",
        &error,
        &keyOut);
    VERIFY(error == nullptr);
    ffLogoPrint();
    VERIFY(instance.config.logo.fallback == nullptr);
    VERIFY(!instance.state.logoSpin.started);
    VERIFY(ffStrbufEqualS(&instance.config.logo.source, "CHAINED\nNEXT"));
    VERIFY(instance.state.logoWidth == 7 + 4);
    VERIFY(instance.state.logoHeight == 1);

    resetLogoTestInstance();
    parseLogoJson(&instance.config.logo,
        "{\"logo\":{\"type\":\"spin\",\"source\":\"/nonexistent/fastfetch-missing-spin.txt\",\"fallback\":null}}",
        &error,
        &keyOut);
    VERIFY(error == nullptr);
    ffLogoPrint();
    VERIFY(instance.config.logo.fallback == nullptr);
    VERIFY(instance.config.logo.type == FF_LOGO_TYPE_NONE);
    VERIFY(!instance.state.logoSpin.started);
    VERIFY(instance.config.display.colorKeys.length == 0);
    VERIFY(instance.config.display.colorTitle.length == 0);
    VERIFY(instance.state.logoWidth == 0 && instance.state.logoHeight == 0);

    resetLogoTestInstance();
    parseLogoJson(&instance.config.logo,
        "{\"logo\":{\"type\":\"kitty\",\"source\":\"missing.png\",\"fallback\":null}}",
        &error,
        &keyOut);
    VERIFY(error == nullptr);
    ffLogoPrint();
    VERIFY(instance.config.logo.fallback == nullptr);
    VERIFY(instance.config.logo.type == FF_LOGO_TYPE_NONE);
    VERIFY(instance.state.logoWidth == 0 && instance.state.logoHeight == 0);
}

int main(void) {
    // Unknown terminal size keeps the historical left-side layout.
    VERIFY_POSITION(FF_LOGO_POSITION_LEFT,
        ffLogoSelectPosition(FF_LOGO_POSITION_AUTO, 0, 40),
        "auto fallback");

    // Auto keeps the logo beside the output while at least 32 columns remain for module text.
    VERIFY_POSITION(FF_LOGO_POSITION_LEFT,
        ffLogoSelectPosition(FF_LOGO_POSITION_AUTO, 72, 40),
        "auto exact side-by-side fit");
    VERIFY_POSITION(FF_LOGO_POSITION_TOP,
        ffLogoSelectPosition(FF_LOGO_POSITION_AUTO, 71, 40),
        "auto narrow terminal");
    VERIFY_POSITION(FF_LOGO_POSITION_TOP,
        ffLogoSelectPosition(FF_LOGO_POSITION_AUTO, 80, UINT32_MAX),
        "auto width overflow");

    // Explicit positions are never overridden.
    VERIFY_POSITION(FF_LOGO_POSITION_LEFT,
        ffLogoSelectPosition(FF_LOGO_POSITION_LEFT, 20, 40),
        "explicit left");
    VERIFY_POSITION(FF_LOGO_POSITION_TOP,
        ffLogoSelectPosition(FF_LOGO_POSITION_TOP, 120, 40),
        "explicit top");
    VERIFY_POSITION(FF_LOGO_POSITION_RIGHT,
        ffLogoSelectPosition(FF_LOGO_POSITION_RIGHT, 20, 40),
        "explicit right");

    VERIFY(strcmp(ffLogoPositionToString(FF_LOGO_POSITION_AUTO), "auto") == 0);
    VERIFY(strcmp(ffLogoPositionToString(FF_LOGO_POSITION_LEFT), "left") == 0);
    VERIFY(strcmp(ffLogoPositionToString(FF_LOGO_POSITION_TOP), "top") == 0);
    VERIFY(strcmp(ffLogoPositionToString(FF_LOGO_POSITION_RIGHT), "right") == 0);

    // A `$` ending a line or the logo is a plain dollar: it keeps the line break and the terminator.
    instance.config.display.pipe = true;
    ffLogoPrintChars("ab$\ncd", true);
    VERIFY(instance.state.logoHeight == 1 && instance.state.logoWidth == 3);
    ffLogoPrintChars("ab$", true);
    VERIFY(instance.state.logoHeight == 0 && instance.state.logoWidth == 3);
    ffLogoPrintChars("a$1b$$c", true);
    VERIFY(instance.state.logoWidth == 4);
    ffLogoPrintRemaining();

    ffInitInstance();
    testJsonFallbackParsing();
    testCliFallbackParsing();
    testGenerateLogoJsonConfig();
    testActivateAndPrintFallback();
    ffLogoPrintRemaining();
    ffDestroyInstance();

    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
    return 0;
}
