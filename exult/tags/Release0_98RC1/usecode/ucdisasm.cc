/*
 *	ucdisasm.cc - Disassembled usecode trace
 *
 *  Copyright (C) 1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include <cstring>
#include <cstdio>
#include <iostream>

using std::strlen;
using std::cout;

#include "ucinternal.h"
#include "../tools/uctools.h"
#include "utils.h"
#include "useval.h"
#include "game.h"

void Usecode_internal::uc_trace_disasm(Usecode_value* locals, int num_locals,
						 uint8* data, uint8* externals, uint8* code,
						 uint8* ip)
{
	int func_ip = (int)(ip - code);
	int opcode = *ip++;
	uint8* param_ip = ip;
	_opcode_desc* pdesc = &(opcode_table[opcode]);
	signed short immed;
	unsigned short offset;
	unsigned short varref;
	unsigned short func;

	std::printf("      %04X: ", func_ip);

	std::printf("%s", pdesc->mnemonic);
	if (strlen(pdesc->mnemonic) < 4)
		std::printf("\t");

	if (pdesc->nbytes > 0) {
		switch( pdesc->type )
			{
			case BYTE:
				std::printf("\t%02x", *ip++);
				break;
			case IMMED:
				// Print immediate operand
				immed = Read2(ip);
				std::printf("\t%04hXH\t\t; %d", immed, immed);
				break;
			case DATA_STRING:
				{
					char* pstr;
					int len;
					// Print data string operand
					offset = Read2(ip);
					pstr = (char*)data + offset;
					len = strlen(pstr);
					if( len > 20 )
						len = 20 - 3;
					std::printf("\tL%04X\t\t; ", offset);
					for(int i = 0; i < len; i++ )
						std::printf("%c", pstr[i]);
					if( len < strlen(pstr) )
						// String truncated
						std::printf("...");
				}
				break;
			case RELATIVE_JUMP:
				// Print jump desination
				offset = Read2(ip);
				std::printf("\t%04X",
							(offset + func_ip+1+pdesc->nbytes)&0xFFFF);
				break;
			case SLOOP:
				if (pdesc->nbytes == 11)
					ip++;
				Read2(ip);
				Read2(ip);
				Read2(ip);
				varref = Read2(ip);
				offset = Read2(ip);
				std::printf("\t[%04X], %04X\t= ", varref, 
					   (offset +func_ip+1+pdesc->nbytes)&0xFFFF);
				locals[varref].print(cout, true); // print value (short format)
				break;
			case IMMED_AND_RELATIVE_JUMP:
				immed = Read2(ip);
				offset = Read2(ip);
				std::printf("\t%04hXH, %04X", immed, 
					   (offset + func_ip+1+pdesc->nbytes)&0xFFFF);
				break;
			case CALL:
				{
					func = Read2(ip);
					immed = *ip++;
					const char **func_table = bg_intrinsic_table;
					if (Game::get_game_type() == SERPENT_ISLE)
						func_table = si_intrinsic_table;
					std::printf("\t_%s@%d\t; %04X", func_table[func], immed, func);
				}
				break;
			case EXTCALL:
				{
					// Print extern call
					offset = Read2(ip);
					std::printf("\t[%04X]\t\t; %04XH", offset,
						   externals[2*offset] + 256*externals[2*offset + 1]);
				}
				break;
			case VARREF:
				// Print variable reference
				varref = Read2(ip);
				std::printf("\t[%04X]\t\t= ", varref);
				locals[varref].print(cout, true); // print value (short)
				break;
			case FLGREF:
				// Print global flag reference
				offset = Read2(ip);
				std::printf("\tflag:[%04X]\t= ", offset);
				if (gflags[offset])
					std::printf("set");
				else
					std::printf("unset");
				break;
			default:
				// Unknown type
				break;
			}
	}
	ip = param_ip; //restore IP back to opcode parameters

	// special cases:
	switch (opcode) {
	case 0x05:		// JNE.
		{
			Usecode_value val;
			if (sp <= stack)
				val = Usecode_value(0);
			else
				val = *(sp - 1); 

			if (val.is_false())
				std::printf("\t\t(jump taken)");
			else
				std::printf("\t\t(jump not taken)");
			break;
		}

		/*
		  maybe predict this?
	case 0x07:		// CMPS.
		{
			int cnt = Read2(ip);	// # strings.
			offset = (short) Read2(ip);
			bool matched = false;
			
			// only try to match if we haven't found an answer yet
			while (!matched && !found_answer && cnt-- > 0) {
				Usecode_value s = pop();
				const char *str = s.get_str_value();
				if (str && strcmp(str, user_choice) == 0) {
					matched = true;
					found_answer = true;
				}
			}
			while (cnt-- > 0)	// Pop rest of stack.
				pop();
			if (!matched)		// Jump if no match.
				ip += offset;
		}
		break;
		*/
	}

	std::printf("\n");
}
