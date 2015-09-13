/*
 * Created: 13 September 2015
 * Name...: findSymbol.c
 * Author.: Pike R. Alpha
 * Purpose: Command line tool to dumps symbol address/data.
 *
 * Updates:
 *			- Made a command line copy of: ~/Projects/Apple/dumpit.c (Pike R. Alpha, September 2015).
 *
 * Compile with: cc findSymbol.c -o findSymbol
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>


//==============================================================================

struct load_command * find_load_command(struct mach_header_64 *aMachHeader, uint32_t aTargetCmd)
{
	struct load_command *loadCommand;
	
	// First LOAD_COMMAND begins after the mach header.
	loadCommand = (struct load_command *)((uint64_t)aMachHeader + sizeof(struct mach_header_64));
	
	while ((uint64_t)loadCommand < (uint64_t)aMachHeader + (uint64_t)aMachHeader->sizeofcmds + sizeof(struct mach_header_64))
	{
		if (loadCommand->cmd == aTargetCmd)
		{
			return (struct load_command *)loadCommand;
		}
		
		// Next load command.
		loadCommand = (struct load_command *)((uint64_t)loadCommand + (uint64_t)loadCommand->cmdsize);
	}
	
	// Return NULL on failure (not found).
	return NULL;
}


//==============================================================================

struct segment_command_64 * find_segment_64(struct mach_header_64 *aMachHeader, const char *aSegmentName)
{
	struct load_command *loadCommand;
	struct segment_command_64 *segment;
	
	// First LOAD_COMMAND begins straight after the mach header.
	loadCommand = (struct load_command *)((uint64_t)aMachHeader + sizeof(struct mach_header_64));

	while ((uint64_t)loadCommand < (uint64_t)aMachHeader + (uint64_t)aMachHeader->sizeofcmds + sizeof(struct mach_header_64))
	{
		// printf("loadCommand->cmdsize: 0x%llx\n", (uint64_t)loadCommand->cmdsize);

		if (loadCommand->cmd == LC_SEGMENT_64)
		{
			// Check load command's segment name.
			segment = (struct segment_command_64 *)loadCommand;

			// printf("segment->segname: %s\n", segment->segname);

			if (strcmp(segment->segname, aSegmentName) == 0)
			{
				return segment;
			}
		}
		
		// Next load command.
		loadCommand = (struct load_command *)((uint64_t)loadCommand + (uint64_t)loadCommand->cmdsize);
	}
	
	// Return NULL on failure (not found).
	return NULL;
}


//==============================================================================

uint64_t _findSymbol(unsigned char * aFileBuffer, const char * aSymbolName)
{
	struct mach_header_64 *machHeader				= (struct mach_header_64 *)((unsigned char *)aFileBuffer);
	
	struct segment_command_64 * textSegment			= NULL;
	struct symtab_command * symtab					= NULL;
	struct nlist_64 * nl							= NULL;
	
	void * stringTable								= NULL;
	void * addr										= NULL;
	
	char * str;
	
	const char * fStrings							= NULL;
	
	uint32_t symbolNumber							= 0;

	if ((textSegment = find_segment_64(machHeader, SEG_TEXT)) == NULL)
	{
		printf("ERROR: Getting __TEXT failed!\n");
		return -1;
	}
	else
	{
		printf("textSegment->vmaddr: %llx\n", textSegment->vmaddr);
	}

	// Lookup SYMTAB load command.
	if ((symtab = (struct symtab_command *)find_load_command(machHeader, LC_SYMTAB)) == NULL)
	{
		printf("ERROR: Getting LC_SYMTAB failed!\n");
		return -1;
	}
	else
	{
		printf("symtab->stroff: 0x%x\n", symtab->symoff);
	}

	stringTable = (void *)((int64_t)aFileBuffer + symtab->stroff);
	nl = (struct nlist_64 *)((int64_t)aFileBuffer + symtab->symoff);
	long symbolLength = 0;
	int consts = 0;

	for (symbolNumber = 0; symbolNumber <= symtab->nsyms; symbolNumber++)
	{
		str = (char *)stringTable + nl->n_un.n_strx;

		printf("\nSymbol number..: %u\n", symbolNumber);
		printf("Current symbol.: %s\n", str);
		
		symbolLength = strlen(str);
		
		if (symbolLength)
		{
			printf("symbol length..: %ld\n", symbolLength);
		}

		printf("nl @...........: 0x%llx\n", (uint64_t)nl - (int64_t)machHeader);
		printf("nl->n_un.n_strx: 0x%x\n", nl->n_un.n_strx);
		printf("nl->n_type.....: 0x%x\n", nl->n_type);
		printf("nl->n_sect.....: 0x%x\n", nl->n_sect);
		printf("nl->n_desc.....: 0x%x\n", nl->n_desc);
		printf("nl->n_value....: 0x%llx\n", nl->n_value);

		if (strcmp(str, aSymbolName) == 0)
		{
			printf("Symbol %s found @ 0x%x", aSymbolName, (symtab->stroff + nl->n_un.n_strx));

			if (nl->n_sect == NO_SECT)
			{
				printf(", no value, skipping\n");
				continue;
			}

			if (nl->n_value && nl->n_sect)
			{
				int64_t offset = (nl->n_value - textSegment->vmaddr);
				printf("offset.........: 0x%llx\n", offset);
				//
				// Do you need the offset, then use this.
				//
				// return offset;


				//
				// The following lines are optional!
				//
				int64_t address = ((int64_t)aFileBuffer + offset);
				str = (char *)address;
				// ASCII character?
				if (str[0] < 32 || str[0] > 126)
				{
					printf("value..........: 0x%08x\n", aFileBuffer[offset]);
				}
				else
				{
					printf("string value...: %s\n", str);
				}

				return address;
			}
		}

		// next symbol.
		nl = (struct nlist_64 *)((uint64_t)nl + sizeof(struct nlist_64));

	}
	
	return 0; // Failure.
}


//==============================================================================

int main(int argc, const char * argv[])
{
	FILE *fp									= NULL;

	unsigned char * fileBuffer					= NULL;
	unsigned char * buffer						= NULL;

	struct fat_header * fatHeader				= NULL;

	unsigned long fileLength					= 0;

	if (argc != 3)
	{
		printf("Usage: <path/filename> <symbol name>\n");
	}
	else
	{
		// Try to open the source file.
		fp = fopen(argv[1], "rb");
		
		// Check file pointer.
		if (fp == NULL)
		{
			printf("Error: Opening of %s failed... exiting\nDone.\n", argv[1]);
		}
		else
		{
			fseek(fp, 0, SEEK_END);
			fileLength = ftell(fp);

			printf("fileLength...: %ld/0x%08lx - %s\n", fileLength, fileLength, argv[1]);

			fseek(fp, 0, SEEK_SET);
			fileBuffer = malloc(fileLength);
			
			if (fileBuffer == NULL)
			{
				printf("ERROR: Failed to allocate file buffer... exiting\nAborted!\n\n");
				fclose(fp);
			}
			else
			{
				printf("fileBuffer: %p\n", fileBuffer);

				fread(fileBuffer, fileLength, 1, fp);
				fatHeader = (struct fat_header *) fileBuffer;

				if (fatHeader->magic == FAT_CIGAM)
				{
					printf("ERROR: fat header magic mismatch\n");
					exit(-1);
				}
				else
				{
					printf("NOTICE: fat header status = OK\n");
				}

				if (_findSymbol(fileBuffer, argv[2]) == 0)
				{
					printf("ERROR: symbol >%s< not found!\n", argv[2]);
				}
				else
				{
					exit(0);
				}
			}
		}
	}

	exit(-1);
}
