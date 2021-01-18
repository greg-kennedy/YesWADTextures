#ifndef TEXTURE_H_
#define TEXTURE_H_

/*  Texture object
    Texture objects have a name, dimensions,
    and possibly 4 mipmap levels + a palette */

#include <stdio.h>

struct s_texture
{
	char name[16];
	unsigned int width, height;

	unsigned char *level[4];

	/* palette_count is the number of "colors"
	   a color is an RGB triplet, so the actual palette is
	   3 * palette_count bytes long */
	unsigned short palette_count;
	unsigned char *palette;
};

// Read a texture from a point in a file.
struct s_texture * read_texture(FILE *file, unsigned int offset);
unsigned int get_texture_size(const struct s_texture *texture);

#endif
