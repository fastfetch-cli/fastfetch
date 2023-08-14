#include "terminalsize.h"
#include "common/io/io.h"

#include <windows.h>

bool ffDetectTerminalSize(FFTerminalSizeResult* result)
{
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        {
            result->columns = (uint16_t) (csbi.srWindow.Right - csbi.srWindow.Left + 1);
            result->rows = (uint16_t) (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
        }
        else
        {
            ffGetTerminalResponse("\033[18t", "\033[8;%hu;%hut", &result->rows, &result->columns);
        }
    }

    if (result->columns == 0 && result->rows == 0)
        return false;

    {
        CONSOLE_FONT_INFO cfi;
        if(GetCurrentConsoleFont(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi)) // Only works for ConHost
        {
            result->width = result->columns * (uint16_t) cfi.dwFontSize.X;
            result->height = result->rows * (uint16_t) cfi.dwFontSize.Y;
        }
        else
        {
            // Pending https://github.com/microsoft/terminal/issues/8581
            // if (result->width == 0 && result->height == 0)
            //     ffGetTerminalResponse("\033[14t", "\033[4;%hu;%hut", &result->height, &result->width);
            return false;
        }
    }

    return result->columns > 0 && result->rows > 0;
}
