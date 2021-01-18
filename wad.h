#ifndef WAD_H_
#define WAD_H_

/*  WAD file
    WAD files have a filename, and a container of texture objects */

struct s_wad
{
	char *name;
	unsigned int texture_count;
	struct s_texture **textures;
};

struct s_wad * read_wad(const char *path);

#endif
