/*
 * mklink -- Create "linkdep1" and "linkdep2" files from the
 *           "usecode" file.
 *
 * Copyright (c) 1999 Keldon Jones
 */

#include <stdio.h>
#include <stdlib.h>


/* Some basic types */
typedef unsigned char byte;
typedef unsigned short u16b;
typedef unsigned long u32b;

/* Info about one usecode function */
typedef struct usecode_func usecode_func;

struct usecode_func
{
	/* Location in usecode */
	u32b where;

	/* Size of function in bytes */
	u16b size;

	/* Number of called functions */
	u16b num_call;

	/* List of called functions */
	u16b *called;

	/* Have we been visited */
	u16b visited;
};

/* List of usecode functions */
usecode_func functions[4096];

/*
 * Read an unsigned 16-bit value from a file.
 */
void read_u16b(FILE *fff, u16b *n)
{
	u16b x;
	byte c;

	c = fgetc(fff);

	x = c;

	c = fgetc(fff);

	x |= c << 8;

	*n = x;
}

/*
 * Read an unsigned 32-bit value from a file.
 */
void read_u32b(FILE *fff, u32b *n)
{
	u32b x;
	byte c;

	c = fgetc(fff);

	x = c;

	c = fgetc(fff);

	x |= c << 8;

	c = fgetc(fff);

	x |= c << 16;

	c = fgetc(fff);

	x |= c << 24;

	*n = x;
}

/*
 * Write an unsigned 16-bit value to a file.
 */
void write_u16b(FILE *fff, u16b n)
{
	byte c;

	c = n & 0xFF;

	fputc(c, fff);

	n >>= 8;

	c = n & 0xFF;

	fputc(c, fff);
}

/*
 * Write an unsigned 32-bit value to a file.
 */
void write_u32b(FILE *fff, u32b n)
{
	byte c;

	c = n & 0xFF;

	fputc(c, fff);

	n >>= 8;

	c = n & 0xFF;

	fputc(c, fff);

	n >>= 8;

	c = n & 0xFF;

	fputc(c, fff);

	n >>= 8;

	c = n & 0xFF;

	fputc(c, fff);
}

/*
 * Return the sum of the sizes of all functions in this tree.
 */
u16b get_total_size(u16b *call_tree, u16b tree_size)
{
	usecode_func *u_ptr;
	u16b size = 0;
	int i;

	/* Add size of each function */
	for (i = 0; i < tree_size; i++)
	{
		/* Get pointer to function info */
		u_ptr = &functions[call_tree[i]];

		/* Add in size of this function */
		size += u_ptr->size;
	}

	/* Return total size */
	return size;
}

/*
 * Return a list of all functions in the call tree of the given function.
 *
 * This function is recursive.
 */
u16b *get_tree(u16b func_num, u16b *num)
{
	usecode_func *u_ptr;
	u16b *sub_tree, sub_size;
	u16b *our_tree, our_size;
	int i, j;

	/* Get pointer to function info */
	u_ptr = &functions[func_num];

	/* No need to return a tree */
	if (u_ptr->visited)
	{
		/* No tree */
		*num = 0;

		return NULL;
	}

	/* Start with ourselves */
	our_size = 1;

	/* Start array */
	our_tree = (u16b *)malloc(sizeof(u16b));

	/* Copy our number into tree */
	our_tree[0] = func_num;

	/* We've visited this function */
	u_ptr->visited = 1;

	/* Add elements from each called function */
	for (i = 0; i < u_ptr->num_call; i++)
	{
		/* Get the sub-tree */
		sub_tree = get_tree(u_ptr->called[i], &sub_size);

		/* No tree returned */
		if (!sub_size) continue;

		/* Increase the number in our tree */
		our_size += sub_size;

		/* Grow our tree */
		our_tree = (u16b *)realloc(our_tree, sizeof(u16b) * our_size);

		/* Copy elements from sub-tree */
		for (j = 0; j < sub_size; j++)
		{
			/* Copy one element */
			our_tree[our_size - j - 1] = sub_tree[j];
		}

		/* Destroy sub-tree */
		free(sub_tree);
	}

	/* Return size */
	*num = our_size;

	return our_tree;
}

/*
 * Clear the "visited" flag from each function.
 */
void clear_visited(void)
{
	int i;

	/* Clear all "visited" flags */
	for (i = 0; i < 4096; i++)
	{
		/* Clear this flag */
		functions[i].visited = 0;
	}
}

/*
 * Compare two function numbers.
 */
int comp_func(const void *one, const void *two)
{
	u16b first = *((u16b *)one);
	u16b second = *((u16b *)two);

	/* Equal */
	if (first == second) return 0;

	/* First smaller */
	if (first < second) return -1;

	/* Second smaller */
	return 1;
}

/*
 * Sort a function call tree and remove duplicates.
 */
void fix_tree(u16b *tree, u16b *tree_size)
{
	/* Just call qsort (I'm feeling lazy) */
	qsort(tree, *tree_size, sizeof(u16b), comp_func);
}

/*
 * Process the "usecode" file and create the two index files.
 */
int main(void)
{
	FILE *usecode, *linkdep1, *linkdep2;
	u16b func_num, data_size, max_func;
	u16b total_size, lnk2_written = 0;
	u16b *func_tree, tree_size;
	usecode_func *u_ptr;
	int i, j;

	/* Open the usecode file for reading */
	usecode = fopen("usecode", "rb");

	/* Check for failure */
	if (!usecode)
	{
		/* Error message */
		fprintf(stderr, "Could not open usecode!\n");

		/* Give up */
		exit(1);
	}

	/* Open the linkdep files for writing */
	linkdep1 = fopen("linkdep1", "wb");
	linkdep2 = fopen("linkdep2", "wb");

	/* Read the first function number */
	read_u16b(usecode, &func_num);

	/* Process the usecode file */
	while (!feof(usecode))
	{
		/* Get pointer to function */
		u_ptr = &functions[func_num];

		/* Remember start of function */
		u_ptr->where = ftell(usecode) - 2;

		/* Read the function size */
		read_u16b(usecode, &u_ptr->size);

		/* Read the data size */
		read_u16b(usecode, &data_size);

		/* Skip the data section */
		fseek(usecode, data_size, SEEK_CUR);

		/* Skip number of args and local vars */
		fseek(usecode, 4, SEEK_CUR);

		/* Read number of called functions */
		read_u16b(usecode, &u_ptr->num_call);

		/* Allocate space for table */
		u_ptr->called = (u16b *)malloc(sizeof(u16b) * u_ptr->num_call);

		/* Read table */
		for (i = 0; i < u_ptr->num_call; i++)
		{
			/* Read this entry */
			read_u16b(usecode, &u_ptr->called[i]);
		}

		/* Skip past end of function */
		fseek(usecode, u_ptr->where + u_ptr->size + 4, SEEK_SET);

		/* Track highest function number */
		max_func = func_num;

		/* Read the next function number */
		read_u16b(usecode, &func_num);
	}

	/* Write data about each function */
	for (i = 0; i <= max_func; i++)
	{
		/* Get pointer to function */
		u_ptr = &functions[i];

		/* Write null data for null function */
		if (!u_ptr->size)
		{
			/* Write nothing */
			write_u16b(linkdep1, lnk2_written);
			write_u16b(linkdep1, 0xffff);

			/* Go to next function */
			continue;
		}

		/* Get function tree */
		func_tree = get_tree(i, &tree_size);

		/* Fix up the function tree */
		fix_tree(func_tree, &tree_size);

		/* Clear visited flags */
		clear_visited();

		/* Get total size of function + all called functions */
		total_size = get_total_size(func_tree, tree_size);

		/* Write to linkdep1 */
		write_u16b(linkdep1, lnk2_written);
		write_u16b(linkdep1, total_size);

		/* Write each pointer */
		for (j = 0; j < tree_size; j++)
		{
			/* Write this pointer to linkdep2 */
			write_u32b(linkdep2, functions[func_tree[j]].where);

			/* One more pointer written */
			lnk2_written++;
		}

		/* Destroy the function tree */
		free(func_tree);
	}

	/* Write ending on linkdep1 */
	write_u16b(linkdep1, lnk2_written);
	write_u16b(linkdep1, 0);

	/* Close files */
	fclose(usecode);
	fclose(linkdep1);
	fclose(linkdep2);

	/* Done */
	return 0;
}
