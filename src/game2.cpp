#include <stdio.h>
#include <stdlib.h>
#include <glut.h>
#include <cmath>
#include <tchar.h>
#include <iostream>
#include <fstream>

#include <Windows.h>

#include "Texture.h"
#include "Ranking.h"
#include "Audio.h"
#include "Wall.h"
#include "Stage.h"
#include "Player.h"
#include "Renderer.h"
#include "GameConfig.h"
#include "ScreenRenderer.h"
#include "Debug.h"


#define _USE_MATH_DEFINES

using namespace std;

const float M_PI = 3.14159265358979f;

Ranking ranking; // Rankingオブジェクトを作成
Audio audio;



// 前フレームの位置を保存する変数
float prevX = 0.0f;
float prevY = 0.0f;

// 反射後数フレーム保持する変数
int reflectCooldown = 0;

// コヨーテタイム(足場から離れて数フレームだけジャンプを許可)変数

const int COYOTE_TIME = 6; // 約0.1秒のコヨーテタイム

Player player;

// 足場（地面）
struct Platform {
    float x, y;
    float w, h;
};

Platform ground;

WallType currentWallType = WALL_NORMAL;


Stage stage; // Stageオブジェクトを作成


// 壁の上面が露出しているか調べる
bool isWallTopExposed(int wallIndex)
{
    Wall& w = stage.getWall(wallIndex);

    float wallTop = w.y + w.h;

    const float EPSILON = 1.0f;

    for (int j = 0; j < stage.getWallCount(); j++)
    {
        if (j == wallIndex)
            continue;

        Wall& other = stage.getWall(j);

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
float lastJumpCharge = 0.0f; // 最後のチャージ時間を保存する変数

// プレイタイム処理
int playStartTime = 0; // プレイタイム計測
int clearTime = 0; // クリアタイムを固定する
int pauseStartTime = 0;   // ポーズを開始した時刻
int totalPauseTime = 0;   // ポーズしていた合計時間

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

// カメラ変数
float cameraX = 0.0f;
float cameraY = 0.0f;


// ウィンドウサイズの設定
int winW = GAME_WIDTH; // ウィンドウの横幅
int winH = GAME_HEIGHT; // ウィンドウの縦幅

bool debugMode = false; // デバックモード

// デバック用テレポート情報構造体
DebugTeleportContext teleportContext{
    player,
    cameraX,
    cameraY,
    KeyLeftON,
    KeyRightON,
    KeyStickON,
    JumpKeyON,
    prevX,
    prevY,
    reflectCooldown,
    currentWallType,
    fallStartY,
    wasFalling,
    fallCounted,
    chargeMaxSoundPlayed
};

// FPS・UPS計測用
int fpsFrameCount = 0;  // 描画回数を数える
int upsUpdateCount = 0; // ゲーム更新回数を数える

int performanceLastTime = 0; // 前回の計測時刻

float currentFPS = 0.0f; // 表示するFPS
float currentUPS = 0.0f; // 表示するUPS

// FPS・UPSの計測値を更新
void updatePerformanceMetrics()
{
    int currentTime = glutGet(GLUT_ELAPSED_TIME);

    if (performanceLastTime == 0)
    {
        performanceLastTime = currentTime;
        return;
    }

    int elapsed = currentTime - performanceLastTime;

    if (elapsed >= 1000)
    {
        currentFPS = fpsFrameCount * 1000.0f / elapsed;
        currentUPS = upsUpdateCount * 1000.0f / elapsed;

        fpsFrameCount = 0;
        upsUpdateCount = 0;

        performanceLastTime = currentTime;
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
        // 通常カメラ(描画位置をずらすことでカメラが追従しているようにしている)
        glTranslatef(-cameraX, -cameraY, 0.0f);
    }

    DrawBackground((float)winW); // ゲーム背景画像描画
    GoalFlagTexture(450, 13020, 100, 100); // ゴール旗描画


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
    const bool isAirborne = (player.state == AIR); // 空中にいるかどうか
    const bool isWallSticking = (player.state == WALL_LEFT || player.state == WALL_RIGHT); // 壁に触れているかどうか
    const bool isWallRight = (player.state == WALL_RIGHT); // 右壁に触れているかどうか
    bool isWallKick = (player.wallKickTimer > 0); // 壁キック中であるかどうか

    // プレイヤー描画
    if (gameScreen == SCREEN_GOAL)
    {
        DrawGoalPlayerTexture(player.x, player.y, player.w, player.h, goalAnimationStep);
    }
    else
    {
        DrawPlayerTexture(player.x, player.y, player.w, player.h, player.playerWalkStep, player.playerDirection, isAirborne, player.vy, isWallSticking, isWallRight, isWallKick);
    }

    // 一番下の地面
    DrawGroundTexture(ground.x, ground.y, ground.w, ground.h);
   
    /*
    glColor3f(0.3, 0.8, 0.3);
    glBegin(GL_QUADS);
    glVertex2f(ground.x, ground.y);
    glVertex2f(ground.x + ground.w, ground.y);
    glVertex2f(ground.x + ground.w, ground.y + ground.h);
    glVertex2f(ground.x, ground.y + ground.h);
    glEnd();
    */

    // ステージ内の壁を描画
    stage.draw();

    // デバッグモード中は当たり判定を表示
    if (debugMode)
    {
        drawDebugHitboxes(player, stage, ground.x, ground.y, ground.w, ground.h);
    }

    // カメラ移動前の座標に戻す
    glPopMatrix();
}


// 描画
void display() {

    // FPS計測
    fpsFrameCount++;
    updatePerformanceMetrics();

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // スタート画面
    if (gameScreen == SCREEN_START) {
        drawStartScreen(menuIndex, titleAnimationStep);
        glutSwapBuffers(); // 実際に描画
        return; // これより下は描画されない
    }

    // タイム画面
    if (gameScreen == SCREEN_TIME) {
        drawTimeScreen(ranking);
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
        drawGameScene();                 // まずはゲーム画面を描画
        drawPauseScreen(pauseMenuIndex); // ポーズメニューを重ねる
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
        drawClearScreen(clearPage, clearTime, jumpCount, fallCount);
        glutSwapBuffers();
        return;
    }


    drawGameScene(); // 通常ゲーム画面描画

    drawPlayTime(playStartTime, totalPauseTime); // プレイタイムを画面右下に表示

    // デバッグ情報の描画
    if (debugMode)
    {
        drawDebugInfo(player, cameraX, cameraY, jumpCount, fallCount, lastJumpCharge, JumpKeyON, currentFPS, currentUPS);

    }

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

    // F3キーでデバッグモードを切り替える(全画面共通)
    if (key == GLUT_KEY_F3)
    {
        debugMode = !debugMode;

        printf("Debug Mode : %s\n", debugMode ? "ON" : "OFF");

        glutPostRedisplay();
        return;
    }


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

                // プレイヤーの初期化
                player.reset();

                lastJumpCharge = 0.0f;

                // カメラの初期化
                cameraX = 0.0f;
                cameraY = 0.0f;

                // 前フレームの位置
                prevX = player.x;
                prevY = player.y;

                // キー入力の初期化
                KeyLeftON = false;
                KeyRightON = false;
                KeyStickON = false;
                JumpKeyON = false;
                
                chargeMaxSoundPlayed = false; 

                // BGM切り替え
                audio.stopTitleBGM();
                audio.startStageBGM(); // ステージBGM再生

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
            audio.stopGoalBGM();
            audio.startTitleBGM();

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
                audio.stopStageBGM();
                audio.startTitleBGM();

                gameScreen = SCREEN_START;
                menuIndex = 0;
            }

            glutPostRedisplay();
            return;
        } 

        return;
    }

    // デバッグモード中のテレポート
    if (debugMode && gameScreen == SCREEN_PLAYING)
    {
        switch (key)
        {
        case '1':
            debugTeleport(teleportContext, 400.0f, 1530.0f);
            break;

        case '2':
            debugTeleport(teleportContext, 420.0f, 4380.0f);
            break;

        case '3':
            debugTeleport(teleportContext, 400.0f, 6000.0f);
            break;

        case '4':
            debugTeleport(teleportContext, 260.0f, 9190.0f);
            break;

        case '5': 
            debugTeleport(teleportContext, 460.0f, 10650.0f);
            break;

        case'6':
            debugTeleport(teleportContext, 126.0f, 11650.0f);
            break;

        case 'r':
        case 'R':
            debugTeleport(teleportContext, 400.0f, 70.0f);
            break;

        case 'g':
        case 'G':
            debugTeleport(teleportContext, 350.0f, 13060.0f);
            break;

        default:
            break;
        }

        if ((key >= '1' && key <= '6') || key == 'r' || key == 'R' || key == 'g' || key == 'G')
        {
            glutPostRedisplay(); // 再描画処理
            return;
        }
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    if (gameScreen != SCREEN_PLAYING) return; // ゲーム中以外はジャンプ処理などをしない

    if (key == ' ') {
        lastJumpCharge = player.jumpCharge; // チャージ時間を記録
        JumpKeyON = false;


        // チャージ量に応じてジャンプ力を変える
        float power = player.calculateJumpPower();


        // 地面ジャンプ
        if (player.jumpFromGround(power)) {

            audio.playJumpSound(); // ジャンプ音再生
            jumpCount++;   // ジャンプ数カウント

            chargeMaxSoundPlayed = false;
            return;
        }

        // 壁ジャンプ(左壁・右壁)
        if (player.jumpFromWall(power))
        {
            KeyStickON = false;

            audio.playJumpSound(); // ジャンプ音を再生
            jumpCount++; // ジャンプ数をカウント

            chargeMaxSoundPlayed = false;
            return;
        }

        player.jumpCharge = 0.0f;  // リセット
        chargeMaxSoundPlayed = false;
    }

    if (key == 'z') {      // 張り付き解除
        KeyStickON = false;
        player.state = AIR;
        currentWallType = WALL_NORMAL;
        player.stickingWallIndex = -1; // 壁から離れたら番号解除
    }
}


// 毎フレーム更新
void timer(int t) {
    // プレイ中以外は氷壁SEを停止
    if (gameScreen != SCREEN_PLAYING)
    {
    
        audio.stopIceSound();
    }

    if (gameScreen == SCREEN_START)
    {
        // タイトルBGMをループ
        audio.updateTitleBGM();

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

        // 4ページ目 → 5ページ目
        else if (clearPage == 3 && clearPageTimer >= 420)
        {
            clearPage = 4;
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
        audio.updateStageBGM();
    }

    // ゲーム中以外はゲーム処理を進めない
    if (gameScreen != SCREEN_PLAYING) {

        glutPostRedisplay();
        glutTimerFunc(16, timer, 0);
        return;
    }

    // UPS計測（ゲームプレイ中のみ）
    upsUpdateCount++;

    // 前フレームの位置を保存
    // float prevX = player.x;
    // float prevY = player.y;

    // 壁の更新処理
    stage.update();

    // 張り付いている壁と一緒に移動
    if (player.stickingWallIndex >= 0 &&
        player.stickingWallIndex < stage.getWallCount() &&
        KeyStickON &&
        (player.state == WALL_LEFT || player.state == WALL_RIGHT)) {

        Wall& stuckWall = stage.getWall(player.stickingWallIndex);

        player.y += stuckWall.getDeltaY();
    }

    // 張り付いている壁から離れたか確認
    if (player.stickingWallIndex >= 0 &&
        player.stickingWallIndex < stage.getWallCount() &&
        (player.state == WALL_LEFT || player.state == WALL_RIGHT)) {

        Wall& stuckWall = stage.getWall(player.stickingWallIndex);

        // プレイヤーと壁が縦方向に重なっているか
        bool overlapY =
            player.y + player.h > stuckWall.y &&
            player.y < stuckWall.y + stuckWall.h;

        const float CONTACT_EPSILON = 1.0f;

        bool touchingSide = false;

        // プレイヤーの右側に壁がある状態
        if (player.state == WALL_LEFT) {
            touchingSide =
                fabs((player.x + player.w) - stuckWall.x)
                <= CONTACT_EPSILON;
        }

        // プレイヤーの左側に壁がある状態
        else if (player.state == WALL_RIGHT) {
            touchingSide =
                fabs(player.x - (stuckWall.x + stuckWall.w))
                <= CONTACT_EPSILON;
        }

        // 壁から外れたら張り付き状態を解除
        if (!overlapY || !touchingSide) {
            player.state = AIR;
            currentWallType = WALL_NORMAL;
            player.stickingWallIndex = -1;

            // KeyStickONは解除しなくてよい
            // Zを押し続けていれば、次に別の壁へ触れたとき再び張り付ける
        }
    }

    // 左右移動（壁張り付き中は横移動禁止）
    if (reflectCooldown > 0) {
        reflectCooldown--;
    }
    else {
        if (player.state != WALL_LEFT && player.state != WALL_RIGHT) {
            
            // 左右の移動速度の更新
            player.updateHorizontalMovement(KeyLeftON, KeyRightON, windMinY, windMaxY);
        }
    }

    // ジャンプチャージ（地面 or 壁張り付き中）
    bool chargeMax = player.updateJumpCharge(JumpKeyON); // ジャンプチャージの更新

    // 最大になった瞬間だけ効果音を鳴らす
    if (chargeMax && chargeMaxSoundPlayed == false)
    {
        audio.playChargeMaxSound();
        chargeMaxSoundPlayed = true;
    }

    // 壁キック画像タイマー
    player.updateWallKickAnimation();


    // --- 重力処理 ---
    if (KeyStickON && (player.state == WALL_LEFT || player.state == WALL_RIGHT)) {

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
    player.updateWalkAnimation();


    // 足場への着地を横衝突より先に処理(足場の角に着地するときの誤判定を防ぐため)
    // 上から来たならまず着地扱い、それ以外なら横衝突などを調べるという優先順位にする
    bool landedOnWall = false;

    if (player.vy <= 0.0f) {

        for (int i = 0; i < stage.getWallCount(); i++) {

            Wall& w = stage.getWall(i);

            // 上に乗れる種類だけ
            bool canStand =
                w.type == WALL_NORMAL ||
                w.type == WALL_NO_STICK ||
                w.type == WALL_BOOST_FX ||
                w.type == WALL_BOOST_RX;

            if (!canStand) {
                continue;
            }

            float oldWallTop = w.getPrevY() + w.h;
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
                player.state = GROUND;
                currentWallType = WALL_NORMAL;
                player.stickingWallIndex = -1;

                // 加速床の場合
                if (w.type == WALL_BOOST_FX ||
                    w.type == WALL_BOOST_RX) {

                    player.vx = w.boostX;
                    player.state = AIR;
                    reflectCooldown = 10;

                    audio.playBoostXSound(); // 加速した瞬間にSEを再生
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
        player.state = GROUND;
    }

    // 壁判定（配列で処理）
    for (int i = 0; i < stage.getWallCount(); i++) {

        Wall& w = stage.getWall(i);

        // プレイヤーがその壁に触れているか（AABB判定）
        bool touching =
            (player.x + player.w > w.x) &&
            (player.x < w.x + w.w) &&
            (player.y + player.h > w.y) &&
            (player.y < w.y + w.h);

        if (!touching)  continue; // 触れていなければ次の壁へ
        

        float oldWallBottom = w.getPrevY();
        float oldWallTop = w.getPrevY() + w.h;

        float newWallBottom = w.y;
        float newWallTop = w.y + w.h;

        // 前フレームの位置(prevX, prevY)と比較して、どの方向から壁に入ってきたかを判定
        // 壁の左面に衝突した
        bool hitLeft =
            prevX + player.w <= w.x &&   // 前フレームは左側
            player.x + player.w > w.x;   // 今フレームで重なった

        // 壁の右面に衝突した
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


                player.state = AIR;
                KeyStickON = false;
                reflectCooldown = 15;  // 15フレーム横移動禁止（調整可）

                audio.playReflectSound(); // 反射した瞬間にSEを再生

                break;
            }

            // 加速壁（左側）
            if (w.type == WALL_BOOST) {
                player.vy = 20.0f;   // 上へ加速
                player.state = AIR;
                KeyStickON = false;

                audio.playBoostSound(); // 加速した瞬間にSEを再生

                break;
            }

            // zキーが押されていて、くっつける壁にいる時
            if (KeyStickON && w.type != WALL_NO_STICK && w.type != WALL_REFLECT) {
                player.state = WALL_LEFT;
                currentWallType = w.type;
                player.stickingWallIndex = i;
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

                player.state = AIR;
                KeyStickON = false;
                reflectCooldown = 15;  // 15フレーム横移動禁止（調整可）

                audio.playReflectSound(); // 反射した瞬間にSEを再生

                break;
            }

            // 加速壁（右側）
            if (w.type == WALL_BOOST) {
                player.vy = 20.0f;   // 上へ加速
                player.state = AIR;
                KeyStickON = false;

                audio.playBoostSound(); // 加速した瞬間にSEを再生

                break;
            }


            if (KeyStickON && w.type != WALL_NO_STICK && w.type != WALL_REFLECT) {
                player.state = WALL_RIGHT;
                currentWallType = w.type;
                player.stickingWallIndex = i;
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
                player.state = AIR;
                player.stickingWallIndex = -1;
            }
            else if (w.type == WALL_REFLECT) {
                // 反射壁：跳ね返る
                player.y = w.y - player.h;
                player.vy = -player.vy;   // 反射
                player.state = AIR;

                audio.playReflectSound(); // 反射した瞬間にSEを再生
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
                player.state = GROUND;
                currentWallType = WALL_NORMAL;
                player.stickingWallIndex = -1;
            }
            else if (w.type == WALL_REFLECT) {
                // 反射壁：跳ね返る
                player.y = w.y + w.h;
                player.vy = -player.vy;   // 反射
                player.state = AIR;

                audio.playReflectSound(); // 反射した瞬間にSEを再生
            }
            else if (w.type == WALL_BOOST_FX || w.type == WALL_BOOST_RX) {
                // 加速床 : x方向に加速
                player.y = w.y + w.h; // プレイヤーを壁の上面に置く
                player.vx = w.boostX; // 横方向へ加速
                player.vy = 0.0f; // 落下を止める
                player.state = AIR;
                player.stickingWallIndex = -1;
                reflectCooldown = 10; // キー入力ですぐに速度を上書きされないようにする

                audio.playBoostXSound(); // 加速した瞬間にSEを再生
            }
        }

        break;
    }

    // 地面・足場に乗っている間はコヨーテタイムを満タンにする
    if (landedOnWall || onGround) {
        player.coyoteTimer = COYOTE_TIME;
    } else {
        // 足場から離れた
        if (player.state == GROUND) player.state = AIR;
        if (player.coyoteTimer > 0) player.coyoteTimer--;
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
        ranking.registerTime(clearTime);

        
        audio.stopIceSound();
   
        audio.stopStageBGM();
        audio.startGoalBGM();
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

    // 氷壁を滑っているか判定
    bool isSlidingIce = KeyStickON &&
        (player.state == WALL_LEFT || player.state == WALL_RIGHT) &&
        currentWallType == WALL_SLIPPERY &&
        player.stickingWallIndex >= 0 &&
        player.stickingWallIndex < stage.getWallCount();

    // 氷壁SEの状態を更新
    audio.updateIceSound(isSlidingIce);

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
    ranking.load();
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

        // ゲームウィンドウにキーボードフォーカスを与える
        SetForegroundWindow(hwnd);
        SetActiveWindow(hwnd);
        SetFocus(hwnd);
    }

    InitTexture();

    audio.initialize(); // 最大チャージSEの初期化
    audio.startTitleBGM();

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
    ground.y = 30;
    ground.w = 720;
    ground.h = 40;

    // 壁の初期化処理(壁の追加)
    stage.initialize();

    glutMainLoop();
    return 0;
}
