
local s32 Main(void)
{
    u32 TestCount = 0;

    // NOTE(vak): Basic
    {
        TestBUF();
        TestTriState();
        TestLogicGates();
    }

    PrintNewLine();

    // NOTE(vak): Multiplexers
    {
        TestMux();
        TestDemux();
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

    // NOTE(vak): Memory
    {
        TestDLatch();
        TestDFlipFlop();
    }

    PrintNewLine();

    // NOTE(vak): Central components
    {
        TestRegister();
        TestALU();
    }

    return (0);
}
