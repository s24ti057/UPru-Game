#include <string>
#include <format>

#include "Debug.h"
#include "Stage.h"
#include "GameConfig.h"
#include "Wall.h"
#include <glut.h>

// プレイヤー状態を文字列に変換
const char* getPlayerStateText(PlayerState state)
{
    switch (state)
    {
    case GROUND:     return "GROUND";
    case AIR:        return "AIR";
    case WALL_LEFT:  return "WALL_LEFT";
    case WALL_RIGHT: return "WALL_RIGHT";
    default:         return "UNKNOWN";
    }
}

// デバックモード用文字列描画関数
void drawDebugText(const std::string& text, float x, float y)
{
    glRasterPos2f(x, y); // 文字を書く開始位置を指定

    // 文字列を一文字ずつ描画
    for (char c : text)
    {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
    }
}

// デバック情報の描画
void drawDebugInfo(const Player& player, float cameraX, float cameraY, int jumpCount, int fallCount, float lastJumpCharge, bool jumpKeyON, float fps, float ups)
{
    glColor3f(1.0f, 1.0f, 1.0f);

    // プレイヤー座標
    drawDebugText(
        std::format("Position : ({:.1f}, {:.1f})", player.x, player.y),
        10.0f, 570.0f
    );

    // 移動速度
    drawDebugText(
        std::format("Velocity : ({:.1f}, {:.1f})", player.vx, player.vy),
        10.0f, 550.0f
    );

    // プレイヤーの状態
    drawDebugText(
        std::format("State : {}", getPlayerStateText(player.state)),
        10.0f, 530.0f
    );

    // カメラ座標
    drawDebugText(
        std::format("Camera : ({:.1f}, {:.1f})", cameraX, cameraY),
        10.0f, 510.0f
    );

    // ジャンプ数
    drawDebugText(
        std::format("Jump : {}", jumpCount),
        10.0f, 490.0f
    );

    // 落下数
    drawDebugText(
        std::format("Fall : {}", fallCount),
        10.0f, 470.0f
    );

    // 表示するチャージ量
    float charge = jumpKeyON ? player.jumpCharge : lastJumpCharge;

    drawDebugText(
        std::format("Charge : {:.1f}", charge),
        10.0f, 450.0f
    );

    // FPS・UPS表示
    drawDebugText(
        std::format("FPS : {:.1f}", fps),
        10.0f, 420.0f
    );

    drawDebugText(
        std::format("UPS : {:.1f}", ups),
        10.0f, 400.0f
    );

    // テレポートキーの説明
    drawDebugText("--- Teleport Key ---", 10.0f, 360.0f);

    drawDebugText("Start  : R", 10.0f, 340.0f);
    drawDebugText("Stage1(player.y=1530)  : 1", 10.0f, 320.0f);
    drawDebugText("Stage2(player.y=4380)  : 2", 10.0f, 300.0f);
    drawDebugText("Stage3(player.y=6000)  : 3", 10.0f, 280.0f);
    drawDebugText("Stage4(player.y=9190)  : 4", 10.0f, 260.0f);
    drawDebugText("Stage5(player.y=10650) : 5", 10.0f, 240.0f);
    drawDebugText("Stage6(player.y=11650) : 6", 10.0f, 220.0f);
    drawDebugText("Goal : G", 10.0f, 200.0f);
}

// 矩形の当たり判定領域を描画
void drawDebugRect(float x, float y, float w, float h)
{
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

// 当たり判定領域の描画
void drawDebugHitboxes(const Player& player, Stage& stage, float groundX, float groundY, float groundW, float groundH)
{
    glLineWidth(2.0f);

    // プレイヤーの当たり判定（緑）
    glColor3f(0.0f, 1.0f, 0.0f);
    drawDebugRect(player.x, player.y, player.w, player.h);

    // 壁の当たり判定（赤）
    glColor3f(1.0f, 0.0f, 0.0f);

    for (int i = 0; i < stage.getWallCount(); i++)
    {
        Wall& wall = stage.getWall(i);

        drawDebugRect(wall.x, wall.y, wall.w, wall.h);
    }

    // 地面の当たり判定（赤）
    drawDebugRect(groundX, groundY, groundW, groundH);

    // 線の太さを元に戻す
    glLineWidth(1.0f);
}

// デバック用テレポート
void debugTeleport(DebugTeleportContext& ctx, float x, float y)
{
    Player& player = ctx.player;

    // プレイヤーの座標
    player.x = x;
    player.y = y;

    // プレイヤーの速度
    player.vx = 0.0f;
    player.vy = 0.0f;

    // プレイヤーの状態
    player.state = AIR;
    player.stickingWallIndex = -1;

    // ジャンプ関連
    player.jumpCharge = 0.0f;
    player.coyoteTimer = 0;
    player.wallKickTimer = 0;

    // キー入力状態
    ctx.keyLeftON = false;
    ctx.keyRightON = false;
    ctx.keyStickON = false;
    ctx.jumpKeyON = false;

    // 前フレームの位置
    ctx.prevX = player.x;
    ctx.prevY = player.y;

    // 反射関連
    ctx.reflectCooldown = 0;
    ctx.currentWallType = WALL_NORMAL;

    // 落下判定
    ctx.fallStartY = player.y;
    ctx.wasFalling = false;
    ctx.fallCounted = false;

    // チャージ音
    ctx.chargeMaxSoundPlayed = false;

    // カメラ位置
    ctx.cameraX = 0.0f;

    const float camOffsetY = 150.0f;

    ctx.cameraY = player.y - GAME_HEIGHT / 2.0f
        + player.h / 2.0f - camOffsetY;

    // カメラの移動範囲
    const float stageBottom = 0.0f;
    const float stageTop = 20000.0f;
    const float maxCameraY = stageTop - GAME_HEIGHT;

    if (ctx.cameraY < stageBottom)
        ctx.cameraY = stageBottom;

    if (ctx.cameraY > maxCameraY)
        ctx.cameraY = maxCameraY;
}
