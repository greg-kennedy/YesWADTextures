#include "texture.h"

#include "utils.h"

// for "exit"
#include <stdlib.h>
// for "toupper"
#include <ctype.h>

struct s_texture* read_texture(FILE *const fp, const unsigned int offset)
{
	// seek to the target
	u_fseek(fp, offset);

	// read the texture header
	unsigned char buf[40];
	u_fread(buf, 40, fp);

	unsigned int width = parseU32(buf+16);
	unsigned int height = parseU32(buf+20);
	unsigned int levelOffs[4];
	for (int i = 0; i < 4; i ++)
		levelOffs[i] = parseU32(buf+24+4*i);

	// create struct to hold the copied texture data
	struct s_texture *ret = (struct s_texture *)u_calloc(sizeof(struct s_texture));

	// copy name - uppercased
	for (int c = 0; c < 15; c ++) {
		if (buf[c] == '\0')
			break;
		ret->name[c] = (char)toupper(buf[c]);
	}

	// copy other values
	ret->width = width;
	ret->height = height;

	// read each of 4 mipmap levels, if present
	//  textures from a BSP may not have these, which indicates a reference to a texture
	if (levelOffs[0] || levelOffs[1] || levelOffs[2] || levelOffs[3]) {
		for (int i = 0; i < 4; i ++)
		{
			//printf(" . Mipmap level %d (at %u)\n", i, offset + levelOffs[i]);

			u_fseek(fp, offset + levelOffs[i]);

			ret->level[i] = (unsigned char *)u_malloc(width * height);
			u_fread(ret->level[i], width * height, fp);
			width >>= 1;
			height >>= 1;
		}

		u_fread(buf, 2, fp);
		ret->palette_count = parseU16(buf);

		ret->palette = (unsigned char *)u_malloc(ret->palette_count * 3);
		u_fread(ret->palette, ret->palette_count * 3, fp);
	}

	return ret;
}

// compute the on-disk size of a texture
unsigned int get_texture_size(const struct s_texture *const texture)
{
	// header is at least 40
	unsigned int size = 40;
	if (texture->palette) {
		// offsets exist
		unsigned int width = texture->width;
		unsigned int height = texture->height;
		for (int i = 0; i < 4; i ++) {
			size += (width * height);
			width >>= 1; height >>= 1;
		}
		// palette stuff
		size += 2;
		size += texture->palette_count * 3;
	}
	return size;
}
