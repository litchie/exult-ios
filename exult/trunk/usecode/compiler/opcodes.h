/**
 **	Opcodes.h - Opcode definitions for Usecode.
 **
 **	Written: 1/1/01 - JSF
 **/

#ifndef INCL_OPCODES
#define INCL_OPCODES 1

const char UC_LOOPTOP = 0x02;
const char UC_CONVERSE = 0x04;
const char UC_JNE = 0x05;
const char UC_JMP = 0x06;
const char UC_CMPS = 0x07;
const char UC_ADD = 0x09;
const char UC_SUB = 0x0a;
const char UC_DIV = 0x0b;
const char UC_MUL = 0x0c;
const char UC_MOD = 0x0d;
const char UC_AND = 0x0e;
const char UC_OR = 0x0f;
const char UC_NOT = 0x10;
const char UC_POP = 0x12;
const char UC_PUSHTRUE = 0x13;
const char UC_PUSHFALSE = 0x14;
const char UC_CMPG = 0x16;
const char UC_CMPL = 0x17;
const char UC_CMPGE = 0x18;
const char UC_CMPLE = 0x19;
const char UC_CMPNE = 0x1a;
const char UC_ADDSI = 0x1c;
const char UC_PUSHS = 0x1d;
const char UC_ARRC = 0x1e;
const char UC_PUSHI = 0x1f;
const char UC_PUSH = 0x21;
const char UC_CMPEQ = 0x22;
const char UC_CALL = 0x24;
const char UC_RET = 0x25;
const char UC_AIDX = 0x26;
const char UC_RET2 = 0x2c;
const char UC_SETR = 0x2d;  // ??
const char UC_LOOP = 0x2e;
const char UC_ADDSV = 0x2f;
const char UC_IN = 0x30;
const char UC_RTS = 0x32;
const char UC_SAY = 0x33;
const char UC_CALLIS = 0x38;
const char UC_CALLI = 0x39;
const char UC_PUSHITEMREF = 0x3e;
const char UC_ABRT = 0x3f;
const char UC_CONVERSELOC = 0x40;	// CONVERSE jmps here.
const char UC_PUSHF = 0x42;		// PUSH global flag.
const char UC_POPF = 0x43;		// POP global flag.
const char UC_PUSHB = 0x44;		// Push byte that follows.
const char UC_POPARR = 0x46;		// Pop into array elem.
const char UC_CALLE = 0x47;
const char UC_PUSHEVENTID = 0x48;
const char UC_ARRA = 0x4a;
const char UC_POPEVENTID = 0x4b;
					// Added for Exult:
const char UC_PUSHSTATIC = 0x50;	// Push static.
const char UC_POPSTATIC = 0x51;		// Pop static.
const char UC_CALLO = 0x52;		// Call original.

#endif
