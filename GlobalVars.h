#pragma once
#include <string>

using namespace std;

/*Global operators, operands, and number prefixes currently recognized by the interpreter under the statement namespace*/

namespace opcode {

	static const int COUNT = 24;
	static const string STRING[COUNT] = { "BR",   "ADD",  "LD",   "ST",   "JSR",  "AND",  "LDR",  "STR",  "RTI",  "NOT",  "LDI",  "STI",  "JMP",  "ILLI", "LEA",  "TRAP", "JSRR",   "RET",  "GETC", "OUT",  "PUTS", "IN",   "PUTSP", "HALT" };
	static const int HEX[COUNT] =       { 0x0000, 0x1000, 0x2000, 0x3000, 0x4800, 0x5000, 0x6000, 0x7000, 0x8000, 0x903F, 0xa000, 0xb000, 0xc000, 0xd000, 0xe000, 0xf000,  0x4000,  0xc1c0, 0xf020, 0xf021, 0xf022, 0xf023, 0xf024,  0xf025 };
	enum				ENUM            {  BR,     ADD,    LD,     ST,     JSR,    AND,    LDR,    STR,    RTI,    NOT,    LDI,    STI,    JMP,    ILLI,   LEA,    TRAP,   JSRR,     RET,    GETC,   OUT,    PUTS,   IN,     PUTSP,   HALT };
	
	static const int BR_COND[4] = { 0x0E00,0x0800,0x0400,0x0200 };
	enum             BR_ENUM      { NZP,   N,     Z,     P };
}

namespace operand {
	static const int COUNT = 8;
	static const string STRING[COUNT] = {  "R0",  "R1",  "R2",  "R3",  "R4",  "R5",  "R6",  "R7" };
	enum				ENUM            {   R0,    R1,    R2,    R3,    R4,    R5,    R6,    R7  };
	static const int DR_HEX[COUNT]	  = { 0x0000,0x0200,0x0400,0x0600,0x0800,0x0A00,0x0C00,0x0E00};
	static const int SR1_HEX[COUNT]   = { 0x0000,0x0040,0x0080,0x00C0,0x0100,0x0140,0x0180,0x01C0};
	static const int SR2_HEX[COUNT]   = { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007};
	static const int* SR_HEX = DR_HEX;
	static const int* BaseR_HEX = SR1_HEX;

	static const int IMM5_SELECT = 0x0020;
}

namespace number {
	static const int COUNT = 4;
	static const char STRING[COUNT] = { 'a',         'x', 'b', '#' };
	enum			     ENUM         { ASSUMED_DIG, HEX, BIN, DIG };
}

namespace compiler {
	enum CONVERSION {HEX, OBJ};
}

namespace offset {
	static const int COUNT = 5;
	enum Type                            { ERROR, FIVE, SIX, NINE, ELEVEN};
	static const int POS_BOUNDS[COUNT] = { 0,     15,   31,  255,  1023 };
	static const int NEG_BOUDNS[COUNT] = { 0,    -16,  -32, -256, -1024 };

}