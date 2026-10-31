#pragma once

#include "Player.h"
#include "Stage.h"


// デバッグ情報の描画
void drawDebugInfo(const Player& player, float cameraX, float cameraY, int jumpCount, int fallCount, float lastJumpCharge, bool jumpKeyON, float fps, float ups);

// 当たり判定領域の描画
void drawDebugHitboxes(const Player& player, Stage& stage, float groundX, float groundY, float groundW, float groundH);

struct DebugTeleportContext
{
    Player& player;

    // カメラ
    float& cameraX;
    float& cameraY;

    // キー入力
    bool& keyLeftON;
    bool& keyRightON;
    bool& keyStickON;
    bool& jumpKeyON;

    // 前フレームの位置
    float& prevX;
    float& prevY;

    // 反射関連
    int& reflectCooldown;
    WallType& currentWallType;

    // 落下判定
    float& fallStartY;
    bool& wasFalling;
    bool& fallCounted;

    // チャージ音
    bool& chargeMaxSoundPlayed;
};

// デバック用テレポート
void debugTeleport(DebugTeleportContext& context, float x, float y);