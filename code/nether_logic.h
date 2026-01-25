
#pragma once

typedef u8  wire;
typedef u32 wire_id;

typedef struct
{
    wire_id First;
    u32     Count;
} wires;

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

local wires   AddWires   (u32 Count);
local u64     GetWires   (wires Wires);
local void    SetWires   (wires Wires, u64 Bits);
local b32     ExpectWires(wires Wires, u64 ExpectedBits);
local void    RandomWires(wires Wires);

// NOTE(vak): Tri-state

local void TriState   (wire_id Input, wire_id Enable, wire_id Output);
local void TriStateNOT(wire_id Input, wire_id Enable, wire_id Output);

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

local void HalfAdder(wires A, wires B,            wires Sum, wire_id Carry);
local void FullAdder(wires A, wires B, wire_id C, wires Sum, wire_id Carry);

// NOTE(vak): Memory

// NOTE(vak):
// Stores the 'Data' bit when 'Enable' is high.
// Minimum pulse time: 2
local void DLatch(wire_id Data, wire_id Enable, wire_id Out, wire_id NotOut);

// NOTE(vak):
// Stores the 'Data' bit after a single clock cycle.
// Minimum pulse time:  2
// Minimum latch cycle: 1
local void DFlipFlop(wire_id Data, wire_id Clock, wire_id Out, wire_id NotOut);

// NOTE(vak): Central components

// NOTE(vak):
// Stores the 'Data' bit after two clock cycles when 'WriteEnable' is high.
// Minimum pulse time:   2
// Minimum write cycles: 2
local void Register(wires Data, wire_id WriteEnable, wire_id Clock, wires Out);

// NOTE(vak):
// If `SubtractOp` is 0: Out = A + B
// If `SubtractOp` is 1: Out = A - B
local void ALU(wires A, wires B, wire_id SubtractOp, wires Out, wire_id Carry);

// NOTE(vak): Testing

#define TestResultPrintPadding (20)

local b32 VerifyTruthTable(
    wire* TruthTable, u32 RowCount,
    wires Inputs, wires Outputs
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

local void TestALU(void);
