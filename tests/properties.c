#include "common/properties.h"
#include "common/textModifier.h"

#include <stdio.h>
#include <stdlib.h>

static void verify(bool expression, const char* expressionStr, int lineNo) {
    if (expression) {
        return;
    }

    fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] %s\n" FASTFETCH_TEXT_MODIFIER_RESET, lineNo, expressionStr);
    exit(1);
}

#define VERIFY(expression) verify((expression), #expression, __LINE__)

static void verifyLine(const char* line, const char* start, bool expectedResult, const char* expectedValue, int lineNo) {
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    const char* cursor = line;
    const bool result = ffParsePropLinePointer(&cursor, start, &buffer);

    if (result != expectedResult || (result && !ffStrbufEqualS(&buffer, expectedValue))) {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] ffParsePropLine(\"%s\", \"%s\"): expected %s \"%s\", got %s \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET,
            lineNo, line, start, expectedResult ? "true" : "false", expectedResult ? expectedValue : "",
            result ? "true" : "false", result ? buffer.chars : "");
        exit(1);
    }
}

#define VERIFY_LINE(...) verifyLine(__VA_ARGS__, __LINE__)

static void verifyLines(const char* lines, const char* start, bool expectedResult, const char* expectedValue, int lineNo) {
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    const bool result = ffParsePropLines(lines, start, &buffer);

    if (result != expectedResult || (result && !ffStrbufEqualS(&buffer, expectedValue))) {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] ffParsePropLines(\"%s\", \"%s\"): expected %s \"%s\", got %s \"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET,
            lineNo, lines, start, expectedResult ? "true" : "false", expectedResult ? expectedValue : "",
            result ? "true" : "false", result ? buffer.chars : "");
        exit(1);
    }
}

#define VERIFY_LINES(...) verifyLines(__VA_ARGS__, __LINE__)

int main(void) {
    // The plain case: `key: value`, with the value running to the end of the line.
    {
        VERIFY_LINE("Name: foo", "Name:", true, "foo");
        VERIFY_LINE("Name: foo\n", "Name:", true, "foo");
        VERIFY_LINE("Name: foo\nBar: baz", "Name:", true, "foo"); // stops at the newline
        VERIFY_LINE("Name:foo", "Name:", true, "foo");
        VERIFY_LINE("Name: ", "Name:", true, ""); // found, but the value is empty
        VERIFY_LINE("Name:", "Name:", true, "");
        VERIFY_LINE("Name: a:b", "Name:", true, "a:b"); // only the first colon is the separator
        VERIFY_LINE("Name: foo bar", "Name:", true, "foo bar"); // spaces inside the value are kept
    }

    // Any amount of whitespace in the format matches any amount of whitespace in the line, including
    // none -- the two sides are independent.
    {
        VERIFY_LINE("Name:   foo", "Name:", true, "foo");
        VERIFY_LINE("Name:foo", "Name: ", true, "foo");
        VERIFY_LINE("Name:   foo", "Name:  ", true, "foo");
        VERIFY_LINE("Name:\tfoo", "Name: ", true, "foo");
        VERIFY_LINE("Name: foo", "Name:\t", true, "foo");
        VERIFY_LINE("  \t Name: foo", "Name:", true, "foo"); // leading whitespace of the line is skipped
    }

    // A mismatch anywhere in the key makes the whole line fail, and the key must match in full.
    {
        VERIFY_LINE("Name: foo", "Value:", false, "");
        VERIFY_LINE("name: foo", "Name:", true, "foo"); // the key is compared case insensitively
        VERIFY_LINE("NAME: foo", "name:", true, "foo");
        VERIFY_LINE("nAmE: foo", "NAME:", true, "foo");
        VERIFY_LINE("NameX: foo", "Name:", false, "");
        VERIFY_LINE("Name :foo", "Name:", false, "");
        VERIFY_LINE("Nam", "Name:", false, ""); // the line ends before the key does
        VERIFY_LINE("", "Name:", false, "");
        VERIFY_LINE("\n", "Name:", false, "");
        VERIFY_LINE("Name", "Name:", false, "");
    }

    // Trailing spaces are trimmed, but nothing else is: a trailing tab survives.
    {
        VERIFY_LINE("Name: foo   ", "Name:", true, "foo");
        VERIFY_LINE("Name: foo\t", "Name:", true, "foo\t");
        VERIFY_LINE("Name: foo  \n", "Name:", true, "foo");
    }

    // A quoted value ends at its closing quote, so trailing text is not part of the value.
    {
        VERIFY_LINE("Name: \"foo bar\"", "Name:", true, "foo bar");
        VERIFY_LINE("Name: 'foo bar'", "Name:", true, "foo bar");
        VERIFY_LINE("Name: \"foo\" trailing", "Name:", true, "foo");
        VERIFY_LINE("Name: \"\"", "Name:", true, "");
        VERIFY_LINE("Name: \"foo", "Name:", true, "foo"); // unterminated quote: the line end closes it

        // The trim still applies, and it only removes the right hand side
        VERIFY_LINE("Name: \"  foo  \"", "Name:", true, "  foo");
    }

    // A key ending in `>` switches the parser to XML mode, where the value ends at `<` instead of at
    // the end of the line.
    {
        VERIFY_LINE("<name>foo</name>", "<name>", true, "foo");
        VERIFY_LINE("<name></name>", "<name>", true, "");
        VERIFY_LINE("<name>foo", "<name>", true, "foo"); // no closing tag
        VERIFY_LINE("<name>foo bar</name>", "<name>", true, "foo bar");
        VERIFY_LINE("<name> foo</name>", "<name>", true, "foo");
        VERIFY_LINE("<name>foo\n", "<name>", true, "foo");
    }

    // ffParsePropLines walks the lines until the key is found, and the first match wins -- including
    // when that match has an empty value.
    {
        VERIFY_LINES("A: 1\nName: foo\nB: 2", "Name:", true, "foo");
        VERIFY_LINES("A: 1\nName: foo", "Name:", true, "foo");
        VERIFY_LINES("Name: foo", "Name:", true, "foo");
        VERIFY_LINES("\n\nName: foo", "Name:", true, "foo");
        VERIFY_LINES("Name: foo\nName: bar", "Name:", true, "foo");
        VERIFY_LINES("Name:\nName: bar", "Name:", true, "");
        VERIFY_LINES("A: 1\nB: 2", "Name:", false, "");
        VERIFY_LINES("A: 1\nB: 2\n", "Name:", false, "");
        VERIFY_LINES("", "Name:", false, "");
    }

    // The value is appended to the buffer, so a caller can collect several keys into one string.
    {
        FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreateS("pre");
        VERIFY(ffParsePropLines("Name: foo", "Name:", &buffer));
        VERIFY(ffStrbufEqualS(&buffer, "prefoo"));
    }

    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
    return 0;
}
