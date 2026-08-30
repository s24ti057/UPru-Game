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