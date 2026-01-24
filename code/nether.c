
// NOTE(vak): Storage

local wire Wires[65536] = {0};
local nand Gates[65536] = {0};

local u32 WireCount = 0;
local u32 GateCount = 0;

// NOTE(vak): State

local void ResetCircuit(void)
{
    WireCount = 0;
    GateCount = 0;
}

local wire_id AddWire(void)
{
    Assert(WireCount < ArrayCount(Wires));

    wire_id Result = WireCount++;
    return (Result);
}

local void NAND(wire_id A, wire_id B, wire_id Out)
{
    Assert(GateCount < ArrayCount(Gates));

    nand* Gate = Gates + GateCount++;

    Gate->A   = A;
    Gate->B   = B;
    Gate->Out = Out;
}

local void Simulate(void)
{
    for (u32 GateIndex = 0; GateIndex < GateCount; GateIndex++)
    {
        nand* Gate = Gates + GateIndex;

        wire_id A   = Gate->A;
        wire_id B   = Gate->B;
        wire_id Out = Gate->Out;

        Wires[Out] = !(Wires[A] && Wires[B]);
    }
}

// NOTE(vak): Logic gates

local void NOT(wire_id In, wire_id Out)
{
    NAND(In, In, Out);
}

local void AND(wire_id A, wire_id B, wire_id Out)
{
    wire_id NotOut = AddWire();

    NAND(A, B, NotOut);
    NOT (NotOut, Out);
}

local void OR(wire_id A, wire_id B, wire_id Out)
{
    wire_id NotA = AddWire();
    wire_id NotB = AddWire();

    NOT (A, NotA);
    NOT (B, NotB);

    NAND(NotA, NotB, Out);
}

local void XOR(wire_id A, wire_id B, wire_id Out)
{
    wire_id C  = AddWire();
    wire_id D = AddWire();

    OR  (A, B, C);
    NAND(A, B, D);

    AND (C, D, Out);
}

// NOTE(vak): Tests

local void TestUnary(string Name, wire_id In, wire_id Out)
{
    // NOTE(vak): Header

    Print(Str("Truth table "));

    Print(Str("("));
    Print(Name);
    Print(Str(")"));

    Print(Str(":"));
    PrintNewLine();

    Println(Str("  In  |  Out  "));

    // NOTE(vak): Test

    for (u8 State = 0; State <= 1; State++)
    {
        Wires[In] = State;

        Simulate();

        Print(Str("  "));
        Print(Wires[In] ? Str("1") : Str("0"));
        Print(Str("   "));

        Print(Str("|"));

        Print(Str("  "));
        Print(Wires[Out] ? Str("1") : Str("0"));
        Print(Str("  "));

        PrintNewLine();
    }
}

local void TestBinary(string Name, wire_id A, wire_id B, wire_id Out)
{
    // NOTE(vak): Header

    Print(Str("Truth table "));

    Print(Str("("));
    Print(Name);
    Print(Str(")"));

    Print(Str(":"));
    PrintNewLine();

    Println(Str("  A  |  B  |  Out  "));

    // NOTE(vak): Test

    for (u8 State = 0; State <= 3; State++)
    {
        Wires[A] = (State >> 1) & 1;
        Wires[B] = (State >> 0) & 1;

        Simulate();

        Print(Str("  "));
        Print(Wires[A] ? Str("1") : Str("0"));
        Print(Str("  "));

        Print(Str("|"));

        Print(Str("  "));
        Print(Wires[B] ? Str("1") : Str("0"));
        Print(Str("  "));

        Print(Str("|"));

        Print(Str("  "));
        Print(Wires[Out] ? Str("1") : Str("0"));
        Print(Str("  "));

        PrintNewLine();
    }
}

local s32 Main(void)
{
    // NOTE(vak): NAND
    {
        ResetCircuit();

        wire_id A   = AddWire();
        wire_id B   = AddWire();
        wire_id Out = AddWire();

        NAND(A, B, Out);

        TestBinary(Str("NAND"), A, B, Out);
    }

    PrintNewLine();

    // NOTE(vak): NOT
    {
        ResetCircuit();

        wire_id In  = AddWire();
        wire_id Out = AddWire();

        NOT(In, Out);

        TestUnary(Str("NOT"), In, Out);
    }

    PrintNewLine();

    // NOTE(vak): AND
    {
        ResetCircuit();

        wire_id A   = AddWire();
        wire_id B   = AddWire();
        wire_id Out = AddWire();

        AND(A, B, Out);

        TestBinary(Str("AND"), A, B, Out);
    }

    PrintNewLine();

    // NOTE(vak): OR
    {
        ResetCircuit();

        wire_id A   = AddWire();
        wire_id B   = AddWire();
        wire_id Out = AddWire();

        OR(A, B, Out);

        TestBinary(Str("OR"), A, B, Out);
    }

    PrintNewLine();

    // NOTE(vak): XOR
    {
        ResetCircuit();

        wire_id A   = AddWire();
        wire_id B   = AddWire();
        wire_id Out = AddWire();

        XOR(A, B, Out);

        TestBinary(Str("XOR"), A, B, Out);
    }

    PrintNewLine();

    return (0);
}
