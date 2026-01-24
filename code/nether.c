
local s32 Main(void)
{
    // NOTE(vak): NAND
    {
        ResetCircuit();

        wire_id A   = AddWire();
        wire_id B   = AddWire();
        wire_id Out = AddWire();

        NAND(A, B, Out);

        TestBinary(Str("NAND"), A, B, Out);
    }

    PrintNewLine();

    // NOTE(vak): NOT
    {
        ResetCircuit();

        wire_id In  = AddWire();
        wire_id Out = AddWire();

        NOT(In, Out);

        TestUnary(Str("NOT"), In, Out);
    }

    PrintNewLine();

    // NOTE(vak): AND
    {
        ResetCircuit();

        wire_id A   = AddWire();
        wire_id B   = AddWire();
        wire_id Out = AddWire();

        AND(A, B, Out);

        TestBinary(Str("AND"), A, B, Out);
    }

    PrintNewLine();

    // NOTE(vak): OR
    {
        ResetCircuit();

        wire_id A   = AddWire();
        wire_id B   = AddWire();
        wire_id Out = AddWire();

        OR(A, B, Out);

        TestBinary(Str("OR"), A, B, Out);
    }

    PrintNewLine();

    // NOTE(vak): NOR
    {
        ResetCircuit();

        wire_id A   = AddWire();
        wire_id B   = AddWire();
        wire_id Out = AddWire();

        NOR(A, B, Out);

        TestBinary(Str("NOR"), A, B, Out);
    }

    PrintNewLine();

    // NOTE(vak): XOR
    {
        ResetCircuit();

        wire_id A   = AddWire();
        wire_id B   = AddWire();
        wire_id Out = AddWire();

        XOR(A, B, Out);

        TestBinary(Str("XOR"), A, B, Out);
    }

    PrintNewLine();

    return (0);
}
