/**
 **	Opcodes.h - Opcode definitions for Usecode.
 **
 **	Written: 1/1/01 - JSF
 **/

#ifndef INCL_OPCODES
#define INCL_OPCODES 1

const int UC_LOOPTOP = 0x02;
const int UC_CATCH = 0x04;
const int UC_JNE = 0x05;
const int UC_JMP = 0x06;
const int UC_CMPS = 0x07;
const int UC_ADD = 0x09;
const int UC_SUB = 0x0a;
const int UC_DIV = 0x0b;
const int UC_MUL = 0x0c;
const int UC_MOD = 0x0d;
const int UC_AND = 0x0e;
const int UC_OR = 0x0f;
const int UC_NOT = 0x10;
const int UC_POP = 0x12;
const int UC_PUSHTRUE = 0x13;
const int UC_PUSHFALSC = 0x14;
const int UC_CMPG = 0x16;
const int UC_CMPL = 0x17;
const int UC_CMPGE = 0x18;
const int UC_CMPLE = 0x19;
const int UC_CMPNE = 0x1a;
const int UC_ADDSI = 0x1c;
const int UC_PUSHS = 0x1d;
const int UC_ARRC = 0x1e;
const int UC_PUSHI = 0x1f;
const int UC_PUSH = 0x21;
const int UC_CMPEQ = 0x22;
const int UC_CALL = 0x24;
const int UC_RET = 0x25;
const int UC_AIDX = 0x26;
const int UC_RET2 = 0x2c;
const int UC_SETR = 0x2d;  // ??
const int UC_LOOP = 0x2e;
const int UC_ADDSV = 0x2f;
const int UC_IN = 0x30;
const int UC_RTS = 0x32;
const int UC_SAY = 0x33;
const int UC_CALLIS = 0x38;
const int UC_CALLI = 0x39;
const int UC_PUSHITEMREF = 0x3e;
const int UC_ABRT = 0x3f;
const int UC_CATCHLOC = 0x40;	// CATCH jmps here.
const int UC_PUSHF = 0x42;		// PUSH global flag.
const int UC_POPF = 0x43;		// POP global flag.
const int UC_PUSHB = 0x44;		// Push byte that follows.
const int UC_POPARR = 0x46;		// Pop into array elem.
const int UC_CALLE = 0x47;
const int UC_PUSHEVENTID = 0x48;
const int UC_ARRA = 0x4a;
const int UC_POPEVENTID = 0x4b;

#endif