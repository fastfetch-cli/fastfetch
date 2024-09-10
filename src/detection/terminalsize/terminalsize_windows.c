#include "terminalsize.h"
#include "common/io/io.h"

#include <windows.h>

bool ffDetectTerminalSize(FFTerminalSizeResult* result)
{
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    FF_AUTO_CLOSE_FD HANDLE hConout = INVALID_HANDLE_VALUE;
    {
        DWORD outputMode;
        if (!GetConsoleMode(hOutput, &outputMode))
        {
            hConout = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, NULL);
            hOutput = hConout;
        }
    }
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(hOutput, &csbi))
        {
            result->columns = (uint16_t) (csbi.srWindow.Right - csbi.srWindow.Left + 1);
            result->rows = (uint16_t) (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
        }
        else
        {
            // Windows Terminal doesn't report `\e` for some reason
            ffGetTerminalResponse("\e[18t", 2, "%*[^;];%hu;%hut", &result->rows, &result->columns);
        }
    }

    if (result->columns == 0 && result->rows == 0)
        return false;

    {
        CONSOLE_FONT_INFO cfi;
        if(GetCurrentConsoleFont(hOutput, FALSE, &cfi)) // Only works for ConHost
        {
            result->width = result->columns * (uint16_t) cfi.dwFontSize.X;
            result->height = result->rows * (uint16_t) cfi.dwFontSize.Y;
        }
        if (result->width == 0 || result->height == 0)
        {
            // Windows Terminal doesn't report `\e` for some reason
            ffGetTerminalResponse("\e[14t", 2, "%*[^;];%hu;%hut", &result->height, &result->width);
        }
    }

    return result->columns > 0 && result->rows > 0;
}
