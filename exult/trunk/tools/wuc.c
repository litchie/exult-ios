#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uctools.h"

static opcode_desc push_table[]=
{
	{"true",0,0x13},
	{"false",0,0x14},
	{"itemref",0,0x3e},
	{"eventid",0,0x48}
};

static const char* compiler_table[]=
{
	".argc",
	".extern",
	".externsize",
	".localc"
};

#define MAX_LABELS 3500

char token[256],*token2,indata;
int byte,word,pass,offset,datasize,codesize;

char labels[MAX_LABELS][10];
int offsets[MAX_LABELS];
int lindex;

FILE *fo,*fi;

void emit_byte(int i)
{
	if (pass>0)
	{
		fputc(i,fo);
		if (indata) datasize++;
	}
	offset++;
	codesize++;
}

void emit_word(int i)
{
	emit_byte(i&0xff);
	emit_byte((i>>8)&0xff);
}

void add_label(void)
{
	int i;
	if (token[strlen(token)-1]==':')
		token[strlen(token)-1]=0;
	if (lindex>=MAX_LABELS)
	{
		printf("Too many labels.\n");
		exit(0);
	}
	for (i=0;i<lindex;i++)
	{
		if (!strcasecmp(token,labels[i]))
		{
			printf("Warning: label '%s' already exists.\n",token);
			return;
		}
	}
	strcpy(labels[lindex],token);
	offsets[lindex++]=offset;
}

int get_label(void)
{
	int i;
	for (i=0;i<lindex;i++)
	{
		if (!strcasecmp(token,labels[i]))
			return(offsets[i]);
	}
	printf("Warning: label '%s' does not exist.\n",token);
	if (token[0]=='L')
		sscanf(token,"L%x",&word);
	else
		sscanf(token,"%x",&word);
	return(word);
}

void read_token(FILE *fi)
{
	int i=0,c=32;
	while (((c==' ') || (c=='\t') || (c=='\n') || (c==',')) && (!feof(fi)))
		c=fgetc(fi);
	while (!((c==' ') || (c=='\t') || (c=='\n') || (c==',')) && (!feof(fi)))
	{
		token[i++]=c;
		if (c==';')
		{
			while ((c=fgetc(fi))!='\n') /* do nothing */ ;
			i=0;
		}
		if (c==39)
		{
			while ((c=fgetc(fi))!='\n')
				token[i++]=c;
			ungetc(c,fi);
			i--;
		}
		c=fgetc(fi);
	}
	token[i]=0;
}

int main(int argc,char *argv[])
{
	int i,opsize=sizeof(opcode_table)/sizeof(opcode_desc),
		pushsize=sizeof(push_table)/sizeof(opcode_desc),
		compsize=sizeof(compiler_table)/sizeof(char*);
	const char **func_table = bg_intrinsic_table;
	int funsize = bg_intrinsic_size;
	int findex = 1;			// Index in argv of 1st filename.
	indata=codesize=datasize=0;
	printf("Wody's Usecode Compiler v0.009\nCopyright (c) 1999 Wody "
		"Dragon (a.k.a. Wouter Dijkslag)\n");
	if (argc<3)
	{
		printf("syntax: %s [-s] infile outfile\n", argv[0]);
		exit(0);
	}
					// Serpent Isle?
	if (strcmp(argv[1], "-s") == 0)
		{
		findex++;
		func_table = si_intrinsic_table;
		funsize = si_intrinsic_size;
		}

 lindex=0;
 for (pass=0;pass<2;pass++)
 {
  printf("Pass %d\n",pass+1);
  if ((fi=fopen(argv[findex],"r"))==NULL)
  {
   printf("Can't open infile for reading\n");
   exit(0);
  }
  if ((fo=fopen(argv[findex + 1],"wb"))==NULL)
  {
   printf("Can't open outfile for writing\n");
   exit(0);
  }
  while (!feof(fi))
  {
   read_token(fi);
   if (pass==0 && token[strlen(token)-1]==':') add_label();
   else
   if (!strcmp(token,".code"))
   {
	indata=0;
	offset=0;
   }
   else
   if (!strcmp(token,".data"))
   {
	indata=1;
	offset=0;
   }
   else
   if (!strcmp(token,".funcnumber"))
   {
	read_token(fi);
	sscanf(token,"%x",&word);
	emit_word(word);
	emit_word(0);
	emit_word(0);
	codesize=2;
   }
   else
   if (token[0]=='.')
   {
	indata=0;
	for (i=0;i<compsize;i++)
		if (!strcasecmp(compiler_table[i],token))
		{
			read_token(fi);
			sscanf(token,"%x",&word);
			emit_word(word);
		}
   }
   else
   if (!strcmp(token,"db"))
   {
	read_token(fi);
	if (token[0]==39)
		for (i=1;i<strlen(token);i++)
			emit_byte(token[i]);
	else
	{
		sscanf(token,"%x",&byte);
		emit_byte(byte);
	}
   }
   else
   if (!strcasecmp(token,"dw"))
   {
	read_token(fi);
	sscanf(token,"%x",&word);
	emit_word(word);
   }
   else
   for (i=0;i<opsize;i++)
   {
	if (!opcode_table[i].mnemonic) continue;
    if (!strcasecmp(opcode_table[i].mnemonic,token))
    {
     if (opcode_table[i].nbytes==0 && opcode_table[i].type==0)
       emit_byte(i);
     else
     switch (opcode_table[i].type)
     {
	case BYTE:
		emit_byte(i);
		read_token(fi);
		sscanf(token,"%x",&word);
		emit_byte(word);
		break;
	case CALL:
		emit_byte(i);
		read_token(fi);
		if ((token2=strchr(token,'@'))!=NULL)
		{
			token2++;
			token[strchr(token,'@')-token]=0;
			strcpy(token,token+1);
			for (i=0;i<funsize;i++)
				if (!strcasecmp(token,func_table[i]))
				{
					emit_word(i);
					break;
				}
			if (i==funsize)
			{
				printf("Do not know function '%s'\n",token);
				exit(0);
			}
			sscanf(token2,"%d",&word);
		}
		else
		{
			sscanf(token,"%x",&word);
			emit_word(word);
			read_token(fi);
			sscanf(token,"%d",&word);
		}
		emit_byte(word);
		break;
	case DATA_STRING:
		emit_byte(i);
		read_token(fi);
		emit_word(get_label());
		break;
	case EXTCALL:
	case VARREF:
		emit_byte(i);
		read_token(fi);
		sscanf(token,"[%x]",&word);
		emit_word(word);
		break;
	case FLGREF:
		emit_byte(i);
		read_token(fi);
		sscanf(token,"flag:[%x]",&word);
		emit_word(word);
		break;
	case PUSH:
		read_token(fi);
		for (i=0;i<pushsize;i++)
		{
			if (!strcasecmp(push_table[i].mnemonic,token))
			{
				emit_byte(push_table[i].type);
				break;
			}
		}
		if (i==pushsize)
		{
			emit_byte(0x21);
			sscanf(token,"[%x]",&word);
			emit_word(word);
		}
		break;
	case IMMED:
		emit_byte(i);
		read_token(fi);
		sscanf(token,"%x",&word);
		emit_word(word);
		break;
	case RELATIVE_JUMP:
		emit_byte(i);
		read_token(fi);
		if (pass==1)
			emit_word(get_label()-offset-2);
		else
			emit_word(-1);
		break;
	case IMMED_AND_RELATIVE_JUMP:
		emit_byte(i);
		read_token(fi);
		sscanf(token,"%x",&word);
		emit_word(word);
		read_token(fi);
		if (pass==1)
			emit_word(get_label()-offset-2);
		else
			emit_word(-1);
		break;
	default:
		break;
     }
    }
   }
  }
  fseek(fo,2,SEEK_SET);
  indata=0;
  i=codesize;
  emit_word(i);
  emit_word(datasize);
  fclose(fo);
  fclose(fi);
 }
 return 0;
}
