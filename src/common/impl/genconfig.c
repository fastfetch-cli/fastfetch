#include "common/genconfig.h"

#include "fastfetch.h"

#include "common/io.h"
#include "common/strutil.h"
#include "detection/terminalsize/terminalsize.h"
#include "fastfetch_datatext.h"
#include "modules/modules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
    #include <termios.h>
    #include <poll.h>
    #include <signal.h>
    #include <unistd.h>
#else
    #include <windows.h>
#endif

// Layout (rows):
//  0: title
//  1: blank
//  2: logo type row
//  3: output mode row
//  4: blank
//  5: modules title row
//  6: blank
//  7..7+listRows-1: module grid
//  7+listRows: description row
//  8+listRows: help line 1
//  9+listRows: help line 2
#define FF_GEN_CONFIG_LIST_TOP 7
#define FF_GEN_CONFIG_BOTTOM_CHROME 3

// Poll wait timeout (ms) while idle. A longer wait avoids frequent full-screen
// redraws (flicker); terminal size changes redraw immediately via SIGWINCH /
// console window-size events.
#define FF_GEN_CONFIG_POLL_TIMEOUT 1000

typedef enum : uint8_t {
    FF_GEN_CONFIG_ITEM_STATUS_UNSELECTED = false,
    FF_GEN_CONFIG_ITEM_STATUS_SELECTED = true,
    FF_GEN_CONFIG_ITEM_STATUS_SPECIAL = 2,
} FFGenConfigItemStatus;

typedef struct FFGenConfigItem {
    FFModuleBaseInfo* baseInfo;
    FFGenConfigItemStatus status;
} FFGenConfigItem;

typedef struct FFGenConfigLayout {
    uint16_t listLeft;
    uint16_t listRows;
    uint32_t colWidth;
    uint32_t columns;
    uint32_t itemsPerColumn;
} FFGenConfigLayout;

typedef struct FFGenConfigUI {
    FFlist items; // FFGenConfigItem
    uint32_t cursor;
    uint32_t viewOffset;
    FFLogoType logoType;
    bool fullConfig;
    bool confirmingOverwrite;
    uint16_t rows;
    uint16_t cols;
    FFGenConfigLayout layout;
} FFGenConfigUI;

typedef enum : uint8_t {
    FF_GEN_KEY_UP,
    FF_GEN_KEY_DOWN,
    FF_GEN_KEY_LEFT,
    FF_GEN_KEY_RIGHT,
    FF_GEN_KEY_ENTER,
    FF_GEN_KEY_ESCAPE,
    FF_GEN_KEY_CHAR,
    FF_GEN_KEY_RESIZE,
    FF_GEN_KEY_UNKNOWN,
} FFGenConfigKey;

#ifndef _WIN32
static struct termios gOriginalTermios;
static bool gRawModeActive = false;
static volatile sig_atomic_t gWindowResized = 0;

static void onWindowResize(int sig) {
    (void) sig;
    gWindowResized = 1;
}

static void restoreRawMode(void) {
    if (gRawModeActive) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &gOriginalTermios);
        gRawModeActive = false;
    }
}

static void enterRawMode(void) {
    if (gRawModeActive) {
        return;
    }
    if (tcgetattr(STDIN_FILENO, &gOriginalTermios) != 0) {
        return;
    }
    struct termios raw = gOriginalTermios;
    raw.c_lflag &= (tcflag_t) ~(ICANON | ECHO | ISIG | IEXTEN);
    raw.c_iflag &= (tcflag_t) ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_cflag |= CS8;
    raw.c_oflag &= (tcflag_t) ~OPOST;
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0) {
        return;
    }
    gRawModeActive = true;

    struct sigaction sa = { .sa_handler = onWindowResize };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGWINCH, &sa, nullptr);
}
#else
static DWORD gOriginalInputMode;
static DWORD gOriginalOutputMode;
static bool gRawModeActive = false;

static void restoreRawMode(void) {
    if (gRawModeActive) {
        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), gOriginalInputMode);
        SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), gOriginalOutputMode);
        gRawModeActive = false;
    }
}

static void enterRawMode(void) {
    if (gRawModeActive) {
        return;
    }
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

    if (GetConsoleMode(hInput, &gOriginalInputMode)) {
        DWORD newMode = gOriginalInputMode;
        newMode &= ~(DWORD) (ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
        newMode |= ENABLE_VIRTUAL_TERMINAL_INPUT | ENABLE_WINDOW_INPUT;
        SetConsoleMode(hInput, newMode);
    }

    if (GetConsoleMode(hOutput, &gOriginalOutputMode)) {
        DWORD newMode = gOriginalOutputMode;
        newMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
        SetConsoleMode(hOutput, newMode);
    }

    FlushConsoleInputBuffer(hInput);
    gRawModeActive = true;
}
#endif

static void getTerminalSize(uint16_t* rows, uint16_t* cols) {
    FFTerminalSizeResult size;
    if (ffDetectTerminalSize(&size) && size.rows > 0 && size.columns > 0) {
        *rows = size.rows;
        *cols = size.columns;
    } else {
        *rows = 24;
        *cols = 80;
    }
}

static int compareModuleInfo(const void* a, const void* b) {
    const FFModuleBaseInfo* const* ma = (const FFModuleBaseInfo* const*) a;
    const FFModuleBaseInfo* const* mb = (const FFModuleBaseInfo* const*) b;
    if ((*ma)->defaultOrder != (*mb)->defaultOrder) {
        return (int) (*ma)->defaultOrder - (int) (*mb)->defaultOrder;
    }
    return strcmp((*ma)->name, (*mb)->name);
}

static void collectModuleInfos(FFlist* modules) {
    ffListInitA(modules, sizeof(FFModuleBaseInfo*), 64);
    for (uint32_t i = 0; i <= 'Z' - 'A'; ++i) {
        for (FFModuleBaseInfo** it = ffModuleInfos[i]; *it; ++it) {
            if ((*it)->defaultOrder == 0) {
                continue;
            }
            *(FFModuleBaseInfo**) ffListAdd(modules, sizeof(FFModuleBaseInfo*)) = *it;
        }
    }
    ffListSort(modules, sizeof(FFModuleBaseInfo*), compareModuleInfo);
}

static bool isInDefaultStructure(const char* moduleName) {
    char* moduleType = nullptr;
    size_t moduleLen = 0;
    FF_STRBUF_AUTO_DESTROY structure = ffStrbufCreateS(FASTFETCH_DATATEXT_STRUCTURE);
    while (ffStrbufGetdelim(&moduleType, &moduleLen, ':', &structure)) {
        if (moduleLen == strlen(moduleName) && ffStrEqualsIgnCase(moduleType, moduleName)) {
            return true;
        }
    }
    return false;
}

static void initItems(FFlist* items, const FFlist* modules) {
    ffListInitA(items, sizeof(FFGenConfigItem), modules->length + 8);
    FF_LIST_FOR_EACH (FFModuleBaseInfo*, info, *modules) {
        FFGenConfigItem* item = FF_LIST_ADD(FFGenConfigItem, *items);
        item->baseInfo = *info;
        if (ffStrEqualsIgnCase((*info)->name, FF_BREAK_MODULE_NAME) ||
            ffStrEqualsIgnCase((*info)->name, FF_SEPARATOR_MODULE_NAME)) {
            item->status = FF_GEN_CONFIG_ITEM_STATUS_SPECIAL;
        } else {
            item->status = isInDefaultStructure((*info)->name)
                ? FF_GEN_CONFIG_ITEM_STATUS_SELECTED
                : FF_GEN_CONFIG_ITEM_STATUS_UNSELECTED;
        }
    }
}

static void moveItemDown(FFGenConfigUI* ui, uint32_t idx) {
    if (idx + 1 >= ui->items.length) {
        return;
    }
    FFGenConfigItem* items = (FFGenConfigItem*) ui->items.data;
    FFGenConfigItem tmp = items[idx];
    items[idx] = items[idx + 1];
    items[idx + 1] = tmp;
    ui->cursor = idx + 1;
}

static void moveItemUp(FFGenConfigUI* ui, uint32_t idx) {
    if (idx == 0) {
        return;
    }
    FFGenConfigItem* items = (FFGenConfigItem*) ui->items.data;
    FFGenConfigItem tmp = items[idx];
    items[idx] = items[idx - 1];
    items[idx - 1] = tmp;
    ui->cursor = idx - 1;
}

static void removeAllBreaksSeparators(FFGenConfigUI* ui) {
    for (uint32_t i = ui->items.length; i-- > 0;) {
        FFGenConfigItem* item = FF_LIST_GET(FFGenConfigItem, ui->items, i);
        if (item->status == FF_GEN_CONFIG_ITEM_STATUS_SPECIAL) {
            FF_LIST_REMOVE_AT(FFGenConfigItem, ui->items, i);
        }
    }
    if (ui->cursor >= ui->items.length && ui->items.length > 0) {
        ui->cursor = ui->items.length - 1;
    }
}

static void computeLayout(FFGenConfigUI* ui) {
    FFGenConfigLayout* layout = &ui->layout;
    if (ui->rows < 10) {
        ui->rows = 10;
    }
    int32_t listRows = (int32_t) ui->rows - FF_GEN_CONFIG_LIST_TOP - FF_GEN_CONFIG_BOTTOM_CHROME;
    layout->listRows = listRows < 0 ? 0 : (uint16_t) listRows;
    layout->listLeft = 2;

    uint32_t maxContentWidth = 0;
    FF_LIST_FOR_EACH (FFGenConfigItem, item, ui->items) {
        uint32_t w = (uint32_t) strlen(item->baseInfo->name) + 4; // "[x] " / "[─] " prefix
        if (w > maxContentWidth) {
            maxContentWidth = w;
        }
    }
    layout->colWidth = maxContentWidth + 2;
    if (layout->colWidth < 1) {
        layout->colWidth = 1;
    }

    uint32_t gridWidth = ui->cols > 4 ? (uint32_t) ui->cols - 4 : 1;
    uint32_t maxColumns = gridWidth / layout->colWidth;
    if (maxColumns < 1) {
        maxColumns = 1;
    }

    uint32_t length = ui->items.length;
    if (length == 0 || layout->listRows == 0) {
        layout->columns = 1;
        layout->itemsPerColumn = length > 0 ? length : 1;
        return;
    }
    uint32_t neededColumns = (length + layout->listRows - 1) / layout->listRows;
    if (neededColumns < 1) {
        neededColumns = 1;
    }
    layout->columns = neededColumns < maxColumns ? neededColumns : maxColumns;
    layout->itemsPerColumn = (length + layout->columns - 1) / layout->columns;
}

static void recomputeView(FFGenConfigUI* ui) {
    const FFGenConfigLayout* layout = &ui->layout;
    uint32_t length = ui->items.length;
    if (length == 0) {
        ui->cursor = 0;
        ui->viewOffset = 0;
        return;
    }
    if (ui->cursor >= length) {
        ui->cursor = length - 1;
    }

    uint32_t rowsPerColumn = layout->itemsPerColumn;
    if (rowsPerColumn <= layout->listRows) {
        ui->viewOffset = 0;
        return;
    }

    uint32_t cursorRow = ui->cursor % rowsPerColumn;
    uint32_t pageTop = ui->viewOffset;
    if (cursorRow < pageTop) {
        pageTop = cursorRow;
    } else if (cursorRow >= pageTop + layout->listRows) {
        pageTop = cursorRow - layout->listRows + 1;
    }
    uint32_t maxTop = rowsPerColumn - layout->listRows;
    if (pageTop > maxTop) {
        pageTop = maxTop;
    }
    ui->viewOffset = pageTop;
}

static void toggleItem(FFGenConfigUI* ui, uint32_t idx) {
    FFGenConfigItem* item = FF_LIST_GET(FFGenConfigItem, ui->items, idx);
    if (item->status == FF_GEN_CONFIG_ITEM_STATUS_SPECIAL) {
        return;
    }
    item->status = !item->status;
}

static void selectAllModules(FFGenConfigUI* ui) {
    FF_LIST_FOR_EACH (FFGenConfigItem, item, ui->items) {
        if (item->status != FF_GEN_CONFIG_ITEM_STATUS_SPECIAL) {
            item->status = FF_GEN_CONFIG_ITEM_STATUS_SELECTED;
        }
    }
}

static void invertAllModules(FFGenConfigUI* ui) {
    FF_LIST_FOR_EACH (FFGenConfigItem, item, ui->items) {
        if (item->status != FF_GEN_CONFIG_ITEM_STATUS_SPECIAL) {
            item->status = !item->status;
        }
    }
}

static void addBreakBelow(FFGenConfigUI* ui, uint32_t idx) {
    FFGenConfigItem br = { .baseInfo = &ffBreakModuleInfo, .status = FF_GEN_CONFIG_ITEM_STATUS_SPECIAL };
    FF_LIST_INSERT_AT(FFGenConfigItem, ui->items, idx + 1, &br);
}

static void addSeparatorBelow(FFGenConfigUI* ui, uint32_t idx) {
    FFGenConfigItem sep = { .baseInfo = &ffSeparatorModuleInfo, .status = FF_GEN_CONFIG_ITEM_STATUS_SPECIAL };
    FF_LIST_INSERT_AT(FFGenConfigItem, ui->items, idx + 1, &sep);
}

static void cycleLogoType(FFGenConfigUI* ui, int delta) {
    switch (ui->logoType) {
        case FF_LOGO_TYPE_AUTO:
            ui->logoType = delta > 0 ? FF_LOGO_TYPE_SMALL : FF_LOGO_TYPE_NONE;
            break;
        case FF_LOGO_TYPE_SMALL:
            ui->logoType = delta > 0 ? FF_LOGO_TYPE_NONE : FF_LOGO_TYPE_AUTO;
            break;
        default:
            ui->logoType = delta > 0 ? FF_LOGO_TYPE_AUTO : FF_LOGO_TYPE_SMALL;
            break;
    }
}

static uint32_t countSelectedModules(const FFlist* items) {
    uint32_t count = 0;
    FF_LIST_FOR_EACH (FFGenConfigItem, item, *items) {
        if (item->status == FF_GEN_CONFIG_ITEM_STATUS_SELECTED) {
            ++count;
        }
    }
    return count;
}

static uint32_t countModules(const FFlist* items) {
    uint32_t count = 0;
    FF_LIST_FOR_EACH (FFGenConfigItem, item, *items) {
        if (item->status != FF_GEN_CONFIG_ITEM_STATUS_SPECIAL) {
            ++count;
        }
    }
    return count;
}

// ---- Row builder -----------------------------------------------------------

typedef struct FFRow {
    FFstrbuf buf;
    uint32_t visualCol;
    uint16_t cols;
} FFRow;

static void rowInit(FFRow* row, uint16_t cols) {
    ffStrbufInitA(&row->buf, (uint32_t) cols + 16);
    row->visualCol = 0;
    row->cols = cols;
}

static void rowAppendRaw(FFRow* row, const char* s) {
    ffStrbufAppendS(&row->buf, s);
}

static void rowAppendVisual(FFRow* row, const char* s) {
    uint32_t len = (uint32_t) strlen(s);
    ffStrbufAppendNS(&row->buf, len, s);
    row->visualCol += ffUtf8StrWidth(s, len);
}

static void rowAppendVisualTruncated(FFRow* row, const char* s, uint32_t maxWidth) {
    uint32_t total = (uint32_t) strlen(s);
    uint32_t bytes = total;
    if (ffUtf8StrWidth(s, total) > maxWidth) {
        bytes = 0;
        uint32_t used = 0;
        const char* p = s;
        uint32_t remaining = total;
        while (remaining > 0 && used < maxWidth) {
            uint8_t w = 0;
            uint8_t cbytes = ffUtf8CharLenWidth(p, remaining, &w);
            if (cbytes == 0 || used + w > maxWidth) {
                break;
            }
            used += w;
            bytes += cbytes;
            p += cbytes;
            remaining -= cbytes;
        }
    }
    ffStrbufAppendNS(&row->buf, bytes, s);
    row->visualCol += ffUtf8StrWidth(s, bytes);
}

static void rowPadVisual(FFRow* row, uint32_t col) {
    while (row->visualCol < col) {
        ffStrbufAppendC(&row->buf, ' ');
        ++row->visualCol;
    }
}

static uint32_t rowAnsiSeqLength(const char* s, uint32_t length) {
    uint32_t i = 1;
    while (i < length) {
        uint8_t c = (uint8_t) s[i];
        if (c >= 0x40 && c <= 0x7e) {
            return i + 1;
        }
        ++i;
    }
    return length;
}

static void truncateRow(FFRow* row) {
    FFstrbuf src = row->buf;
    ffStrbufInitA(&row->buf, row->cols + 32);
    uint32_t used = 0;
    uint32_t remaining = src.length;
    const char* p = src.chars;
    while (remaining > 0 && used < row->cols) {
        if (*p == '\x1b') {
            uint32_t len = rowAnsiSeqLength(p, remaining);
            ffStrbufAppendNS(&row->buf, len, p);
            p += len;
            remaining -= len;
        } else {
            uint8_t w = 0;
            uint8_t cbytes = ffUtf8CharLenWidth(p, remaining, &w);
            if (cbytes == 0) {
                cbytes = 1;
                w = 1;
            }
            if (used + w > row->cols) {
                break;
            }
            ffStrbufAppendNS(&row->buf, cbytes, p);
            used += w;
            p += cbytes;
            remaining -= cbytes;
        }
    }
    ffStrbufDestroy(&src);
    row->visualCol = used;
}

static void finishRow(FFRow* row, FFstrbuf* out, bool isLastRow) {
    if (row->visualCol > row->cols) {
        truncateRow(row);
        ffStrbufAppendS(&row->buf, "\e[m");
    }
    if (row->visualCol < row->cols) {
        rowPadVisual(row, row->cols);
    }
    ffStrbufAppend(out, &row->buf);
    ffStrbufAppendS(out, "\e[K");
    if (!isLastRow) {
        ffStrbufAppendS(out, "\r\n");
    }
    ffStrbufDestroy(&row->buf);
}

static void drawModuleItem(FFRow* row, const FFGenConfigItem* item, uint32_t cellWidth, bool highlight) {
    const uint32_t nameMax = cellWidth > 4 ? cellWidth - 4 : 0;

    if (item->baseInfo == &ffBreakModuleInfo) {
        rowAppendRaw(row, highlight ? "\e[1;95;7m" : "\e[1;95m");
        ffStrbufAppendS(&row->buf, "[─]");
        row->visualCol += 3;
        rowAppendRaw(row, "\e[m");
        rowAppendVisual(row, " ");
        rowAppendRaw(row, highlight ? "\e[1;95;7m" : "\e[1;95m");
        rowAppendVisualTruncated(row, item->baseInfo->name, nameMax);
        rowAppendRaw(row, "\e[m");
        return;
    }
    if (item->baseInfo == &ffSeparatorModuleInfo) {
        rowAppendRaw(row, highlight ? "\e[1;96;7m" : "\e[1;96m");
        ffStrbufAppendS(&row->buf, "[═]");
        row->visualCol += 3;
        rowAppendRaw(row, "\e[m");
        rowAppendVisual(row, " ");
        rowAppendRaw(row, highlight ? "\e[1;96;7m" : "\e[1;96m");
        rowAppendVisualTruncated(row, item->baseInfo->name, nameMax);
        rowAppendRaw(row, "\e[m");
        return;
    }

    if (item->status == FF_GEN_CONFIG_ITEM_STATUS_SELECTED) {
        rowAppendRaw(row, highlight ? "\e[1;92;7m" : "\e[1;92m");
        ffStrbufAppendS(&row->buf, "[x]");
        row->visualCol += 3;
        rowAppendRaw(row, "\e[m");
        rowAppendVisual(row, " ");
        rowAppendRaw(row, highlight ? "\e[1;97;7m" : "\e[1;97m");
        rowAppendVisualTruncated(row, item->baseInfo->name, nameMax);
        rowAppendRaw(row, "\e[m");
    } else {
        rowAppendRaw(row, highlight ? "\e[90;7m" : "\e[90m");
        ffStrbufAppendS(&row->buf, "[ ]");
        row->visualCol += 3;
        rowAppendRaw(row, "\e[m");
        rowAppendVisual(row, " ");
        rowAppendVisualTruncated(row, item->baseInfo->name, nameMax);
        rowAppendRaw(row, "\e[m");
    }
}

static void drawItemCell(FFRow* row, uint32_t startCol, uint32_t cellWidth, const FFGenConfigItem* item, bool highlight) {
    rowPadVisual(row, startCol);
    drawModuleItem(row, item, cellWidth, highlight);
    rowPadVisual(row, startCol + cellWidth);
}

static void drawLogoOption(FFRow* row, const char* name, bool active) {
    if (active) {
        rowAppendRaw(row, "\e[1;92m");
        rowAppendVisual(row, "● ");
        rowAppendRaw(row, "\e[m");
        rowAppendRaw(row, "\e[1m");
        rowAppendVisual(row, name);
        rowAppendRaw(row, "\e[m");
    } else {
        rowAppendRaw(row, "\e[90m");
        rowAppendVisual(row, "○ ");
        rowAppendVisual(row, name);
        rowAppendRaw(row, "\e[m");
    }
}

static void renderFrame(FFGenConfigUI* ui, FFstrbuf* out) {
    getTerminalSize(&ui->rows, &ui->cols);
    computeLayout(ui);
    recomputeView(ui);

    const FFGenConfigLayout* layout = &ui->layout;
    const uint16_t cols = ui->cols;
    const uint32_t selectedCount = countSelectedModules(&ui->items);
    const uint32_t moduleCount = countModules(&ui->items);

    ffStrbufAppendS(out, "\e[?25l\e[H");
    uint32_t rowCount = 0;

    FFRow row;

    // Row 0: title
    rowInit(&row, cols);
    rowAppendVisual(&row, "  ");
    rowAppendRaw(&row, "\e[1;36m");
    rowAppendVisual(&row, "fastfetch");
    rowAppendRaw(&row, "\e[m");
    rowAppendRaw(&row, "\e[1m");
    rowAppendVisual(&row, " configuration");
    rowAppendRaw(&row, "\e[m");
    if (cols >= 48) {
        rowAppendVisual(&row, "    ");
        rowAppendRaw(&row, "\e[90m");
        rowAppendVisual(&row, "interactive config generator");
        rowAppendRaw(&row, "\e[m");
    }
    finishRow(&row, out, ++rowCount == ui->rows);

    // Row 1: blank
    rowInit(&row, cols);
    finishRow(&row, out, ++rowCount == ui->rows);

    // Row 2: logo type
    rowInit(&row, cols);
    rowAppendVisual(&row, "  ");
    rowAppendRaw(&row, "\e[1;97m");
    rowAppendVisual(&row, "Logo type:");
    rowAppendRaw(&row, "\e[m");
    rowAppendVisual(&row, "  ");
    drawLogoOption(&row, "default", ui->logoType == FF_LOGO_TYPE_AUTO);
    rowAppendVisual(&row, "  ");
    drawLogoOption(&row, "small", ui->logoType == FF_LOGO_TYPE_SMALL);
    rowAppendVisual(&row, "  ");
    drawLogoOption(&row, "none", ui->logoType == FF_LOGO_TYPE_NONE);
    if (cols >= 72) {
        rowAppendVisual(&row, "    ");
        rowAppendRaw(&row, "\e[90m");
        rowAppendVisual(&row, "(l)");
        rowAppendRaw(&row, "\e[m");
    }
    finishRow(&row, out, ++rowCount == ui->rows);

    // Row 3: output mode
    rowInit(&row, cols);
    rowAppendVisual(&row, "  ");
    rowAppendRaw(&row, "\e[1;97m");
    rowAppendVisual(&row, "Output:");
    rowAppendRaw(&row, "\e[m");
    rowAppendVisual(&row, "  ");
    drawLogoOption(&row, "minimal", !ui->fullConfig);
    rowAppendVisual(&row, "  ");
    drawLogoOption(&row, "full", ui->fullConfig);
    if (cols >= 64) {
        rowAppendVisual(&row, "    ");
        rowAppendRaw(&row, "\e[90m");
        rowAppendVisual(&row, "(o)");
        rowAppendRaw(&row, "\e[m");
    }
    finishRow(&row, out, ++rowCount == ui->rows);

    // Row 4: blank
    rowInit(&row, cols);
    finishRow(&row, out, ++rowCount == ui->rows);

    // Row 5: modules title
    rowInit(&row, cols);
    rowAppendVisual(&row, "  ");
    rowAppendRaw(&row, "\e[1;97m");
    rowAppendVisual(&row, "Modules:");
    rowAppendRaw(&row, "\e[m");
    rowAppendRaw(&row, "\e[92m");
    FF_STRBUF_AUTO_DESTROY counter = ffStrbufCreateA(32);
    ffStrbufAppendF(&counter, "  [%u/%u selected]", selectedCount, moduleCount);
    rowAppendVisual(&row, counter.chars);
    rowAppendRaw(&row, "\e[m");
    finishRow(&row, out, ++rowCount == ui->rows);

    // Row 6: blank
    rowInit(&row, cols);
    finishRow(&row, out, ++rowCount == ui->rows);

    // Grid rows
    const uint32_t rowsPerColumn = layout->itemsPerColumn;
    const uint32_t length = ui->items.length;
    for (uint16_t screenRow = 0; screenRow < layout->listRows; ++screenRow) {
        rowInit(&row, cols);
        for (uint32_t c = 0; c < layout->columns; ++c) {
            uint32_t startCol = layout->listLeft + c * layout->colWidth;
            uint32_t index = (ui->viewOffset + screenRow) + c * rowsPerColumn;
            if (screenRow < rowsPerColumn && index < length) {
                const FFGenConfigItem* item = FF_LIST_GET(FFGenConfigItem, ui->items, index);
                drawItemCell(&row, startCol, layout->colWidth, item, ui->cursor == index);
            } else {
                rowPadVisual(&row, startCol + layout->colWidth);
            }
        }
        finishRow(&row, out, ++rowCount == ui->rows);
    }

    // Row after grid: description
    rowInit(&row, cols);
    if (ui->cursor < ui->items.length) {
        const FFGenConfigItem* item = FF_LIST_GET(FFGenConfigItem, ui->items, ui->cursor);
        if (item->baseInfo && item->baseInfo->description) {
            rowAppendVisual(&row, "  ");
            rowAppendRaw(&row, "\e[90m");
            rowAppendVisualTruncated(&row, item->baseInfo->description, cols > 4 ? cols - 4 : 1);
            rowAppendRaw(&row, "\e[m");
        }
    }
    finishRow(&row, out, ++rowCount == ui->rows);

    // Help lines
    rowInit(&row, cols);
    rowAppendRaw(&row, "\e[90m");
    rowAppendVisual(&row, "  ↑/↓ k/j move  ←/→ col  Space toggle  f/F all/invert  K/J reorder  b/B break/sep  d/D del");
    if (ui->confirmingOverwrite) {
        rowAppendRaw(&row, "\e[m");
        rowAppendRaw(&row, "\e[1;93m");
        rowAppendVisual(&row, "  File exists. Overwrite? (y/N)");
        rowAppendRaw(&row, "\e[m");
    }
    finishRow(&row, out, ++rowCount == ui->rows);

    rowInit(&row, cols);
    rowAppendRaw(&row, "\e[90m");
    rowAppendVisual(&row, "  l logo  o minimal/full  s/Enter save  q/Esc quit  g/G top/bottom");
    rowAppendRaw(&row, "\e[m");
    finishRow(&row, out, ++rowCount == ui->rows);

    ffStrbufAppendS(out, "\e[J");
}

// ---- Key input -------------------------------------------------------------

static FFGenConfigKey readKey(char* outChar) {
#ifndef _WIN32
    if (gWindowResized) {
        gWindowResized = 0;
        return FF_GEN_KEY_RESIZE;
    }
    struct pollfd pfd = {
        .fd = STDIN_FILENO,
        .events = POLLIN,
    };
    if (poll(&pfd, 1, FF_GEN_CONFIG_POLL_TIMEOUT) <= 0) {
        if (gWindowResized) {
            gWindowResized = 0;
            return FF_GEN_KEY_RESIZE;
        }
        return FF_GEN_KEY_UNKNOWN;
    }
#else
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    if (WaitForSingleObject(hInput, FF_GEN_CONFIG_POLL_TIMEOUT) != WAIT_OBJECT_0) {
        return FF_GEN_KEY_UNKNOWN;
    }
    DWORD numEvents = 0;
    if (GetNumberOfConsoleInputEvents(hInput, &numEvents) && numEvents > 0) {
        bool consumed = false;
        INPUT_RECORD record;
        while (numEvents > 0) {
            DWORD n = 0;
            if (!PeekConsoleInputW(hInput, &record, 1, &n) || n == 0) {
                break;
            }
            if (record.EventType == KEY_EVENT) {
                break; // keep key events for ffReadFDData
            }
            if (!ReadConsoleInputW(hInput, &record, 1, &n) || n == 0) {
                break;
            }
            consumed = true;
            --numEvents;
        }
        if (consumed) {
            return FF_GEN_KEY_RESIZE;
        }
    }
#endif

    char buf[16];
    ssize_t n = ffReadFDData(FFUnixFD2NativeFD(STDIN_FILENO), sizeof(buf), buf);
    if (n <= 0) {
        return FF_GEN_KEY_UNKNOWN;
    }
    if (buf[0] == '\r' || buf[0] == '\n') {
        return FF_GEN_KEY_ENTER;
    }
    if (buf[0] != '\x1b') {
        *outChar = buf[0];
        return FF_GEN_KEY_CHAR;
    }
    if (n == 1) {
        return FF_GEN_KEY_ESCAPE;
    }

    if (buf[1] == '[' || buf[1] == 'O') {
        size_t i = 2;
        if (i < (size_t) n && buf[i] >= 'A' && buf[i] <= 'D') {
            switch (buf[i]) {
                case 'A':
                    return FF_GEN_KEY_UP;
                case 'B':
                    return FF_GEN_KEY_DOWN;
                case 'C':
                    return FF_GEN_KEY_RIGHT;
                case 'D':
                    return FF_GEN_KEY_LEFT;
            }
        }
        uint32_t param = 0;
        while (i < (size_t) n && buf[i] >= '0' && buf[i] <= '9') {
            param = param * 10 + (uint32_t) (buf[i] - '0');
            ++i;
        }
        while (i < (size_t) n && buf[i] == ';') {
            ++i;
            while (i < (size_t) n && buf[i] >= '0' && buf[i] <= '9') {
                ++i;
            }
        }
        if (i < (size_t) n) {
            switch (buf[i]) {
                case 'A':
                    return FF_GEN_KEY_UP;
                case 'B':
                    return FF_GEN_KEY_DOWN;
                case 'C':
                    return FF_GEN_KEY_RIGHT;
                case 'D':
                    return FF_GEN_KEY_LEFT;
            }
        }
        return FF_GEN_KEY_UNKNOWN;
    }

    return FF_GEN_KEY_ESCAPE;
}

static void moveCursorLeft(FFGenConfigUI* ui) {
    uint32_t rowsPerColumn = ui->layout.itemsPerColumn;
    if (rowsPerColumn == 0) {
        return;
    }
    if (ui->cursor >= rowsPerColumn) {
        ui->cursor -= rowsPerColumn;
    }
}

static void moveCursorRight(FFGenConfigUI* ui) {
    uint32_t rowsPerColumn = ui->layout.itemsPerColumn;
    if (rowsPerColumn == 0) {
        return;
    }
    uint32_t newCursor = ui->cursor + rowsPerColumn;
    if (newCursor < ui->items.length) {
        ui->cursor = newCursor;
    }
}

static void moveCursorUp(FFGenConfigUI* ui) {
    if (ui->cursor > 0) {
        --ui->cursor;
    } else {
        ui->cursor = ui->items.length > 0 ? ui->items.length - 1 : 0;
    }
}

static void moveCursorDown(FFGenConfigUI* ui) {
    if (ui->items.length == 0) {
        return;
    }
    if (ui->cursor + 1 < ui->items.length) {
        ++ui->cursor;
    } else {
        ui->cursor = 0;
    }
}

// Returns 1 on save, 0 on quit, -1 on continue
static int handleKey(FFGenConfigUI* ui, FFGenConfigKey key, char ch, bool fileExists) {
    if (key == FF_GEN_KEY_UNKNOWN) {
        return -1;
    }
    if (key == FF_GEN_KEY_RESIZE) {
        return -1; // Terminal resized: redraw with new size
    }

    if (ui->confirmingOverwrite) {
        if (key == FF_GEN_KEY_CHAR && (ch == 'y' || ch == 'Y')) {
            return 1;
        }
        if ((key == FF_GEN_KEY_CHAR && (ch == 'n' || ch == 'N')) || key == FF_GEN_KEY_ESCAPE || key == FF_GEN_KEY_ENTER) {
            ui->confirmingOverwrite = false;
        }
        return -1;
    }

    switch (key) {
        case FF_GEN_KEY_UP:
            moveCursorUp(ui);
            break;
        case FF_GEN_KEY_DOWN:
            moveCursorDown(ui);
            break;
        case FF_GEN_KEY_LEFT:
            moveCursorLeft(ui);
            break;
        case FF_GEN_KEY_RIGHT:
            moveCursorRight(ui);
            break;
        case FF_GEN_KEY_ENTER:
            if (fileExists) {
                ui->confirmingOverwrite = true;
            } else {
                return 1;
            }
            break;
        case FF_GEN_KEY_ESCAPE:
            return 0;
        case FF_GEN_KEY_CHAR:
            if (ch == 'q') {
                return 0;
            } else if (ch == 's') {
                if (fileExists) {
                    ui->confirmingOverwrite = true;
                } else {
                    return 1;
                }
            } else if (ch == ' ') {
                if (ui->cursor < ui->items.length) {
                    toggleItem(ui, ui->cursor);
                }
            } else if (ch == 'f') {
                selectAllModules(ui);
            } else if (ch == 'F') {
                invertAllModules(ui);
            } else if (ch == 'o') {
                ui->fullConfig = !ui->fullConfig;
            } else if (ch == 'j') {
                moveCursorDown(ui);
            } else if (ch == 'k') {
                moveCursorUp(ui);
            } else if (ch == 'J') {
                if (ui->cursor < ui->items.length) {
                    moveItemDown(ui, ui->cursor);
                }
            } else if (ch == 'K') {
                if (ui->cursor < ui->items.length) {
                    moveItemUp(ui, ui->cursor);
                }
            } else if (ch == 'b') {
                if (ui->cursor < ui->items.length) {
                    addBreakBelow(ui, ui->cursor);
                }
            } else if (ch == 'B') {
                if (ui->cursor < ui->items.length) {
                    addSeparatorBelow(ui, ui->cursor);
                }
            } else if (ch == 'd') {
                if (ui->cursor < ui->items.length) {
                    FFGenConfigItem* item = FF_LIST_GET(FFGenConfigItem, ui->items, ui->cursor);
                    if (item->status == FF_GEN_CONFIG_ITEM_STATUS_SPECIAL) {
                        FF_LIST_REMOVE_AT(FFGenConfigItem, ui->items, ui->cursor);
                        if (ui->cursor >= ui->items.length && ui->items.length > 0) {
                            ui->cursor = ui->items.length - 1;
                        }
                    }
                }
            } else if (ch == 'D') {
                removeAllBreaksSeparators(ui);
            } else if (ch == 'g') {
                ui->cursor = 0;
            } else if (ch == 'G') {
                ui->cursor = ui->items.length > 0 ? ui->items.length - 1 : 0;
            } else if (ch == 'l') {
                cycleLogoType(ui, 1);
            } else if (ch == '\x03' || ch == '\x04' || ch == '\x1a' || ch == '\x1c') {
                return 0;
            }
            break;
        default:
            break;
    }
    return -1;
}

static int runCui(FFGenConfigUI* ui, bool fileExists) {
    while (true) {
        FF_STRBUF_AUTO_DESTROY frame = ffStrbufCreateA((uint32_t) ui->cols * ((uint32_t) ui->rows + 4));
        renderFrame(ui, &frame);
        ffWriteFDData(FFUnixFD2NativeFD(STDOUT_FILENO), frame.length, frame.chars);
        fflush(stdout);

        char ch = 0;
        FFGenConfigKey key = readKey(&ch);
        int result = handleKey(ui, key, ch, fileExists);
        if (result != -1) {
            return result;
        }
    }
}

static FFLogoType initialLogoType(void) {
    switch (instance.config.logo.type) {
        case FF_LOGO_TYPE_SMALL:
            return FF_LOGO_TYPE_SMALL;
        case FF_LOGO_TYPE_NONE:
            return FF_LOGO_TYPE_NONE;
        default:
            return FF_LOGO_TYPE_AUTO;
    }
}

static bool applyConfig(FFdata* data, const FFlist* items, bool fullConfig) {
    if (data->resultDoc) {
        fputs("Error: duplicated `--gen-config` or `--format json` flags found\n", stderr);
        return false;
    }

    FF_STRBUF_AUTO_DESTROY structure = ffStrbufCreateA(256);
    FF_LIST_FOR_EACH (FFGenConfigItem, item, *items) {
        if (item->status == FF_GEN_CONFIG_ITEM_STATUS_UNSELECTED) {
            continue;
        }
        ffStrbufAppendS(&structure, item->baseInfo->name);
        ffStrbufAppendC(&structure, ':');
    }
    ffStrbufTrimRight(&structure, ':');
    ffStrbufSet(&data->structure, &structure);

    data->docType = fullConfig ? FF_RESULT_DOC_TYPE_CONFIG_FULL : FF_RESULT_DOC_TYPE_CONFIG;
    data->resultDoc = yyjson_mut_doc_new(nullptr);
    return true;
}

bool ffGenConfigInteractive(FFdata* data) {
    FF_LIST_AUTO_DESTROY modules;
    collectModuleInfos(&modules);

    FFGenConfigUI ui = {
        .items = ffListCreate(),
        .cursor = 0,
        .viewOffset = 0,
        .logoType = initialLogoType(),
        .fullConfig = false,
        .confirmingOverwrite = false,
        .rows = 24,
        .cols = 80,
    };
    initItems(&ui.items, &modules);

    getTerminalSize(&ui.rows, &ui.cols);
    enterRawMode();
    ffWriteFDData(FFUnixFD2NativeFD(STDOUT_FILENO), strlen("\e[?1049h\e[?25l"), "\e[?1049h\e[?25l");

    const bool fileExists = ffPathExists(data->genConfigPath.chars, FF_PATHTYPE_ANY);
    int result = runCui(&ui, fileExists);

    restoreRawMode();
    ffWriteFDData(FFUnixFD2NativeFD(STDOUT_FILENO), strlen("\e[?1049l\e[?25h\e[m"), "\e[?1049l\e[?25h\e[m");

    bool success = false;
    if (result == 1) {
        if (applyConfig(data, &ui.items, ui.fullConfig)) {
            instance.config.logo.type = ui.logoType;
            success = true;
        }
    } else {
        fputs("\nConfig generation cancelled.\n", stderr);
    }

    ffListDestroy(&ui.items);
    return success;
}
