
#pragma once

typedef u8  wire;
typedef u32 wire_id;

// NOTE(vak): Circuit

local void RandomizeWireState(void);

local void ResetCircuit      (void);
local void ResetGate         (void);
local void SimulateCircuit   (void);

local void SimulateClockPulse(wire_id Clock, u32 PulseTime);
local void SimulateClockCycle(wire_id Clock, u32 PulseTime);

// NOTE(vak): Wires

local wire_id AddWire   (void);
local wire    GetWire   (wire_id ID);
local void    SetWire   (wire_id ID , wire Bit);
local b32     ExpectWire(wire_id ID, wire ExpectedBit);
local void    RandomWire(wire_id ID);

local void    AddWires   (wire_id* IDs, u32 Count);
local u64     GetWires   (wire_id* IDs, u32 Count);
local void    SetWires   (wire_id* IDs, u32 Count, u64 Bits);
local b32     ExpectWires(wire_id* IDs, u32 Count, u64 ExpectedBits);
local void    RandomWires(wire_id* IDs, u32 Count);

// NOTE(vak): Tri-state

local void TriState(wire_id Input, wire_id Enable, wire_id Output);

// NOTE(vak): Logic gates

local void NAND(wire_id A, wire_id B, wire_id Out);
local void AND (wire_id A, wire_id B, wire_id Out);
local void OR  (wire_id A, wire_id B, wire_id Out);
local void NOR (wire_id A, wire_id B, wire_id Out);
local void XOR (wire_id A, wire_id B, wire_id Out);
local void NOT (wire_id In, wire_id Out);

// NOTE(vak): Adder

local void HalfAdder1(wire_id A, wire_id B,            wire_id Sum, wire_id Carry); // NOTE(vak): Out, Carry = (A + B)
local void FullAdder1(wire_id A, wire_id B, wire_id C, wire_id Sum, wire_id Carry); // NOTE(vak): Out, Carry = (A + B + C)

local void HalfAdder(u32 BitCount, wire_id* A, wire_id* B,            wire_id* Sum, wire_id Carry);
local void FullAdder(u32 BitCount, wire_id* A, wire_id* B, wire_id C, wire_id* Sum, wire_id Carry);

// NOTE(vak): Memory

// NOTE(vak):
// Stores the 'Data' bit when 'Enable' is high.
// Minimum PulseTime: 2
local void DLatch(wire_id Data, wire_id Enable, wire_id Out, wire_id NotOut);

// NOTE(vak):
// Stores the 'Data' bit after a single clock cycle.
// Minimum PulseTime: 2
// Minimum cycles:    1
local void DFlipFlop(wire_id Data, wire_id Clock, wire_id Out, wire_id NotOut);

// NOTE(vak):
// Stores the 'Data' bit after a single clock cycle when 'WriteEnable' is high.
// Minimum PulseTime:    2
// Minimum write cycles: 2
local void Register(u32 BitCount, wire_id* Data, wire_id WriteEnable, wire_id Clock, wire_id* Out);

// NOTE(vak): Testing

#define TestResultPrintPadding (20)

local b32 VerifyTruthTable(
    wire* TruthTable, u32 RowCount,
    wire_id* Inputs, u32 InputCount,
    wire_id* Outputs, u32 OutputCount
);

local void OutputTestResult(string Name, b32 Successful);

local void TestTriState(void);
local void TestLogicGates(void);

local void TestHalfAdder1(void);
local void TestFullAdder1(void);
local void TestHalfAdder(void);
local void TestFullAdder(void);

local void TestDLatch(void);
local void TestDFlipFlop(void);
local void TestRegister(void);
