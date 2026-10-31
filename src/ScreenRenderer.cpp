#include "ScreenRenderer.h"
#include "Texture.h"
#include "Renderer.h"
#include "GameConfig.h"
#include "Ranking.h"
#include <glut.h>
#include <stdio.h>

// スタート画面の描画
void drawStartScreen(int menuIndex, int titleAnimationStep) {

    // タイトル背景の描画
    DrawTitleBackground(0.0f, 0.0f, GAME_WIDTH, GAME_HEIGHT);
    // タイトル画面用キャラクター
    DrawTitlePlayerTexture(590.0f, 280.0f, 120.0f, 120.0f, titleAnimationStep);

    // --下部40％:メニュー全体--
    float menuX = 220.0f;
    float menuY = 20.0f;
    float menuWidth = 360.0f;
    float menuHeight = 200.0f;

    // メニュー背景
    glColor3f(0.12f, 0.14f, 0.25f);
    drawFilledRectangle(menuX, menuY, menuWidth, menuHeight);
    // メニュー全体の外枠
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(3.0f);
    drawRectangleOutline(menuX, menuY, menuWidth, menuHeight);

    // --START--
    if (menuIndex == 0) {
        glColor3f(0.9f, 0.75f, 0.2f); // 選択中は黄色
        drawFilledRectangle(250.f, 155.0f, 300.0f, 45.0f);
        glColor3f(0.0f, 0.0f, 0.0f);
    }
    else {
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    // 文字列(START)の描画
    glRasterPos3f(370.0f, 170.0f, 0.0f);
    const char* startText = "START";

    while (*startText) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *startText);
        ++startText;
    }

    // --TIME--
    if (menuIndex == 1) {
        glColor3f(0.9f, 0.75f, 0.2f);
        drawFilledRectangle(250.f, 95.0f, 300.0f, 45.0f);
        glColor3f(0.0f, 0.0f, 0.0f);
    }
    else {
        glColor3f(1.0f, 1.0f, 1.0f);
    }

    // 文字列(TIME)の描画
    glRasterPos3f(375.0f, 110.0f, 0.0f);
    const char* timeText = "TIME";

    while (*timeText) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *timeText);
        ++timeText;
    }

    // --HOW TO PLAY--
    if (menuIndex == 2) {
        glColor3f(0.9f, 0.75f, 0.2f);
        drawFilledRectangle(250.0f, 35.0f, 300.0f, 45.0f);
        glColor3f(0.0f, 0.0f, 0.0f);
    }
    else {
        glColor3f(1.0f, 1.0f, 1.0f);
    }

    // 文字列(HOW TO PLAY)の描画
    glRasterPos3f(345.0f, 50.0f, 0.0f);
    const char* howToPlayText = "HOW TO PLAY";

    while (*howToPlayText) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *howToPlayText);
        ++howToPlayText;
    }
}

// タイム画面の描画
void drawTimeScreen(Ranking& ranking) {

    // 背景
    TimeTexture(0.0f, 0.0f, GAME_WIDTH, GAME_HEIGHT);

    // タイトル
    glColor3f(1.0f, 1.0f, 0.0f);
    drawCenteredText("CLEAR TIME RANKING", 500.0f);

    // ランキング
    for (int i = 0; i < ranking.getSize(); i++)
    {
        int time = ranking.getTime(i);
        char rankingText[64];

        // 記録なし
        if (time == -1)
        {
            sprintf_s(rankingText, sizeof(rankingText), "%d.  --:--:--", i + 1);
        }

        // 記録あり
        else
        {
            int minutes = time / 60000;

            int seconds = (time % 60000) / 1000;

            int centiseconds = (time % 1000) / 10;

            sprintf_s(rankingText, sizeof(rankingText), "%d.  %d:%02d:%02d", i + 1, minutes, seconds, centiseconds);
        }

        glColor3f(1.0f, 1.0f, 1.0f);
        drawCenteredText(rankingText, 410.0f - i * 60.0f);
    }

    // 戻る
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCenteredText("ESC : RETURN TO TITLE", 110.0f);
}

// 遊び方の描画
void drawHowToPlayScreen() {
    DrawHowToPlayTexture(0.0f, 0.0f, GAME_WIDTH, GAME_HEIGHT);
}

// クリア画面の描画
void drawClearScreen(int clearPage, int clearTime, int jumpCount, int fallCount)
{
    // 背景
    glColor3f(0.0f, 0.0f, 0.0f);
    drawFilledRectangle(0.0f, 0.0f, GAME_WIDTH, GAME_HEIGHT);

    // 1枚目
    if (clearPage == 0)
    {
        // ===== GAME CLEAR!!! =====
        const char* clearText = "GAME CLEAR!!!";
        float clearScale = 0.35f;

        // ストローク文字の元の横幅を計算
        float clearTextWidth = 0.0f;

        const char* p = clearText;
        while (*p)
        {
            clearTextWidth += glutStrokeWidth(GLUT_STROKE_ROMAN, *p);
            p++;
        }

        // 拡大縮小後の横幅
        float scaledClearWidth = clearTextWidth * clearScale;

        // 画面中央に来るX座標
        float clearX = (GAME_WIDTH - scaledClearWidth) / 2.0f;

        glColor3f(1.0f, 1.0f, 0.0f);
        glLineWidth(3.0f);

        glPushMatrix();
        glTranslatef(clearX, 370.0f, 0.0f);
        glScalef(clearScale, clearScale, 1.0f);

        p = clearText;
        while (*p)
        {
            glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
            p++;
        }

        glPopMatrix();

        // ===== タイム =====
        int minutes = clearTime / 60000;
        int seconds = (clearTime % 60000) / 1000;
        int centiseconds = (clearTime % 1000) / 10;

        char timeText[64];
        sprintf_s(
            timeText,
            sizeof(timeText),
            "CLEAR TIME : %d:%02d:%02d",
            minutes,
            seconds,
            centiseconds
        );

        // ビットマップ文字列の横幅を計算
        int timeTextWidth = 0;

        const char* str = timeText;
        while (*str)
        {
            timeTextWidth += glutBitmapWidth(
                GLUT_BITMAP_HELVETICA_18,
                *str
            );

            str++;
        }

        // タイム文字列を画面中央に配置
        float timeX =
            (GAME_WIDTH - static_cast<float>(timeTextWidth)) / 2.0f;

        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2f(timeX, 300.0f);

        str = timeText;
        while (*str)
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *str);
            str++;
        }

        DrawClearPlayerTexture(40, 30, 400, 400); // キャラクター描画

        return;
    }

    // 2枚目
    if (clearPage == 1)
    {
        // BGM CREDIT
        const char* creditTitle = "BGM CREDIT";
        float titleScale = 0.30f;

        // 文字幅を計算
        float titleWidth = 0.0f;
        const char* p = creditTitle;

        while (*p)
        {
            titleWidth += glutStrokeWidth(GLUT_STROKE_ROMAN, *p);
            p++;
        }

        float scaledTitleWidth = titleWidth * titleScale;
        float titleX =
            (GAME_WIDTH - scaledTitleWidth) / 2.0f;

        // タイトル描画
        glColor3f(1.0f, 1.0f, 0.0f);
        glLineWidth(3.0f);

        glPushMatrix();
        glTranslatef(titleX, 500.0f, 0.0f);
        glScalef(titleScale, titleScale, 1.0f);

        p = creditTitle;

        while (*p)
        {
            glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
            p++;
        }
        glPopMatrix();

        glColor3f(1.0f, 1.0f, 1.0f);

        // TITLE BGM
        drawCenteredText("TITLE BGM", 420.0f);
        drawCenteredText("Far Horizon", 390.0f);
        drawCenteredText("Music : Yuki Nozawa", 360.0f);

        // STAGE BGM
        drawCenteredText("STAGE BGM", 290.0f);
        drawCenteredText("Never Surrender", 260.0f);
        drawCenteredText("Music : YouFulca", 230.0f);

        // GOAL BGM
        drawCenteredText("GOAL BGM", 160.0f);
        drawCenteredText("Hoshi Carnival", 130.0f);
        drawCenteredText("Music : Momijiba Music", 100.0f);
        drawCenteredText("Official Website : https://music.storyinvention.com/", 70.0f);

    }

    // 3枚目
    if (clearPage == 2)
    {
        glColor3f(1.0f, 1.0f, 1.0f);

        // JUMP SE
        drawCenteredText("JUMP SE", 420.0f);
        drawCenteredText("jump09", 390.0f);
        drawCenteredText("SE : TK'S FREE SOUND FX", 360.0f);
        drawCenteredText("Official Website : https://taira-komori.net/", 330.0f);

        // CHARGE MAX SE
        drawCenteredText("CHARGE MAX SE", 260.0f);
        drawCenteredText("Confirm Button 20", 230.0f);
        drawCenteredText("SE : Sound Effect Lab", 200.0f);


        return;
    }

    // 4枚目
    if (clearPage == 3)
    {
        // リザルト画面の背景描画
        DrawResultTexture(0.0f, 0.0f, GAME_WIDTH, GAME_HEIGHT);
        glColor3f(1.0f, 1.0f, 0.0f);

        // CLEAR TIME 
        int minutes = clearTime / 60000;
        int seconds = (clearTime % 60000) / 1000;
        int centiseconds = (clearTime % 1000) / 10;

        char timeText[64];

        sprintf_s(
            timeText,
            sizeof(timeText),
            "CLEAR TIME : %d:%02d:%02d",
            minutes,
            seconds,
            centiseconds
        );

        drawCenteredText(timeText, 400.0f);

        // JUMP COUNT
        drawCenteredValue("JUMP COUNT", jumpCount, 350.0f);

        // FALL COUNT
        drawCenteredValue("FALL COUNT", fallCount, 300.0f);

        drawCenteredText("Enter : RETURN TO TITLE", 230.0f);

        return;
    }

}

// プレイタイムを右下に表示
void drawPlayTime(int playStartTime, int totalPauseTime)
{
    // ゲーム開始からの経過時間（ミリ秒）
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    int elapsedMilliseconds = currentTime - playStartTime - totalPauseTime;


    // 分・秒・1/100秒に変換
    int minutes = elapsedMilliseconds / 60000;
    int seconds = (elapsedMilliseconds % 60000) / 1000;
    int centiseconds = (elapsedMilliseconds % 1000) / 10;

    char timeText[64];

    sprintf_s(timeText, sizeof(timeText), "TIME: %d:%02d:%02d", minutes, seconds, centiseconds);

    // 文字色
    glColor3f(1.0f, 1.0f, 1.0f);

    // ゲーム画面の右下
    glRasterPos3f(625.0f, 20.0f, 0.0f);

    const char* str = timeText;

    while (*str) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *str);
        ++str;
    }
}

// ポーズ画面描画関数
void drawPauseScreen(int pauseMenuIndex)
{

    // 半透明の黒を重ねる
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    drawFilledRectangle(0.0f, 0.0f, GAME_WIDTH, GAME_HEIGHT);

    glDisable(GL_BLEND);

    // PAUSE
    glColor3f(1.0f, 1.0f, 0.0f);
    drawCenteredText("PAUSE", 400.0f);

    // メニュー
    if (pauseMenuIndex == 0)
    {
        glColor3f(1.0f, 1.0f, 0.0f);
        drawCenteredText("> CONTINUE <", 300.0f);

        glColor3f(1.0f, 1.0f, 1.0f);
        drawCenteredText("QUIT TO TITLE", 250.0f);
    }
    else
    {
        glColor3f(1.0f, 1.0f, 1.0f);
        drawCenteredText("CONTINUE", 300.0f);

        glColor3f(1.0f, 1.0f, 0.0f);
        drawCenteredText("> QUIT TO TITLE <", 250.0f);
    }
}