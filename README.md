# YesWADTextures
Greg Kennedy, 2021

Reduce the size of GoldSrc (Half-Life, Counter-Strike 1.6, etc) .BSP files by eliminating embedded textures that already exist in .WAD files.

## Usage
    ./yeswadtextures input.bsp output.bsp wad1.wad <wad2.wad wad3.wad ...>

## Overview
A .WAD file is a collection of textures, used by games based on the Quake engine.  GoldSrc uses version 3 of the .WAD file to store and organize textures.  Half-Life ships with a handful of .WAD files, while mods (Counter-Strike) or expansions (Opposing Force) often include additional packs for their content.  Map authors may also distribute a .WAD file to provide custom textures for their map.

Meanwhile, GoldSrc .BSP files have a "texture" lump that stores texture information.  These can be texture names alone (which would reference textures from a .WAD file), or they may contain a full copy of a texture instead.  This allows a map to be distributed with custom textures that does not need a supplemental .WAD file.

The original Half-Life BSP compiler `qcsg.exe` provided a command-line switch `-nowadtextures` to embed texture data into .BSP files.  The drawback is that it copied *every* used texture into the map, including those from .WAD files that everyone already has (because they came with the game or mod).  Later versions of the tool (ZHLT) added more flexible ways to control texture inclusion, but retained `-nowadtextures` for backwards compatibility.

All this has led to a lot of .BSP files floating around with embedded textures that don't need to be there.  GoldSrc does not support texture compression, so textures can make up a significant portion of the filesize.  In addition, GoldSrc downloads from a remote server (unless properly configured for fast downloads) are throttled at something like 10-20kbps.  Players who don't already have a copy of the map are forced to wait a very long time - just to download copies of textures they already have.

`yeswadtextures` is a tool to undo the effect of `-nowadtextures`.  It accepts a `.BSP` file as input, and one or more `.WAD` files to search.  For every embedded texture in the `.BSP`, `yeswadtextures` will look for a match in the provided `.WAD` list.  If found, the embedded texture is stripped from the `.BSP` and a reference to the `.WAD` is created instead.

`yeswadtextures` will also "clean up" the WAD list in the `entity` lump of a `.BSP`.  All absolute paths are turned into relative paths, and any verifiably unused `.WAD` files are removed from the list.  This can help with errors where authors had many `.WAD` files loaded, but only used textures from a few: the map inaccurately states that 

## Further Information
These specifications were essential in developing `yeswadtextures`:
* HLBSP Project: [Unofficial BSP v30 File Spec](http://hlbsp.sourceforge.net/index.php?content=bspdef)
* HLBSP Project: [Unofficial WAD3 File Spec](http://hlbsp.sourceforge.net/index.php?content=waddef)
* Valve Developer Community: [WAD](https://developer.valvesoftware.com/wiki/WAD)

## License
Released under the BSD 3-Clause License, see [LICENSE](LICENSE) for more information.
