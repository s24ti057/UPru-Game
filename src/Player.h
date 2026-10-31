#pragma once

// プレイヤーの状態
enum PlayerState {
    GROUND,     // 地面
    AIR,        // 空中
    WALL_LEFT,  // 左壁
    WALL_RIGHT  // 右壁
};

class Player
{
public:
    // 位置
    float x = 0.0f;
    float y = 0.0f;

    // サイズ
    float w = 0.0f;
    float h = 0.0f;

    // 速度
    float vx = 0.0f;
    float vy = 0.0f;

    PlayerState state = AIR; // 現在のプレイヤーの状態

    // 現在張り付いている壁の番号
    // -1 はどの壁にも張り付いていない
    int stickingWallIndex = -1;

    // ジャンプ
    float jumpCharge = 0.0f; // ジャンプチャージ変数
    int coyoteTimer = 0; // コヨーテタイム(足場から離れて数フレームだけジャンプを許可)変数
    int wallKickTimer = 0; // 壁キック表示用の変数

    // 向き・歩行アニメーション
    int playerDirection = 1; // 1：右向き、-1：左向き
    int playerWalkStep = 1;
    int playerWalkTimer = 0;


    void reset(); // プレイヤーの初期化処理
    void updateWalkAnimation(); // 歩行アニメーションの更新
    void updateWallKickAnimation(); // 壁キックアニメーションの更新
    void updateHorizontalMovement(bool keyLeft, bool keyRight, float windMinY, float windMaxY); // プレイヤーの移動速度更新
    bool updateJumpCharge(bool jumpKeyOn); // ジャンプチャージの更新

    bool jumpFromGround(float power); // 地面ジャンプ
    bool jumpFromWall(float power); // 壁ジャンプ

    float calculateJumpPower() const; // ジャンプ力の計算
};