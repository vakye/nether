
local usize Print(string Message)
{
    usize Result = 0;

    HANDLE StdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if ((StdOut != 0) && (StdOut != INVALID_HANDLE_VALUE))
    {
        usize BytesRemaining = Message.Size;
        char* BytesFrom      = Message.Data;

        while (BytesRemaining > 0)
        {
            DWORD BytesToWrite = (DWORD)Minimum(BytesRemaining, U32Max);
            DWORD BytesWritten = 0;

            WriteFile(StdOut, BytesFrom, BytesToWrite, &BytesWritten, 0);

            Assert(BytesWritten == BytesToWrite);

            BytesRemaining -= BytesWritten;
            BytesFrom      += BytesWritten;
        }

        Result = Message.Size;
    }

    return (Result);
}

local usize Println(string Message)
{
    usize Result = 0;

    Result += Print(Message);
    Result += Print(Str("\n"));

    return (Result);
}

local usize PrintNewLine(void)
{
    usize Result = Print(Str("\n"));
    return (Result);
}


local usize PrintRepeat(string Message, usize RepeatCount)
{
    usize Result = 0;

    while (RepeatCount--)
        Result += Print(Message);

    return (Result);
}
