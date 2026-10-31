#pragma once

// 塗りつぶした四角形を描画
void drawFilledRectangle(float x, float y, float width, float height);

// 四角形の枠線を描画
void drawRectangleOutline(float x, float y, float width, float height);

// 文字列を画面中央に描画
void drawCenteredText(const char* text, float y);

// 数値付き文字列を画面中央に描画
void drawCenteredValue(const char* label, int value, float y);