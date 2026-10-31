#include "Wall.h"
#include <glut.h>

// コンストラクタ
Wall::Wall(float posX, float posY, float width, float height, WallType wallType)
{
    x = posX;
    y = posY;
    w = width;
    h = height;
    type = wallType;
}

// 移動壁の設定
Wall& Wall::setMovement(float speedY, float minY, float maxY)
{
    isMoving = true;
    vy = speedY;
    moveMinY = minY;
    moveMaxY = maxY;
    return *this;
}

//  加速床の横方向加速力設定
Wall& Wall::setBoostX(float boost)
{
    boostX = boost;
    return *this;
}

// 壁の色設定
Wall& Wall::setColor(float red, float green, float blue)
{
    r = red;
    g = green;
    b = blue;
    return *this;

}

// 通常壁のテクスチャ設定
Wall& Wall::setTexture(WallTextureType texture, float size)
{
    textureType = texture;
    tileSize = size;
    return *this;
}

// 張り付き不可壁のテクスチャ設定
Wall& Wall::setTexture(NoStickWallTextureType texture, float size)
{
    noStickTextureType = texture;
    tileSize = size;
    return *this;
}

// 氷壁のテクスチャ設定
Wall& Wall::setTexture(SlipperyWallTextureType texture, float size)
{
    slipperyTextureType = texture;
    tileSize = size;
    return *this;
}


// 加速壁のテクスチャ設定
Wall& Wall::setTexture(BoostWallTextureType texture, float size)
{
    boostTextureType = texture;
    tileSize = size;
    return *this;
}

// 反射壁のテクスチャ設定
Wall& Wall::setTexture(ReflectWallTextureType texture, float size)
{
    reflectTextureType = texture;
    tileSize = size;
    return *this;
}

// 加速床(正方向)のテクスチャ設定
Wall& Wall::setTexture(BoostFXWallTextureType texture, float size)
{
    boostFXTextureType = texture;
    tileSize = size;
    return *this;
}

// 加速床(負方向のテクスチャ設定)
Wall& Wall::setTexture(BoostRXWallTextureType texture, float size)
{
    boostRXTextureType = texture;
    tileSize =  size;
    return *this;
}


// 前フレームのY座標を取得
float Wall::getPrevY() const
{
    return prevY;
}

// 今フレームの移動量を取得
float Wall::getDeltaY() const
{
    return deltaY;
}


// 移動壁の前フレーム位置を初期化
void Wall::resetPreviousY()
{
    prevY = y;
}

// 壁の更新処理
void Wall::update() 
{
    // 動く壁の更新処理

    prevY = y; // 動かす前の位置を保存
    deltaY = 0.0f; // 毎フレーム初期化

    if (isMoving) {

        float oldY = y;

        y += vy;

        // 上限・下限で反転
        if (y < moveMinY) {
            y = moveMinY;
            vy *= -1.0f;
        }

        if (y > moveMaxY) {
            y = moveMaxY;
            vy *= -1.0f;
        }

        // 実際に移動した量
        deltaY = y - oldY;
    }
}


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

// 壁の描画処理
void Wall::draw() const
{
    if (type == WALL_BOOST) {
        DrawBoostWallTexture(boostTextureType, x, y, w, h, tileSize); // 加速壁
        return;
    }

    if (type == WALL_REFLECT) {
        DrawReflectWallTexture(reflectTextureType, x, y, w, h, tileSize); // 反射壁
        return;
    }

    if (type == WALL_BOOST_FX) {
        DrawBoostFxWallTexture(boostFXTextureType, x, y, w, h, tileSize); // 加速床(正方向) 
        return;
    }

    if (type == WALL_BOOST_RX) {
        DrawBoostRxWallTexture(boostRXTextureType, x, y, w, h, tileSize); // 加速床(負方向)
        return;
    }

    if (type == WALL_NORMAL) {
        DrawWallTexture(textureType, x, y, w, h, tileSize); // 通常壁
        return;
    }

    if (type == WALL_NO_STICK) {
        DrawNoStickWallTexture(noStickTextureType, x, y, w, h, tileSize); // 特殊壁(赤)
        return;
    }

    if (type == WALL_SLIPPERY) {
        DrawSlipperyWallTexture(slipperyTextureType, x, y, w, h, tileSize); // 氷壁(水色)
        return;
    }

    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}