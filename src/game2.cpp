#include <stdio.h>
#include <stdlib.h>
#include <glut.h>
#include <cmath>
#include <tchar.h>
#include <iostream>
#include <fstream>

#include <Windows.h>
#include <mmsystem.h>

#include "Texture.h"

#pragma comment(lib, "winmm.lib")

#define _USE_MATH_DEFINES

using namespace std;

const float M_PI = 3.14159265358979f;


// プレイヤー
struct Player {
    float x, y; // 位置
    float w, h;  // サイズ
    float vx, vy;  // 速度
};

// 前フレームの位置を保存する変数
float prevX = 0.0f;
float prevY = 0.0f;

// 反射後数フレーム保持する変数
int reflectCooldown = 0;

// コヨーテタイム(足場から離れて数フレームだけジャンプを許可)変数
int coyoteTimer = 0;
const int COYOTE_TIME = 6; // 約0.1秒のコヨーテタイム

int wallKickTimer = 0; // 壁キック表示用の変数

Player player;

// 足場（地面）
struct Platform {
    float x, y;
    float w, h;
};

Platform ground;

// 壁の種類
enum WallType {
    WALL_NORMAL, // 通常壁
    WALL_NO_STICK, // くっつけない壁
    WALL_SLIPPERY, // 氷壁
    WALL_BOOST, // 加速壁
    WALL_REFLECT, // 反射壁
    WALL_BOOST_FX, // X軸正に加速する加速床
    WALL_BOOST_RX // X軸負に加速する加速床
};

WallType currentWallType = WALL_NORMAL;

// 壁
struct Wall {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    WallType type = WALL_NORMAL;

    bool isMoving = false; // 壁が動くかどうか
    float vy = 0.0f; // 壁の上下移動速度
    float moveMinY = 0.0f;
    float moveMaxY = 0.0f;

    float prevY = 0.0f; // 前フレームの壁のY座標
    float deltaY = 0.0f; // 今フレームに動いた量(前フレームからの移動量)

    float boostX = 0.0f; // 横方向の加速力

    // 通常壁(床)の色(初期の色)
    float r = 0.6f;
    float g = 0.6f;
    float b = 1.0f;
};

const int MAX_WALLS = 200;
Wall walls[MAX_WALLS];
int wallCount = 0;
// 壁の上面が露出しているか調べる
bool isWallTopExposed(int wallIndex)
{
    Wall& w = walls[wallIndex];

    float wallTop = w.y + w.h;

    const float EPSILON = 1.0f;

    for (int j = 0; j < wallCount; j++)
    {
        if (j == wallIndex)
            continue;

        Wall& other = walls[j];

        // X方向に重なっているか
        bool overlapX =
            w.x < other.x + other.w &&
            w.x + w.w > other.x;

        // この壁の上面に別の壁の下面が接しているか
        bool touchingTop =
            fabs(wallTop - other.y) <= EPSILON;

        if (overlapX && touchingTop)
        {
            // 上に別の壁があるので、この上面は床ではない
            return false;
        }
    }

    return true;
}

// 現在張り付いている壁の番号
// -1 はどの壁にも張り付いていない
int stickingWallIndex = -1;

// Wall leftWall;
//  Wall rightWall;

// プレイヤーの状態
enum PlayerState {
    GROUND,
    AIR,
    WALL_LEFT,
    WALL_RIGHT
};

PlayerState state = AIR;

int playerWalkStep = 1;
int playerWalkTimer = 0;
// 1：右向き、-1：左向き
int playerDirection = 1;

int titleAnimationStep = 0;
int titleAnimationTimer = 0;

// ゴール演出処理
int goaltimer;
float goalZoom = 1.0f;
int goalAnimationStep = 0;
int goalAnimationTimer = 0;

// クリア画面変数
int clearPage = 0;
int clearPageTimer = 0;
int jumpCount = 0; // ジャンプカウント
int fallCount = 0; // 落下カウント
float fallStartY = 0.0f;
bool wasFalling = false;
bool fallCounted = false; // 落下フラグ

enum GameScreen {
    SCREEN_START,       // スタート画面
    SCREEN_PLAYING,     // ゲーム中
    SCREEN_TIME,        // タイム画面
    SCREEN_HOW_TO_PLAY, // 遊び方画面
    SCREEN_PAUSE,
    SCREEN_GOAL,        // ゴール演出
    SCREEN_CLEAR        // クリア画面
};

GameScreen gameScreen = SCREEN_START; // 最初はスタート画面

// メニューの選択位置
// 0：スタート
// 1：タイム
// 2：遊び方
int menuIndex = 0;

// ポーズ画面の選択位置
// 0 : countinue
// 1 : quit to title
int pauseMenuIndex = 0;

bool chargeMaxSoundPlayed = false; // 最大ため音を管理する変数

// プレイタイム処理
int playStartTime = 0; // プレイタイム計測
int clearTime = 0; // クリアタイムを固定する
int pauseStartTime = 0;   // ポーズを開始した時刻
int totalPauseTime = 0;   // ポーズしていた合計時間

// タイムを保存する変数
const int RANKING_SIZE = 5; // 上位5位まで保存
int bestTimes[RANKING_SIZE] ={ -1, -1, -1, -1, -1};

// 風エリアの設定
// 風が吹くY座標の範囲
float windMinY = 500.0f;
float windMaxY = 700.0f;

float windSpeedX = 1.5f; // 風の横方向の強さ(正の値なら右、負の値なら左)

// キー入力フラグ
bool KeyLeftON = false;
bool KeyRightON = false;
bool KeyStickON = false;
bool JumpKeyON = false;

float jumpCharge = 0.0f; // ジャンプチャージ変数

// カメラ変数
float cameraX = 0.0f;
float cameraY = 0.0f;

const int GAME_WIDTH = 800;
const int GAME_HEIGHT = 600;

// ウィンドウサイズの設定
int winW = GAME_WIDTH; // ウィンドウの横幅
int winH = GAME_HEIGHT; // ウィンドウの縦幅

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

    while (*p)
    {
        textWidth += glutBitmapWidth(
            GLUT_BITMAP_HELVETICA_18,
            *p
        );

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
        glutBitmapCharacter(
            GLUT_BITMAP_HELVETICA_18,
            *p
        );

        p++;
    }
}

void drawCenteredValue(const char* label, int value, float y)
{
    char text[64];

    sprintf_s(text, sizeof(text), "%s : %d", label, value);

    drawCenteredText(text, y);
}

// スタート画面の描画
void drawStartScreen() {
    // --背景--
   /* glColor3f(0.08f, 0.10f, 0.20f);
    drawFilledRectangle(0.0f, 0.0f, GAME_WIDTH, GAME_HEIGHT);
    
    // --上部60％:タイトル--
    glColor3f(1.0f, 1.0f, 1.0f);
    // タイトル1行目
    glRasterPos3f(335.0f, 460.0f, 0.0f);

    const char* title = "JUST CLIMB...";

    while (*title) {
        glutBitmapCharacter(
            GLUT_BITMAP_TIMES_ROMAN_24,
            *title
        );

        ++title;
    }

    // --上部60％と下部40％の境界線--
    glColor3f(0.8f, 0.8f, 0.8f);
    glLineWidth(2.0f);

    glBegin(GL_LINES);
    glVertex2f(80.0f, 240.0f);
    glVertex2f(720.0f, 240.0f);
    glEnd();
    */
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
void drawTimeScreen() {
    // 背景
    /*
    glColor3f(0.08f, 0.10f, 0.20f);
    drawFilledRectangle(0.0f, 0.0f, GAME_WIDTH, GAME_HEIGHT);
    */
    TimeTexture(0.0f, 0.0f, GAME_WIDTH, GAME_HEIGHT);
    
    // タイトル
    glColor3f(1.0f, 1.0f, 0.0f);
    drawCenteredText("CLEAR TIME RANKING", 500.0f);

    // ランキング
    for (int i = 0; i < RANKING_SIZE; i++)
    {
        char rankingText[64];

        // 記録なし
        if (bestTimes[i] == -1)
        {
            sprintf_s(rankingText, sizeof(rankingText), "%d.  --:--:--", i + 1);
        }

        // 記録あり
        else
        {
            int minutes = bestTimes[i] / 60000;

            int seconds = (bestTimes[i] % 60000) / 1000;

            int centiseconds = (bestTimes[i] % 1000) / 10;

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
void drawClearScreen()
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
            glutBitmapCharacter(
                GLUT_BITMAP_HELVETICA_18,
                *str
            );

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
            titleWidth += glutStrokeWidth(
                GLUT_STROKE_ROMAN,
                *p
            );
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
            glutStrokeCharacter(
                GLUT_STROKE_ROMAN,
                *p
            );
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
        DrawResultTexture(0.0f, 0.0f, GAME_WIDTH, GAME_HEIGHT); // リザルト画面の背景描画

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
void drawPlayTime()
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
        glutBitmapCharacter(
            GLUT_BITMAP_HELVETICA_18,
            *str
        );

        ++str;
    }
}

// 背景描画関数 

void drawGameBackground()
{
    // 氷エリア
    if (player.y >= 6000.0f) {

        // 水色の背景
        glColor3f(0.65f, 0.85f, 0.95f);
        drawFilledRectangle(
            0.0f,
            0.0f,
            GAME_WIDTH,
            14000.0f
        );

        // 上側を少し白くして、冷たい雰囲気を出す
        if (player.y >= 8500.0f)
        {
            glColor3f(0.82f, 0.94f, 1.0f);
            drawFilledRectangle(
                0.0f,
                0.0f,
                GAME_WIDTH,
                11500.0f
            );
        }

        // 氷の筋のような模様
        glColor3f(0.90f, 0.98f, 1.0f);
        glLineWidth(2.0f);

        glBegin(GL_LINES);

        glVertex2f(50.0f, 500.0f);
        glVertex2f(250.0f, 350.0f);

        glVertex2f(300.0f, 550.0f);
        glVertex2f(470.0f, 420.0f);

        glVertex2f(550.0f, 500.0f);
        glVertex2f(750.0f, 330.0f);

        glVertex2f(80.0f, 200.0f);
        glVertex2f(220.0f, 100.0f);

        glVertex2f(500.0f, 220.0f);
        glVertex2f(680.0f, 100.0f);

        glEnd();
    }

    // 通常エリア
    else {
        // 空
        glColor3f(0.32f, 0.52f, 0.28f);
        drawFilledRectangle(0.0f, 0.0f, GAME_WIDTH, 6000.0f);

        // 遠くの木（少し暗い）
        glColor3f(0.08f, 0.25f, 0.08f);
        /*
        drawTree(80.0f, 120.0f);
        drawTree(210.0f, 80.0f);
        drawTree(360.0f, 160.0f);
        drawTree(520.0f, 100.0f);
        drawTree(690.0f, 140.0f); 
        */
    }
}

/*
void drawGameBackground1()
{
    DrawForestBackground(0.0f, 0.0f, (float)winW, (float)winH);
}
*/

// 加速壁の描画関数
void drawBoostWall(const Wall& w) {

    // --- 黄色のベース ---
    glColor3f(1.0f, 1.0f, 0.0f);  // 黄色
    glBegin(GL_QUADS);
    glVertex2f(w.x, w.y);
    glVertex2f(w.x + w.w, w.y);
    glVertex2f(w.x + w.w, w.y + w.h);
    glVertex2f(w.x, w.y + w.h);
    glEnd();

    // --- 黒い細い枠線 ---
    glColor3f(0.0f, 0.0f, 0.0f);  // 黒
    glLineWidth(2.0f);           // 細い線
    glBegin(GL_LINE_LOOP);
    glVertex2f(w.x, w.y);
    glVertex2f(w.x + w.w, w.y);
    glVertex2f(w.x + w.w, w.y + w.h);
    glVertex2f(w.x, w.y + w.h);
    glEnd();

    // --- 黒い ▲ を縦に3つ描く ---
    float segmentH = w.h / 3.0f;      // 縦に3分割
    float centerX = w.x + w.w * 0.5f; // 中央X

    for (int i = 0; i < 3; i++) {

        float yBottom = w.y + segmentH * i;        // 三角形の下位置
        float yTop = yBottom + segmentH * 0.9f; // 三角形の上位置（少し余白）

        float leftX = w.x + w.w * 0.2f;          // 左端
        float rightX = w.x + w.w * 0.8f;          // 右端

        glBegin(GL_TRIANGLES);

        // ▲（塗りつぶし）
        glVertex2f(centerX, yTop);     // 上の頂点
        glVertex2f(leftX, yBottom);  // 左下
        glVertex2f(rightX, yBottom);  // 右下

        glEnd();
    }
}

// 反射壁の描画関数
void drawReflectWall(const Wall& w) {

    // --- 紫のベース ---
    glColor3f(0.6f, 0.2f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(w.x, w.y);
    glVertex2f(w.x + w.w, w.y);
    glVertex2f(w.x + w.w, w.y + w.h);
    glVertex2f(w.x, w.y + w.h);
    glEnd();

    // --- 黒い枠線 ---
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(w.x, w.y);
    glVertex2f(w.x + w.w, w.y);
    glVertex2f(w.x + w.w, w.y + w.h);
    glVertex2f(w.x, w.y + w.h);
    glEnd();

    // --- 巨大な左右三角形（底辺＝壁の高さ） ---
    float cx = w.x + w.w * 0.5f;      // 中央X
    float cy = w.y + w.h * 0.5f;      // 中央Y

    float halfW = w.w * 0.45f;        // 横方向は壁のほぼ全体
    float halfH = w.h * 0.5f;         // ← 壁の高さの半分＝底辺が壁の高さになる

    glColor3f(0.0f, 0.0f, 0.0f);

    glBegin(GL_TRIANGLES);

    // 右向き三角形
    glVertex2f(cx - halfW, cy + halfH);  // 上の底辺
    glVertex2f(cx - halfW, cy - halfH);  // 下の底辺
    glVertex2f(cx, cy);          // 先端（中央）

    // 左向き三角形
    glVertex2f(cx + halfW, cy + halfH);
    glVertex2f(cx + halfW, cy - halfH);
    glVertex2f(cx, cy);

    glEnd();
}

// 加速床(正方向)の描画関数
void drawBoostFloorFX(const Wall& w) {

    // --- オレンジのベース ---
    glColor3f(1.0f, 0.5f, 0.0f);  // オレンジ
    glBegin(GL_QUADS);
    glVertex2f(w.x, w.y);
    glVertex2f(w.x + w.w, w.y);
    glVertex2f(w.x + w.w, w.y + w.h);
    glVertex2f(w.x, w.y + w.h);
    glEnd();

    // --- 黒い枠線 ---
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(w.x, w.y);
    glVertex2f(w.x + w.w, w.y);
    glVertex2f(w.x + w.w, w.y + w.h);
    glVertex2f(w.x, w.y + w.h);
    glEnd();

    // --- （右向き三角形を3つ） ---
    float segmentW = w.w / 3.0f;
    float centerY = w.y + w.h * 0.5f;

    for (int i = 0; i < 3; i++) {

        float xLeft = w.x + segmentW * i;
        float xRight = xLeft + segmentW * 0.9f;
        float midX = xLeft + segmentW * 0.45f;

        float topY = centerY + w.h * 0.3f;
        float botY = centerY - w.h * 0.3f;

        glColor3f(0.0f, 0.0f, 0.0f);

        glBegin(GL_TRIANGLES);
        glVertex2f(xLeft, botY);
        glVertex2f(xLeft, topY);
        glVertex2f(xRight, centerY);
        glEnd();
    }
}

// 加速床(負方向)の描画関数
void drawBoostFloorRX(const Wall& w) {

    // --- オレンジのベース ---
    glColor3f(1.0f, 0.5f, 0.0f);  // オレンジ
    glBegin(GL_QUADS);
    glVertex2f(w.x, w.y);
    glVertex2f(w.x + w.w, w.y);
    glVertex2f(w.x + w.w, w.y + w.h);
    glVertex2f(w.x, w.y + w.h);
    glEnd();

    // --- 黒い枠線 ---
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(w.x, w.y);
    glVertex2f(w.x + w.w, w.y);
    glVertex2f(w.x + w.w, w.y + w.h);
    glVertex2f(w.x, w.y + w.h);
    glEnd();

    // --- （左向き三角形を3つ） ---
    float segmentW = w.w / 3.0f;
    float centerY = w.y + w.h * 0.5f;

    for (int i = 0; i < 3; i++) {

        float xRight = w.x + w.w - segmentW * i;
        float xLeft = xRight - segmentW * 0.9f;
        float midX = xRight - segmentW * 0.45f;

        float topY = centerY + w.h * 0.3f;
        float botY = centerY - w.h * 0.3f;

        glColor3f(0.0f, 0.0f, 0.0f);

        glBegin(GL_TRIANGLES);
        glVertex2f(xRight, botY);
        glVertex2f(xRight, topY);
        glVertex2f(xLeft, centerY);
        glEnd();
    }
}

// ゲーム画面描画関数
void drawGameScene()
{
    // カメラの影響を受けるゲーム世界
    glPushMatrix();
    // ゲーム中だけカメラ移動
    if (gameScreen == SCREEN_GOAL)
    {
        // プレイヤーの中心
        float playerCenterX = player.x + player.w / 2.0f;
        float playerCenterY = player.y + player.h / 2.0f;

        // 画面中央へ移動
        glTranslatef(GAME_WIDTH / 2.0f, GAME_HEIGHT / 2.0f, 0.0f);

        // ズーム
        glScalef(goalZoom, goalZoom, 1.0f);

        // プレイヤー中心を原点へ
        glTranslatef(-playerCenterX, -playerCenterY, 0.0f);
    }
    else
    {
        // 通常カメラ
        glTranslatef(-cameraX, -cameraY, 0.0f);
    }

    DrawBackground((float)winW);
    GoalFlagTexture(450, 13020, 100, 100);


    // 風エリアを薄い水色で表示
    /*
    glColor3f(0.7f, 0.9f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, windMinY);
    glVertex2f(winW, windMinY);
    glVertex2f(winW, windMaxY);
    glVertex2f(0.0f, windMaxY);
    glEnd();
    */

    // プレイヤー
    /*
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_QUADS);
    glVertex2f(player.x, player.y);
    glVertex2f(player.x + player.w, player.y);
    glVertex2f(player.x + player.w, player.y + player.h);
    glVertex2f(player.x, player.y + player.h);
    glEnd();
    */
    const bool isAirborne = (state == AIR); // 空中にいるかどうか
    const bool isWallSticking = (state == WALL_LEFT || state == WALL_RIGHT); // 壁に触れているかどうか
    const bool isWallRight = (state == WALL_RIGHT); // 右壁に触れているかどうか
    bool isWallKick = (wallKickTimer > 0); // 壁キック中であるかどうか

    // プレイヤー描画
    if (gameScreen == SCREEN_GOAL)
    {
        DrawGoalPlayerTexture(player.x, player.y, player.w, player.h, goalAnimationStep);
    }
    else
    {
        DrawPlayerTexture(player.x, player.y, player.w, player.h, playerWalkStep, playerDirection, isAirborne, player.vy, isWallSticking, isWallRight, isWallKick);
    }

    // 地面
    glColor3f(0.3, 0.8, 0.3);
    glBegin(GL_QUADS);
    glVertex2f(ground.x, ground.y);
    glVertex2f(ground.x + ground.w, ground.y);
    glVertex2f(ground.x + ground.w, ground.y + ground.h);
    glVertex2f(ground.x, ground.y + ground.h);
    glEnd();

    // 壁の描画
    for (int i = 0; i < wallCount; i++) {

        Wall& w = walls[i];

        if (w.type == WALL_BOOST) {
            drawBoostWall(w);   // 加速壁は専用描画
            continue;           // 通常の四角描画をスキップ
        }

        if (w.type == WALL_REFLECT) {
            drawReflectWall(w); // 反射壁は専用描画
            continue;
        }

        if (w.type == WALL_BOOST_FX) {
            drawBoostFloorFX(w); // 加速床(正方向)は専用描画
            continue;
        }

        if (w.type == WALL_BOOST_RX) {
            drawBoostFloorRX(w); // 加速床(負方向)は専用描画
            continue;
        }

        if (walls[i].type == WALL_NORMAL) {
            glColor3f(w.r, w.g, w.b);   // 通常壁（青）
        }
        else if (walls[i].type == WALL_NO_STICK) {
            glColor3f(1.0, 0.3, 0.3);   // 特殊壁（赤）
        }
        else if (walls[i].type == WALL_SLIPPERY) {
            glColor3f(0.3, 0.8, 1.0); // 氷壁 (水色)
        }


        glBegin(GL_QUADS);
        glVertex2f(walls[i].x, walls[i].y);
        glVertex2f(walls[i].x + walls[i].w, walls[i].y);
        glVertex2f(walls[i].x + walls[i].w, walls[i].y + walls[i].h);
        glVertex2f(walls[i].x, walls[i].y + walls[i].h);
        glEnd();
    }

    // カメラ移動前の座標に戻す
    glPopMatrix();
}

// ポーズ画面描画関数
void drawPauseScreen()
{
    drawGameScene(); // まずゲーム画面を描画

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

// ランキング保存関数
void saveRanking()
{
    std::ofstream file("ranking.txt"); // ファイルへ書き込み(ファイルがない場合、新しく作成)

    // エラー処理
    if (!file)
    {
        return;
    }

    for (int i = 0; i < RANKING_SIZE; i++)
    {
        file << bestTimes[i] << "\n";
    }

    file.close(); // ranking.txtへの書き込み終了してファイルを閉じる
}

// 読み込み関数
void loadRanking()
{
    std::ifstream file("ranking.txt"); // ファイルからの読み込み

    // ファイルがまだ存在しない場合(一度もクリアしていない場合)
    if (!file)
    {
        return;
    }

    // ファイルから過去のランキングデータを一行ずつ読み込む
    for (int i = 0; i < RANKING_SIZE; i++)
    {
        file >> bestTimes[i];

        if (!file)
        {
            bestTimes[i] = -1;
        }
    }

    file.close();
}


// クリアタイムをランキングに入れる関数(引数は今回クリアしたタイム)
void registerClearTime(int newTime)
{
    for (int i = 0; i < RANKING_SIZE; i++)
    {
        // 記録なし、または新しいタイムの方が速い
        if (bestTimes[i] == -1 || newTime < bestTimes[i])
        {
            // 下の順位を1つずつ後ろへずらす
            for (int j = RANKING_SIZE - 1; j > i; j--)
            {
                bestTimes[j] = bestTimes[j - 1];
            }

            bestTimes[i] = newTime;

            // ランキングをファイル保存(ranking.txtへ書き込み)
            saveRanking();

            break;
        }
    }
}

// BGMを再生する
void StartBGM()
{
   mciSendString(TEXT("close bgm"), NULL, 0, NULL); // 前回のステージBGMを閉じる

   // WAVファイルの"stage_bgm.wav"を開くという操作をbgmとする 
   mciSendString(TEXT("open \"stage_bgm1.wav\" type waveaudio alias bgm"), NULL, 0, NULL);

   mciSendString(TEXT("setaudio bgm volume to 300"), NULL, 0, NULL); // 音量調整

   mciSendString(TEXT("play bgm"), NULL, 0, NULL); // bgmを再生する
    
}

// BGM停止
void StopBGM()
{
    mciSendString(TEXT("stop bgm"), NULL, 0, NULL);
    mciSendString(TEXT("close bgm"), NULL, 0, NULL);
}

// ステージBGMのループ確認
void UpdateStageBGM()
{
    TCHAR mode[32];

    mciSendString(TEXT("status bgm mode"), mode, 32, NULL);

    if (_tcscmp(mode, TEXT("stopped")) == 0)
    {
        mciSendString(TEXT("play bgm from 0"), NULL, 0, NULL);
    }
}

// ジャンプ音
void PlayJumpSound()
{
    PlaySound(TEXT("jump.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}


// 最大溜め音
void PlayChargeMaxSound()
{
    mciSendString(TEXT("play charge from 0"), NULL, 0, NULL);
}

// タイトルBGM開始
void StartTitleBGM()
{
    mciSendString(TEXT("close title"), NULL, 0, NULL); // 前回のタイトルBGMを閉じる

    // WAVファイルの"8bit_Far-Horizon.wav"を開くという操作をtitleとする
    mciSendString(TEXT("open \"8bit_Far-Horizon.wav\" type waveaudio alias title"), NULL, 0, NULL);

    mciSendString(TEXT("play title"), NULL, 0, NULL); // titleを再生する
}

// タイトルBGM停止
void StopTitleBGM()
{
    mciSendString(TEXT("stop title"), NULL, 0, NULL);
    mciSendString(TEXT("close title"), NULL, 0, NULL);
}

// タイトルBGMのループ確認
void UpdateTitleBGM()
{
    TCHAR mode[32]; // MCIから返ってきたBGMの状態を入れておく文字列用の箱

    mciSendString(TEXT("status title mode"), mode, 32, NULL); // titleの現在のmodeをMCIに聞く

    // BGMが終了していたらはじめから再生する
    if (_tcscmp(mode, TEXT("stopped")) == 0)
    {
        mciSendString(TEXT("play title from 0"), NULL, 0, NULL);
    }
}

// ゴールBGM開始
void StartGoalBGM()
{
    mciSendString(TEXT("close goal"), NULL, 0, NULL);

    mciSendString(TEXT("open \"goal_bgm.wav\" type waveaudio alias goal"), NULL, 0, NULL);

    mciSendString(TEXT("play goal"), NULL, 0, NULL);
}

// ゴールBGM停止
void StopGoalBGM()
{
    mciSendString(TEXT("stop goal"), NULL, 0, NULL);
    mciSendString(TEXT("close goal"), NULL, 0, NULL);
}

// 描画
void display() {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // スタート画面
    if (gameScreen == SCREEN_START) {
        drawStartScreen();
        glutSwapBuffers(); // 実際に描画
        return; // これより下は描画されない
    }

    // タイム画面
    if (gameScreen == SCREEN_TIME) {
        drawTimeScreen();
        glutSwapBuffers();
        return;
    }

    // 遊び方画面
    if (gameScreen == SCREEN_HOW_TO_PLAY) {
        drawHowToPlayScreen();
        glutSwapBuffers();
        return;
    } 

    // ポーズ画面
    if (gameScreen == SCREEN_PAUSE)
    {
        drawPauseScreen();
        glutSwapBuffers();
        return;
    }

    // ゴール演出
    if (gameScreen == SCREEN_GOAL)
    {
        drawGameScene();
        glutSwapBuffers();
        return;
    }

    // クリア画面
    if (gameScreen == SCREEN_CLEAR) {
        drawClearScreen();
        glutSwapBuffers();
        return;
    }


    drawGameScene(); // 通常ゲーム画面描画

    drawPlayTime(); // プレイタイムを画面右下に表示

    glutSwapBuffers();
}




// 2D座標系
void reshape(int w, int h) {
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;

    // ウィンドウ全体を描画領域にする
    glViewport(0, 0, w, h);

    float gameAspect =
        static_cast<float>(GAME_WIDTH) /
        static_cast<float>(GAME_HEIGHT);

    float windowAspect =
        static_cast<float>(w) /
        static_cast<float>(h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (windowAspect > gameAspect) {
        // 横長の画面
        // 左右の表示範囲を均等に広げる
        float visibleWidth =
            GAME_HEIGHT * windowAspect;

        float extraWidth =
            (visibleWidth - GAME_WIDTH) / 2.0f;

        const float zoom = 1.2f;

        float viewWidth = GAME_WIDTH * zoom;
        float viewHeight = GAME_HEIGHT * zoom;

        float extraX = (viewWidth - GAME_WIDTH) / 2.0f;
        float extraY = viewHeight - GAME_HEIGHT;

        glOrtho(
            -extraX,
            GAME_WIDTH + extraX,
            0.0f,
            GAME_HEIGHT + extraY * 0.8f,
            -1,
            1
        );
    }
    else {
        // 縦長の画面
        // 上下の表示範囲を均等に広げる
        float visibleHeight =
            GAME_WIDTH / windowAspect;

        float extraHeight =
            (visibleHeight - GAME_HEIGHT) / 2.0f;

        glOrtho(
            0.0,
            GAME_WIDTH,
            -extraHeight,
            GAME_HEIGHT + extraHeight,
            -1.0,
            1.0
        );
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

   
}

// 特殊キー押下
void specialKey(int key, int x, int y) {
    // スタート画面での操作
    if (gameScreen == SCREEN_START) {
        if (key == GLUT_KEY_UP) { // 上キー
            menuIndex--;
            if (menuIndex < 0) menuIndex = 2;
        }
        if (key == GLUT_KEY_DOWN) { // 下キー
            menuIndex++;
            if (menuIndex > 2) menuIndex = 0;
        }
        glutPostRedisplay(); // display()をもう一度呼び出すことを要請
        return;
    }

    // ゲーム中の左右移動
    if (gameScreen == SCREEN_PLAYING) {
        if (key == GLUT_KEY_LEFT)  KeyLeftON = true;
        if (key == GLUT_KEY_RIGHT) KeyRightON = true;
    }

    // ポーズ画面での操作
    if (gameScreen == SCREEN_PAUSE)
    {
        if (key == GLUT_KEY_UP)
        {
            pauseMenuIndex--;

            if (pauseMenuIndex < 0) pauseMenuIndex = 1;
        }

        if (key == GLUT_KEY_DOWN)
        {
            pauseMenuIndex++;

            if (pauseMenuIndex > 1) pauseMenuIndex = 0;
        }

        glutPostRedisplay();
        return;
    }
}

// 特殊キー離し
void specialKeyUp(int key, int x, int y) {
    if (gameScreen != SCREEN_PLAYING) return; // ゲーム中以外は何もしない   

    if (key == GLUT_KEY_LEFT)  KeyLeftON = false;
    if (key == GLUT_KEY_RIGHT) KeyRightON = false;
}

// スペースキーでジャンプ
void keyboard(unsigned char key, int x, int y) {
    // スタート画面
    if (gameScreen == SCREEN_START) {
        if (key == ' ' || key == 13) { // SpaceまたはEnterで決定
            // START
            if (menuIndex == 0) {
                // プレイヤーを初期位置へ戻す
                jumpCount = 0;   
                fallCount = 0; 
                fallCounted = false;

                player.x = 400.0f; // 初期位置：400.0f
                player.y = 70.0f; // 初期位置： 70.0f
                player.vx = 0.0f;
                player.vy = 0.0f;

                state = GROUND;
                stickingWallIndex = -1;
                cameraX = 0.0f;
                cameraY = 0.0f;

                prevX = player.x;
                prevY = player.y;

                KeyLeftON = false;
                KeyRightON = false;
                KeyStickON = false;
                JumpKeyON = false;
                jumpCharge = 0.0f;
                chargeMaxSoundPlayed = false; 

                StopTitleBGM();
                StartBGM(); // ゲームBGM再生

                gameScreen = SCREEN_PLAYING; // スタート画面→ゲーム画面移行
                // StartBGM(); // ゲームBGM再生
                
                // ゲーム開始時刻を記録
                playStartTime = glutGet(GLUT_ELAPSED_TIME);
                // ポーズ時間をリセット
                pauseStartTime = 0;
                totalPauseTime = 0;
            }
            // TIME
            else if (menuIndex == 1) {
                gameScreen = SCREEN_TIME;
            }
            // HOW TO PLAY
            else if (menuIndex == 2) {
                gameScreen = SCREEN_HOW_TO_PLAY;
            }
            glutPostRedisplay();
        }
        return;
    }

    // クリア画面(EnterかSpaceでタイトル画面に戻る)
    if (gameScreen == SCREEN_CLEAR) {
        if (key == ' ' || key == 13) {
            StopGoalBGM();
            StartTitleBGM();

            gameScreen = SCREEN_START;
            menuIndex = 0;
            glutPostRedisplay();
            return;
        }
    }

    // TIME・HOW TO PLAY画面
    if (gameScreen == SCREEN_TIME || gameScreen == SCREEN_HOW_TO_PLAY) {
        // Escキー
        if (key == 27) gameScreen = SCREEN_START;
        glutPostRedisplay();
        return;
    }
    

    // ゲーム中
    if (gameScreen == SCREEN_PLAYING) {
        if (key == 27) // ESC
        {
            pauseStartTime = glutGet(GLUT_ELAPSED_TIME);
            gameScreen = SCREEN_PAUSE;
            pauseMenuIndex = 0;

            glutPostRedisplay();
            return;
        }

        if (key == ' ') {
            JumpKeyON = true;
        }
        if (key == 'z') {      // 張り付き開始
            KeyStickON = true;
        }

    }

    // ポーズ画面
    if (gameScreen == SCREEN_PAUSE)
    {
        // ESCでもゲームに戻る
        if (key == 27)
        {
            // 今回のポーズ時間を合計に追加
            totalPauseTime += glutGet(GLUT_ELAPSED_TIME) - pauseStartTime;

            gameScreen = SCREEN_PLAYING;
            glutPostRedisplay();
            return;
        }

        // Enter または Space で決定
        if (key == 13 || key == ' ')
        {
            // CONTINUE
            if (pauseMenuIndex == 0)
            {
                // 今回のポーズ時間を合計に追加
                totalPauseTime += glutGet(GLUT_ELAPSED_TIME) - pauseStartTime;

                gameScreen = SCREEN_PLAYING;
            }

            // QUIT TO TITLE
            else if (pauseMenuIndex == 1)
            {
                StopBGM();
                StartTitleBGM();

                gameScreen = SCREEN_START;
                menuIndex = 0;
            }

            glutPostRedisplay();
            return;
        }

        return;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    if (gameScreen != SCREEN_PLAYING) return; // ゲーム中以外はジャンプ処理などをしない

    if (key == ' ') {
        JumpKeyON = false;


        // チャージ量に応じてジャンプ力を変える
        float power = 8.0f + jumpCharge * 0.5f;  // 例：最大で +10 くらい
        if (power > 20.0f) power = 20.0f;



        // 地面ジャンプ
        if (state == GROUND || coyoteTimer > 0) {
            player.vy = power;
            state = AIR;
            coyoteTimer = 0; // 一度ジャンプしたら使用済みにする
            PlayJumpSound(); // ジャンプ音再生
            jumpCount++;   // ジャンプ数カウント

            jumpCharge = 0.0f;
            chargeMaxSoundPlayed = false;
            return;
        }

        // 左壁ジャンプ(右へ飛ぶ)
        if (state == WALL_LEFT) {
            state = AIR;          //  先に AIR にする（これが重要）
            KeyStickON = false;
            stickingWallIndex = -1;
            player.vy = power;
            player.vx = 6.0f + jumpCharge * 0.2f;  // ← 横方向も強化できる
            playerDirection = 1; // 右向きに更新
            wallKickTimer = 12; // 8フレームだけキック画像
      
            PlayJumpSound();
            jumpCount++;   // ジャンプ数カウント

            jumpCharge = 0.0f;
            chargeMaxSoundPlayed = false;
            return;
        }

        // 右壁ジャンプ（左へ飛ぶ）
        if (state == WALL_RIGHT) {
            state = AIR;          // 先に AIR にする（これが重要）
            KeyStickON = false;
            stickingWallIndex = -1;
            player.vy = power;
            player.vx = -6.0f - jumpCharge * 0.2f;
            playerDirection = -1; // 左向きに更新
            wallKickTimer = 12; // 8フレームだけキック画像
           
            PlayJumpSound();
            jumpCount++;   // ジャンプ数カウント

            jumpCharge = 0.0f;
            chargeMaxSoundPlayed = false;
            return;
        }

        jumpCharge = 0.0f;  // リセット
        chargeMaxSoundPlayed = false;
    }

    if (key == 'z') {      // 張り付き解除
        KeyStickON = false;
        state = AIR;
        currentWallType = WALL_NORMAL;
        stickingWallIndex = -1; // 壁から離れたら番号解除
    }
}


// 毎フレーム更新
void timer(int t) {
    if (gameScreen == SCREEN_START)
    {
        // タイトルBGMをループ
        UpdateTitleBGM();

        titleAnimationTimer++;

        if (titleAnimationTimer >= 6)
        {
            titleAnimationTimer = 0;

            titleAnimationStep++;

            if (titleAnimationStep >= 4)
            {
                titleAnimationStep = 0;
            }
        }
    }

    // ゴール演出
    if (gameScreen == SCREEN_GOAL) 
    {
        goaltimer++;

        // ズーム
        if (goalZoom < 2.0f)
        {
            goalZoom += 0.02f;
        }
        
        // ゴールアニメ
        goalAnimationTimer++;

        if (goalAnimationTimer >= 8)
        {
            goalAnimationTimer = 0;

            if (goalAnimationStep < 3)
            {
                goalAnimationStep++;
            }
        }


        // ゴール演出時間カウント
        if (goaltimer >= 300) 
        {
            clearPage = 0;
            clearPageTimer = 0;
            gameScreen = SCREEN_CLEAR;
        }
        glutPostRedisplay();
        glutTimerFunc(16, timer, 0);
        return;
    }

    // クリア画面切り替え
    if (gameScreen == SCREEN_CLEAR)
    {
        clearPageTimer++;

        // 1ページ目 → 2ページ目
        if (clearPage == 0 && clearPageTimer >= 300)
        {
            clearPage = 1;
            clearPageTimer = 0;
        }

        // 2ページ目 → 3ページ目
        else if (clearPage == 1 && clearPageTimer >= 420)
        {
            clearPage = 2;
            clearPageTimer = 0;
        }

        // 3ページ目 → 4ページ目
        else if (clearPage == 2 && clearPageTimer >= 420)
        {
            clearPage = 3;
            clearPageTimer = 0;
        }

        glutPostRedisplay();
        glutTimerFunc(16, timer, 0);
        return;
    }

    // ゲーム画面
    if (gameScreen == SCREEN_PLAYING)
    {
        // ステージBGMをループ
        UpdateStageBGM();
    }

    // ゲーム中以外はゲーム処理を進めない
    if (gameScreen != SCREEN_PLAYING) {

        glutPostRedisplay();
        glutTimerFunc(16, timer, 0);
        return;
    }

    // 前フレームの位置を保存
    // float prevX = player.x;
    // float prevY = player.y;

    // --- 動く壁の更新処理 ---
    for (int i = 0; i < wallCount; i++) {

        Wall& w = walls[i];

        w.prevY = w.y; // 動かす前の位置を保存
        w.deltaY = 0.0f; // 毎フレーム初期化

        if (w.isMoving) {

            float oldY = w.y;

            w.y += w.vy;

            // 上限・下限で反転
            if (w.y < w.moveMinY) {
                w.y = w.moveMinY;
                w.vy *= -1.0f;
            }

            if (w.y > w.moveMaxY) {
                w.y = w.moveMaxY;
                w.vy *= -1.0f;
            }

            // 実際に移動した量
            w.deltaY = w.y - oldY;
        }

    }

    // 張り付いている壁と一緒に移動
    if (stickingWallIndex >= 0 &&
        stickingWallIndex < wallCount &&
        KeyStickON &&
        (state == WALL_LEFT || state == WALL_RIGHT)) {

        Wall& stuckWall = walls[stickingWallIndex];

        player.y += stuckWall.deltaY;
    }

    // 張り付いている壁から離れたか確認
    if (stickingWallIndex >= 0 &&
        stickingWallIndex < wallCount &&
        (state == WALL_LEFT || state == WALL_RIGHT)) {

        Wall& stuckWall = walls[stickingWallIndex];

        // プレイヤーと壁が縦方向に重なっているか
        bool overlapY =
            player.y + player.h > stuckWall.y &&
            player.y < stuckWall.y + stuckWall.h;

        const float CONTACT_EPSILON = 1.0f;

        bool touchingSide = false;

        // プレイヤーの右側に壁がある状態
        if (state == WALL_LEFT) {
            touchingSide =
                fabs((player.x + player.w) - stuckWall.x)
                <= CONTACT_EPSILON;
        }

        // プレイヤーの左側に壁がある状態
        else if (state == WALL_RIGHT) {
            touchingSide =
                fabs(player.x - (stuckWall.x + stuckWall.w))
                <= CONTACT_EPSILON;
        }

        // 壁から外れたら張り付き状態を解除
        if (!overlapY || !touchingSide) {
            state = AIR;
            currentWallType = WALL_NORMAL;
            stickingWallIndex = -1;

            // KeyStickONは解除しなくてよい
            // Zを押し続けていれば、次に別の壁へ触れたとき再び張り付ける
        }
    }

    // 左右移動（壁張り付き中は横移動禁止）
    if (reflectCooldown > 0) {
        reflectCooldown--;
    }
    else {
        if (state != WALL_LEFT && state != WALL_RIGHT) {

            float inputSpeedX = 0.0f; // プレイヤー自身の入力速度

            if (KeyLeftON) {
                inputSpeedX = -4.0f;
                playerDirection = -1; // 空中でも左向きに変更
            }
            else if (KeyRightON) {
                inputSpeedX = 4.0f;
                playerDirection = 1; // 空中でも右向きに変更
            }

            // プレイヤーの上下端
            float playerBottom = player.y;
            float playerTop = player.y + player.h;

            // プレイヤーの一部でも y = windMinY ～ windMaxY に入っているか
            bool insideWindArea =
                (playerTop > windMinY) &&
                (playerBottom < windMaxY);
            float currentWindX = 0.0f; // 現在かかっている風

            if (insideWindArea) {
                currentWindX = 0.0f;  // 右向きの風(ここで風を調整)
            }
            player.vx = inputSpeedX + currentWindX; // キー入力と風を合成
        }
    }

    // ジャンプチャージ（地面 or 壁張り付き中）
    if (JumpKeyON && (state == GROUND || state == WALL_LEFT || state == WALL_RIGHT)) {
        jumpCharge += 0.3f;
        if (jumpCharge > 20.0f) {
            jumpCharge = 20.0f;

            // 最大まで溜まった瞬間だけ鳴らす
            if (chargeMaxSoundPlayed == false) {
                PlayChargeMaxSound();
                chargeMaxSoundPlayed = true;
            }
        }
    }

    if (wallKickTimer > 0)
    {
        wallKickTimer--;
    }


    // --- 重力処理 ---
    if (KeyStickON && (state == WALL_LEFT || state == WALL_RIGHT)) {

        // 壁張り付き中の挙動は currentWallType に従う
        if (currentWallType == WALL_NORMAL) {
            player.vy = 0;        // 完全停止
        }
        else if (currentWallType == WALL_SLIPPERY) {
            player.vy = -1.0f;    // ゆっくり落下
        }

    }
    else {
        // 通常の重力
        player.vy -= 0.6f;
    }

    // 速度を座標に反映
    player.x += player.vx;
    player.y += player.vy;

    // 歩行アニメーション
    bool isMoving = fabs(player.vx) > 0.1f && state == GROUND; // 動いているかの判定

    if (isMoving) {
        playerWalkTimer++;

        if (playerWalkTimer >= 6)
        {
            playerWalkTimer = 0;
            playerWalkStep++;

            if (playerWalkStep >= 4) {
                playerWalkStep = 0;
            }
        }

    }
    else {
        playerWalkStep = 1;      // 真ん中の画像
        playerWalkTimer = 0;
    }


    // 足場への着地を横衝突より先に処理
    bool landedOnWall = false;

    if (player.vy <= 0.0f) {

        for (int i = 0; i < wallCount; i++) {

            Wall& w = walls[i];

            // 上に乗れる種類だけ
            bool canStand =
                w.type == WALL_NORMAL ||
                w.type == WALL_NO_STICK ||
                w.type == WALL_BOOST_FX ||
                w.type == WALL_BOOST_RX;

            if (!canStand) {
                continue;
            }

            float oldWallTop = w.prevY + w.h;
            float newWallTop = w.y + w.h;

            // 横方向が重なっている
            bool overlapX =
                player.x + player.w > w.x &&
                player.x < w.x + w.w;

            // 前フレームでは足場の上にいて、
            // 今フレームで足場の上面まで落ちてきた
            bool crossedTop =
                prevY >= oldWallTop &&
                player.y <= newWallTop;

            if (overlapX && crossedTop && isWallTopExposed(i)) {

                player.y = newWallTop;
                player.vy = 0.0f;
                state = GROUND;
                currentWallType = WALL_NORMAL;
                stickingWallIndex = -1;

                // 加速床の場合
                if (w.type == WALL_BOOST_FX ||
                    w.type == WALL_BOOST_RX) {

                    player.vx = w.boostX;
                    state = AIR;
                    reflectCooldown = 10;
                }

                landedOnWall = true;
                break;
            }
        }
    }

    // 地面の上に乗る
    bool onGround =
        (player.x + player.w > ground.x) &&
        (player.x < ground.x + ground.w) &&
        (player.y <= ground.y + ground.h);

    if (onGround && player.vy <= 0) {
        player.y = ground.y + ground.h;
        player.vy = 0;
        state = GROUND;
    }

    // 壁判定（配列で処理）
    for (int i = 0; i < wallCount; i++) {

        Wall& w = walls[i];

        // プレイヤーがその壁に触れているか（AABB判定）
        bool touching =
            (player.x + player.w > w.x) &&
            (player.x < w.x + w.w) &&
            (player.y + player.h > w.y) &&
            (player.y < w.y + w.h);

        if (!touching)  continue; // 触れていなければ次の壁へ
        

        float oldWallBottom = w.prevY;
        float oldWallTop = w.prevY + w.h;

        float newWallBottom = w.y;
        float newWallTop = w.y + w.h;

        // 前フレームの位置(prevX, prevY)と比較して、どの方向から壁に入ってきたかを判定
        bool hitLeft =
            prevX + player.w <= w.x &&   // 前フレームは左側
            player.x + player.w > w.x;   // 今フレームで重なった


        bool hitRight =
            prevX >= w.x + w.w &&        // 前フレームは右側
            player.x < w.x + w.w;        // 今フレームで重なった


        bool hitBottom =
            (prevY + player.h <= oldWallBottom) &&
            (player.y + player.h >= newWallBottom);

        bool hitTop =
            (prevY >= oldWallTop) &&
            (player.y <= newWallTop);

        // --- 横衝突優先 ---
        if (hitLeft) {
            player.x = w.x - player.w; // 壁の左側に押し戻す(めり込み防止)

            // 反射壁
            if (w.type == WALL_REFLECT) {
                float speed = 12.0f;   // 好きな反射強さに調整してOK
                float angle = M_PI / 4.0f; // 45度（ラジアン

                // 左壁 → 左上45度へ飛ばす
                player.vx = -speed * cos(angle);   // 正のX方向
                player.vy = speed * sin(angle);   // 正のY方向


                state = AIR;
                KeyStickON = false;
                reflectCooldown = 15;  // 5フレーム横移動禁止（調整可）

                break;
            }

            // 加速壁（左側）
            if (w.type == WALL_BOOST) {
                player.vy = 20.0f;   // 上へ加速
                state = AIR;
                KeyStickON = false;
                break;
            }

            // zキーが押されていて、くっつける壁にいる時
            if (KeyStickON && w.type != WALL_NO_STICK && w.type != WALL_REFLECT) {
                state = WALL_LEFT;
                currentWallType = w.type;
                stickingWallIndex = i;
                player.vx = 0.0f;

                if (w.type == WALL_NORMAL) player.vy = 0; // 通常壁→停止
                if (w.type == WALL_SLIPPERY) player.vy = -1.0f;// 氷壁→ゆっくり落下

            }
        }
        else if (hitRight) {
            player.x = w.x + w.w; // 壁の右側に押し戻す(めり込み防止)

            // 反射壁
            if (w.type == WALL_REFLECT) {
                float speed = 12.0f;   // 好きな反射強さに調整してOK
                float angle = M_PI / 4.0f; // 45度（ラジアン

                // 右壁 → 右上45度へ飛ばす
                player.vx = speed * cos(angle);   // 正のX方向
                player.vy = speed * sin(angle);   // 正のY方向

                state = AIR;
                KeyStickON = false;
                reflectCooldown = 15;  // 5フレーム横移動禁止（調整可）

                break;
            }

            // 加速壁（右側）
            if (w.type == WALL_BOOST) {
                player.vy = 20.0f;   // 上へ加速
                state = AIR;
                KeyStickON = false;
                break;
            }


            if (KeyStickON && w.type != WALL_NO_STICK && w.type != WALL_REFLECT) {
                state = WALL_RIGHT;
                currentWallType = w.type;
                stickingWallIndex = i;
                player.vx = 0.0f;

                if (w.type == WALL_NORMAL) player.vy = 0; // 通常壁→停止
                if (w.type == WALL_SLIPPERY) player.vy = -1.0f; // 氷壁→ゆっくり落下

            }
        }
        // --- 縦衝突 ---
        else if (hitBottom) { // 壁の下からぶつかったとき(天井)
            if (w.type == WALL_NORMAL || w.type == WALL_NO_STICK || w.type == WALL_BOOST_FX || w.type == WALL_BOOST_RX || w.type == WALL_SLIPPERY) {
                player.y = w.y - player.h; // プレイヤーを壁の下面まで押し戻す
                player.vy = 0.0f; // 上昇を止める
                state = AIR;
                stickingWallIndex = -1;
            }
            else if (w.type == WALL_REFLECT) {
                // 反射壁：跳ね返る
                player.y = w.y - player.h;
                player.vy = -player.vy;   // 反射
                state = AIR;
            }

        }
        else if (hitTop) { 
            // 上面が別の壁でふさがれているなら、床として処理せず次の壁判定へ
            if (!isWallTopExposed(i))
            {
                continue;
            }
            // 壁の上から着地したとき
            if (w.type == WALL_NORMAL || w.type == WALL_NO_STICK) {
                player.y = w.y + w.h; // プレイヤーを壁の上面に置く
                player.vy = 0.0f; // 落下を止める
                state = GROUND;
                currentWallType = WALL_NORMAL;
                stickingWallIndex = -1;
            }
            else if (w.type == WALL_REFLECT) {
                // 反射壁：跳ね返る
                player.y = w.y + w.h;
                player.vy = -player.vy;   // 反射
                state = AIR;
            }
            else if (w.type == WALL_BOOST_FX || w.type == WALL_BOOST_RX) {
                // 加速床 : x方向に加速
                player.y = w.y + w.h; // プレイヤーを壁の上面に置く
                player.vx = w.boostX; // 横方向へ加速
                player.vy = 0.0f; // 落下を止める
                state = AIR;
                stickingWallIndex = -1;
                reflectCooldown = 10; // キー入力ですぐに速度を上書きされないようにする
            }
        }

        break;
    }

    // 地面・足場に乗っている間はコヨーテタイムを満タンにする
    if (landedOnWall || onGround) {
        coyoteTimer = COYOTE_TIME;
    } else {
        // 足場から離れた
        if (state == GROUND) state = AIR;
        if (coyoteTimer > 0) coyoteTimer--;
    }

    // 落下カウント
    // 落下開始
    if (player.vy < 0.0f && !wasFalling)
    {
        fallStartY = player.y;
        wasFalling = true;
        fallCounted = false;
    }

    // 落下中
    if (wasFalling && player.vy < 0.0f)
    {
        if (!fallCounted && fallStartY - player.y >= 200.0f)
        {
            fallCount++;
            fallCounted = true;
        }
    }

    // 落下終了
    if (player.vy >= 0.0f)
    {
        wasFalling = false;
    }

    // ゴール判定
    if (player.x + player.w >= 470.0f && player.y >= 13040.0f) {

        // スタート時間と合計ポーズ時間を差し引いてクリアタイムを計算
        clearTime = glutGet(GLUT_ELAPSED_TIME) - playStartTime - totalPauseTime;
        // ランキングに登録
        registerClearTime(clearTime);

        StopBGM();
        StartGoalBGM();
        goalAnimationStep = 0;
        goalAnimationTimer = 0;
        goalZoom = 1.0f;
        goaltimer = 0;
        gameScreen = SCREEN_GOAL;

        player.vx = 0.0f;
        player.vy = 0.0f;


        glutPostRedisplay();
        glutTimerFunc(16, timer, 0);
        return;
    }

    // カメラをプレイヤーに追従
    cameraX = 0.0f; // 横方向は固定
    float camOffsetY = 150.0f; // 縦方向のみプレイヤーを追従

    cameraY = player.y - GAME_HEIGHT / 2.0f + player.h / 2.0f - camOffsetY;

    // ステージ端でカメラを止める
    float stageBottom = 0.0f;
    float stageTop = 20000.0f;
    float maxCameraY = stageTop - GAME_HEIGHT;

    if (cameraY < stageBottom) {
        cameraY = stageBottom;
    }

    if (cameraY > maxCameraY) {
        cameraY = maxCameraY;
    }

    // 前フレームの位置を保存
    prevX = player.x;
    prevY = player.y;



    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}


// メイン
int main(int argc, char** argv) {

    char currentDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, currentDir);

    MessageBoxA(
        NULL,
        currentDir,
        "Current Directory",
        MB_OK
    );

    glutInit(&argc, argv);
    loadRanking();
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("UPる");
    // GLUTで作ったウィンドウを取得
    HWND hwnd = FindWindowA(NULL, "UPる");

    if (hwnd)
    {
        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        // サイズ変更できる枠を無効化
        style &= ~WS_THICKFRAME;
        // 最大化ボタンを無効化
        style &= ~WS_MAXIMIZEBOX;
      
        SetWindowLong(hwnd, GWL_STYLE, style);

        // 変更したスタイルを反映
        SetWindowPos(
            hwnd,
            NULL,
            0, 0, 0, 0,
            SWP_NOMOVE |
            SWP_NOSIZE |
            SWP_NOZORDER |
            SWP_FRAMECHANGED
        );
    }

    InitTexture();

    mciSendString(TEXT("open \"charge_max1.wav\" type waveaudio alias charge"), NULL, 0, NULL);
    StartTitleBGM();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKey);
    glutSpecialUpFunc(specialKeyUp);
    glutTimerFunc(16, timer, 0);

    // プレイヤー初期位置（地面の上）
    player.w = 40;
    player.h = 40;
    player.x = 400; // 初期位置400
    player.y = 70;  // ground.y + ground.h (player.y = 50 + 20 = 70が初期位置)
    player.vx = 0;
    player.vy = 0;

    // 地面
    ground.x = 40;
    ground.y = 50;
    ground.w = 720;
    ground.h = 20;

    // 壁の追加
    walls[0] = { 0, 0, 40, 300, WALL_NORMAL };      // 左壁
    walls[1] = { 760, 0, 40, 770, WALL_NORMAL };    // 右壁

    // 特殊壁（例：中央に配置）
    walls[2] = { 0, 300, 40, 1050, WALL_NO_STICK };   // ← 張り付けない壁
    walls[3] = { 0, 1350, 40, 240, WALL_NORMAL };

    walls[4] = { 300, 200, 200, 20, WALL_NORMAL };
    walls[5] = { 100, 400, 150, 20, WALL_NORMAL };
    walls[6] = { 660, 400, 100, 20, WALL_NORMAL };
    walls[7] = { 500, 720, 40, 200, WALL_NORMAL };
    // 特定の足場を緑色にする
    for (int i = 4; i <= 7; i++)
    {
        walls[i].r = 0.0f; walls[i].g = 0.4f; walls[i].b = 0.0f;
    }
    walls[8] = { 500, 920, 40, 200, WALL_NO_STICK };
    walls[9] = { 760, 770, 40, 300, WALL_NO_STICK };
    walls[10] = { 760, 1070, 40, 460, WALL_NORMAL };
    walls[11] = { 540, 1120, 40, 40, WALL_NO_STICK };

    walls[12] = { 500, 1300, 100, 20, WALL_NORMAL };
    walls[13] = { 240, 1450, 520, 80, WALL_NORMAL };
    walls[14] = { 300, 1350, 100, 20, WALL_NORMAL };
    walls[15] = { 100, 1200, 70, 20, WALL_NORMAL };
    walls[16] = { 250, 800, 90, 90 ,WALL_NORMAL };
    // 特定の足場を緑色にする
    for (int i = 12; i <= 16; i++)
    {
        walls[i].r = 0.0f; walls[i].g = 0.4f; walls[i].b = 0.0f;
    }
 
    walls[17] = { 200, 1450, 40, 80, WALL_NO_STICK };
    walls[18] = { 200, 1600, 40, 200, WALL_NO_STICK };
    walls[19] = { 180, 1590, 20, 10, WALL_NO_STICK };
    walls[20] = { 0, 1590, 40, 410, WALL_NO_STICK };
    walls[21] = { 0, 2000, 40, 870, WALL_NORMAL };
    walls[22] = { 180, 1810, 20, 10, WALL_NO_STICK };
    walls[23] = { 160, 1820, 20, 10, WALL_NO_STICK };
    
    walls[24] = { 660, 1650, 20 ,200, WALL_BOOST };
    walls[25] = { 400, 1850, 20, 200, WALL_BOOST };
    walls[26] = { 640, 2250, 40, 100, WALL_BOOST };
    walls[27] = { 380, 2500, 40, 50, WALL_BOOST };
    

    walls[28] = { 760, 1530, 40, 1470, WALL_NO_STICK };
    walls[29] = { 760, 3000, 40, 950, WALL_NORMAL };

    walls[30] = { 150, 2600, 20, 100, WALL_NORMAL };
   
    walls[31] = { 400, 3050, 20, 100, WALL_NORMAL };
    walls[31].isMoving = true; walls[31].vy = 5.0f; 
    walls[31].moveMinY = 2950.0f; walls[31].moveMaxY = 3150.0f;

    walls[32] = { 0, 2870, 40, 1830, WALL_NO_STICK };
    walls[33] = { 0, 4700, 40 , 50, WALL_NORMAL };
    
    walls[34] = { 5, 2870, 40, 130, WALL_REFLECT };
    walls[35] = { 140, 2950, 30, 20, WALL_NORMAL };
    walls[36] = { 170, 2930, 20, 20, WALL_NO_STICK };
    walls[37] = { 190, 2910, 20, 20, WALL_NO_STICK };
    walls[38] = { 120, 2950, 20, 20, WALL_NO_STICK };
    walls[39] = { 720, 2670, 40, 20, WALL_NORMAL };
    
    walls[40] = { 650, 3300, 20, 100, WALL_NORMAL };
    walls[40].isMoving = true; walls[40].vy = 4.0f;
    walls[40].moveMinY = 3200.0f; walls[40].moveMaxY = 3350.0f;
    
    walls[41] = { 550, 3500, 20, 250, WALL_REFLECT };
    walls[42] = { 755, 3650, 20, 150, WALL_REFLECT };
    walls[43] = { 755, 3550, 20, 50, WALL_REFLECT };
    walls[44] = { 300, 3500, 150, 20, WALL_BOOST_RX };
    walls[44].boostX = -20.0f;

    walls[45] = { 5, 3450, 40, 400, WALL_BOOST };
    walls[46] = { 200, 4100, 30, 30, WALL_NO_STICK };
    walls[47] = { 430, 4300, 30, 30, WALL_NO_STICK };
    walls[48] = { 540, 3950, 220, 20, WALL_BOOST_RX };
    walls[48].boostX = -12.0f;

    walls[48] = { 260, 4400, 30, 30, WALL_NO_STICK };
    walls[49] = { 130, 4550, 30, 30, WALL_NO_STICK };
    walls[50] = { 150, 4640, 130, 20, WALL_NO_STICK };
    walls[51] = { 650, 4500, 60, 30, WALL_NORMAL };

    walls[52] = { 760, 3600, 40, 50, WALL_NO_STICK };

    walls[53] = { 0, 4750, 40, 250, WALL_NO_STICK };
    

    walls[54] = { 150, 4660, 20, 240, WALL_NO_STICK };
    walls[55] = { 150, 4900, 20, 50, WALL_NORMAL };
    walls[56] = { 150, 4950, 20, 300, WALL_NO_STICK };
    walls[57] = { 150, 5250, 20, 40, WALL_NORMAL };
    walls[58] = { 150, 5290, 20, 110, WALL_NO_STICK };

    walls[59] = { 0, 5000, 40, 20, WALL_NORMAL };
    walls[60] = { 0, 5020, 40, 980, WALL_NO_STICK };

    walls[61] = { 300, 5600, 60, 20, WALL_NORMAL };
    walls[62] = { 200, 5800, 380, 200, WALL_NORMAL };
    walls[62].r = 0.3f; walls[62].g = 0.8f; walls[62].b = 1.0f;
    walls[63] = { 580, 5800, 20, 200, WALL_NO_STICK };
    walls[64] = { 380, 5700, 60, 20, WALL_NORMAL };
    walls[65] = { 560, 5700, 60, 20, WALL_NORMAL };

    walls[66] = { 760, 3950, 40, 2050, WALL_NO_STICK };


    // 氷ステージ
    walls[67] = { 0, 6000, 40, 14000, WALL_SLIPPERY };
    walls[68] = { 760, 6000, 40, 14000, WALL_SLIPPERY };
    walls[69] = { 755, 5900, 20, 200, WALL_REFLECT };

    walls[70] = { 300, 6100, 20, 200, WALL_SLIPPERY };
    walls[71] = { 710, 6150, 50, 30, WALL_NORMAL };
    walls[72] = { 40, 6400, 80, 80, WALL_SLIPPERY };
    walls[73] = { 120, 6410, 70, 70, WALL_SLIPPERY };
    walls[74] = { 190, 6420, 60, 60, WALL_SLIPPERY };
    walls[75] = { 40, 6480, 210, 10, WALL_NORMAL };
    walls[76] = { 500, 6250, 20, 200, WALL_SLIPPERY };
    walls[77] = { 755, 6180, 5, 3820, WALL_NO_STICK };

    walls[78] = { 190, 6780, 60, 20, WALL_NORMAL };
    walls[79] = { 500, 6850, 20, 200, WALL_SLIPPERY };
    walls[79].isMoving = true; walls[79].vy = 4.0f;
    walls[79].moveMinY = 6750.0f; walls[79].moveMaxY = 6950.0f;
    walls[80] = { 300, 7100, 20, 200, WALL_SLIPPERY };
    walls[80].isMoving = true; walls[80].vy = 4.0f;
    walls[80].moveMinY = 7000.0f; walls[80].moveMaxY = 7200.0f;
    walls[81] = { 520, 7300, 40, 250, WALL_SLIPPERY };
    walls[81].isMoving = true; walls[81].vy = 3.0f;
    walls[81].moveMinY = 7250.0f; walls[81].moveMaxY = 7400.0f;
    walls[82] = { 650, 7600, 20, 230, WALL_SLIPPERY };
    walls[82].isMoving = true; walls[82].vy = 4.0f;
    walls[82].moveMinY = 7500.0f; walls[82].moveMaxY = 7850.0f;
    walls[83] = { 710, 7100, 50, 20, WALL_NORMAL };
    walls[84] = { 400, 7900, 60, 20, WALL_NORMAL };
    walls[85] = { 40, 7000, 5, 13000, WALL_NO_STICK };
    walls[86] = { 395, 8100, 20, 400, WALL_SLIPPERY };
    walls[87] = { 395, 8500, 20, 10, WALL_NORMAL };

    walls[88] = { 300, 8650, 20, 100, WALL_REFLECT };
    walls[89] = { 500, 8750, 20, 100, WALL_REFLECT };
    walls[90] = { 300, 8800, 20, 100, WALL_REFLECT };
    walls[91] = { 500, 8900, 20, 100, WALL_REFLECT };
    walls[92] = { 300, 8950, 20, 100, WALL_REFLECT };
    walls[93] = { 500, 9050, 20, 100, WALL_REFLECT };

    walls[94] = { 170, 9150, 190, 20, WALL_NORMAL };
    walls[95] = { 310, 9350, 50, 20, WALL_NORMAL };
    walls[96] = { 220, 9450, 40, 20, WALL_NORMAL };
    walls[97] = { 60, 9650, 20, 150, WALL_REFLECT };
    walls[97].isMoving = true; walls[97].vy = 4.0f;
    walls[97].moveMinY = 9550.0f; walls[97].moveMaxY = 9750.0f;
    walls[98] = { 290, 9770, 30, 30, WALL_NORMAL };
    walls[99] = { 350, 9970, 20, 150, WALL_BOOST };

    walls[100] = { 320, 10500, 40, 40, WALL_BOOST_FX };
    walls[100].boostX = 12.0f;
    walls[101] = { 440, 10500, 40, 40, WALL_BOOST_RX };
    walls[101].boostX = -12.0f;
    walls[102] = { 280, 10540, 40, 40, WALL_BOOST_FX };
    walls[102].boostX = 12.0f;
    walls[103] = { 480, 10540, 40, 40, WALL_BOOST_RX };
    walls[103].boostX = -12.0f;
    walls[104] = { 240, 10580, 40, 40, WALL_BOOST_FX };
    walls[104].boostX = 12.0f;
    walls[105] = { 520, 10580, 40, 40, WALL_BOOST_RX };
    walls[105].boostX = -12.0f;
    walls[106] = { 200, 10620, 40, 40, WALL_BOOST_FX };
    walls[106].boostX = 12.0f;
    walls[107] = { 560, 10620, 40, 40, WALL_BOOST_RX };
    walls[107].boostX = -12.0f;
    walls[108] = { 160, 10660, 40, 40, WALL_BOOST_FX };
    walls[108].boostX = 12.0f;
    walls[109] = { 600, 10660, 40, 40, WALL_BOOST_RX };
    walls[109].boostX = -12.0f;

    walls[110] = { 440, 10240, 40, 100, WALL_BOOST };
    walls[111] = { 450, 10620, 40, 10, WALL_NO_STICK };
    walls[112] = { 210, 10750, 5, 20, WALL_NO_STICK };
    walls[113] = { 140, 10900, 10, 150, WALL_SLIPPERY };
    walls[114] = { 390, 11000, 30, 30, WALL_NO_STICK };
    walls[114].isMoving = true; walls[114].vy = 5.0f;
    walls[114].moveMinY = 10920.0f; walls[114].moveMaxY = 11150.0f;
    walls[115] = { 550, 11420, 40, 40, WALL_NORMAL };
    walls[116] = { 590, 11440, 40, 40, WALL_NORMAL };
    walls[117] = { 630, 11460, 40, 40, WALL_NORMAL };
    walls[118] = { 670, 11480, 40, 20, WALL_NORMAL };
    walls[119] = { 710, 11490, 50, 10, WALL_NORMAL };

    // ステージ補強
    walls[120] = { 150, 1840, 10, 25, WALL_NO_STICK };
    walls[121] = { 35, 2200, 10, 670, WALL_NO_STICK };
    walls[122] = { 45, 2200, 17, 10, WALL_NO_STICK };
    walls[123] = { 40, 2190, 10, 10, WALL_NO_STICK };
    walls[124] = { 40, 2210, 30, 10, WALL_NO_STICK };
    walls[125] = { 45, 2220, 25, 3, WALL_NORMAL };
    walls[125].r = 0.0f; walls[125].g = 0.4f; walls[125].b = 0.0f;
    walls[126] = { 40, 750, 40, 20, WALL_NORMAL };
    walls[126].r = 0.0f; walls[126].g = 0.4f; walls[126].b = 0.0f;

    walls[127] = { 350, 11650, 20, 200, WALL_SLIPPERY };
    walls[127].isMoving = true; walls[127].vy = 3.0f;
    walls[127].moveMinY = 11520.0f; walls[127].moveMaxY = 11800.0f;
    walls[128] = { 100, 11940, 20, 170, WALL_SLIPPERY };
    walls[129] = { 120, 11580, 40, 40, WALL_NORMAL };
    
    walls[130] = { 100, 12220, 20, 70, WALL_SLIPPERY };
    
    walls[131] = { 100, 12380, 20, 70, WALL_SLIPPERY };
    
    walls[132] = { 100, 12540, 20, 60, WALL_SLIPPERY };
    
    walls[133] = { 140, 12710, 20, 70, WALL_SLIPPERY };
    walls[134] = { 170, 12930, 20, 180, WALL_SLIPPERY };
    walls[135] = { 350, 13020, 150, 20, WALL_NORMAL };
    walls[135].r = 0.0f; walls[135].g = 0.0f; walls[135].b = 0.0f;
    walls[136] = { 755, 11450, 5, 5500, WALL_NO_STICK };


    wallCount = 150;


    for (int i = 0; i < wallCount; i++) {
        walls[i].prevY = walls[i].y;
    }


    glutMainLoop();
    return 0;
}