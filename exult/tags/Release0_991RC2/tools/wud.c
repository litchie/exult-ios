#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uctools.h"

// Prints module's data segment
void printdataseg(FILE* f, unsigned int ds)
{
	long pos;
	unsigned int off = 0;
	unsigned char* p;	unsigned char* pp;
	unsigned char* tempstr,*tempstr2;
	tempstr = malloc(60 + 1);
	pos = ftell(f);
	pp = p = malloc(ds);
	fread(p, 1, ds, f);
	fseek(f, pos, SEEK_SET);
	printf("\t\t.data\n");
	while( off < ds )
	{
		int len;
		unsigned int localoff = 0;
		while( (len = ( strlen(pp) > 60 ) ? 60 : strlen(pp)) )
		{
			memcpy(tempstr, pp, len);
			tempstr[len] = '\0';
			if (!localoff)
				printf("L%04X:",off);
			  while (strchr(tempstr,13))
			  {
			   tempstr2=strchr(tempstr,13)+2;
 			   tempstr[strchr(tempstr,13) - (char *) tempstr]=0;
			   printf("\tdb\t\'%s\'\n\tdb\t0d\n\tdb\t0a\n", tempstr);
			   strcpy(tempstr,tempstr2);
			  }
			if (tempstr)
				printf("\tdb\t\'%s\'\n", tempstr);
			localoff += len; 
			pp += len;
		}
		pp++;
		printf("L%04X:\tdb\t00\n", off+localoff);
		off += localoff + 1;
	}
	free(p);
	free(tempstr);
}

// Prints single opcode
// Return number of bytes to advance the code pointer
// Prints first characters of strings referenced
unsigned int print_opcode(unsigned char* ptrc, unsigned int coffset,
	unsigned char* pdataseg,unsigned short* pextern,
	unsigned int externsize, const char **func_table, int funsize)
{
	unsigned int nbytes;
	unsigned int i;
	// Find the description
	opcode_desc* pdesc = ( *ptrc >= ( sizeof(opcode_table) / sizeof( opcode_desc ) ) ) ?
							NULL : opcode_table + ( *ptrc );
	if( pdesc && ( pdesc->mnemonic == NULL ) )
	{
		printf("Unknown opcode: %x\n",*ptrc);
		// Unknown opcode
		pdesc = NULL;
	}
	// Number of bytes to print
	nbytes = pdesc ? ( pdesc->nbytes + 1 ) : 1;
	// Print label
	printf("%04X: ", coffset);
	// Print bytes
	for( i = 0; i < nbytes; i++ )
		printf("%02X ", ptrc[i]);
	// Print mnemonic
	if( nbytes < 4 )
		printf("\t");
        if (pdesc)
		printf("\t%s", pdesc->mnemonic);
	else
		printf("\tdb %x\t???",ptrc[0]);
	// Print operands if any
	if( ( nbytes == 1 ) || ( pdesc == NULL && pdesc->type == 0) )
	{
		printf("\n");
		return nbytes;
	}
	switch( pdesc->type )
	{
	case BYTE:
		printf("\t%x\n",*(unsigned char*)(ptrc+1));
		break;
	case PUSH:
		for ( i = 1; i < nbytes; i++)
			printf("\n%04X:\t\t\tdb %x",coffset+i,ptrc[i]);
		printf("\n");
		break;
	case IMMED:
		// Print immediate operand
		printf("\t%04XH\t\t\t; %d\n", *(unsigned short*)( ptrc + 1 ), *(short*)( ptrc + 1 ));
		break;
	case IMMED32:
		printf("\t%04XH\t\t\t; %d\n", *(unsigned int*)( ptrc + 1 ), *(int*)( ptrc + 1 ));
		break;
	case DATA_STRING:
	case DATA_STRING32:
		{
			unsigned char* pstr;
			int len;
			// Print data string operand
			if (pdesc->type == DATA_STRING)
				pstr = pdataseg + *(unsigned short*)( ptrc + 1 );
			else
				pstr = pdataseg + *(unsigned int*)( ptrc + 1 );

			len = strlen(pstr);
			if( len > 20 )
				len = 20 - 3;

			if (pdesc->type == DATA_STRING)
				printf("\tL%04X\t\t\t; ", *(unsigned short*)( ptrc + 1 ));
			else
				printf("\tL%04X\t\t\t; ", *(unsigned int*)( ptrc + 1 ));
			for( i = 0; i < len; i++ )
				printf("%c", pstr[i]);
			if( len < strlen(pstr) )
				// String truncated
				printf("...");
			printf("\n");
		}
		break;
	case RELATIVE_JUMP:
		// Print jump desination
//		printf("\t%04X\n", *(short*)( ptrc + 1 ) + (short)coffset + 3);
		printf("\t%04X\n", *(short*)( ptrc + nbytes - 2) + 
				coffset + nbytes);
// debugging printf("nbytes=%d, coffset=%d\n", nbytes, coffset);
		break;
	case RELATIVE_JUMP32:
		printf("\t%04X\n", *(int*)( ptrc + nbytes - 4) + 
			    coffset + nbytes);
		break;
	case SLOOP:  /* WJP */
		printf("\t[%04X], [%04X], [%04X], [%04X], %04X\n", 
			   *(unsigned short*)( ptrc + nbytes - 10 ),
			   *(unsigned short*)( ptrc + nbytes - 8 ),
			   *(unsigned short*)( ptrc + nbytes - 6 ),
			   *(unsigned short*)( ptrc + nbytes - 4 ),
			   *(short*)( ptrc + nbytes - 2) + coffset + nbytes);
		break;
	case SLOOP32:  /* WJP */
		printf("\t[%04X], [%04X], [%04X], [%04X], %04X\n", 
			   *(unsigned short*)( ptrc + nbytes - 12 ),
			   *(unsigned short*)( ptrc + nbytes - 10 ),
			   *(unsigned short*)( ptrc + nbytes - 8 ),
			   *(unsigned short*)( ptrc + nbytes - 6 ),
			   *(int*)( ptrc + nbytes - 4) + coffset + nbytes);
		break;
	case IMMED_AND_RELATIVE_JUMP:	/* JSF */
		printf("\t%04XH, %04X\n", *(unsigned short*)( ptrc + 1 ),
				*(short*)( ptrc + 3 ) + coffset + 5);
		break;
	case IMMED_RELJUMP32:
		printf("\t%04XH, %04X\n", *(unsigned short*)( ptrc + 1 ),
				*(int*)( ptrc + 3 ) + coffset + 5);
		break;
	case CALL:
		{
			// Print call operand
			unsigned short func = *(unsigned short*)( ptrc + 1 );
			if( ( func < funsize ) &&
				 func_table[func] )
				// Known function
				printf("\t_%s@%d (%04X)\n", func_table[func], ptrc[3], func);
			else
				// Unknown function
				printf("\t%04X, %d\n", func, ptrc[3]);
		}
		break;
	case EXTCALL:
		{
			// Print extern call
			unsigned short externpos = *(unsigned short*)( ptrc + 1 );
			if( externpos < externsize )
				printf("\t[%04X]\t\t\t; %04XH\n", externpos, pextern[externpos]);
			else
				printf("\t[%04X]\t\t\t; ????\n", externpos);
		}
		break;
	case VARREF:
		// Print variable reference
		printf("\t[%04X]\n", *(unsigned short*)( ptrc + 1 ));
		break;
	case FLGREF:
		// Print global flag reference
		printf("\tflag:[%04X]\n", *(unsigned short*)( ptrc + 1 ));
		break;
	default:
		// Unknown type
		printf("\n");
		break;
	}
	return nbytes;
}

void printcodeseg(FILE* f, unsigned int ds, unsigned int s,
				const char **func_table, int funsize, int extended)
{
	long pos;
	unsigned int size;
	unsigned int externsize;
	unsigned int i;
	unsigned int offset;
	unsigned int nbytes;
	unsigned char* p;
	unsigned char* pp;
	unsigned char* pdata;
	unsigned short* pextern;
	pos = ftell(f);
	if (extended == 0) {
		size = s - ds - sizeof(unsigned short);
	} else {
		size = s - ds - sizeof(unsigned int);
	}

	pp = p = malloc(size);
	pdata = malloc(ds);
	fread(pdata, 1, ds, f);
	printf("\t\t.code\n");
	fread(p, 1, size, f);
	fseek(f, pos, SEEK_SET);
	// Print code segment header
	if( size < 3 * sizeof(unsigned short) )
	{
		printf("Code segment bad!\n");
		free(p);
		free(pdata);
		return;
	}
	// Print argument counter
	printf("\t\t.argc %04XH\n", *(unsigned short*)pp);
	pp += sizeof(unsigned short);
	// Print locals counter
	printf("\t\t.localc %04XH\n", *(unsigned short*)pp);
	pp += sizeof(unsigned short);
	// Print externs section
	externsize = *(unsigned short*)pp;
	printf("\t\t.externsize %04XH\n", externsize);
	pp += sizeof(unsigned short);
	if( size < ( ( 3 + externsize ) * sizeof(unsigned short) ) )
	{
		printf("Code segment bad!\n");
		free(p);
		free(pdata);
		return;
	}
	size -= ( ( 3 + externsize ) * sizeof(unsigned short) );
	pextern = (unsigned short*)pp;
	for( i = 0; i < externsize; i++ )
	{
		printf("\t\t.extern %04XH\n", *(unsigned short*)pp);
		pp += sizeof(unsigned short);
	}
	offset = 0;
	// Print opcodes
	while( offset < size )
	{
		nbytes = print_opcode(pp, offset, pdata, pextern, externsize,
						func_table, funsize);
		pp += nbytes;
		offset += nbytes;
	}
	free(p);
	free(pdata);
}

/*
 *	Note:  func = -1 just to print header, funcnum to print it all, or
 *		-2 for any function.
 */
void printfunc(FILE* f, long func, int i, const char **func_table, int funsize)
{
	unsigned short funcnum;
	unsigned int s, ds;	
	unsigned short temp;
	long off, bodyoff;
	int extended = 0;
	// Save start offset
	off = ftell(f);
	// Read function header
	fread(&funcnum, sizeof(unsigned short), 1, f);

	if (funcnum == 0xFFFF) {
		fread(&funcnum, sizeof(unsigned short), 1, f);
		fread(&s, 4, 1, f);
		bodyoff = ftell(f);
		fread(&ds, 4, 1, f); 
		extended = 1;
	} else {
		fread(&temp, sizeof(unsigned short), 1, f);
		s = temp;
		// Save body offset
		bodyoff = ftell(f);
		fread(&temp, sizeof(unsigned short), 1, f);
		ds = temp;
	}

	if( func == -1 )
		printf("\tFunction #%d (%04XH), offset = %08lx, size = %04x, data = %04x\n", i,
			funcnum, off, s, ds);
	if( funcnum == func || func == -2)
	{
		printf("\t\t.funcnumber\t%04XH\n", funcnum);
		if (extended == 1)
			printf("\t\t.ext32\n");
		// Dump function contents
		printdataseg(f, ds);
		printcodeseg(f, ds, s, func_table, funsize, extended);
	}
	// Seek back, then to next function
	fseek(f, bodyoff, SEEK_SET);
	fseek(f, s, SEEK_CUR);
}

int main(int argc, char** argv)
{
	unsigned long func = -1;
	long sz;
	int i = 0;
	FILE* f;
	int findex = 1;			// Argv index of file.
	const char **func_table = bg_intrinsic_table;
	int funsize = bg_intrinsic_size;


	if(argc<2) {
		fprintf(stderr, "Usage\n%s [-s] usecode_file [func_num]\n", argv[0]);
		return -1;
	}
					// Serpent Isle?
	if (strcmp(argv[1], "-s") == 0)
		{
		findex++;
		func_table = si_intrinsic_table;
		funsize = si_intrinsic_size;
		}
	f = fopen(argv[findex], "rb");
	if( f == NULL )
	{
		fprintf(stderr,"Failed to open %s\n\n", argv[findex]);
		return 0;
	}
	fseek(f, 0, SEEK_END);
	sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	if( argc > findex + 1 )
	{
		char* stopstr;
		if (strcmp(argv[findex + 1], "-a") == 0)
			func = -2;	// Print ALL functions.
		else
			func = strtoul(argv[findex + 1], &stopstr, 16);
	}
	while( ftell(f) < sz )
	{		
		printfunc(f, func, i, func_table, funsize);
		i++;
	}
	if( func == -1 )
	{
		if( ftell(f) != sz )
			fprintf(stderr,"Problem, tell = %ld!\n", ftell(f));
		printf("Functions: %d\n", i);
	}
	fclose(f);
	return 0;
}
