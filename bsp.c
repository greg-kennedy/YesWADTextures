#include "bsp.h"

#include "texture.h"

#include "utils.h"

// for exit
#include <stdlib.h>
// for memset
#include <string.h>

// defines for lump types we care to parse
#define LUMP_ENTITIES 0
#define LUMP_TEXTURES 2

struct s_bsp * read_bsp(const char * path)
{
	unsigned char buf[8];
	FILE *fp = u_fopen(path, "rb");

	// Check the BSP header for v30
	u_fread(buf, 4, fp);
	unsigned int version = parseU32(buf);
	if (version != 30) {
		fprintf( stderr, "%s does not appear to be a BSPv30 file (got %u)\n", path, version );
		exit(EXIT_FAILURE);
	}

	// create a struct to hold our info
	struct s_bsp *ret = u_malloc(sizeof(struct s_bsp));

	// get all lump info
	unsigned int lumpOffset[15];
	for (int i = 0; i < 15; i ++) {
		u_fread(buf, 8, fp);
		lumpOffset[i] = parseU32(buf);
		ret->raw_lump_size[i] = parseU32(buf+4);
	}

	// ///////////////////////
	// read all lumps except those needing special handling
	for (int i = 0; i < 15; i ++) {
		if (i == LUMP_ENTITIES || i == LUMP_TEXTURES) continue;

		u_fseek( fp, lumpOffset[i] );
		ret->raw_lump[i] = u_malloc( ret->raw_lump_size[i] );
		u_fread(ret->raw_lump[i], ret->raw_lump_size[i], fp );
	}

	// ///////////////////////
	// entity lump
	u_fseek( fp, lumpOffset[LUMP_ENTITIES] );
	ret->entity_lump_size = ret->raw_lump_size[LUMP_ENTITIES];
	ret->entity_lump = u_malloc( ret->entity_lump_size );
	u_fread(ret->entity_lump, ret->entity_lump_size, fp );

	// ///////////////////////
	// read the Textures block
	u_fseek( fp, lumpOffset[LUMP_TEXTURES] );

	u_fread(buf, 4, fp);
	ret->texture_count = parseU32(buf);
	ret->textures = u_malloc(ret->texture_count * sizeof(struct texture*));

	// get the offsets to each texture
	unsigned int *mipTexOffsets = u_malloc(ret->texture_count * sizeof(unsigned int));
	for (unsigned int i = 0; i < ret->texture_count; i ++)
	{
		u_fread(buf, 4, fp);
		mipTexOffsets[i] = parseU32(buf);
	}

	// now go read each texture
	for (unsigned int i = 0; i < ret->texture_count; i ++)
	{
		ret->textures[i] = read_texture(fp, lumpOffset[LUMP_TEXTURES] + mipTexOffsets[i]);
	}

	// don't need these any more
	free(mipTexOffsets);

	fclose(fp);

	return ret;
}

void write_bsp(const struct s_bsp *const bsp, const char* path)
{
	FILE *fp = u_fopen(path, "wb");

	unsigned char buf[4];

	// calculate the size of the new texture block
	//  begin with 4 bytes (uint32 texture_count)
	unsigned int texLumpSize = 4;
	// then a list of offsets to textures
	texLumpSize += bsp->texture_count * 4;
	// finally the combined size of each texture
	for (unsigned int i = 0; i < bsp->texture_count; i ++)
		texLumpSize += get_texture_size(bsp->textures[i]);

	// Ready to write the output BSP
	// Write v30 header
	packU32(buf, 30U);
	u_fwrite(buf, 4, fp);

	// offsets and lengths of all items
	//  begin with uint32 version, plus 8 bytes for each of 15 lumps
	unsigned int offset = 4 + (8 * 15);
	for (int i = 0; i < 15; i ++) {
		// offset
		packU32(buf, offset);
		u_fwrite(buf, 4, fp);

		// size
		unsigned int lumpSize;
		if (i == LUMP_ENTITIES) {
			lumpSize = bsp->entity_lump_size;
		} else if (i == LUMP_TEXTURES) {
			lumpSize = texLumpSize;
		} else {
			lumpSize = bsp->raw_lump_size[i];
		}
		packU32(buf, lumpSize);
		offset += lumpSize;

		// all offsets should be dword-aligned
		offset = (offset+3)&~3;
		u_fwrite(buf, 4, fp);
	}

	// now write all lumps
	for (int i = 0; i < 15; i ++) {
		if (i == LUMP_ENTITIES) {
			// Just write the Entities lump
			u_fwrite(bsp->entity_lump, bsp->entity_lump_size, fp );
		} else if (i == LUMP_TEXTURES) {
			// uint32 texture_count
			packU32(buf, bsp->texture_count);
			u_fwrite(buf, 4, fp);

			// offsets to each texture
			offset = 4 + (4 * bsp->texture_count);
			for (unsigned int j = 0; j < bsp->texture_count; j ++)
			{
				packU32(buf, offset);
				u_fwrite(buf, 4, fp);

				offset += get_texture_size(bsp->textures[j]);
			}

			// texture data
			for (unsigned int j = 0; j < bsp->texture_count; j ++)
			{
				// texture name
				u_fwrite(bsp->textures[j]->name, 16, fp);
				// texture width and height
				packU32(buf, bsp->textures[j]->width);
				u_fwrite(buf, 4, fp);
				packU32(buf, bsp->textures[j]->height);
				u_fwrite(buf, 4, fp);
				// texture offsets?
				if (bsp->textures[j]->palette) {
					// this is an embedded texture
					//  start at the 40-byte header
					offset = 40;
					int width = bsp->textures[j]->width;
					int height = bsp->textures[j]->height;
					for (int k = 0; k < 4; k ++) {
						packU32(buf, offset);
						u_fwrite(buf, 4, fp);
						offset += (width * height);
						width >>= 1; height >>= 1;
					}
					// write the texture data now
					width = bsp->textures[j]->width;
					height = bsp->textures[j]->height;
					for (int k = 0; k < 4; k ++) {
						u_fwrite(bsp->textures[j]->level[k], width * height, fp);
						width >>= 1; height >>= 1;
					}
					// finally, the palette
					packU16(buf, bsp->textures[j]->palette_count);
					u_fwrite(buf, 2, fp);
					u_fwrite(bsp->textures[j]->palette, bsp->textures[j]->palette_count * 3, fp);
				} else {
					// no embedded texture, no mipmap offsets.  just write 4x zero
					memset(buf, 0, 4);
					for (int k = 0; k < 4; k ++)
						u_fwrite(buf, 4, fp);
				}
			}
		} else {
			// Raw unparsed lump, write it as-is
			u_fwrite(bsp->raw_lump[i], bsp->raw_lump_size[i], fp );
		}

		// padding
		unsigned int file_len = ftell(fp);
		if (file_len & 3) {
			u_fwrite("\0\0", 4 - (file_len & 3), fp);
		}
	}

	fclose(fp);
}

