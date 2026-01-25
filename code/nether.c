
local s32 Main(void)
{
    // NOTE(vak): Logic gates
    {
        TestLogicGates();
    }

    PrintNewLine();

    // NOTE(vak): Adders
    {
        TestHalfAdder1();
        TestFullAdder1();
        TestHalfAdder();
        TestFullAdder();
    }

    PrintNewLine();

    // NOTE(vak): Latches and flip flops
    {
        TestDLatch();
        TestDFlipFlop();
    }

    return (0);
}
