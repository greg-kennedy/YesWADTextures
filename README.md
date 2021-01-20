# YesWADTextures
Greg Kennedy, 2021

Reduce the size of GoldSrc (Half-Life, Counter-Strike 1.6, etc) .BSP files by eliminating embedded textures that already exist in .WAD files.

The official site for YesWADTextures is https://github.com/greg-kennedy/YesWADTextures.

Windows users can find a recent download in the "Releases" section.  Other users: clone (or download) the code, then run `make` in the root folder.

## Usage
Invoke the program like this:

    ./yeswadtextures input.bsp output.bsp wad1.wad <wad2.wad wad3.wad ...>

`input.bsp` will be scanned for embedded textures, and if found, each texture is tested against the supplied .WAD files for an exact match on texture data.  If a match is found, the redundant data is eliminated from the .BSP file.  After processing texture data, the Required WAD list will be updated, and the resulting new .BSP is written to `output.bsp`.

For example, [dm\_office.bsp](https://www.moddb.com/games/half-life/addons/dm-office) (from the cancelled Dreamcast port of Half-Life) is 802,136 bytes in size and contains a number of copied textures from `halflife.wad`.  This command:

    ./yeswadtextures dm_office.bsp dm_office_fixed.bsp halflife.wad

will prune all but one texture from `dm_office.bsp`, and update its Required WAD list to include `halflife.wad`.  The new output file measures 572,828 bytes - a 28.5% decrease!

## Warning
**This tool has not been extensively tested and may have .BSP-damaging bugs.  Keep backups.  It does not warn before overwriting the output file - be careful not to accidentally overwrite something valuable.  You have been warned!**

## Overview
A `.WAD` file is a collection of textures, used by games based on the Quake engine.  GoldSrc uses version 3 of the .WAD file to store and organize textures.  Half-Life ships with a handful of .WAD files, while mods (Counter-Strike) or expansions (Opposing Force) include additional packs for their content.  Map authors may also distribute a .WAD file to provide custom textures for their map.

Meanwhile, GoldSrc `.BSP` map files have a "texture" lump that stores texture information.  These can be texture names alone (which would reference textures from a .WAD file), or they may contain a full copy of a texture instead.  This allows a map to be distributed with custom textures that does not need a supplemental .WAD file.

The original Half-Life map compiler `qcsg.exe` provided a command-line switch `-nowadtextures` to embed texture data into .BSP files.  The drawback is that it copied *every* used texture into the map, including those from .WAD files that everyone already has (because they came with the game or mod).  Later versions of the tool (ZHLT) added more flexible ways to control texture inclusion, but retained `-nowadtextures` for backwards compatibility.

All this has led to a lot of .BSP files floating around with embedded textures that don't need to be there.  GoldSrc does not support texture compression, so textures can make up a significant portion of the filesize.  In addition, GoldSrc downloads from a remote server (unless properly configured for fast downloads) are throttled at something like 10-20kbps.  Players who don't already have a copy of the map are forced to wait a very long time - just to download copies of textures they already have.

`yeswadtextures` is a tool to undo the effect of `-nowadtextures`.  It accepts a .BSP file as input, and one or more .WAD files to search.  For every embedded texture in the .BSP, `yeswadtextures` will look for a match in the provided .WAD file list.  If found, the embedded texture is stripped from the .BSP and a reference to the .WAD is created instead.

`yeswadtextures` will also "clean up" the Required WAD list in the Entities Lump of a .BSP.  All absolute paths are turned into relative paths, and any verifiably unused .WAD files are removed from the list.  This is similar to the `-wadautodetect` switch in modern `hlcsg.exe` compilers, and can help resolve some "Couldn't find 'xyz.wad'" errors when the map author unintentionally included an unused .WAD file.

## Bugs and TODOs
It is recommended to specify all .WAD files on command line (or at least, those mentioned in the original Required WAD list).  `yeswadtextures` can prune unused .WAD file dependencies, but only if it can verify that they are actually unused.

Subtle issues may arise if multiple .WAD files have textures with the same name.  New .WAD file dependencies are added to the end of the list - this is usually correct, but try rearranging the command line in case of unusual output.  The tool does warn indicate when .WAD files have duplicate textures.  Check the output file for correctness carefully.

Eventual updates to this tool may include
* the ability to recognize and remove duplicate textures, regardless of name
* "texture normalization" to identify duplicates despite palette reordering, or changes in unused palette colors
* strip invisible (tool) textures, despite difference in actual texture data
* more robust entity lump parsing
* greater control over .WAD priority / warnings for .WAD priority issues

Please report other problems, bugs, etc. on the Issues tab.

## Further Information
These specifications were essential in developing `yeswadtextures`:
* HLBSP Project: [Unofficial BSP v30 File Spec](http://hlbsp.sourceforge.net/index.php?content=bspdef)
* HLBSP Project: [Unofficial WAD3 File Spec](http://hlbsp.sourceforge.net/index.php?content=waddef)
* Valve Developer Community: [WAD](https://developer.valvesoftware.com/wiki/WAD)

## License
Released under the BSD 3-Clause License, see [LICENSE](LICENSE) for more information.
