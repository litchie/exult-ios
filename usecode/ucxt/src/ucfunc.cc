#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "ucdata.h"
#include "ucfunc.h"
#include "newopcode.h"
#include <set>
#include <strstream>
#include <algorithm>

#include "opcodes.h"

#if 0
	#define DEBUG_PARSE
	#define DEBUG_PARSE2
	#define DEBUG_PARSE2a
	#define DEBUG_READ
	#define DEBUG_PRINT
	#define DEBUG_READ_PAIR(X, Y) cout << '\t' << X << '\t' << Y << endl;
#else
	#undef DEBUG_PARSE
	#undef DEBUG_PARSE2
	#undef DEBUG_READ
	#undef DEBUG_PRINT
	#define DEBUG_READ_PAIR(X, Y)
#endif

//#define DEBUG_PARSE2
//#define DEBUG_PARSE2a
//#define DEBUG_PRINT

const string VARNAME = "var";
const string VARPREFIX = "var";
const unsigned int ASM_DISP_STR_LEN=20;

void print_asm_opcode(ostream &o, UCFunc &ucf, const FuncMap &funcmap, const vector<UCOpcodeData> &optab, const map<unsigned int, string> &intrinsics, const UCc &op);
string demunge_ocstring(UCFunc &ucf, const FuncMap &funcmap, const string &asmstr, const vector<string> &param_types, const vector<unsigned int> &params, const map<unsigned int, string> &intrinsics, const UCc &op, bool ucs_output);

/* Assumption the 'var's are in their 'zeroed' state on initialization,
   unless something else is assigned to them. */

inline ostream &tab_indent(const unsigned int indent, ostream &o)
{
	#ifdef DEBUG_PARSE2
	o << indent;
	#endif
	
	switch(indent)
	{
		case 0:                    break;
	 	case 1: o << '\t';         break;
	 	case 2: o << "\t\t";       break;
	 	case 3: o << "\t\t\t";     break;
	 	case 4: o << "\t\t\t\t";   break;
	 	case 5: o << "\t\t\t\t\t"; break;
	 	default:
			for(unsigned int i=0; i<indent; ++i) o << '\t';
			break;
	}
	return o;
}

void UCFunc::output_ucs(ostream &o, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics, bool uselesscomment, bool gnubraces)
{
	unsigned int indent=0;
	
	if(_externs.size()) tab_indent(indent, o) << "// externs" << endl;
	// output the 'externs'
	for(vector<unsigned short>::iterator e=_externs.begin(); e!=_externs.end(); e++)
	{
		FuncMap::const_iterator fmp = funcmap.find(*e);
		output_ucs_funcname(tab_indent(indent, o) << "extern ", *e, fmp->second.num_args, fmp->second.return_var) << ';' << endl;
	}
	
	if(_externs.size()) o << endl;
	
	// output the function name
	output_ucs_funcname(tab_indent(indent, o), _funcid, _num_args, _return_var) << endl;
	// start of func
	tab_indent(indent++, o) << '{' << endl;
	
	for(unsigned int i=_num_args; i<_num_args+_num_locals; i++)
		tab_indent(indent, o) << VARNAME << ' ' << VARPREFIX << setw(4) << i << ';' << endl;
		
	if(_return_var) tab_indent(indent, o) << VARNAME << ' ' << "rr" << ';' << endl;
	
	if(_num_locals>0) o << endl;
	
	output_ucs_data(o, funcmap, intrinsics, uselesscomment, indent);
	
	tab_indent(--indent, o) << '}' << endl;
}

/* outputs the general 'function name' in long format. For function
	declarations and externs */
ostream &UCFunc::output_ucs_funcname(ostream &o, unsigned int funcid,
                                     unsigned int numargs, bool return_var)
{
	// do we return a variable
	if(return_var) o << VARNAME << ' ';
	
	// output the "function name"
	// TODO: Probably want to grab this from a file in the future...
	o << "Func" << setw(4) << funcid
	// output the "function number"
	  << " 0x" << funcid
	// output ObCurly braces
	  << " (";
	
	for(unsigned int i=0; i<numargs; i++)
		o << VARNAME << ' ' << VARPREFIX << setw(4) << i << ((i==numargs-1) ? "" : ", ");
	
	o << ")";
	
	return o;
}

void UCFunc::output_ucs_data(ostream &o, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics, bool uselesscomment, unsigned int indent)
{
	for(vector<GotoSet>::iterator i=gotoset.begin(); i!=gotoset.end(); ++i)
	{
		// we don't want to output the first "jump" (the start of the function)
		if(i!=gotoset.begin())
			tab_indent(indent++, o) << setbase(16) << "labelFunc" << setw(4) << _funcid << "_" << setw(4) << i->offset() << ":" << endl;

		for(GotoSet::iterator j=(*i)().begin(); j!=(*i)().end(); j++)
		{
			const UCc &ucc = *(j->first);
			
			if(uselesscomment)
				tab_indent(indent, o) << "// Offset: " << setw(4) << ucc._offset << endl;

			output_ucs_opcode(o, funcmap, opcode_table_data, ucc, intrinsics, indent);
		}
		if(i!=gotoset.begin()) --indent; //decrement it again to skip the label statement.
		
	}
}

void UCFunc::output_ucs_opcode(ostream &o, const FuncMap &funcmap, const vector<UCOpcodeData> &optab, const UCc &op, const map<unsigned int, string> &intrinsics, unsigned int indent)
{
	tab_indent(indent, o) << demunge_ocstring(*this, funcmap, optab[op._id].ucs_nmo, optab[op._id].param_types, op._params_parsed, intrinsics, op, true) << ';' << endl;
	
	#ifdef DEBUG_PRINT
	for(vector<UCc *>::const_iterator i=op._popped.begin(); i!=op._popped.end(); i++)
	{
		if((*i)->_popped.size())
			output_ucs_opcode(o, funcmap, opcode_table_data, **i, intrinsics, indent+1);
		else
//			tab_indent(indent+1, o) << demunge_ocstring(*this, funcmap, optab[(*i)->_id].ucs_nmo, optab[(*i)->_id].param_types, op._params_parsed, **i) << endl;
			tab_indent(indent+1, o) << optab[(*i)->_id].ucs_nmo << endl;
	}
	#endif
}

void UCFunc::output_ucs_node(ostream &o, const FuncMap &funcmap, UCNode* ucn, const map<unsigned int, string> &intrinsics, unsigned int indent)
{
	//if(gnubraces) o << '\t';
	if(!ucn->nodelist.empty()) tab_indent(indent, o) << '{' << endl;
	
	//assert(ucn->ucc!=0);
	if(ucn->ucc!=0)
		print_asm_opcode(tab_indent(indent, o), *this, funcmap, opcode_table_data, intrinsics, *(ucn->ucc));
	
	if(ucn->nodelist.size())
		for(vector<UCNode *>::iterator i=ucn->nodelist.begin(); i!=ucn->nodelist.end(); i++)
		{
			//tab_indent(indent, o);
			output_ucs_node(o, funcmap, *i, intrinsics, indent+1);
		}
			
	// end of func
	//if(gnubraces) o << '\t';
	if(!ucn->nodelist.empty()) tab_indent(indent, o) << '}' << endl;
}

/* Just a quick function to remove all the ucc structured flagged as removable */
inline void gc_gotoset(vector<GotoSet> &gotoset)
{
	for(vector<GotoSet>::iterator i=gotoset.begin(); i!=gotoset.end(); i++)
	{
		i->gc();
		#ifdef DEBUG_GOTOSET
		cout << "----" << endl;
		#endif
	}
}

void UCFunc::parse_ucs(const FuncMap &funcmap, const map<unsigned int, string> &intrinsics, bool basic)
{
	for(vector<UCc>::iterator i=_opcodes.begin(); i!=_opcodes.end(); i++)
		node.nodelist.push_back(new UCNode(i));
	
	parse_ucs_pass1(node.nodelist);
	parse_ucs_pass2(gotoset, funcmap, intrinsics);
	gc_gotoset(gotoset);
	
	if(!basic)
	{
		parse_ucs_pass3(gotoset, intrinsics);
	}
	
	#ifdef DEBUG_PARSE2
	for(vector<GotoSet>::iterator i=gotoset.begin(); i!=gotoset.end(); i++)
	{
		cout << setw(4) << i->offset() << endl;
		
		for(GotoSet::iterator j=(*i)().begin(); j!=(*i)().end(); j++)
		{
			cout << '\t' << setw(4) << j->first->_offset << '\t' << j->first->_id << endl;
		}
	}
	#endif
}

/* Pass 1 turns the 1-dimentional vector of opcodes, into a 2-dimentional array
   consisting of all the opcodes within two 'goto target offsets'. */
void UCFunc::parse_ucs_pass1(vector<UCNode *> &nodes)
{
	vector<unsigned int> jumps;

	// collect jump references
	for(unsigned int i=0; i<nodes.size(); i++)
	{
		if(nodes[i]->ucc!=0)
		{
			unsigned int isjump=0;
			for(vector<pair<unsigned int, unsigned int> >::iterator op=opcode_jumps.begin(); op!=opcode_jumps.end(); op++)
				if(op->first==nodes[i]->ucc->_id)
				{
					isjump=op->second;
					break;
				}
			
			if(isjump!=0)
			{
				assert(nodes[i]->ucc->_params_parsed.size()>=isjump);
				jumps.push_back(nodes[i]->ucc->_params_parsed[isjump-1]);
			}
		}
	}

	gotoset.push_back(GotoSet());

	for(unsigned int i=0; i<nodes.size(); i++)
	{
		if(nodes[i]->ucc!=0)
		{
			if(count(jumps.begin(), jumps.end(), nodes[i]->ucc->_offset))
			{
				gotoset.push_back(nodes[i]->ucc);
			}
			else
				gotoset.back().add(nodes[i]->ucc);
		}
	}
}

/* In Pass 2 we convert our 2-dimensional 'GotoSet' array into an array with
   each UCc, having it's parameters sitting in it's UCc::_popped vector. Elements
   that are parameters are flagged for removal (Gotoset::()[i]->second=true) from
   the original GotoSet. */
void UCFunc::parse_ucs_pass2(vector<GotoSet> &gotoset, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics)
{
	for(vector<GotoSet>::iterator i=gotoset.begin(); i!=gotoset.end(); ++i)
	{
		parse_ucs_pass2a((*i)().rbegin(), (*i)(), 0, funcmap, intrinsics);
	}
}

vector<UCc *> UCFunc::parse_ucs_pass2a(vector<pair<UCc *, bool> >::reverse_iterator current, vector<pair<UCc *, bool> > &vec, unsigned int opsneeded, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics)
{
	vector<UCc *> vucc;
	unsigned int opsfound=0;
	
	#ifdef DEBUG_PARSE2
	print_asm_opcode(tab_indent(4, cout), *this, funcmap, opcode_table_data, intrinsics, *(current->first));
	#endif
	
	for(;vec.rend()!=current; current++)
	{
		#ifdef DEBUG_PARSE2
		print_asm_opcode(tab_indent(3, cout), *this, funcmap, opcode_table_data, intrinsics, *(current->first));
		#endif
		
		if(current->second==false)
		{
			if((opcode_table_data[current->first->_id].num_pop!=0) || (opcode_table_data[current->first->_id].call_effect!=0))
			{
				//if(opcode_table_data[current->first->_id].num_pop<0x7F)
				{
					#ifdef DEBUG_PARSE2
					print_asm_opcode(tab_indent(3, cout << "0x" << setw(2) << current->first->_id << "-"), *this, funcmap, opcode_table_data, intrinsics, *(current->first));
					tab_indent(3, cout << "0x" << setw(2) << current->first->_id << "-") << opcode_table_data[current->first->_id].num_pop << endl;
					#endif
					
					unsigned int num_args=0;
					
					if(opcode_table_data[current->first->_id].num_pop>0x7F)
					{
						#ifdef DEBUG_PARSE2a
						cout << "CALL EFFECT: " << opcode_table_data[current->first->_id].num_pop << '\t';
						#endif
						
						unsigned int offset = 0x100 - opcode_table_data[current->first->_id].num_pop - 1;
						num_args = current->first->_params_parsed[offset];
						
						#ifdef DEBUG_PARSE2a
						cout << num_args << endl;
						#endif
					}
					else if(opcode_table_data[current->first->_id].call_effect!=0)
					{
						assert(current->first->_params_parsed.size()>=1);
						assert(_externs.size()>=current->first->_params_parsed[0]);
						FuncMap::const_iterator fmp = funcmap.find(_externs[current->first->_params_parsed[0]]);
						assert(fmp!=funcmap.end());
						#ifdef DEBUG_PARSE2
						cout << "CALL:     " << fmp->second.funcid << '\t' << fmp->second.num_args << endl;
						#endif
						
						num_args = fmp->second.num_args;
					}
					else
					{
						#ifdef DEBUG_PARSE2
						cout << "Non-CALL: \t" << opcode_table_data[current->first->_id].num_pop << endl;
						#endif
						num_args = opcode_table_data[current->first->_id].num_pop;
					}
					
					if(num_args>0)
					{
						/* save the 'current' value as the return value and increment it so it's
						   pointing at the 'next' current value */
						vector<pair<UCc *, bool> >::reverse_iterator ret(current);
						
						ret->first->_popped = parse_ucs_pass2a(++current, vec, num_args, funcmap, intrinsics);
						
						assert(current!=ret);
						
						--current;
						
						assert(current==ret);
						#ifdef DEBUG_PARSE2a
						print_asm_opcode(tab_indent(1, cout), *this, funcmap, opcode_table_data, intrinsics, *(ret->first));
						
						for(vector<UCc *>::iterator i=ret->first->_popped.begin(); i!=ret->first->_popped.end(); i++)
							print_asm_opcode(tab_indent(2, cout), *this, funcmap, opcode_table_data, intrinsics, **i);
						#endif
					}
				}
			}
			if((opsneeded!=0) && (current->second==false))
			{
				// if it's a 'push' opcode and we need items to return that we've popped off the stack...
				if(opcode_table_data[current->first->_id].num_push!=0)
				{
					#ifdef DEBUG_PARSE2
					print_asm_opcode(tab_indent(4, cout << "P-"), *this, funcmap, opcode_table_data, intrinsics, *(current->first));
					#endif
					
					opsfound+=opcode_table_data[current->first->_id].num_push;
					vucc.push_back(current->first);
					current->second=true;
				}
				// if it's a call to a function that returns a variable...
				else if(opcode_table_data[current->first->_id].call_effect!=0)
				{
					FuncMap::const_iterator fmp = funcmap.find(_externs[current->first->_params_parsed[0]]);
					assert(fmp!=funcmap.end());
					
					if(fmp->second.return_var)
					{
						#ifdef DEBUG_PARSE2
						print_asm_opcode(tab_indent(4, cout << "C-"), *this, funcmap, opcode_table_data, intrinsics, *(current->first));
						#endif
					
						opsfound+=1;
						vucc.push_back(current->first);
						current->second=true;
					}
				}
				else
					current->second=true;
					
				// if we've found all the ops we were searching for, return them
				if(opsfound>=opsneeded)
				{
					return vucc;
				}
			}
		}
	}
	
	if(vucc.size()>0) cout << "DID NOT FIND ALL OPCODE PARAMETERS." << endl;
	return vucc;
}

/* The 'optomisation' phase. Attempting to remove as many goto...labels as possible. */
void UCFunc::parse_ucs_pass3(vector<GotoSet> &gotoset, const map<unsigned int, string> &intrinsics)
{

}

/*void UCFunc::process_code_seg(vector<unsigned char> &intrinsic_buf,
                          int mute,
                          int count_all_opcodes,
                          int count_all_intrinsic,
//                          vector<UCc> &uc_codes,
                          const char** func_table)
{
  long pos;
//  unsigned short size;
  unsigned short externsize;
  unsigned short i;
  unsigned short offset;
  unsigned char* p;
  unsigned char* pp;
  unsigned char* pdata;
  unsigned short* pextern;

  pos = ftell();
  unsigned int size = _funcsize - _datasize - SIZEOF_USHORT;
  pp = p = (unsigned char *)malloc(size);
  pdata = (unsigned char *)malloc(_datasize);
  read_vbytes(pdata, _datasize);
  _code_offset = ftell();

  read_vbytes(p, size);
  fseekbeg(pos);
  // Print code segment header
  if( size < 3 * SIZEOF_USHORT )
  {
    printf("Code segment bad!\n");
    free(p);
    free(pdata);
    return;
  }
  // Print argument counter
  _argc = read_ushort(pp);
  pp += SIZEOF_USHORT;
  // Print locals counter
  _localc = read_ushort(pp);
  pp += SIZEOF_USHORT;
  // Print externs section
  externsize = read_ushort(pp);

  pp += SIZEOF_USHORT;
  if( size < ( ( 3 + externsize ) * SIZEOF_USHORT ) )
  {
    printf("Code segment bad!\n");
    free(p);
    free(pdata);
    return;
  }
  size -= ( ( 3 + externsize ) * SIZEOF_USHORT );
  pextern = (unsigned short*)pp;
  for( i = 0; i < externsize; i++ )
  {
    _externs.push_back(read_ushort(pp));

    pp += SIZEOF_USHORT;
  }
  offset = 0;
  // Print opcodes
  while( offset < size )
  {
    unsigned int nbytes = print_opcode(pp, offset, pdata, pextern, externsize,
                              intrinsic_buf, mute,
                              count_all_opcodes,
                              count_all_intrinsic,
                              func_table);
    pp += nbytes;
    offset += nbytes;
  }
  free(p);
  free(pdata);
}

void UCFunc::process_old(ifstream *file, long func, int* found,
                          vector<unsigned char> &intrinsic_buf,
                          bool scan_mode,
                          unsigned long opcode,
                          unsigned long intrinsic,
                          unsigned int &uc_funcid,
                          const char** func_table)
{
  _file = file;

  long bodyoff;
  // Save start offset
  _offset = ftell();
  // Read function header
  _funcid = read_ushort();
  _funcsize = read_ushort();

  // Save body offset
  bodyoff = ftell();
  _datasize = read_ushort();

  if( ( _funcid == func ) || scan_mode || ( opcode != -1 ) || ( intrinsic != -1 ) )
  {
    // Only for matching function or in one of the scan modes
    if( _funcid == func )
    {
      *found = 1;
//      printf("Function at file offset %08lX\n\t.funcnumber\t%04XH\n"
//        "\t.msize\t%04XH\n\t.dsize\t%04XH\n", _offset, _funcid, _funcsize, _datasize);
      { //PATRICK
        uc_funcid = _funcid;
      }
    }
    // Dump function contents
    if( !scan_mode && ( opcode == -1 ) && ( intrinsic == -1 ) )
      process_data_seg();
//    if( opcode != -1 )
//      memset(opcode_buf, 0, 256);
    if( intrinsic != -1 ) 
    	for(unsigned int i=0; i<intrinsic_buf.size(); i++)
			intrinsic_buf[i]=0;
			
    process_code_seg(intrinsic_buf,
              scan_mode || ( opcode != -1 ) || ( intrinsic != -1 ),
              ( opcode != -1 ), ( intrinsic != -1 ),
              func_table);
    if( !scan_mode && ( opcode == -1 ) && ( intrinsic == -1 ) )
    {
      do_decompile();
      if(TEST_V3) do_print();
    }

    if( ( ( opcode != -1 ) && _unknown_opcode_count[opcode] > 0 ) ||
      ( ( intrinsic != -1 ) && intrinsic_buf[intrinsic] > 0 ) )
    {
      // Found
      *found = 1;
      if( intrinsic != -1 )
        printf("\tFound function (%04XH) - %d times\n", _funcid,
                                intrinsic_buf[intrinsic]);
      else
        printf("\tFound function (%04XH) - %d times\n", _funcid, _unknown_opcode_count[opcode]);
    }
  }

  genflags();
  // Seek back, then to next function
  fseekbeg(bodyoff);
  fseekcur(_funcsize);
}
*/

/*void process_func(FILE* f, long func, int i, int* found,
                            unsigned char* opcode_buf,
                          unsigned char* intrinsic_buf,
                          int scan_mode,
                          unsigned long opcode,
                          unsigned long intrinsic,
                          unsigned int &uc_funcid,
                          map<unsigned int, string, less<unsigned int> > &uc_data,
                          unsigned int &uc_argc,
                          unsigned int &uc_localc,
                          vector<unsigned int> &uc_externs,
                          vector<UCc> &uc_codes,
                          const char** func_table)
{
  unsigned short s, ds, funcnum;  
  long off, bodyoff;
   * Save start offset *
  off = ftell(f);
   * Read function header *
  fread(&funcnum, sizeof(unsigned short), 1, f);
  fread(&s, sizeof(unsigned short), 1, f);
   * Save body offset *
  bodyoff = ftell(f);
  fread(&ds, sizeof(unsigned short), 1, f);
  if( ( ( func == -1 ) || scan_mode ) && ( opcode == -1 ) && ( intrinsic == -1 ) )
  {
    //cout << "Function     offset    size  data  diff" << endl;
     * Only for general list & scan mode *
    printf("#%03d: %04XH  %08lx  %04x  %04x  %04x\n", i,
                                    funcnum, off, s, ds, s-ds);
  }
  if( ( funcnum == func ) || scan_mode || ( opcode != -1 ) || ( intrinsic != -1 ) )
  {
     * Only for matching function or in one of the scan modes *
    if( funcnum == func )
    {
      *found = 1;
      printf("Function at file offset %08lX\n\t.funcnumber\t%04XH\n"
        "\t.msize\t%04XH\n\t.dsize\t%04XH\n",
        off, funcnum, s, ds);
      { //PATRICK
        uc_funcid = funcnum;
      }
    }
     * Dump function contents *
    if( !scan_mode && ( opcode == -1 ) && ( intrinsic == -1 ) )
      process_data_seg(f, ds, uc_data);
    if( opcode != -1 ) 
      memset(opcode_buf, 0, 256);
    if( intrinsic != -1 ) 
      memset(intrinsic_buf, 0, 256);
    process_code_seg(f, ds, s, opcode_buf, intrinsic_buf,
              scan_mode || ( opcode != -1 ) || ( intrinsic != -1 ),
              ( opcode != -1 ), ( intrinsic != -1 ),
              uc_argc, uc_localc, uc_externs, uc_codes, func_table);
    if( ( ( opcode != -1 ) && opcode_buf[opcode] > 0 ) ||
      ( ( intrinsic != -1 ) && intrinsic_buf[intrinsic] > 0 ) )
    {
       * Found *
      *found = 1;
      if( intrinsic != -1 )
        printf("\tFound function (%04XH) - %d times\n", funcnum,
                                intrinsic_buf[intrinsic]);
      else
        printf("\tFound function (%04XH) - %d times\n", funcnum, opcode_buf[opcode]);
    }
  }
   * Seek back, then to next function *
  fseek(f, bodyoff, SEEK_SET);
  fseek(f, s, SEEK_CUR);
}*/


/*unsigned short UCFunc::read_ushort()
{
  return ((unsigned short) ((unsigned int)get() + (((unsigned int)get()) << 8)));
}

unsigned short UCFunc::read_ushort(const unsigned char *buff)
{
//  assert(((unsigned short) ((unsigned int)buff[0] + (((unsigned int)buff[1]) << 8)))
//         ==(*(unsigned short *)buff));
  return ((unsigned short) ((unsigned int)buff[0] + (((unsigned int)buff[1]) << 8)));
}

void UCFunc::read_vchars(char *buffer, const unsigned long nobytes)
{
	_file->read(buffer, nobytes);
}

void UCFunc::read_vbytes(unsigned char *buffer, const unsigned long nobytes)
{
	_file->read(buffer, nobytes);
}

void UCFunc::do_decompile()
{
  // TODO: Generate Unknown opcode count (_unknown_opcode_count)
  // TODO: Generate opcode count (_opcode_count)
  //       eg: _opcode_count[ucc._id]++;

  genflags();

  // setup the extern values
  for(unsigned int i=0; i<_raw_opcodes.size(); i++)
    _raw_opcodes[i]->pass1(_externs);

  // setup the string values
  for(unsigned int i=0; i<_raw_opcodes.size(); i++)
    _raw_opcodes[i]->pass2(_data);

  re_order();
}

void UCFunc::do_print()
{
//  for(unsigned int i=0; i<_raw_opcodes.size(); i++)
//    _raw_opcodes[i]->print_asm(cout);

  print_asm();
  print_c_externs();
  print_c_head();
  print_c_local();
  print_c_body();
  print_c_tail();

  cout << "----------" << endl << endl << endl;
}

void UCFunc::re_order()
{
  set<unsigned int> _jumps;

  // create a list of jump targets
  for(unsigned int i=0; i<_raw_opcodes.size(); i++)
  {
    if(_raw_opcodes[i]->IsaRelativeJump())
      _jumps.insert(dynamic_cast<OpcodeGenericJump *>(_raw_opcodes[i])->target_offset());
  }

  // create the "first" label from the zeroth offset
  Label *currlbl = new Label(_funcid, 0);

  //for each opcode
  for(unsigned int i=0; i<_raw_opcodes.size(); i++)
  {
    // test if the offset is a jump target
    for(set<unsigned int>::iterator j=_jumps.begin(); j!=_jumps.end(); j++)
    {
      // create a new label
      if(_raw_opcodes[i]->offset()==*j)
      {
        _opcodes.push_back(currlbl);
        currlbl=0;
        currlbl = new Label(_funcid, _raw_opcodes[i]->offset());
        break;
      }
    }
    // add the opcode to the current label
    (*currlbl)().push_back(_raw_opcodes[i]);
  }

  // make sure we add the last one
  if((*currlbl)().size()!=0)
    _opcodes.push_back(currlbl);
  currlbl=0;
}

// prints out the external function calls in C output
void UCFunc::print_c_externs()
{
  strstream ss_externs;

  ss_externs << "// externs" << endl;

  for(unsigned int i=0; i<_externs.size(); i++)
    ss_externs << "void " << extern_tostr(_externs[i]) << ";" << endl;
  ss_externs << endl << ends;

  cout << ss_externs.str();
}

string UCFunc::extern_tostr(const unsigned int uc_extern)
{
  strstream ss;
  ss << setfill('0') << setbase(16) << "Func" << setw(4) << uc_extern << "()" << ends;
  return ss.str();
}
*/
/* prints the "head" of the C function, eg:
     void Func0001(uvar var0001)
     {
*/
/*void UCFunc::print_c_head()
{
    strstream ss_func_head;

    ss_func_head << setbase(16) << setfill('0');

    // TODO: replace this "void" with the data type when we find it out
    ss_func_head << "void";

    ss_func_head << " Func" << setw(4) << _funcid << "(";

    for(unsigned int i=0; i<_argc; i++)
    {
      if(i!=0)
        ss_func_head << ", ";
      ss_func_head << VARNAME << " " << VARPREFIX << setw(4) << i;
    }
    ss_func_head << ")" << endl
                 << "{" << endl << ends;

    // TODO: indent the function by two characters
    //uc_indentlvl+=2;

    cout << ss_func_head.str();
}
*/
/* outputs the local variable definitions of the C output */

/*void UCFunc::print_c_local()
{
  strstream strlocal;

  strlocal << setfill('0') << setbase(16);

  vector<unsigned int> ocvi;
  MiscOpcode oc(1, 1, ocvi);
  oc.indent_inc();

  if(_localc)
  {
    for(unsigned int i=_argc; i<(_argc+_localc); i++)
    {
      // TODO: more indenting fuss
      oc.indent(strlocal);
      strlocal << VARNAME << " " << VARPREFIX << setw(4) << i << ";" << endl;
    }
    strlocal << endl;
  }
  strlocal << ends;

  cout << strlocal.str() << endl;
}*/

/* prints out the body of the C function, should be RELATIVLY simple,
   since most of the work should be done */
/*void UCFunc::print_c_body()
{
  cout << endl;
  for(unsigned int i=0; i<_opcodes.size(); i++)
  {
    cout << "-----" << endl;
    _opcodes[i]->print_c(cout);
  }
}*/

/* prints out the tail of the C function */
/*void UCFunc::print_c_tail()
{
  vector<unsigned int> ocvi;
  MiscOpcode oc(1, 1, ocvi);
  oc.indent_dec();

  cout << endl << "}" << endl << endl;
}*/

/*void UCFunc::genflags(const vector<UCc> &uc_codes)
{
  cout << uc_codes.size() << endl;
  for(unsigned int i=0; i<uc_codes.size(); i++)
  {
    if(uc_codes[i]._id==UCC_PUSHF)
      _flagcount.push_back(new FlagData(_funcid, uc_codes[i]._offset, (((unsigned int)uc_codes[i]._params[1])<<8) + (unsigned int)uc_codes[i]._params[0], FlagData::GETFLAG));
    else if(uc_codes[i]._id==UCC_POPF)
      _flagcount.push_back(new FlagData(_funcid, uc_codes[i]._offset, (((unsigned int)uc_codes[i]._params[1])<<8) + (unsigned int)uc_codes[i]._params[0], FlagData::SETFLAG));
  }

}*/

/*void UCFunc::genflags()
{
  //cout << "flags genned" << endl;
  //cout << "gen flags func: " << _funcid << "(" << _raw_opcodes.size() << ")" << endl;

  for(unsigned int i=0; i<_raw_opcodes.size(); i++)
  {
    if(_raw_opcodes[i]->IsaFlag())
    {
      assert(_raw_opcodes[i]->I_Push() || _raw_opcodes[i]->I_Pop());
      assert(!(_raw_opcodes[i]->I_Push() && _raw_opcodes[i]->I_Pop()));

      bool action=true;
      if(_raw_opcodes[i]->I_Push())     action=FlagData::PUSH;
      else if(_raw_opcodes[i]->I_Pop()) action=FlagData::POP;

      //cout << "    " << setw(4) << _funcid << " " << _raw_opcodes[i]->offset() << " "
      //     << (dynamic_cast<OpcodeGenericFlag *> (_raw_opcodes[i]))->flag() << " "
      //     << (action==FlagData::POP ? "pop" : "push") << endl;
    }
  }
}*/

/* prints the "assembler" output of the usecode, currently trying to duplicate
   the output of the original ucdump... */
/*void UCFunc::print_asm()
{
  cout << "Code segment at file offset " << setw(8) << _code_offset << "H" << endl;
  cout << Opcode::SPACER << ".funcnumber" << Opcode::SPACER << setw(4) << _funcid << "H" << endl;
  cout << Opcode::SPACER << ".msize" << Opcode::SPACER << setw(4) << _funcsize << "H" << endl;
  cout << Opcode::SPACER << ".dsize" << Opcode::SPACER << setw(4) << _datasize << "H" << endl;

  // debugging remove comments!
  if(_data.size())
    print_asm_data();

  cout << "Code segment at file offset " << setw(8) << _code_offset << endl;

  cout << Opcode::SPACER << Opcode::SPACER << ".argc " << setw(4) << _argc << "H" << endl;
  cout << Opcode::SPACER << Opcode::SPACER << ".localc " << setw(4) << _localc << "H" << endl;
  cout << Opcode::SPACER << Opcode::SPACER << ".externsize " << setw(4) << _externs.size() << "H" << endl;

  for(unsigned int i=0; i<_externs.size(); i++)
    cout << Opcode::SPACER << Opcode::SPACER << ".extern " << setw(4) << _externs[i] << "H" << endl;

  for(unsigned int i=0; i<_opcodes.size(); i++)
  {
    cout << "-----" << endl;
    _opcodes[i]->print_asm(cout);
  }
}*/

/*void UCFunc::print_asm_data()
{
  // limit of about 70 chars to a line, wrap to the next line if longer then this...
  for(map<unsigned int, string, less<unsigned int> >::iterator i=_data.begin(); i!=_data.end(); i++)
  {
    for(unsigned int j=0; j<i->second.size(); j++)
    {
      if(j==0)
        cout << setw(4) << i->first;
      if((j!=0) && !(j%70))
        cout << "'" << endl;
      if(!(j%70))
        cout << Opcode::SPACER << "db" << Opcode::SPACER << "'";

      cout << i->second[j];
    }
    cout << "'" << endl;
    cout << Opcode::SPACER << "db" << Opcode::SPACER << "00" << endl;
  }
}*/

/*unsigned short UCFunc::read_ushort(const unsigned char *buff)
{
//  assert(((unsigned short) ((unsigned int)buff[0] + (((unsigned int)buff[1]) << 8)))
//         ==(*(unsigned short *)buff));
  return ((unsigned short) ((unsigned int)buff[0] + (((unsigned int)buff[1]) << 8)));
}

void UCFunc::read_vchars(char *buffer, const unsigned long nobytes)
{
	_file->read(buffer, nobytes);
}

void UCFunc::read_vbytes(unsigned char *buffer, const unsigned long nobytes)
{
	_file->read(buffer, nobytes);
}*/

inline unsigned short read_ushort(ifstream &f)
{
  return ((unsigned short) ((unsigned int)f.get() + (((unsigned int)f.get()) << 8)));
}

inline unsigned char read_ubyte(ifstream &f)
{
  return (unsigned char)f.get();
}

void ucc_parse_parambytes(UCc &ucop, const UCOpcodeData &otd);

void readbin_UCFunc(ifstream &f, UCFunc &ucf)
{

	// offset to start of function
	ucf._offset = f.tellg();
	DEBUG_READ_PAIR("Offset: ", ucf._offset);

	// Read Function Header
	
	ucf._funcid = read_ushort(f);
	DEBUG_READ_PAIR("FuncID: ", ucf._funcid);
	ucf._funcsize = read_ushort(f);	
	DEBUG_READ_PAIR("FuncSize: ", ucf._funcsize);
	
	// save body offset in case we need it
	ucf._bodyoffset = f.tellg();
	
	ucf._datasize = read_ushort(f);
	
	// process ze data segment!
	{
		streampos pos = f.tellg(); // paranoia
	
		unsigned short off = 0;
		// Load all strings & their offsets
		while( off < ucf._datasize )
		{
			assert(!f.eof());
	
			char c;
			string data;
	
			while((c=f.get())!=0x00)
				data+=c;
	
			ucf._data.insert(pair<unsigned int, string>(off, data));
	
			off+=data.size()+1;
	
		}
		f.seekg(pos, ios::beg); // paranoia
		f.seekg(ucf._datasize, ios::cur); // paranoia
	}

	#if 0	
	if(ucf._funcid==_search_func)
		for(map<unsigned int, string>::iterator i=ucf._data.begin(); i!=ucf._data.end(); i++)
			cout << i->first << "\t" << i->second << endl;
	#endif
	
	// process code segment
	{
		//streampos start_of_code_seg = f.tellg(); // what's this used for?
		ucf._codeoffset = f.tellg();

		// get the number of arguments to the function
		ucf._num_args = read_ushort(f);

		// get the number of local variables
		ucf._num_locals = read_ushort(f);

		// get the number of external function numbers
		ucf._num_externs = read_ushort(f);
		
		// load the external function numbers
		for(unsigned int i=0; i<ucf._num_externs; i++)
			ucf._externs.push_back(read_ushort(f));
		
		// ok, now to load the usecode
		unsigned int code_offset=0;

		unsigned int code_size = ucf._funcsize - ucf._datasize - ((3+ucf._num_externs) * SIZEOF_USHORT);
		
		DEBUG_READ_PAIR("Code Size: ", code_size);

		while(code_offset<(code_size-2)) //TODO: Why the -2?!? it doesn't work otherwise
		{
			UCc ucop;

			ucop._offset = code_offset;

			ucop._id = read_ubyte(f);
			code_offset++;

			const UCOpcodeData &otd = opcode_table_data[ucop._id];

			//assert(((otd.asm_nmo.size()!=0) && (otd.ucs_nmo.size()!=0)));
			for(unsigned int i=0; i<otd.num_bytes; i++)
				ucop._params.push_back(read_ubyte(f));

			// parse the parameters
			ucc_parse_parambytes(ucop, otd);

			code_offset+=otd.num_bytes;

			/* if we're an opcode that sets a return value, we need to mark the
				function as one that returns a value */
			if(otd.flag_return==true)
				ucf._return_var=true;
			
			ucf._opcodes.push_back(ucop);

			#ifdef DEBUG_READ
			cout << setw(4) << code_size << "\t" << setw(4) << code_offset << "\t" << setw(4) << (unsigned int)ucop._offset << "\t" << setw(2) << (unsigned int)ucop._id << "\t";
			for(unsigned int i=0; i<ucop._params.size(); i++)
				cout << setw(2) << (unsigned int)ucop._params[i] << ',';
			cout << endl;
			#endif
		}
	}
}

/* calculates the relative offset jump location, used in opcodes jmp && jne */
inline int calcreloffset(const UCc &op, unsigned int param)
{
	/* forumla:
	   real offset = offset of start of current opcode
	               + int of parameter (since you can jump backwards)
	               + 1 (size of "opcode")
	               + size of "opcode" parameter data
	   NOTE: since param is unsigned, a twos-complimant is required:
	         formula: 0xFFFF - (unsigned short)param + 1
	                  ^^^^^^ max of unsighed short
	*/
	return op._offset + ((param>>15) ? (-1 * (0xFFFF - (unsigned short)param + 1)) : (int)param) + 1 + op._params.size();
}

void ucc_parse_parambytes(UCc &ucop, const UCOpcodeData &otd)
{
	unsigned int first=0;
	
	for(vector<pair<unsigned int, bool> >::const_iterator s=otd.param_sizes.begin(); s!=otd.param_sizes.end(); ++s)
	{
		assert(first<ucop._params.size());
		
		unsigned int ssize=s->first;
		bool offset_munge=s->second;
		
		assert(ssize!=0);

		if(ssize==1)
			ucop._params_parsed.push_back((unsigned short)((unsigned int)ucop._params[first++]));
		else if(ssize==2)
			if(offset_munge)
			{
				unsigned int reloffset = calcreloffset(ucop, (unsigned short) ((unsigned int)ucop._params[first++] + (((unsigned int)ucop._params[first++]) << 8)));
				ucop._params_parsed.push_back(reloffset);
				ucop._jump_offsets.push_back(reloffset);
			}
			else
				ucop._params_parsed.push_back((unsigned short) ((unsigned int)ucop._params[first++] + (((unsigned int)ucop._params[first++]) << 8)));
	}
}


void print_asm_data(UCFunc &ucf, ostream &o);
void print_asm_opcodes(ostream &o, UCFunc &ucf, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics, const vector<UCOpcodeData> &optab);

/* prints the "assembler" output of the usecode, currently trying to duplicate
   the output of the original ucdump... */
void print_asm(UCFunc &ucf, ostream &o, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics, const UCData &uc)
{
	if(uc.verbose()) cout << "Printing function..." << endl;

	o << "Function at file offset " << setw(8) << ucf._offset << "H" << endl;
	o << "\t.funcnumber " << setw(4) << ucf._funcid << "H" << endl;
	o << "\t.msize      " << setw(4) << ucf._funcsize << "H" << endl;
	o << "\t.dsize      " << setw(4) << ucf._datasize << "H" << endl;

	// debugging remove comments!
	if(ucf._data.size())
    print_asm_data(ucf, o);

	o << "Code segment at file offset " << setw(8) << ucf._codeoffset << "H" << endl;
	o << "\t.argc       " << setw(4) << ucf._num_args << "H" << endl;
	o << "\t.localc     " << setw(4) << ucf._num_locals << "H" << endl;
	o << "\t.externsize " << setw(4) << ucf._externs.size() << "H" << endl;

	for(unsigned int i=0; i<ucf._externs.size(); i++)
		o << Opcode::SPACER << "  .extern   " << setw(4) << ucf._externs[i] << "H" << endl;

	//o << "-----" << endl;
    //_opcodes[i]->print_asm(cout);
	for(vector<UCc>::iterator op=ucf._opcodes.begin(); op!=ucf._opcodes.end(); op++)
		print_asm_opcode(o, ucf, funcmap, opcode_table_data, intrinsics, *op);
}

void print_asm_data(UCFunc &ucf, ostream &o)
{
	static const unsigned int nochars=60;
	// limit of about 60 chars to a line, wrap to the next line if longer then this...
	for(map<unsigned int, string, less<unsigned int> >::iterator i=ucf._data.begin(); i!=ucf._data.end(); i++)
	{
		for(unsigned int j=0; j<i->second.size(); j++)
		{
			if(j==0)
				o << setw(4) << i->first;
			if((j!=0) && !(j%nochars))
				o << "'" << endl;
			if(!(j%nochars))
				o << "\tdb\t'";

			o << i->second[j];
		}
		o << "'" << endl;
		o << "\tdb\t00" << endl;
	}
}

void output_raw_opcodes(ostream &o, const UCc &op);

extern UCData uc;

void print_asm_opcode(ostream &o, UCFunc &ucf, const FuncMap &funcmap, const vector<UCOpcodeData> &optab, const map<unsigned int, string> &intrinsics, const UCc &op)
{
	// offset
	o << setw(4) << op._offset << ':';

	if(uc.rawops()) output_raw_opcodes(o, op);
	else            o << '\t';

	o << demunge_ocstring(ucf, funcmap, optab[op._id].asm_nmo, optab[op._id].param_types, op._params_parsed, intrinsics, op, false);

	if(uc.autocomment())
		o << demunge_ocstring(ucf, funcmap, optab[op._id].asm_comment, optab[op._id].param_types, op._params_parsed, intrinsics, op, false);

	o << endl;
}

void output_raw_opcodes(ostream &o, const UCc &op)
{
	// chars in opcode
	o << ' ' << setw(2) << (unsigned int)op._id;
	if(op._params.size()) cout << ' ';

	for(unsigned int i=0; i<op._params.size(); i++)
	{
		o << setw(2) << (unsigned int) op._params[i];
		if(i!=op._params.size())
			o << ' ';
	}

	// seperator
	unsigned int numsep = op._params.size();
	//cout << endl << numsep << endl;
	if(numsep>6)
		o << endl << "\t\t\t";
	else if (numsep>5)
		o << " ";
	else if (numsep>2)
		o << "\t";
	else
		o << "\t\t";
}

inline unsigned int charnum2uint(const char c)
{
	switch(c)
	{
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		default:  return 0;
	}
	return 0; // can't happen
}

string demunge_ocstring(UCFunc &ucf, const FuncMap &funcmap, const string &asmstr, const vector<string> &param_types, const vector<unsigned int> &params, const map<unsigned int, string> &intrinsics, const UCc &op, bool ucs_output)
{
	strstream str;
	str << setfill('0') << setbase(16);
	str.setf(ios::uppercase);
	size_t	len=asmstr.length();

	if(len==0) return string(); // for the degenerate case

	bool finished=false; // terminating details are at end-of-while
	unsigned int i=0; // istr index
	unsigned int width=0; // width value for setw()

	if(ucs_output && opcode_table_data[op._id].flag_paren) str << '(';
	
	while(!finished&&i<len)
	{
		bool special_call(false); // FIXME: <sigh> temporary exception handling for call (0x24)
		
		char c = asmstr[i];
		
		width = 4; // with defaults to 4
		
		switch(c)
		{
			case '\\':
				i++;
				c = asmstr[i];
				switch(c)
				{
					case '\\': str << '\\'; break;
					case 'n':  str << '\n'; break;
					case 't':  str << '\t'; break;
					case '\'': str << '\''; break;
					case '"':  str << '\"'; break;
					case 'b':  // bell is invalid
					default:   // we'll silently drop errors... it's the only "clean" way
						str << '\\' << c;
				}
				break;
			case '%':
				{
					i++;
					c = asmstr[i];

					// if it's a "byte" set width to 2, and get the next char
					if(c=='b')      { i++; c = asmstr[i]; width=2; }
					// if it's a "short" set width to 4, and get the next char
					else if(c=='s') { i++; c = asmstr[i]; width=4; }
					// if it's the character representation of a text data string we want
					else if(c=='t')
					{
						bool commentformat=false;
						i++; c = asmstr[i];
						
						// if we only want to output the 'short' format of the text (comment format)
						if(c=='c')
						{
							commentformat=true;
							i++; c = asmstr[i];
						}
						
						unsigned int t = charnum2uint(c);
						
						assert(params.size()>=t);
						assert(t!=0);
						string s = ucf._data.find(params[t-1])->second;
						
						if(commentformat)
							if(s.size()>17) s = s.substr(0, 17) + string("...");
						
						// escape the appropriate characters...
						// we'll only do it in the 'full' text output for the moment.
						if(!commentformat)
							for(string::iterator z=s.begin(); z!=s.end(); z++)
								if(((*z)=='\"') || ((*z)=='\\'))
								{
									z = s.insert(z, 1, '\\');
									++z;
								}
						
						str << s;
						break;
					}
					// if it's the intrinsic name we want
					else if(c=='i')
					{
						i++; c = asmstr[i];
						unsigned int t = charnum2uint(c);
						
						assert(params.size()>=t);
						assert(t!=0);
						string s = intrinsics.find(params[t-1])->second;
						str << s;
						break;
					}
					// if it's external function name we want
					else if(c=='f')
					{
						i++; c = asmstr[i];
						
						if(c=='*')
						{
							str << "Func" << setw(4) << ucf._funcid;
						}
						else
						{
							unsigned int t = charnum2uint(c);
							
							assert(ucf._externs.size()>=t);
							assert(t!=0);
							assert(op._params_parsed.size()>=1);
							str << "Func" << setw(4) << ucf._externs[op._params_parsed[t-1]];
						}
						break;
					}
					// if it's the character representation of a text data string we want
					else if(c=='p')
					{
						i++; c = asmstr[i];
						unsigned int t = charnum2uint(c);
						
						// FIXME: this is the special 'call' case, it may be a good idea to make more general
						if((t==0) && (c==','))
						{
							special_call=true;
						
							for(vector<UCc *>::const_iterator i=op._popped.begin(); i!=op._popped.end();)
							{
								str << demunge_ocstring(ucf, funcmap, opcode_table_data[(*i)->_id].ucs_nmo, opcode_table_data[(*i)->_id].param_types, (*i)->_params_parsed, intrinsics, **i, ucs_output);
								if(++i!=op._popped.end())
									str << ", ";
							}
						}
						
						if(t!=0)
						{
							if(t>op._popped.size())
								str << "SOMETHING_GOES_HERE()";
							else
							{
								UCc &ucc(*op._popped[t-1]);
								str << demunge_ocstring(ucf, funcmap, opcode_table_data[ucc._id].ucs_nmo, opcode_table_data[ucc._id].param_types, ucc._params_parsed, intrinsics, ucc, ucs_output);
							}
						}
						break;
					}
					
					if(special_call!=true)
					{
						unsigned int t = charnum2uint(c);
						if(t!=0)
						{
							assert(params.size()>=t);
							str << setw(width) << params[t-1];
						}
						else if(c=='%')
							str << '%';
					}
				}
				break;
			default: // it's just a character, leave it be
				str << c;
		}

		i++;
		if(i==asmstr.size()) finished=true;
	}
	
	if(ucs_output && opcode_table_data[op._id].flag_paren) str << ')';
	
	str << ends;
	return str.str();
}

