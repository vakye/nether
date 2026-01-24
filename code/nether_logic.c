
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

        wire_id A = AddWire();
        wire_id B = AddWire();
        wire_id C = AddWire();

        wire_id Inputs [2] = {A, B};
        wire_id Outputs[1] = {C};

        #define DoBinaryTest(Name) \
            { \
                usize SoFar = Print(Str("Testing " #Name "... ")); \
                \
                if (SoFar < TestResultPrintPadding) \
                    PrintRepeat(Str(" "), TestResultPrintPadding - SoFar); \
                \
                ResetGates(); \
                Name(A, B, C); \
                \
                if (VerifyTruthTable(Truth##Name, 4, Inputs, 2, Outputs, 1)) \
                { \
                    Println(Str("SUCCESS")); \
                } \
                else \
                { \
                    Println(Str("FAILED")); \
                } \
            }

        DoBinaryTest(NAND)
        DoBinaryTest(AND)
        DoBinaryTest(OR)
        DoBinaryTest(NOR)
        DoBinaryTest(XOR)
    }

    // NOTE(vak): Unary logic gates
    {
        ResetCircuit();

        wire_id A = AddWire();
        wire_id B = AddWire();

        wire_id Inputs[1]  = {A};
        wire_id Outputs[1] = {B};

        usize SoFar = Print(Str("Testing NOT... "));

        if (SoFar < TestResultPrintPadding)
            PrintRepeat(Str(" "), TestResultPrintPadding - SoFar);

        NOT(A, B);

        if (VerifyTruthTable(TruthNOT, 2, Inputs, 1, Outputs, 1))
        {
            Println(Str("SUCCESS"));
        }
        else
        {
            Println(Str("FAILED"));
        }
    }
}

local void TestHalfAdder1(void)
{
    usize SoFar = Print(Str("Testing HalfAdder1... "));

    if (SoFar < TestResultPrintPadding)
        PrintRepeat(Str(" "), TestResultPrintPadding - SoFar);

    ResetCircuit();

    wire_id A     = AddWire();
    wire_id B     = AddWire();
    wire_id Sum   = AddWire();
    wire_id Carry = AddWire();

    HalfAdder1(A, B, Sum, Carry);

    b32 Successful = true;

    for (u32 State = 0; (State <= 3) & (Successful); State++)
    {
        SetWire(A, (State >> 1) & 1);
        SetWire(B, (State >> 0) & 1);

        SimulateCircuit();

        u32 Desired = GetWire(A) + GetWire(B);

        wire DesiredSum   = (wire)((Desired >> 0) & 1);
        wire DesiredCarry = (wire)((Desired >> 1) & 1);

        Successful &= (GetWire(Sum)   == DesiredSum);
        Successful &= (GetWire(Carry) == DesiredCarry);
    }

    Println(Successful ? Str("SUCCESS") : Str("FAILED"));
}

local void TestFullAdder1(void)
{
    usize SoFar = Print(Str("Testing FullAdder1... "));

    if (SoFar < TestResultPrintPadding)
        PrintRepeat(Str(" "), TestResultPrintPadding - SoFar);

    ResetCircuit();

    wire_id A     = AddWire();
    wire_id B     = AddWire();
    wire_id C     = AddWire();
    wire_id Sum   = AddWire();
    wire_id Carry = AddWire();

    FullAdder1(A, B, C, Sum, Carry);

    b32 Successful = true;

    for (u32 State = 0; (State <= 7) & (Successful); State++)
    {
        SetWire(A, (State >> 2) & 1);
        SetWire(B, (State >> 1) & 1);
        SetWire(C, (State >> 0) & 1);

        SimulateCircuit();

        u32 Desired = GetWire(A) + GetWire(B) + GetWire(C);

        wire DesiredSum   = (wire)((Desired >> 0) & 1);
        wire DesiredCarry = (wire)((Desired >> 1) & 1);

        Successful &= (GetWire(Sum)   == DesiredSum);
        Successful &= (GetWire(Carry) == DesiredCarry);
    }

    Println(Successful ? Str("SUCCESS") : Str("FAILED"));
}

local void TestHalfAdder(void)
{
    usize SoFar = Print(Str("Testing HalfAdder... "));

    if (SoFar < TestResultPrintPadding)
        PrintRepeat(Str(" "), TestResultPrintPadding - SoFar);

    ResetCircuit();

    wire_id Sum[8]               = {0};
    wire_id A  [ArrayCount(Sum)] = {0};
    wire_id B  [ArrayCount(Sum)] = {0};

    u32 BitCount = ArrayCount(Sum);

    AddWires(A,   BitCount);
    AddWires(B,   BitCount);
    AddWires(Sum, BitCount);

    wire_id Carry = AddWire();

    HalfAdder(BitCount, A, B, Sum, Carry);

    b32 Successful = true;

    u64 SumMask  = (1ull << BitCount) - 1;
    u64 MaxState = (1ull << BitCount*2) - 1;

    for (u32 State = 0; (State <= MaxState) & (Successful); State++)
    {
        SetWires(A, BitCount, SumMask & (State >> BitCount*0));
        SetWires(B, BitCount, SumMask & (State >> BitCount*1));

        SimulateCircuit();

        u64 Desired      = GetWires(A, BitCount) + GetWires(B, BitCount);
        u64 DesiredSum   = Desired  & SumMask;
        u64 DesiredCarry = (Desired >> BitCount) & 1;

        Successful &= (GetWires(Sum, BitCount) == DesiredSum);
        Successful &= (GetWire(Carry) == DesiredCarry);
    }

    Println(Successful ? Str("SUCCESS") : Str("FAILED"));
}

local void TestFullAdder(void)
{
    usize SoFar = Print(Str("Testing FullAdder... "));

    if (SoFar < TestResultPrintPadding)
        PrintRepeat(Str(" "), TestResultPrintPadding - SoFar);

    ResetCircuit();

    wire_id Sum[8]               = {0};
    wire_id A  [ArrayCount(Sum)] = {0};
    wire_id B  [ArrayCount(Sum)] = {0};

    u32 BitCount = ArrayCount(Sum);

    AddWires(A,   BitCount);
    AddWires(B,   BitCount);
    AddWires(Sum, BitCount);

    wire_id C     = AddWire();
    wire_id Carry = AddWire();

    FullAdder(BitCount, A, B, C, Sum, Carry);

    b32 Successful = true;

    u64 SumMask  = (1ull << BitCount) - 1;
    u64 MaxState = (1ull << BitCount*2);

    for (u32 State = 0; (State <= MaxState) & (Successful); State++)
    {
        SetWires(A, BitCount, SumMask & (State >> BitCount*0));
        SetWires(B, BitCount, SumMask & (State >> BitCount*1));

        SetWire(C, (State >> BitCount*2) & 1);

        SimulateCircuit();

        u64 Desired      = GetWires(A, BitCount) + GetWires(B, BitCount) + GetWire(C);
        u64 DesiredSum   = Desired  & SumMask;
        u64 DesiredCarry = (Desired >> BitCount) & 1;

        Successful &= (GetWires(Sum, BitCount) == DesiredSum);
        Successful &= (GetWire(Carry) == DesiredCarry);
    }

    Println(Successful ? Str("SUCCESS") : Str("FAILED"));
}
