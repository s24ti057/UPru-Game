#include "Renderer.h"
#include "GameConfig.h"
#include <glut.h>
#include <stdio.h>


// 塗りつぶされた四角形を描画
void drawFilledRectangle(float x, float y, float width, float height) {
    glBegin(GL_QUADS);

    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);

    glEnd();
}

// 四角形の枠線を描画
void drawRectangleOutline(float x, float y, float width, float height) {
    glBegin(GL_LINE_LOOP);

    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);

    glEnd();
}

// 中央揃え専用関数
void drawCenteredText(const char* text, float y)
{
    // 文字列全体の横幅を計算
    int textWidth = 0;

    const char* p = text;

    // 文字列の横幅を計算
    while (*p)
    {
        textWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *p);
        p++;
    }

    // 中央に配置するためのX座標
    float x =
        (GAME_WIDTH - static_cast<float>(textWidth)) / 2.0f;

    // 描画位置
    glRasterPos2f(x, y);

    // 文字を描画
    p = text;

    while (*p)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
        p++;
    }
}

// 中央揃えの数値付き文字列を描画
void drawCenteredValue(const char* label, int value, float y)
{
    char text[64];

    sprintf_s(text, sizeof(text), "%s : %d", label, value);

    drawCenteredText(text, y);
}