
#pragma once

typedef u8  wire;
typedef u32 wire_id;

typedef struct
{
    wire_id A;
    wire_id B;

    wire_id Out;
} nand;

local s32 Main(void);
