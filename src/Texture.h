#pragma once


void InitTexture();
void DrawForestBackground(float x, float y, float width, float height);
void DrawIceBackground(float x, float y, float width, float height);
void DrawSnowBackground(float x, float y, float width, float height);

void DrawBackground(float screenWidth);
void DrawPlayerTexture(float x, float y, float width, float height, int animationStep, int direction, bool isAirborne, float velocityY, bool isWallSticking, bool isWallRight, bool isWallKick);
void DrawTitleBackground(float x, float y, float width, float height);
void DrawTitlePlayerTexture(float x, float y, float width, float height, int animationStep);
void DrawGoalPlayerTexture(float x, float y, float width, float height, int animationStep);
void DrawClearPlayerTexture(float x, float y, float width, float height);
void DrawResultTexture(float x, float y, float width, float height);
void DrawHowToPlayTexture(float x, float y, float width, float height);
void TimeTexture(float x, float y, float width, float height);
void GoalFlagTexture(float x, float y, float width, float height);

enum WallTextureType
{
    WALL_TEXTURE_TREE,
    WALL_TEXTURE_TREE1,
    WALL_TEXTURE_TREE2,
    WALL_TEXTURE_GRASS,
    WALL_TEXTURE_GRASS1,
    WALL_TEXTURE_STONE,
    WALL_TEXTURE_STONE1,
    WALL_SNOW_GRASS,
    WALL_SNOW_GRASS1,
    WALL_SNOW_STONE
};

enum NoStickWallTextureType
{
    NO_STICK_WALL_NORMAL,
    NO_STICK_WALL_ICE,
    NO_STICK_WALL_SKY
};

enum SlipperyWallTextureType
{
    SLIPPERY_WALL_NORMAL
};

enum BoostWallTextureType
{
    BOOST_WALL_NORMAL,
    BOOST_WALL_ICE
};

enum ReflectWallTextureType
{
    REFLECT_WALL_NORMAL,
    REFLECT_WALL_ICE
};

enum BoostFXWallTextureType
{
    BOOST_FX_WALL_NORMAL,
    BOOST_FX_WALL_ICE
};

enum BoostRXWallTextureType
{
    BOOST_RX_WALL_NORMAL,
    BOOST_RX_WALL_ICE
};

void DrawGroundTexture(float x, float y, float width, float height);

void DrawWallTexture(WallTextureType textureType, float x, float y, float width, float height, float tileSize);
void DrawNoStickWallTexture(NoStickWallTextureType textureType, float x, float y, float width, float height, float tileSize);

void DrawSlipperyWallTexture(SlipperyWallTextureType textureType, float x, float y, float width, float height, float tileSize);
void DrawBoostWallTexture(BoostWallTextureType textureType, float x, float y, float width, float height, float tileSize);
void DrawReflectWallTexture(ReflectWallTextureType textureType, float x, float y, float width, float height, float tileSize);
void DrawBoostFxWallTexture(BoostFXWallTextureType textureType, float x, float y, float width, float height, float tileSize);
void DrawBoostRxWallTexture(BoostRXWallTextureType textureType, float x, float y, float width, float height, float tileSize);
