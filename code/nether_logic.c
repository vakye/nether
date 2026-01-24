
typedef struct
{
    wire_id A;
    wire_id B;

    wire_id Out;
} nand;

// NOTE(vak): Storage

local wire Wires[65536] = {0};
local nand Gates[65536] = {0};

local u32 WireCount = 0;
local u32 GateCount = 0;

// NOTE(vak): Circuit

local void ResetCircuit(void)
{
    WireCount = 0;
    GateCount = 0;
}

local void SimulateCircuit(void)
{
    for (u32 GateIndex = 0; GateIndex < GateCount; GateIndex++)
    {
        nand* Gate = Gates + GateIndex;

        wire_id A   = Gate->A;
        wire_id B   = Gate->B;
        wire_id Out = Gate->Out;

        SetWire(Out, !(GetWire(A) && GetWire(B)));
    }
}

// NOTE(vak): Wires

local wire_id AddWire(void)
{
    Assert(WireCount < ArrayCount(Wires));

    wire_id Result = WireCount++;
    return (Result);
}

local wire GetWire(wire_id ID)
{
    Assert(ID < WireCount);

    wire Result = (Wires[ID] & 1);
    return (Result);
}

local void SetWire(wire_id ID, wire Bit)
{
    Assert(ID < WireCount);

    Wires[ID] = (Bit & 1);
}

// NOTE(vak): Logic gates

local void NAND(wire_id A, wire_id B, wire_id Out)
{
    Assert(GateCount < ArrayCount(Gates));

    nand* Gate = Gates + GateCount++;

    Gate->A   = A;
    Gate->B   = B;
    Gate->Out = Out;
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

local void NOR(wire_id A, wire_id B, wire_id Out)
{
    wire_id NotOut = AddWire();
    wire_id NotB = AddWire();

    OR (A, B, NotOut);
    NOT(NotOut, Out);
}

local void XOR(wire_id A, wire_id B, wire_id Out)
{
    wire_id C  = AddWire();
    wire_id D = AddWire();

    OR  (A, B, C);
    NAND(A, B, D);

    AND (C, D, Out);
}

local void NOT(wire_id In, wire_id Out)
{
    NAND(In, In, Out);
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
        SetWire(In, State);

        SimulateCircuit();

        Print(Str("  "));
        Print(GetWire(In) ? Str("1") : Str("0"));
        Print(Str("   "));

        Print(Str("|"));

        Print(Str("  "));
        Print(GetWire(Out) ? Str("1") : Str("0"));
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
        SetWire(A, (State >> 1) & 1);
        SetWire(B, (State >> 0) & 1);

        SimulateCircuit();

        Print(Str("  "));
        Print(GetWire(A) ? Str("1") : Str("0"));
        Print(Str("  "));

        Print(Str("|"));

        Print(Str("  "));
        Print(GetWire(B) ? Str("1") : Str("0"));
        Print(Str("  "));

        Print(Str("|"));

        Print(Str("  "));
        Print(GetWire(Out) ? Str("1") : Str("0"));
        Print(Str("  "));

        PrintNewLine();
    }
}
