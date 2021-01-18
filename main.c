#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "texture.h"
#include "bsp.h"
#include "wad.h"

// This is a tool to "normalize" the textures of a BSP file,
//  by eliminating duplicate textures that users already have.
// Older versions of Half-life build tools allowed a parameter
//  -nowadtextures which would bundle ALL textures into the map,
//  even those that came with the base game or mod.
//  This pointlessly inflates the size of BSP files.

int main (int argc, char *argv[])
{
	if (argc < 4) {
		fprintf( stderr, "Usage: %s input.bsp output.bsp wadfile.wad <wadfile.wad ...>\n", argv[0] );
		exit(0);
	}

	// read BSP input
	struct s_bsp *bsp = read_bsp(argv[1]);

	// read every WAD file and get the Directory out
	int wad_count = argc - 3;
	struct s_wad** wad = calloc(wad_count, sizeof(struct wad*));
	for (int i = 3; i < argc; i ++)
		wad[i-3] = read_wad(argv[i]);

	// identify duplicates
	for (unsigned int i = 0; i < bsp->texture_count; i ++) {
		printf("%03u: BSP texture '%s' (%u x %u)\n", i, bsp->textures[i]->name, bsp->textures[i]->width, bsp->textures[i]->height);

		if (! bsp->textures[i]->palette) {
			printf("\tNo embedded texture data\n");
		} else {
			for (int j = 0; j < wad_count; j ++) {
				for (unsigned int k = 0; k < wad[j]->texture_count; k ++) {
					if (strcmp(bsp->textures[i]->name, wad[j]->textures[k]->name) == 0 &&
							bsp->textures[i]->width == wad[j]->textures[k]->width &&
							bsp->textures[i]->height == wad[j]->textures[k]->height &&
							bsp->textures[i]->palette_count == wad[j]->textures[k]->palette_count &&
							memcmp(bsp->textures[i]->palette, wad[j]->textures[k]->palette, 3 * bsp->textures[i]->palette_count) == 0 &&
							memcmp(bsp->textures[i]->level[0], wad[j]->textures[k]->level[0], bsp->textures[i]->width * bsp->textures[i]->height) == 0 &&
							memcmp(bsp->textures[i]->level[1], wad[j]->textures[k]->level[1], (bsp->textures[i]->width >> 1) * (bsp->textures[i]->height >> 1)) == 0 &&
							memcmp(bsp->textures[i]->level[2], wad[j]->textures[k]->level[2], (bsp->textures[i]->width >> 2) * (bsp->textures[i]->height >> 2)) == 0 &&
							memcmp(bsp->textures[i]->level[3], wad[j]->textures[k]->level[3], (bsp->textures[i]->width >> 3) * (bsp->textures[i]->height >> 3)) == 0)
					{
						printf("\tMatched to texture in %s\n", wad[j]->name);

						// clean the texture out
						free(bsp->textures[i]->palette); bsp->textures[i]->palette = NULL;
						free(bsp->textures[i]->level[0]); bsp->textures[i]->level[0] = NULL;
						free(bsp->textures[i]->level[1]); bsp->textures[i]->level[1] = NULL;
						free(bsp->textures[i]->level[2]); bsp->textures[i]->level[2] = NULL;
						free(bsp->textures[i]->level[3]); bsp->textures[i]->level[3] = NULL;
						bsp->textures[i]->palette_count = 0;

						goto found;
					}
				}
			}
			printf("\tNo match to texture found in wad files\n");
found:
			;

		}
	}

	// fix entity wad list


	// write BSP output
	write_bsp(bsp, argv[2]);
}
