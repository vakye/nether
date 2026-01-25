
local s32 Main(void)
{
    // NOTE(vak): Tri-state
    {
        TestTriState();
    }

    PrintNewLine();

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
