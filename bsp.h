#ifndef BSP_H_
#define BSP_H_

// This is only a partial decoding of a BSP file.
//  Since we only care about the ENTITY and TEXTURE lumps,
//  all others are just stored as blocks of data.
struct s_bsp
{
	// entity lump is a large multi-line ASCII string,
	//  zero-terminated
	unsigned int entity_lump_size;
	char *entity_lump;

	// texture lump is a series of texture structs
	unsigned int texture_count;
	struct s_texture **textures;

	// other lumps are copied wholesale
	unsigned int raw_lump_size[15];
	unsigned char * raw_lump[15];

	// order of lumps - helps minimize change when writing
	unsigned int lump_order[15];
};

struct s_bsp * read_bsp(const char * path);
void write_bsp(const struct s_bsp* bsp, const char* path);

#endif
