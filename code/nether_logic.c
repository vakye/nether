
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

local void ResetGates(void)
{
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

local void AddWires(wire_id* IDs, u32 Count)
{
    Assert(WireCount + Count <= ArrayCount(Wires));

    for (u32 Index = 0; Index < Count; Index++)
    {
        IDs[Index] = WireCount + Index;
    }

    WireCount += Count;
}

local u64 GetWires(wire_id* IDs, u32 Count)
{
    Assert(Count <= 64);

    u64 Result = 0;

    for (u32 Index = 0; Index < Count; Index++)
        Result |= ((u64)GetWire(IDs[Index])) << Index;

    return (Result);
}

local void SetWires(wire_id* IDs, u32 Count, u64 Bits)
{
    Assert(Count <= 64);

    for (u32 Index = 0; Index < Count; Index++)
        SetWire(IDs[Index], (Bits >> Index) & 1);
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

// NOTE(vak): Adder

local void HalfAdder1(wire_id A, wire_id B, wire_id Sum, wire_id Carry)
{
    XOR(A, B, Sum);
    AND(A, B, Carry);
}

local void FullAdder1(wire_id A, wire_id B, wire_id C, wire_id Sum, wire_id Carry)
{
    // NOTE(vak): Sum
    {
        wire_id SumAB = AddWire();

        XOR(A, B, SumAB);
        XOR(C, SumAB, Sum);
    }

    // NOTE(vak): Carry
    {
        wire_id AndAB = AddWire();
        wire_id AndAC = AddWire();
        wire_id AndBC = AddWire();

        AND(A, B, AndAB);
        AND(A, C, AndAC);
        AND(B, C, AndBC);

        wire_id D = AddWire();

        OR(AndAB, AndAC, D);
        OR(D, AndBC, Carry);
    }
}

local void HalfAdder(u32 BitCount, wire_id* A, wire_id* B, wire_id* Sum, wire_id Carry)
{
    Assert(BitCount >= 1);

    wire_id NextCarry = (BitCount == 1) ? (Carry) : (AddWire());

    HalfAdder1(A[0], B[0], Sum[0], NextCarry);

    for (u32 BitIndex = 1; BitIndex < BitCount; BitIndex++)
    {
        wire_id LastCarry = NextCarry;

        NextCarry = ((BitIndex + 1) == BitCount) ? (Carry) : (AddWire());

        FullAdder1(
            A[BitIndex],
            B[BitIndex],
            LastCarry,
            Sum[BitIndex],
            NextCarry
        );
    }
}

local void FullAdder(u32 BitCount, wire_id* A, wire_id* B, wire_id C, wire_id* Sum, wire_id Carry)
{
    Assert(BitCount >= 1);

    wire_id LastCarry = C;
    wire_id NextCarry = (BitCount == 1) ? (Carry) : (AddWire());

    FullAdder1(A[0], B[0], LastCarry, Sum[0], NextCarry);

    for (u32 BitIndex = 1; BitIndex < BitCount; BitIndex++)
    {
        LastCarry = NextCarry;
        NextCarry = ((BitIndex + 1) == BitCount) ? (Carry) : (AddWire());

        FullAdder1(
            A[BitIndex],
            B[BitIndex],
            LastCarry,
            Sum[BitIndex],
            NextCarry
        );
    }
}

// NOTE(vak): Tests

local void OutputTestResult(string Name, b32 Successful)
{
    usize SoFar = 0;

    SoFar += Print(Str("["));
    SoFar += Print(Name);
    SoFar += Print(Str("]"));
    SoFar += Print(Str(":"));

    if (SoFar < TestResultPrintPadding)
        PrintRepeat(Str(" "), TestResultPrintPadding - SoFar);

    Println(Successful ? Str("[SUCCESS]") : Str("[FAILED]"));
}

local b32 VerifyTruthTable(
    wire* TruthTable, u32 RowCount,
    wire_id* Inputs, u32 InputCount,
    wire_id* Outputs, u32 OutputCount
)
{
    u32 ColumnCount = InputCount + OutputCount;

    for (u32 RowIndex = 0; RowIndex < RowCount; RowIndex++)
    {
        wire* TestTable = TruthTable + (RowIndex * ColumnCount);

        wire* InputValues = TestTable;
        wire* OutputValues = TestTable + InputCount;

        for (u32 Index = 0; Index < InputCount; Index++)
        {
            SetWire(Inputs[Index], InputValues[Index]);
        }

        SimulateCircuit();

        for (u32 Index = 0; Index < OutputCount; Index++)
        {
            if (GetWire(Outputs[Index]) != OutputValues[Index])
            {
                goto Failed;
                break;
            }
        }
    }

    return (true);

Failed:
    return (false);
}

local void TestLogicGates(void)
{
    persist wire TruthNAND[3 * 4] =
    {
        0, 0,    1,
        0, 1,    1,
        1, 0,    1,
        1, 1,    0,
    };

    persist wire TruthAND[3 * 4] =
    {
        0, 0,    0,
        0, 1,    0,
        1, 0,    0,
        1, 1,    1,
    };

    persist wire TruthOR[3 * 4] =
    {
        0, 0,    0,
        0, 1,    1,
        1, 0,    1,
        1, 1,    1,
    };

    persist wire TruthNOR[3 * 4] =
    {
        0, 0,    1,
        0, 1,    0,
        1, 0,    0,
        1, 1,    0,
    };

    persist wire TruthXOR[3 * 4] =
    {
        0, 0,    0,
        0, 1,    1,
        1, 0,    1,
        1, 1,    0,
    };

    persist wire TruthNOT[2 * 2] =
    {
        0, 1,
        1, 0,
    };

    // NOTE(vak): Binary logic gates
    {
        ResetCircuit();

        wire_id Inputs [2] = {0};
        wire_id Outputs[1] = {0};

        AddWires(Inputs,  ArrayCount(Inputs));
        AddWires(Outputs, ArrayCount(Outputs));

        #define DoBinaryTest(Name) \
            ResetGates(); \
            Name(Inputs[0], Inputs[1], Outputs[0]); \
            \
            OutputTestResult(Str(#Name), VerifyTruthTable(Truth##Name, 4, Inputs, 2, Outputs, 1));

        DoBinaryTest(NAND)
        DoBinaryTest(AND)
        DoBinaryTest(OR)
        DoBinaryTest(NOR)
        DoBinaryTest(XOR)
    }

    // NOTE(vak): Unary logic gates
    {
        ResetCircuit();

        wire_id Inputs[1]  = {AddWire()};
        wire_id Outputs[1] = {AddWire()};

        NOT(Inputs[0], Outputs[0]);

        OutputTestResult(Str("NOT"), VerifyTruthTable(TruthNOT, 2, Inputs, 1, Outputs, 1));
    }
}

local void TestHalfAdder1(void)
{
    ResetCircuit();

    persist wire TruthHalfAdder[4 * 4] =
    {
        0, 0,    0, 0,
        0, 1,    1, 0,
        1, 0,    1, 0,
        1, 1,    0, 1,
    };

    wire_id A     = AddWire();
    wire_id B     = AddWire();
    wire_id Sum   = AddWire();
    wire_id Carry = AddWire();

    wire_id Inputs[2]  = {A, B};
    wire_id Outputs[2] = {Sum, Carry};

    HalfAdder1(A, B, Sum, Carry);

    OutputTestResult(Str("HalfAdder1"), VerifyTruthTable(TruthHalfAdder, 4, Inputs, 2, Outputs, 2));
}

local void TestFullAdder1(void)
{
    ResetCircuit();

    persist wire TruthHalfAdder[5 * 8] =
    {
        0, 0, 0,    0, 0,
        0, 0, 1,    1, 0,
        0, 1, 0,    1, 0,
        0, 1, 1,    0, 1,
        1, 0, 0,    1, 0,
        1, 0, 1,    0, 1,
        1, 1, 0,    0, 1,
        1, 1, 1,    1, 1,
    };

    wire_id A     = AddWire();
    wire_id B     = AddWire();
    wire_id C     = AddWire();
    wire_id Sum   = AddWire();
    wire_id Carry = AddWire();

    wire_id Inputs[3]  = {A, B, C};
    wire_id Outputs[2] = {Sum, Carry};

    FullAdder1(A, B, C, Sum, Carry);

    OutputTestResult(Str("FullAdder1"), VerifyTruthTable(TruthHalfAdder, 8, Inputs, 3, Outputs, 2));
}
