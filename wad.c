#include "wad.h"

#include "texture.h"

#include "utils.h"

// for "exit"
#include <stdlib.h>
// for "memcmp"
#include <string.h>
// for "tolower"
#include <ctype.h>

struct s_wad * read_wad(const char* path)
{
	struct dir_entry
	{
		unsigned int offset;
		unsigned int diskSize;
		unsigned int size;
		unsigned char type;
		unsigned char compression;
		char name[16];
	};

	unsigned char buf[40];

	// open the provided file
	FILE *fp = u_fopen(path, "rb");

	// read WAD3 header
	u_fread(buf, 12, fp);
	if ( memcmp(buf, "WAD3", 4) ) {
		fprintf(stderr, "%s does not appear to be in WAD3 format\n", path );
		exit(EXIT_FAILURE);
	}
	unsigned int numEntries = parseU32(buf+4);
	unsigned int dirOffset = parseU32(buf+8);

	// initialize a structure to hold our data
	struct s_wad* ret = u_calloc(sizeof(struct s_wad));
	// copy base path into name
	//  HL tends to use lowercased WAD names and uppercased texture names
	const char *base = path + strlen(path);
	do {
		base--;
	} while (base > path && *base != '/' && *base != '\\');

	ret->name = u_calloc(strlen(base) + 1);
	for (unsigned int i = 0; i < strlen(base); i ++)
		ret->name[i] = tolower(base[i]);

	unsigned int texture_limit = 0;

	// read directory entries
	printf("WAD3 '%s': %u entries beginning at offset %u\n", ret->name, numEntries, dirOffset);
	u_fseek( fp, dirOffset );

	struct dir_entry* directory = u_calloc(numEntries * sizeof(struct dir_entry));

	for (unsigned int i = 0; i < numEntries; i ++)
	{
		u_fread(buf, 32, fp);
		directory[i].offset = parseU32(buf);
		directory[i].diskSize = parseU32(buf+4);
		directory[i].size = parseU32(buf+8);
		directory[i].type = buf[12];
		directory[i].compression = buf[13];
		// two dummy bytes skipped here
		strncpy(directory[i].name, (const char*)buf+16, 15);
	}

	// iterate through directory and read every texture
	for (unsigned int i = 0; i < numEntries; i ++)
	{
		// reject some entries for problems
		if (directory[i].type != 0x43) {
			fprintf( stderr, "WAD %s, entry %u ('%s'): skipping object type %u\n", path, i, directory[i].name, directory[i].type );
			continue;
		}
		else if (directory[i].compression) {
			fprintf( stderr, "WAD %s, entry %u ('%s'): compressed entries are not supported\n", path, i, directory[i].name );
			continue;
		}
		else if (directory[i].diskSize != directory[i].size) {
			fprintf( stderr, "WAD %s, entry %u ('%s'): diskSize %u != size %u\n", path, i, directory[i].name, directory[i].diskSize, directory[i].size );
			continue;
		}

		// get the miptex from this point
		struct s_texture *tex = read_texture(fp, directory[i].offset);

		// add it to the list
		if (ret->texture_count == texture_limit) {
			texture_limit = (texture_limit << 1) + 1;
			ret->textures = u_realloc(ret->textures, texture_limit * sizeof(struct s_texture*));
		}
		ret->textures[ret->texture_count] = tex;
		ret->texture_count ++;
	}

	// shrink texture list
	//ret->textures = u_realloc(ret->textures, ret->texture_count * sizeof(struct s_texture*));

	// we are done with the directory now
	free(directory);

	// also done with the ret
	fclose(fp);

	return ret;
}
