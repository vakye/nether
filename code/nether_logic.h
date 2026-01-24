
#pragma once

typedef u8  wire;
typedef u32 wire_id;

// NOTE(vak): Circuit

local void ResetCircuit(void);
local void SimulateCircuit(void);

// NOTE(vak): Wires

local wire_id AddWire(void);
local wire    GetWire(wire_id ID);
local void    SetWire(wire_id ID, wire Bit);

// NOTE(vak): Logic gates

local void NAND(wire_id A, wire_id B, wire_id Out);
local void AND (wire_id A, wire_id B, wire_id Out);
local void OR  (wire_id A, wire_id B, wire_id Out);
local void NOR (wire_id A, wire_id B, wire_id Out);
local void XOR (wire_id A, wire_id B, wire_id Out);
local void NOT (wire_id In, wire_id Out);

// NOTE(vak): Tests

local void TestUnary (string Name, wire_id In, wire_id Out);
local void TestBinary(string Name, wire_id A, wire_id B, wire_id Out);
