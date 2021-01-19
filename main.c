#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "texture.h"
#include "bsp.h"
#include "wad.h"

#include "utils.h"

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
	struct s_wad** wad = u_calloc(wad_count * sizeof(struct wad*));
	for (int i = 3; i < argc; i ++)
		wad[i-3] = read_wad(argv[i]);

	// some flags before we begin
	unsigned int unreferenced_textures = 0;
	unsigned int embedded_textures = 0;
	unsigned int *wad_used = u_calloc(wad_count * sizeof(unsigned int));

	// identify duplicates
	for (unsigned int i = 0; i < bsp->texture_count; i ++) {
		printf("%03u: BSP texture '%s' (%u x %u)\n", i, bsp->textures[i]->name, bsp->textures[i]->width, bsp->textures[i]->height);

		if (! bsp->textures[i]->palette) {
			printf("\tNo embedded texture data\n");
			for (int j = 0; j < wad_count; j ++) {
				for (unsigned int k = 0; k < wad[j]->texture_count; k ++) {
					if (strcmp(bsp->textures[i]->name, wad[j]->textures[k]->name) == 0 &&
							bsp->textures[i]->width == wad[j]->textures[k]->width &&
							bsp->textures[i]->height == wad[j]->textures[k]->height) {
						// found a match in this WAD to referenced texture (same name, height and width)
						wad_used[j] ++;
						goto found;
					}
				}
			}
			// No match for this texture in any wad!  Could be unreferenced.
			unreferenced_textures ++;
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

						// found a match in this WAD to referenced texture (same name, height and width)
						wad_used[j] ++;

						goto found;
					}
				}
			}
			embedded_textures ++;
			printf("\tNo match to texture found in wad files\n");

		}
found:
		;
	}

	printf("Search results:\n\t%u embedded textures\n\t%u unreferenced (missing) textures\n", embedded_textures, unreferenced_textures);
	for (int i = 0; i < wad_count; i ++)
	{
		printf("\t%u textures from %s\n", wad_used[i], wad[i]->name);
	}

	// fix entity wad list
	//  we need to retrieve the "wad" key from "worldspawn" entity
	//  easiest way to do this is search for the string
	//  \n"wad" "

	unsigned int wadlist_count = 0;
	char **wadlist = NULL;

	unsigned char *p = strstr(bsp->raw_lump[0], "\n\"wad\" \"");
	if (p) {
		printf("Initial WAD list: '");

		// advance pointer until we are looking at the wad list itself
		p += 8;

		// parse wad list

		unsigned char *start = p;
		unsigned char *end = p;
		while (*end != '"') {
			printf("%c", *end);

			if (*end == ';') {
				// WAD list separator
				char *temp_wad_name = u_calloc(end - start);
				for (int i=0; i < end - start; i ++)
				{
					temp_wad_name[i] = tolower(start[i]);
				}

				// see if the WAD is in the wadlist already
				for (int i = 0; i < wadlist_count; i ++) {
					if (strcmp(wadlist[i], temp_wad_name) == 0) {
						// is a duplicate, can be skipped
						free(temp_wad_name);
						goto done;
					}
				}

				// see if the WAD is in the command-line?
				int matched_wad = -1;
				for (int i = 0; i < wad_count; i ++) {
					if (strcmp(wad[i]->name, temp_wad_name) == 0) {
						matched_wad = i;
						break;
					}
				}

				// WAD in wad-list but not on command-line
				if (matched_wad < 0) {
					if (! unreferenced_textures) {
						// consider it safe to prune but only if we also have zero
						//  unreferenced textures
						free(temp_wad_name);
					} else {
						// must be kept because the unreferenced texture might
						//  be in the Myster WAD
						wadlist = u_realloc(wadlist, sizeof(char *) * (wadlist_count + 1));
						wadlist[wadlist_count] = temp_wad_name;
						wadlist_count ++;
					}
				} else {
					// WAD is on CLI - we should keep it unless it has no used textures
					if (! wad_used[matched_wad]) {
						// WAD specified but unused, prune it
						free(temp_wad_name);
					} else {
						// must be kept
						wadlist = u_realloc(wadlist, sizeof(char *) * (wadlist_count + 1));
						wadlist[wadlist_count] = temp_wad_name;
						wadlist_count ++;
					}
				}
done:
				start = end + 1;
			} else if (*end == '/' || *end == '\\') {
				// directory separator
				//  advance start to end+1
				start = end + 1;
			}
			end ++;
		}
		// see if anything is in the final
		if (end > start) {
			// WAD list separator
			char *temp_wad_name = u_calloc(end - start);
			for (int i=0; i < end - start; i ++)
			{
				temp_wad_name[i] = tolower(start[i]);
			}

			// see if the WAD is in the wadlist already
			for (int i = 0; i < wadlist_count; i ++) {
				if (strcmp(wadlist[i], temp_wad_name) == 0) {
					// is a duplicate, can be skipped
					free(temp_wad_name);
					goto done2;
				}
			}

			// see if the WAD is in the command-line?
			int matched_wad = -1;
			for (int i = 0; i < wad_count; i ++) {
				if (strcmp(wad[i]->name, temp_wad_name) == 0) {
					matched_wad = i;
					break;
				}
			}

			// WAD in wad-list but not on command-line
			if (matched_wad < 0) {
				if (! unreferenced_textures) {
					// consider it safe to prune but only if we also have zero
					//  unreferenced textures
					free(temp_wad_name);
				} else {
					// must be kept because the unreferenced texture might
					//  be in the Myster WAD
					wadlist = u_realloc(wadlist, sizeof(char *) * (wadlist_count + 1));
					wadlist[wadlist_count] = temp_wad_name;
					wadlist_count ++;
				}
			} else {
				// WAD is on CLI - we should keep it unless it has no used textures
				if (! wad_used[matched_wad]) {
					// WAD specified but unused, prune it
					free(temp_wad_name);
				} else {
					// must be kept
					wadlist = u_realloc(wadlist, sizeof(char *) * (wadlist_count + 1));
					wadlist[wadlist_count] = temp_wad_name;
					wadlist_count ++;
				}
			}
		}
done2:

		// final check: WADs on the CLI that are used but not in the wadlist
		for (int i = 0; i < wad_count; i ++) {
			if (! wad_used[i]) continue;

			// see if CLI wad is accounted for
			int is_matched = 0;
			for (int j = 0; j < wadlist_count; j ++) {
				if (strcmp(wad[i]->name, wadlist[j]) == 0) {
					is_matched = i;
					break;
				}
			}

			// used WAD from CLI not in list - append it
			if (! is_matched) {
				// must be kept
				wadlist = u_realloc(wadlist, sizeof(char *) * (wadlist_count + 1));
				wadlist[wadlist_count] = strdup(wad[i]->name);
				wadlist_count ++;
			}
		}

		printf("'\n\tFinal wadlist: '");
		for (int j = 0; j < wadlist_count; j ++) {
			printf("%s;", wadlist[j]);
		}
		printf("'\n");

		// smash all these together into a new Entity List
		unsigned int new_entity_list_size = (p - bsp->raw_lump[0]) + (bsp->raw_lump_size[0] - (end - bsp->raw_lump[0])) + 1;
		for (int j = 0; j < wadlist_count; j ++) {
			if (j > 0) new_entity_list_size ++;
			new_entity_list_size += strlen(wadlist[j]);
		}

		char *entity_list = u_calloc( new_entity_list_size );
		strncpy(entity_list, bsp->raw_lump[0], p - bsp->raw_lump[0]);

		for (int j = 0; j < wadlist_count; j ++) {
			if (j > 0) strcat(entity_list, ";");
			strcat(entity_list, wadlist[j]);
		}

		strcat(entity_list, end);

		// nuke the old entity list and replace
		free(bsp->raw_lump[0]);
		bsp->raw_lump[0] = entity_list;
		bsp->raw_lump_size[0] = new_entity_list_size;

	} else {
		fprintf(stderr, "Failed to find key \"wad\" in entity list\n");
		exit(EXIT_FAILURE);
	}


	// write BSP output
	write_bsp(bsp, argv[2]);
}
