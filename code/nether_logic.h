
#pragma once

typedef u8  wire;
typedef u32 wire_id;

// NOTE(vak): Circuit

local void ResetCircuit(void);
local void SimulateCircuit(void);

// NOTE(vak): Wires

local wire_id AddWire(void);
local wire    GetWire(wire_id ID);
local void    SetWire(wire_id ID , wire Bit);

local void    AddWires(wire_id* IDs, u32 Count);
local u64     GetWires(wire_id* IDs, u32 Count);
local void    SetWires(wire_id* IDs, u32 Count, u64 Bits);

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

// NOTE(vak): Tests

local void TestHalfAdder1(void);
local void TestFullAdder1(void);

local void TestHalfAdder(void);
local void TestFullAdder(void);
