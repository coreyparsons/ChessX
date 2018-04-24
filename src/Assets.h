#ifndef ASSETS_H
#define ASSETS_H

enum OtherTextures
{
	OT_BLANK,
	OT_CLOCK,
	OT_SIZE
};

struct Assets
{
	bool init = false;
	Texture pieceTextures[PC_SIZE][PT_SIZE] = {};
	Texture otherTextures[OT_SIZE] = {};
	TTF_Font* hemiHead;
};

#endif
