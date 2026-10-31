#pragma once

#include "Texture.h"

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

class Wall
{
private:
    // 移動壁
    bool isMoving = false; // 壁が動くかどうか
    float vy = 0.0f; // 壁の上下移動速度
    float moveMinY = 0.0f;
    float moveMaxY = 0.0f;

    float prevY = 0.0f; // 前フレームの壁のY座標
    float deltaY = 0.0f; // 今フレームに動いた量(前フレームからの移動量)


public:
    Wall() = default;

    Wall(float posX, float posY, float width, float height, WallType wallType);

    // 位置・サイズ
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    // 壁の種類
    WallType type = WALL_NORMAL;

    // テクスチャ(壁の見た目)
    WallTextureType textureType = WALL_TEXTURE_STONE; // 通常
    NoStickWallTextureType noStickTextureType = NO_STICK_WALL_NORMAL; // 張り付かない壁
    SlipperyWallTextureType slipperyTextureType = SLIPPERY_WALL_NORMAL; // 氷壁
    BoostWallTextureType boostTextureType = BOOST_WALL_NORMAL; // 加速壁
    ReflectWallTextureType reflectTextureType = REFLECT_WALL_NORMAL; // 反射壁
    BoostFXWallTextureType boostFXTextureType = BOOST_FX_WALL_NORMAL; // 加速床(正方向)
    BoostRXWallTextureType boostRXTextureType = BOOST_RX_WALL_NORMAL; // 加速床(負方向)
    float tileSize = 40.0f; // 壁繰り返し単位


    // 加速床
    float boostX = 0.0f; // 横方向の加速力

    // 通常壁(床)の色(初期の色)
    float r = 0.6f;
    float g = 0.6f;
    float b = 1.0f;


    Wall& setMovement(float speedY, float minY, float maxY); // 壁移動の設定
    Wall& setBoostX(float boost); // 加速床の横方向加速力設定
    Wall& setColor(float red, float green, float blue); // 壁の色設定

    Wall& setTexture(WallTextureType texture, float size = 40.0f); // 通常壁のテクスチャ設定
    Wall& setTexture(NoStickWallTextureType texture, float size = 40.0f); // 張り付き不可壁のテクスチャ設定
    Wall& setTexture(SlipperyWallTextureType texture, float size = 40.0f); // 氷壁のテクスチャ設定
    Wall& setTexture(BoostWallTextureType texture, float size = 40.0f); // 加速壁のテクスチャ設定
    Wall& setTexture(ReflectWallTextureType texture, float size = 40.0f); // 反射壁のテクスチャ設定
    Wall& setTexture(BoostFXWallTextureType texture, float size = 40.0f); // 加速床(正方向)のテクスチャ設定
    Wall& setTexture(BoostRXWallTextureType texture, float size = 40.0f); // 加速床(負方向)のテクスチャ設定

    // 移動壁の情報を取得
    float getPrevY() const;
    float getDeltaY() const;


    void resetPreviousY(); // 移動壁の前フレーム位置を初期化

    void update();

    void draw() const;

};