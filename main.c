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
	struct s_wad** wad = (struct s_wad **)u_calloc(wad_count * sizeof(struct wad*));
	for (int i = 3; i < argc; i ++)
		wad[i-3] = read_wad(argv[i]);

	// Warnings for duplicate texture names
	for (int i = 0; i < wad_count - 1; i ++)
		for (int j = i + 1; j < wad_count; j ++)
			for (unsigned int k = 0; k < wad[i]->texture_count; k ++)
				for (unsigned int l = 0; l < wad[j]->texture_count; l ++)
					if (strcmp(wad[i]->textures[k]->name, wad[j]->textures[l]->name) == 0)
						printf("Note: Texture '%s' found in both '%s' and '%s', '%s' will be used.\n",
							wad[i]->textures[k]->name,
							wad[i]->name,
							wad[j]->name,
							wad[i]->name);

	// some flags before we begin
	unsigned int unreferenced_textures = 0;
	unsigned int embedded_textures = 0;
	unsigned int *wad_used = (unsigned int *)u_calloc(wad_count * sizeof(unsigned int));

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

	int wadlist_count = 0;
	char **wadlist = NULL;

	char *p = strstr(bsp->entity_lump, "\n\"wad\" \"");
	if (p) {
		printf("Initial WAD list: \"");

		// advance pointer until we are looking at the wad list itself
		p += 8;

		// parse wad list

		char *start = p;
		char *end = p - 1;
		do {
			end ++;
			printf("%c", *end);

			if (*end == '"' || *end == ';') {
				if (end > start) {
					// Quotes or semicolons terminate a WAD name
					char *temp_wad_name = (char *)u_calloc(end - start);
					for (int i=0; i < end - start; i ++)
					{
						temp_wad_name[i] = (char)tolower(start[i]);
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
							//  be in the Mystery WAD
							wadlist = (char **)u_realloc(wadlist, sizeof(char *) * (wadlist_count + 1));
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
							wadlist = (char **)u_realloc(wadlist, sizeof(char *) * (wadlist_count + 1));
							wadlist[wadlist_count] = temp_wad_name;
							wadlist_count ++;
						}
					}
				}
				// in any case we set start ptr to right after the terminator
done:
				start = end + 1;
			} else if (*end == '/' || *end == '\\') {
				// directory separator
				//  advance start to end+1 (right after the sep.)
				start = end + 1;
			}
		} while (*end != '"');

		// final check: WADs on the CLI that are used but not in the wadlist
		for (int i = 0; i < wad_count; i ++) {
			if (! wad_used[i]) continue;

			// see if CLI wad is accounted for
			int is_matched = 0;
			for (int j = 0; j < wadlist_count; j ++) {
				if (strcmp(wad[i]->name, wadlist[j]) == 0) {
					is_matched = 1;
					break;
				}
			}

			// used WAD from CLI not in list - append it
			if (! is_matched) {
				// must be kept
				wadlist = (char **)u_realloc(wadlist, sizeof(char *) * (wadlist_count + 1));
				wadlist[wadlist_count] = strdup(wad[i]->name);
				wadlist_count ++;
			}
		}

		printf("\nFinal wadlist: \"");
		for (int j = 0; j < wadlist_count; j ++) {
			if (j > 0) printf(";");
			printf("%s", wadlist[j]);
		}
		printf("\"\n");

		// smash all these together into a new Entity List
		                                     // before and up to "        // " until end (incl. trailing null)
		size_t new_entity_lump_size = (p - bsp->entity_lump) + (bsp->entity_lump_size - (end - bsp->entity_lump));
		for (int j = 0; j < wadlist_count; j ++) {
			// semicolon separator for each WAD name beyond the first
			if (j > 0) new_entity_lump_size ++;
			// length of each WAD name
			new_entity_lump_size += strlen(wadlist[j]);
		}

		// we have calculated the size of the output string, ready to allocate it
		char *new_entity_lump = (char *)u_calloc( new_entity_lump_size );
		// copy first N bytes - everything up to, and including, "wad" "
		strncpy(new_entity_lump, bsp->entity_lump, p - bsp->entity_lump);

		// fill the wad list
		for (int j = 0; j < wadlist_count; j ++) {
			if (j > 0) strcat(new_entity_lump, ";");
			strcat(new_entity_lump, wadlist[j]);
		}

		// concatenate everything through the end
		strcat(new_entity_lump, end);

		// nuke the old entity list and replace
		free(bsp->entity_lump);
		bsp->entity_lump = new_entity_lump;
		bsp->entity_lump_size = new_entity_lump_size;

	} else {
		// TODO: consider other ways to search for "wad", or add a new one to the "classname" "worldspawn"
		fprintf(stderr, "Failed to find key \"wad\" in entity list\n");
		exit(EXIT_FAILURE);
	}


	// write BSP output
	write_bsp(bsp, argv[2]);
}
