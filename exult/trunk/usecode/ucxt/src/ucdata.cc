
#include "ucdata.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <vector>
#include "printucc.h"
#include <algorithm>

UCData::UCData()  : _search_opcode((unsigned long) -1),
                    _search_intrinsic((unsigned long) -1),
                    _search_func((unsigned long)-1), _mode(0), 
                    _filesize(0)

{
  memset(_opcode_buf, 0, MAX_OPCODE_BUF);
  memset(_intrinsic_buf, 0, MAX_INTRINSIC_BUF);
}

UCData::~UCData()
{
  fclose(_file);
  for(unsigned int i=0; i<_funcs.size(); i++)
    delete _funcs[i];
}

void UCData::dump_unknowns()
{
  /* Dump unknowns */
  if( ( _mode == MODE_DISASSEMBLY ) || ( _mode == MODE_OPCODE_SCAN ) )
  {
    if( _opcode_buf )
    {
      int found = 0;
      for( unsigned int i=0; i<MAX_OPCODE_BUF; i++ )
        if( _opcode_buf[i] )
        {
          if( !found )
          {
            cout << "Undefined opcodes found" << endl;
            found = 1;
          }
          cout << "0x" << setbase(16) << setfill('0') << setw(2) << i
               << " (" << setbase(10) << (unsigned int)_opcode_buf[i]
               << " time" << ((_intrinsic_buf[i]>1) ? "s" : "") << ")" << endl;
          //printf("0x%02lx (%d times)\n", i, _opcode_buf[i]);
        }
    }
    if( _intrinsic_buf )
    {
      int found = 0;
      for( unsigned int i=0; i<MAX_INTRINSIC_BUF; i++ )
        if( _intrinsic_buf[i] )
        {
          if( !found )
          {
            cout << "Undefined intrinsic functions found" << endl;
            found = 1;
          }
          cout << "0x" << setbase(16) << setfill('0') << setw(2) << i
               << " (" << setbase(10) << (unsigned int)_intrinsic_buf[i]
               << " time" << ((_intrinsic_buf[i]>1) ? "s" : "") << ")" << endl;
          //printf("0x%02lx (%d times)\n", i, _intrinsic_buf[i]);
        }
    }
  }
}

void UCData::parse_params(const int argc, char **argv)
{
  /* Parse command line */
/*  if( argc == 3 )
  {
    if( !strcmp(argv[1], "-o") )
    {
      char* stopstr;
      // Opcode search
      _search_opcode = strtoul(argv[2], &stopstr, 16);
      if( stopstr - argv[2] < strlen(argv[2]) )
        _search_opcode = (unsigned long) -1;
      else
        // Hex opcode OK
        _mode = MODE_OPCODE_SEARCH;
    }
    else if( !strcmp(argv[1], "-i") )
    {
      char* stopstr;
      // Intrinsic function search
      _search_intrinsic = strtoul(argv[2], &stopstr, 16);
      if( stopstr - argv[2] < strlen(argv[2]) )
        _search_intrinsic = (unsigned long) -1;
      else
        // Hex opcode OK
        _mode = MODE_INTRINSIC_SEARCH;
    }
  }
  else*/
  if( argc == 2 )
  {
    if(!strcmp(argv[1], "-l")) // List mode
      _mode = MODE_LIST;
    if(!strcmp(argv[1], "-a")) // List mode
      _mode = MODE_DISASSEMBLE_ALL;
    /*else if( !strcmp(argv[1], "-c") )
      // Opcode scan mode
      _mode = MODE_OPCODE_SCAN;*/
    else if(!strcmp(argv[1], "-f"))
      _mode = MODE_FLAG_DUMP;
    else
    {
      char* stopstr;
      /* Disassembly mode */
      _search_func = strtoul(argv[1], &stopstr, 16);
      if( stopstr - argv[1] < strlen(argv[1]) )
        /* Invalid number */
        _search_func = (unsigned long) -1;
      else
        _mode = MODE_DISASSEMBLY;
    }
  }
}

void UCData::open_usecode(const string &filename)
{
  /* Open a usecode file */
  _file = fopen(filename.c_str(), "rb");
  if( _file == NULL )
  {
    cout << "Failed to open usecode file: " << filename << endl << endl;
    exit(-2);
  }
  fseek(_file, 0, SEEK_END);
  _filesize = ftell(_file);
  fseek(_file, 0, SEEK_SET);

}

void UCData::disassamble(const char **func_table)
{
  load_funcs(func_table);

  cout << "Looking for function number " << setw(8) << _search_func << endl << endl;

  if(mode()==MODE_DISASSEMBLY)
    for(unsigned int i=0; i<_funcs.size(); i++)
    {
      //cout << _funcs[i]->funcid() << ":" << _funcid << endl;
      if(_funcs[i]->funcid()==_funcid)
      {
        output_ucfunc(_funcid, _funcs[i]->data(), _funcs[i]->argc(), _funcs[i]->localc(), _funcs[i]->externs(),
                      _funcs[i]->codes(), func_table);
        break;
      }
      else if(i==_funcs.size()-1)
        printf("Function not found.\n");
    }

  if( _search_func == -1 )
  {
    if( ftell(_file) != filesize() )
      printf("Problem, tell = %d!\n", ftell(_file));
    printf("Functions: %d\n", _funcs.size());
  }

//  if(uc.mode()==MODE_OPCODE_SEARCH)
//    output_flags(funcs);

  dump_unknowns();

  // GC
}

void UCData::disassamble_all(const char ** func_table)
{
  cout << "Loading Funcs..." << endl;
  load_funcs(func_table);
  cout << "Funcs Loaded..." << endl;

  for(unsigned int i=0; i<_funcs.size(); i++)
  {
    cout << "Outputting " << i << endl;
    output_ucfunc(_funcs[i]->funcid(), _funcs[i]->data(), _funcs[i]->argc(), _funcs[i]->localc(), _funcs[i]->externs(),
                  _funcs[i]->codes(), func_table);
    cout << "Outputted " << i << endl;
  }

  dump_unknowns();
}

void UCData::load_funcs(const char **func_table)
{
//  int num_functions = 0;
  int found = 0;

  while( ftell(_file) < filesize() )
  {
    UCFunc *ucfunc = new UCFunc();

/*    if( ( ( uc._search_func == -1 )
        || ( uc.mode() == MODE_OPCODE_SCAN ) )
       && ( uc._search_opcode == -1 ) && ( uc._search_intrinsic == -1 ) )
      cout << "#" << setbase(10) << setw(3) << func.size() * current function number* << setbase(16) << ": ";*/

    ucfunc->process_old(_file, _search_func, &found, /*_opcode_buf,*/ _intrinsic_buf, ( mode() == MODE_OPCODE_SCAN ),
                    _search_opcode, _search_intrinsic,
                    _funcid, /*_codes,*/ func_table);
//    if( ( ( uc.mode() != MODE_OPCODE_SEARCH ) && ( uc.mode() !=MODE_INTRINSIC_SEARCH ) ) || found )
//      num_functions++;
//    if( ( uc.mode() == MODE_OPCODE_SEARCH ) || ( uc.mode() == MODE_INTRINSIC_SEARCH ) )
//      found = 0;

    _funcs.push_back(ucfunc);
  }
}

void UCData::dump_flags(const char **func_table)
{
  load_funcs(func_table);

/*  // don't need to delete "flags" the pointers are already owned.
  vector<FlagData *> flags;

  // *BLEH* ugly!
  for(unsigned int i=0; i<_funcs.size(); i++)
    for(unsigned int j=0; j<_funcs[i]->flags().size(); j++)
      flags.push_back(_funcs[i]->flags()[j]);

  cout << "Number of flags found: " << setbase(10) << flags.size() << endl;

  // output per function
  {
    sort(flags.begin(), flags.end(), SortFlagDataLessFunc());

    cout << setbase(16) << setfill('0');
    unsigned int currfunc = (unsigned int)-1;
    for(unsigned int i=0; i<flags.size(); i++)
    {
      if(currfunc!=flags[i]->func())
      {
        cout << "Function: " << setw(4) << flags[i]->func() << endl;
        currfunc=flags[i]->func();
        cout << "              flag  offset" << endl;
      }

      cout << "        ";
      if(flags[i]->access()==FlagData::GETFLAG)
        cout << "push  ";
      else if(flags[i]->access()==FlagData::SETFLAG)
        cout << "pop   ";
      cout << setw(4) << flags[i]->flag()   << "  "
           << setw(4) << flags[i]->offset() << endl;
    }
  }
  // output per flag
  {
    sort(flags.begin(), flags.end(), SortFlagDataLessFlag());

    cout << setbase(16) << setfill('0');
    unsigned int currflag = (unsigned int)-1;
    for(unsigned int i=0; i<flags.size(); i++)
    {
      if(currflag!=flags[i]->flag())
      {
        cout << "Flag: " << setw(4) << flags[i]->flag() << endl;
        currflag=flags[i]->flag();
        cout << "              func  offset" << endl;
      }

      cout << "        ";
      if(flags[i]->access()==FlagData::GETFLAG)
        cout << "push  ";
      else if(flags[i]->access()==FlagData::SETFLAG)
        cout << "pop   ";
      cout << setw(4) << flags[i]->func()   << "  "
           << setw(4) << flags[i]->offset() << endl;
    }
  }
  // don't need to delete "flags" the pointers are already owned.*/
}

void UCData::list_funcs(const char **func_table)
{
  load_funcs(func_table);

/*  if( ( ( uc._search_func == -1 ) || (uc.mode()==MODE_OPCODE_SCAN) )
     && ( uc._search_opcode == -1 ) && ( uc._search_intrinsic == -1 ) )*/
    cout << "Function     offset    size  data  code" << endl;

  for(unsigned int i=0; i<_funcs.size(); i++)
  {
    cout << "#" << setbase(10) << setw(3) << i << setbase(16) << ": "
         << setw(4) << _funcs[i]->funcid()   << "H  "
         << setw(8) << _funcs[i]->offset()   << "  "
         << setw(4) << _funcs[i]->funcsize() << "  "
         << setw(4) << _funcs[i]->datasize() << "  "
         << setw(4) << _funcs[i]->codesize() << "  "
         << endl;
  }
  cout << endl << "Functions: " << setbase(10) << _funcs.size() << setbase(16) << endl;
}

