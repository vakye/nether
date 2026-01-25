
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

local wire CircuitWires[65536] = {0};
local gate CircuitGates[65536] = {0};

local u32 CircuitWireCount = 0;
local u32 CircuitGateCount = 0;

// NOTE(vak): Circuit

local void RandomizeWireState(void)
{
    u32 State = GetWallClock() & 0xFFFFFFFF;

    for (u32 Index = 0; Index < CircuitWireCount; Index++)
    {
        State ^= (State << 13);
        State ^= (State >> 17);
        State ^= (State << 5);

        wire Bit = (State & 1);

        CircuitWires[Index] = Bit;
    }
}

local void ResetCircuit(void)
{
    CircuitWireCount = 0;
    CircuitGateCount = 0;
}

local void ResetGates(void)
{
    CircuitGateCount = 0;
}

local void SimulateCircuit(void)
{
    for (u32 GateIndex = 0; GateIndex < CircuitGateCount; GateIndex++)
    {
        gate* Gate = CircuitGates + GateIndex;

        switch (Gate->Kind)
        {
            InvalidDefaultCase;

            case GateKind_NAND:
            {
                wire_id A   = Gate->A;
                wire_id B   = Gate->B;
                wire_id Out = Gate->Out;

                CircuitWires[Out] = !(CircuitWires[A] & CircuitWires[B]);
            } break;

            case GateKind_TriState:
            {
                wire_id Input  = Gate->A;
                wire_id Enable = Gate->B;
                wire_id Output = Gate->Out;

                if (CircuitWires[Enable])
                    CircuitWires[Output] = CircuitWires[Input];
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
    Assert(CircuitWireCount < ArrayCount(CircuitWires));

    wire_id Result = CircuitWireCount++;
    return (Result);
}

local wire GetWire(wire_id ID)
{
    Assert(ID < CircuitWireCount);

    wire Result = (CircuitWires[ID] & 1);
    return (Result);
}

local void SetWire(wire_id ID, wire Bit)
{
    Assert(ID < CircuitWireCount);

    CircuitWires[ID] = (Bit & 1);
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

local wires AddWires(u32 Count)
{
    Assert(CircuitWireCount + Count <= ArrayCount(CircuitWires));

    wires Result = {CircuitWireCount, Count};

    CircuitWireCount += Count;

    return (Result);
}

local u64 GetWires(wires Wires)
{
    Assert(Wires.Count <= 64);

    u64 Result = 0;

    for (u32 Index = 0; Index < Wires.Count; Index++)
        Result |= ((u64)GetWire(Wires.First + Index)) << Index;

    u64 Limit = (Wires.Count < 64) ? (1ull << Wires.Count) : (U64Max);

    Assert(Result <= Limit);

    return (Result);
}

local void SetWires(wires Wires, u64 Bits)
{
    Assert(Wires.Count <= 64);

    for (u32 Index = 0; Index < Wires.Count; Index++)
        SetWire(Wires.First + Index, (Bits >> Index) & 1);
}

local b32 ExpectWires(wires Wires, u64 ExpectedBits)
{
    b32 Result = GetWires(Wires) == (ExpectedBits);
    return (Result);
}

local void RandomWires(wires Wires)
{
    u32 State = GetWallClock() & 0xFFFFFFFF;

    for (u32 Index = 0; Index < Wires.Count; Index++)
    {
        State ^= (State << 13);
        State ^= (State >> 17);
        State ^= (State << 5);

        SetWire(Wires.First + Index, State & 1);
    }
}

// NOTE(vak): Tri-state

local void TriState(wire_id Input, wire_id Enable, wire_id Output)
{
    Assert(Input  < CircuitWireCount);
    Assert(Enable < CircuitWireCount);
    Assert(Output < CircuitWireCount);

    Assert(CircuitGateCount < ArrayCount(CircuitGates));

    gate* Gate = CircuitGates + CircuitGateCount++;

    Gate->Kind = GateKind_TriState;
    Gate->A    = Input;
    Gate->B    = Enable;
    Gate->Out  = Output;
}

local void TriStateNOT(wire_id Input, wire_id Enable, wire_id Output)
{
    wire_id Result = AddWire();

    NOT     (Input, Result);
    TriState(Result, Enable, Output);
}

// NOTE(vak): Logic gates

local void NAND(wire_id A, wire_id B, wire_id Out)
{
    Assert(A   < CircuitWireCount);
    Assert(B   < CircuitWireCount);
    Assert(Out < CircuitWireCount);

    Assert(CircuitGateCount < ArrayCount(CircuitGates));

    gate* Gate = CircuitGates + CircuitGateCount++;

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

local void HalfAdder(wires A, wires B, wires Sum, wire_id Carry)
{
    u32 BitCount = Sum.Count;

    Assert(BitCount >= 1);
    Assert(A.Count == BitCount);
    Assert(B.Count == BitCount);

    wires Carries = AddWires(BitCount - 1);

    wire_id NextCarry = (BitCount == 1) ? (Carry) : (Carries.First);

    HalfAdder1(
        A.First,
        B.First,
        Sum.First,
        NextCarry
    );

    for (u32 BitIndex = 1; BitIndex < BitCount; BitIndex++)
    {
        wire_id LastCarry = NextCarry;

        if (BitIndex + 1 < BitCount)
            NextCarry = Carries.First + BitIndex;
        else
            NextCarry = Carry;

        FullAdder1(
            A.First + BitIndex,
            B.First + BitIndex,
            LastCarry,
            Sum.First + BitIndex,
            NextCarry
        );
    }
}

local void FullAdder(wires A, wires B, wire_id C, wires Sum, wire_id Carry)
{
    u32 BitCount = Sum.Count;

    Assert(BitCount >= 1);
    Assert(A.Count == BitCount);
    Assert(B.Count == BitCount);

    wires Carries = AddWires(BitCount - 1);

    wire_id NextCarry = (BitCount == 1) ? (Carry) : (Carries.First);

    FullAdder1(
        A.First,
        B.First,
        C,
        Sum.First,
        NextCarry
    );

    for (u32 BitIndex = 1; BitIndex < BitCount; BitIndex++)
    {
        wire_id LastCarry = NextCarry;

        if (BitIndex + 1 < BitCount)
            NextCarry = Carries.First + BitIndex;
        else
            NextCarry = Carry;

        FullAdder1(
            A.First + BitIndex,
            B.First + BitIndex,
            LastCarry,
            Sum.First + BitIndex,
            NextCarry
        );
    }
}

// NOTE(vak): Memory

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

// NOTE(vak): Central components

local void Register(wires Data, wire_id WriteEnable, wire_id Clock, wires Out)
{
    u32 BitCount = Out.Count;

    Assert(BitCount >= 1);
    Assert(Data.Count == BitCount);

    wire_id W    = WriteEnable;
    wire_id NotW = AddWire();

    NOT(W, NotW);

    for (u32 BitIndex = 0; BitIndex < BitCount; BitIndex++)
    {
        wire_id D = AddWire();
        wire_id NotOut = AddWire();

        TriState(Data.First + BitIndex, W,    D);
        TriState(Out.First + BitIndex,  NotW, D);

        DFlipFlop(D, Clock, Out.First + BitIndex, NotOut);
    }
}

local void ALU(wires A, wires B, wire_id SubtractOp, wires Out, wire_id Carry)
{
    u32 BitCount = Out.Count;

    Assert(BitCount >= 2); // NOTE(vak): Need atleast 2 bits since one bit is used for the sign.
    Assert(A.Count == BitCount);
    Assert(B.Count == BitCount);

    wire_id NotSubtractOp = AddWire();

    wires InA = A;
    wires InB = AddWires(B.Count);

    NOT(SubtractOp, NotSubtractOp);

    for (u32 Index = 0; Index < B.Count; Index++)
    {
        TriState   (B.First + Index, NotSubtractOp, InB.First + Index);
        TriStateNOT(B.First + Index,    SubtractOp, InB.First + Index);
    }

    wire_id CarryBuffer = AddWire();

    FullAdder(InA, InB, SubtractOp, Out, CarryBuffer);
    XOR      (SubtractOp, CarryBuffer, Carry);
}

// NOTE(vak): Tests

local b32 VerifyTruthTable(
    wire* TruthTable, u32 RowCount,
    wires Inputs, wires Outputs
)
{
    RandomizeWireState();

    u32 ColumnCount = Inputs.Count + Outputs.Count;

    for (u32 RowIndex = 0; RowIndex < RowCount; RowIndex++)
    {
        wire* TestTable = TruthTable + (RowIndex * ColumnCount);

        wire* InputValues = TestTable;
        wire* OutputValues = TestTable + Inputs.Count;

        for (u32 Index = 0; Index < Inputs.Count; Index++)
        {
            SetWire(Inputs.First + Index, InputValues[Index]);
        }

        SimulateCircuit();

        for (u32 Index = 0; Index < Outputs.Count; Index++)
        {
            if (!ExpectWire(Outputs.First + Index, OutputValues[Index]))
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

    persist wire TruthTriState[3 * 2] =
    {
        0, 1,    0,
        1, 1,    1,
    };

    wires Inputs  = AddWires(2);
    wires Outputs = AddWires(1);

    ResetGates();
    TriState(Inputs.First, Inputs.First + 1, Outputs.First);

    OutputTestResult(Str("TriState"), VerifyTruthTable(TruthTriState, 2, Inputs, Outputs));

    persist wire TruthTriStateNOT[3 * 2] =
    {
        0, 1,    1,
        1, 1,    0,
    };

    ResetGates();
    TriStateNOT(Inputs.First, Inputs.First + 1, Outputs.First);

    OutputTestResult(Str("TriStateNOT"), VerifyTruthTable(TruthTriStateNOT, 2, Inputs, Outputs));
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

        wires Inputs  = AddWires(2);
        wires Outputs = AddWires(1);

        #define DoBinaryTest(Name) \
            ResetGates(); \
            Name(Inputs.First + 0, Inputs.First + 1, Outputs.First); \
            \
            OutputTestResult(Str(#Name), VerifyTruthTable(Truth##Name, 4, Inputs, Outputs));

        DoBinaryTest(NAND)
        DoBinaryTest(AND)
        DoBinaryTest(OR)
        DoBinaryTest(NOR)
        DoBinaryTest(XOR)
    }

    // NOTE(vak): Unary logic gates
    {
        ResetCircuit();

        wires Inputs  = AddWires(1);
        wires Outputs = AddWires(1);

        NOT(Inputs.First, Outputs.First);

        OutputTestResult(Str("NOT"), VerifyTruthTable(TruthNOT, 2, Inputs, Outputs));
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

    wires Inputs  = AddWires(2);
    wires Outputs = AddWires(2);

    wire_id A     = Inputs.First  + 0;
    wire_id B     = Inputs.First  + 1;
    wire_id Sum   = Outputs.First + 0;
    wire_id Carry = Outputs.First + 1;

    HalfAdder1(A, B, Sum, Carry);

    OutputTestResult(Str("HalfAdder1"), VerifyTruthTable(TruthHalfAdder, 4, Inputs, Outputs));
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

    wires Inputs  = AddWires(3);
    wires Outputs = AddWires(2);

    wire_id A     = Inputs.First  + 0;
    wire_id B     = Inputs.First  + 1;
    wire_id C     = Inputs.First  + 2;
    wire_id Sum   = Outputs.First + 0;
    wire_id Carry = Outputs.First + 1;

    FullAdder1(A, B, C, Sum, Carry);

    OutputTestResult(Str("FullAdder1"), VerifyTruthTable(TruthFullAdder, 8, Inputs, Outputs));
}

local void TestHalfAdder(void)
{
    b32 Successful = true;

    ResetCircuit();

    for (u32 Index = 0; Index < 10; Index++)
    {
        u32 BitCount = 1 + (GetWallClock() & 63);

        ResetCircuit();

        wires   A     = AddWires(BitCount);
        wires   B     = AddWires(BitCount);
        wires   Sum   = AddWires(BitCount);
        wire_id Carry = AddWire();

        HalfAdder(A, B, Sum, Carry);

        u64 Mask = (BitCount == 64) ? (U64Max) : ((1ull << BitCount) - 1);

        for (u32 TestIndex = 0; TestIndex < 1024; TestIndex++)
        {
            RandomizeWireState();
            SimulateCircuit();

            u64 ValueA = GetWires(A);
            u64 ValueB = GetWires(B);

            u64  Computed      = ValueA + ValueB;
            u64  ExpectedSum   = Computed & Mask;
            wire ExpectedCarry = ExpectedSum < ValueA;

            Successful &= ExpectWires(Sum,   ExpectedSum);
            Successful &= ExpectWire (Carry, ExpectedCarry);
        }
    }

    OutputTestResult(Str("HalfAdder"), Successful);
}

local void TestFullAdder(void)
{
    b32 Successful = true;

    ResetCircuit();

    for (u32 Index = 0; Index < 10; Index++)
    {
        u32 BitCount = 1 + (GetWallClock() & 63);

        ResetCircuit();

        wires   A     = AddWires(BitCount);
        wires   B     = AddWires(BitCount);
        wire_id C     = AddWire();
        wires   Sum   = AddWires(BitCount);
        wire_id Carry = AddWire();

        FullAdder(A, B, C, Sum, Carry);

        u64 Mask = (BitCount == 64) ? (U64Max) : ((1ull << BitCount) - 1);

        for (u32 TestIndex = 0; TestIndex < 1024; TestIndex++)
        {
            RandomizeWireState();
            SimulateCircuit();

            u64 ValueA = GetWires(A);
            u64 ValueB = GetWires(B);
            u64 ValueC = GetWire (C);

            u64  Computed      = ValueA + ValueB + ValueC;
            u64  ExpectedSum   = Computed & Mask;
            wire ExpectedCarry = (ValueC) ? (ExpectedSum <= ValueA) : (ExpectedSum < ValueA);

            Successful &= ExpectWires(Sum,   ExpectedSum);
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

    for (u32 Index = 0; Index < 10; Index++)
    {
        u32 BitCount = 1 + (GetWallClock() & 63);

        ResetCircuit();

        wires   Data        = AddWires(BitCount);
        wires   Out         = AddWires(BitCount);
        wire_id Clock       = AddWire();
        wire_id WriteEnable = AddWire();

        Register(Data, WriteEnable, Clock, Out);

        u32 PulseTime = 2 + (GetWallClock() & 7);

        for (u32 TestIndex = 0; TestIndex < 128; TestIndex++)
        {
            RandomizeWireState();

            RandomWires(Data);
            SetWire(WriteEnable, 1);

            u64 Expected = GetWires(Data);

            u32 WriteCycles = 2 + (GetWallClock() & 7);
            for (u32 Cycle = 0; Cycle < WriteCycles; Cycle++)
            {
                SimulateClockCycle(Clock, PulseTime);
            }

            u64 Debug = GetWires(Out);

            Successful &= ExpectWires(Out, Expected);

            SetWire(WriteEnable, 0);

            u32 ReadCycles = 1 + (GetWallClock() & 7);
            for (u32 Cycle = 0; Cycle < ReadCycles; Cycle++)
            {
                RandomWires(Data);
                SimulateClockCycle(Clock, PulseTime);

                Successful &= ExpectWires(Out, Expected);
            }
        }
    }

    OutputTestResult(Str("Register"), Successful);
}

local void TestALU(void)
{
    b32 Successful = true;

    for (u32 Index = 0; Index < 10; Index++)
    {
        u32 BitCount = 2 + (GetWallClock() & 62);

        ResetCircuit();

        wires   A          = AddWires(BitCount);
        wires   B          = AddWires(BitCount);
        wire_id SubtractOp = AddWire();
        wires   Out        = AddWires(BitCount);
        wire_id Carry      = AddWire();

        ALU(A, B, SubtractOp, Out, Carry);

        u64 Mask = (BitCount == 64) ? (U64Max) : ((1ull << BitCount) - 1);

        for (u32 TestIndex = 0; TestIndex < 128; TestIndex++)
        {
            RandomizeWireState();
            SimulateCircuit();

            u64 ValueA = GetWires(A);
            u64 ValueB = GetWires(B);

            u64 Computed = 0;

            if (GetWire(SubtractOp))
                Computed = ValueA - ValueB;
            else
                Computed = ValueA + ValueB;

            u64  ExpectedOut   = Computed & Mask;
            wire ExpectedCarry = (GetWire(SubtractOp)) ? (ExpectedOut > ValueA) : (ExpectedOut < ValueA);

            Successful &= ExpectWires(Out,   ExpectedOut);
            Successful &= ExpectWire (Carry, ExpectedCarry);
        }
    }

    OutputTestResult(Str("ALU"), Successful);
}
