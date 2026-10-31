#include "Player.h"
#include <cmath>

// プレイヤーの初期化処理
void Player::reset()
{
    x = 400.0f; // 初期位置：400.0f
    y = 70.0f; // 初期位置： 70.0f
    vx = 0.0f;
    vy = 0.0f;

    state = GROUND;
    stickingWallIndex = -1;

    jumpCharge = 0.0f;
}


// 歩行アニメーションの更新
void Player::updateWalkAnimation()
{
    
    bool isMoving = fabs(vx) > 0.1f && state == GROUND; // 動いているかの判定

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
}

// 壁キックアニメーションのタイマー更新
void Player::updateWallKickAnimation()
{
    if (wallKickTimer > 0)
    {
        wallKickTimer--;
    }
}

// 左右移動速度の更新
void Player::updateHorizontalMovement(bool keyLeft, bool keyRight, float windMinY, float windMaxY)
{
    float inputSpeedX = 0.0f; // プレイヤー自身の入力速度

    if (keyLeft) {
        inputSpeedX = -4.0f;
        playerDirection = -1; // 空中でも左向きに変更
    }
    else if (keyRight) {
        inputSpeedX = 4.0f;
        playerDirection = 1; // 空中でも右向きに変更
    }

    // プレイヤーの上下端
    float playerBottom = y;
    float playerTop = y + h;

    // プレイヤーの一部でも y = windMinY ～ windMaxY に入っているか
    bool insideWindArea =
        (playerTop > windMinY) &&
        (playerBottom < windMaxY);
    float currentWindX = 0.0f; // 現在かかっている風

    if (insideWindArea) {
        currentWindX = 0.0f;  // 右向きの風(ここで風を調整)
    }
    vx = inputSpeedX + currentWindX; // キー入力と風を合成
}

// ジャンプチャージの更新
bool Player::updateJumpCharge(bool jumpKeyOn)
{
    if (jumpKeyOn && (state == GROUND || state == WALL_LEFT || state == WALL_RIGHT))
    {
        jumpCharge += 0.3f;

        if (jumpCharge > 20.0f)
        {
            jumpCharge = 20.0f;
            return true;
        }
    }

    return false;
}


// 地面ジャンプ(状態と速度の更新)
bool Player::jumpFromGround(float power)
{
    // 地面にいる、またはコヨーテタイム中ならジャンプ可能
    if (state == GROUND || coyoteTimer > 0)
    {
        vy = power;
        state = AIR;
        coyoteTimer = 0; // 一度ジャンプしたら使用済みにする

        jumpCharge = 0.0f;

        return true;
    }

    return false;
}

// 壁ジャンプ(左壁・右壁)
bool Player::jumpFromWall(float power)
{
    // 左右どちらの壁にも張り付いていなければジャンプしない
    if (state != WALL_LEFT && state != WALL_RIGHT) {
        return false;
    }

    // ジャンプする方向を決定
    int direction = (state == WALL_LEFT) ? 1 : -1;

    // 壁張り付き状態を解除
    state = AIR; // 先にAIRにする
    stickingWallIndex = -1;

    // ジャンプ速度を設定
    vy = power;
    vx = direction * (6.0f + jumpCharge * 0.2f);

    playerDirection = direction; // プレイヤーの向きを更新

    wallKickTimer = 12; // 壁キックアニメーション(12フレームだけキック画像を表示)

    jumpCharge = 0.0f; // ジャンプチャージをリセット

    return true;
}


// チャージ量からジャンプ力を計算
float Player::calculateJumpPower() const
{
    float power = 8.0f + jumpCharge * 0.5f; // 最大+10くらい

    if (power > 20.0f) {
        power = 20.0f;
    }

    return power;
}