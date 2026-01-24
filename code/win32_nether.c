
#include "nether_shared.h"
#include "nether_shared.c"

#include "nether_platform.h"

#include "win32_nether.h"
#include "win32_platform.c"

local void Win32SetupConsole(void)
{
    HANDLE StdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if ((StdOut == 0) || (StdOut == INVALID_HANDLE_VALUE))
    {
        if (!AttachConsole(ATTACH_PARENT_PROCESS))
        {
            AllocConsole();
        }
    }
}

void WinMainCRTStartup()
{
    Win32SetupConsole();

    Println(Str("Hello, world!"));

    ExitProcess(0);
}
