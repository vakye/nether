
typedef enum
{
    GateKind_Unknown = 0,

    GateKind_NAND,
    GateKind_TriState,
} gate_kind;

typedef struct
{
    gate_kind Kind;

    wire_id A;
    wire_id B;
    wire_id Out;
} gate;

// NOTE(vak): Storage

local wire Wires[65536] = {0};
local gate Gates[65536] = {0};

local u32 WireCount = 0;
local u32 GateCount = 0;

// NOTE(vak): Circuit

local void RandomizeWireState(void)
{
    u32 State = GetWallClock() & 0xFFFFFFFF;

    for (u32 Index = 0; Index < WireCount; Index++)
    {
        State ^= (State << 13);
        State ^= (State >> 17);
        State ^= (State << 5);

        wire Bit = (State & 1);

        Wires[Index] = Bit;
    }
}

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
        gate* Gate = Gates + GateIndex;

        switch (Gate->Kind)
        {
            InvalidDefaultCase;

            case GateKind_NAND:
            {
                wire_id A   = Gate->A;
                wire_id B   = Gate->B;
                wire_id Out = Gate->Out;

                Wires[Out] = !(Wires[A] & Wires[B]);
            } break;

            case GateKind_TriState:
            {
                wire_id Input  = Gate->A;
                wire_id Enable = Gate->B;
                wire_id Output = Gate->Out;

                if (Wires[Enable])
                    Wires[Output] = Wires[Input];
            } break;
        }
    }
}

local void SimulateClockPulse(wire_id Clock, u32 PulseTime)
{
    SetWire(Clock, !GetWire(Clock));

    for (u32 Time = 0; Time < PulseTime; Time++)
        SimulateCircuit();
}

local void SimulateClockCycle(wire_id Clock, u32 PulseTime)
{
    SimulateClockPulse(Clock, PulseTime);
    SimulateClockPulse(Clock, PulseTime);
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

local b32 ExpectWire(wire_id ID, wire ExpectedBit)
{
    b32 Result = GetWire(ID) == ExpectedBit;
    return (Result);
}

local void RandomWire(wire_id ID)
{
    u32 State = GetWallClock() & 0xFFFFFFFF;

    State ^= (State << 13);
    State ^= (State >> 17);
    State ^= (State << 5);

    SetWire(ID, State & 1);
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

local b32 ExpectWires(wire_id* IDs, u32 Count, u64 ExpectedBits)
{
    u64 Mask = (1ull << Count) - 1;
    b32 Result = GetWires(IDs, Count) == (ExpectedBits & Mask);

    return (Result);
}

local void RandomWires(wire_id* IDs, u32 Count)
{
    u32 State = GetWallClock() & 0xFFFFFFFF;

    for (u32 Index = 0; Index < Count; Index++)
    {
        State ^= (State << 13);
        State ^= (State >> 17);
        State ^= (State << 5);

        SetWire(IDs[Index], State & 1);
    }
}

// NOTE(vak): Tri-state

local void TriState(wire_id Input, wire_id Enable, wire_id Output)
{
    Assert(Input  < WireCount);
    Assert(Enable < WireCount);
    Assert(Output < WireCount);

    Assert(GateCount < ArrayCount(Gates));

    gate* Gate = Gates + GateCount++;

    Gate->Kind = GateKind_TriState;
    Gate->A    = Input;
    Gate->B    = Enable;
    Gate->Out  = Output;
}

// NOTE(vak): Logic gates

local void NAND(wire_id A, wire_id B, wire_id Out)
{
    Assert(A < WireCount);
    Assert(B < WireCount);
    Assert(Out < WireCount);

    Assert(GateCount < ArrayCount(Gates));

    gate* Gate = Gates + GateCount++;

    Gate->Kind = GateKind_NAND;
    Gate->A    = A;
    Gate->B    = B;
    Gate->Out  = Out;
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

// NOTE(vak): Latches

local void DLatch(wire_id Data, wire_id Enable, wire_id Out, wire_id NotOut)
{
    wire_id NotData = AddWire();

    wire_id A = AddWire();
    wire_id B = AddWire();

    NOT (Data, NotData);

    NAND(   Data, Enable, A);
    NAND(NotData, Enable, B);

    NAND(A, NotOut, Out);
    NAND(B, Out, NotOut);
}

local void DFlipFlop(wire_id Data, wire_id Clock, wire_id Out, wire_id NotOut)
{
    wire_id NotClock = AddWire();

    wire_id A    = AddWire();
    wire_id NotA = AddWire();

    NOT(Clock, NotClock);

    DLatch(Data, Clock, A, NotA);
    DLatch(A, NotClock, Out, NotOut);
}

// NOTE(vak): Memory units

local void Register(u32 BitCount, wire_id* Data, wire_id WriteEnable, wire_id Clock, wire_id* Out)
{
    Assert(BitCount >= 1);

    wire_id W    = WriteEnable;
    wire_id NotW = AddWire();

    NOT(W, NotW);

    for (u32 BitIndex = 0; BitIndex < BitCount; BitIndex++)
    {
        wire_id D = AddWire();
        wire_id NotOut = AddWire();

        TriState(Data[BitIndex], W,    D);
        TriState(Out[BitIndex],  NotW, D);

        DFlipFlop(D, Clock, Out[BitIndex], NotOut);
    }
}

// NOTE(vak): Tests

local b32 VerifyTruthTable(
    wire* TruthTable, u32 RowCount,
    wire_id* Inputs, u32 InputCount,
    wire_id* Outputs, u32 OutputCount
)
{
    RandomizeWireState();

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
            if (!ExpectWire(Outputs[Index], OutputValues[Index]))
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

local void TestTriState(void)
{
    b32 Successful = true;

    ResetCircuit();

    wire_id In  = AddWire();
    wire_id En  = AddWire();
    wire_id Out = AddWire();

    TriState(In, En, Out);

    RandomizeWireState();
    SimulateCircuit();

    SetWire(En, 1);

    SetWire(In, 0);
    SimulateCircuit();

    Successful &= ExpectWire(Out, 0);

    SetWire(In, 1);
    SimulateCircuit();

    Successful &= ExpectWire(Out, 1);

    OutputTestResult(Str("TriState"), Successful);
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

    persist wire TruthFullAdder[5 * 8] =
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

    OutputTestResult(Str("FullAdder1"), VerifyTruthTable(TruthFullAdder, 8, Inputs, 3, Outputs, 2));
}

local void TestHalfAdder(void)
{
    b32 Successful = true;

    ResetCircuit();

    u32 BitCounts[] = {1, 3, 6, 8, 14, 16, 20, 27, 32};

    wire_id A[32]   = {0};
    wire_id B[32]   = {0};
    wire_id Sum[32] = {0};

    for (u32 Index = 0; Index < ArrayCount(BitCounts); Index++)
    {
        u32 BitCount = BitCounts[Index];

        ResetCircuit();

        AddWires(A,   BitCount);
        AddWires(B,   BitCount);
        AddWires(Sum, BitCount);

        wire_id Carry = AddWire();

        HalfAdder(BitCount, A, B, Sum, Carry);

        u64 Mask = (1ull << BitCount) - 1;

        for (u32 TestIndex = 0; TestIndex < 1024; TestIndex++)
        {
            RandomizeWireState();
            SimulateCircuit();

            u64 Computed = GetWires(A, BitCount) + GetWires(B, BitCount);

            u64  ExpectedSum = Computed & Mask;
            wire ExpectedCarry = (Computed >> BitCount) & 1;

            Successful &= ExpectWires(Sum, BitCount, ExpectedSum);
            Successful &= ExpectWire (Carry, ExpectedCarry);
        }
    }

    OutputTestResult(Str("HalfAdder"), Successful);
}

local void TestFullAdder(void)
{
    b32 Successful = true;

    ResetCircuit();

    u32 BitCounts[] = {1, 3, 6, 8, 14, 16, 20, 27, 32};

    wire_id A[32]   = {0};
    wire_id B[32]   = {0};
    wire_id Sum[32] = {0};

    for (u32 Index = 0; Index < ArrayCount(BitCounts); Index++)
    {
        u32 BitCount = BitCounts[Index];

        ResetCircuit();

        AddWires(A,   BitCount);
        AddWires(B,   BitCount);
        AddWires(Sum, BitCount);

        wire_id C     = AddWire();
        wire_id Carry = AddWire();

        FullAdder(BitCount, A, B, C, Sum, Carry);

        u64 Mask = (1ull << BitCount) - 1;

        for (u32 TestIndex = 0; TestIndex < 1024; TestIndex++)
        {
            RandomizeWireState();
            SimulateCircuit();

            u64 Computed = GetWires(A, BitCount) + GetWires(B, BitCount) + GetWire(C);

            u64  ExpectedSum = Computed & Mask;
            wire ExpectedCarry = (Computed >> BitCount) & 1;

            Successful &= ExpectWires(Sum, BitCount, ExpectedSum);
            Successful &= ExpectWire (Carry, ExpectedCarry);
        }
    }

    OutputTestResult(Str("FullAdder"), Successful);
}

local void TestDLatch(void)
{
    ResetCircuit();

    b32 Successful = true;

    wire_id Data   = AddWire();
    wire_id Clock  = AddWire();
    wire_id Out    = AddWire();
    wire_id NotOut = AddWire();

    DLatch(Data, Clock, Out, NotOut);

    u32 PulseTime = 2 + (GetWallClock() & 15);

    RandomizeWireState();
    SimulateClockCycle(Clock, PulseTime);

    SetWire(Clock, 0);

    {
        SetWire(Data, 1);
        SimulateClockPulse(Clock, PulseTime);

        Successful &= ExpectWire(Out, 1);
        Successful &= ExpectWire(NotOut, 0);

        for (u32 Index = 0; Index < 15; Index++)
        {
            SimulateClockPulse(Clock, PulseTime);

            Successful &= ExpectWire(Out, 1);
            Successful &= ExpectWire(NotOut, 0);
        }

        SetWire(Data, 0);
        SimulateClockPulse(Clock, PulseTime);

        Successful &= ExpectWire(Out, 0);
        Successful &= ExpectWire(NotOut, 1);

        for (u32 Index = 0; Index < 15; Index++)
        {
            SimulateClockPulse(Clock, PulseTime);

            Successful &= ExpectWire(Out, 0);
            Successful &= ExpectWire(NotOut, 1);
        }
    }

    OutputTestResult(Str("DLatch"), Successful);
}

local void TestDFlipFlop(void)
{
    ResetCircuit();

    b32 Successful = true;

    wire_id Data   = AddWire();
    wire_id Clock  = AddWire();
    wire_id Out    = AddWire();
    wire_id NotOut = AddWire();

    DFlipFlop(Data, Clock, Out, NotOut);

    u32 PulseTime = 2 + (GetWallClock() & 15);

    RandomizeWireState();
    SimulateClockCycle(Clock, PulseTime);

    SetWire(Clock, 0);

    {
        SetWire(Data, 1);
        SimulateClockCycle(Clock, PulseTime);

        Successful &= ExpectWire(Out, 1);
        Successful &= ExpectWire(NotOut, 0);

        SetWire(Data, 0);
        SimulateClockPulse(Clock, PulseTime);

        Successful &= ExpectWire(Out, 1);
        Successful &= ExpectWire(NotOut, 0);

        SetWire(Data, 1);
        SimulateClockPulse(Clock, PulseTime);

        Successful &= ExpectWire(Out, 0);
        Successful &= ExpectWire(NotOut, 1);

        for (u32 Index = 0; Index < 15; Index++)
        {
            SimulateClockCycle(Clock, PulseTime);

            Successful &= ExpectWire(Out, 1);
            Successful &= ExpectWire(NotOut, 0);
        }
    }

    OutputTestResult(Str("DFlipFlop"), Successful);
}

local void TestRegister(void)
{
    b32 Successful = true;

    u32 BitCounts[] = {1, 2, 3, 4, 7, 8, 13, 16, 20, 27, 32};

    wire_id Data[32] = {0};
    wire_id Out[32]  = {0};

    for (u32 Index = 0; Index < ArrayCount(BitCounts); Index++)
    {
        u32 BitCount = BitCounts[Index];

        ResetCircuit();

        AddWires(Data, BitCount);
        AddWires(Out,  BitCount);

        wire_id Clock       = AddWire();
        wire_id WriteEnable = AddWire();

        Register(BitCount, Data, WriteEnable, Clock, Out);

        u32 PulseTime = 2 + (GetWallClock() & 7);

        for (u32 TestIndex = 0; TestIndex < 128; TestIndex++)
        {
            RandomizeWireState();

            RandomWires(Data, BitCount);
            SetWire(WriteEnable, 1);

            u64 Expected = GetWires(Data, BitCount);

            u32 WriteCycles = 2 + (GetWallClock() & 7);
            for (u32 Cycle = 0; Cycle < WriteCycles; Cycle++)
            {
                SimulateClockCycle(Clock, PulseTime);
            }

            Successful &= ExpectWires(Out, BitCount, Expected);

            SetWire(WriteEnable, 0);

            u32 ReadCycles = 1 + (GetWallClock() & 7);
            for (u32 Cycle = 0; Cycle < ReadCycles; Cycle++)
            {
                RandomWires(Data, 8);
                SimulateClockCycle(Clock, PulseTime);

                Successful &= ExpectWires(Out, BitCount, Expected);
            }
        }
    }

    OutputTestResult(Str("Register"), Successful);
}
