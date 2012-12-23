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
const char UC_RET = 0x25;	// No return value.
const char UC_AIDX = 0x26;
const char UC_RET2 = 0x2c;	// Identical to UC_RET.
const char UC_RETV = 0x2d;  // Returns value from stack.
const char UC_LOOP = 0x2e;
const char UC_ADDSV = 0x2f;
const char UC_IN = 0x30;
const char UC_RETZ = 0x32;	// Returns zero.
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
const char UC_CALLIND = 0x53;		// Call indirect.  Addr. on stack.
const char UC_PUSHTHV = 0x54;	// Push this->var.
const char UC_POPTHV = 0x55;		// Pop this->var.
const char UC_CALLM = 0x56;		// Call method (index is on stack).
const char UC_CALLMS = 0x57;		// Call method (index is on stack).
const char UC_CLSCREATE = 0x58;	// Create class instance.
const char UC_CLASSDEL = 0x59;	// Delete class instance.
const char UC_AIDXS = 0x5a;		// Pop static array elem.
const char UC_POPARRS = 0x5b;		// Pop into static array elem.
const char UC_LOOPTOPS = 0x5c;		// Loop with static array
const char UC_AIDXTHV = 0x5d;		// Pop this->var array elem.
const char UC_POPARRTHV = 0x5e;		// Pop this->var array elem.
const char UC_LOOPTOPTHV = 0x5f;		// Loop with this->var array.
const char UC_PUSHCHOICE = (char)0x60;	// Pushes last selected user choice.
const char UC_PUSHFVAR = (char)0xc2;		// PUSH global flag using stack value.
const char UC_POPFVAR = (char)0xc3;		// POP global flag using stack value.
const char UC_CALLINDEX = (char)0xD3;		// Call indirect.  Addr. on stack. with arguments

// 32-bit usecode
const char UC_LOOPTOP32 = (char)0x82;
const char UC_CONVERSE32 = (char)0x84;
const char UC_JNE32 = (char)0x85;
const char UC_JMP32 = (char)0x86;
const char UC_CMPS32 = (char)0x87;
const char UC_LOOPTOPS32 = (char)0xdc;		// Loop with static array
const char UC_LOOPTOPTHV32 = (char)0xdf;		// Loop with this->var array.
/*	No real reason for this.
 *	const char UC_LOOP32 = 0xae;
*/
const char UC_ADDSI32 = (char)0x9c;
const char UC_PUSHS32 = (char)0x9d;
const char UC_PUSHI32 = (char)0x9f;
const char UC_CALL32 = (char)0xa4;
const char UC_CALLE32 = (char)0xc7;

#endif
