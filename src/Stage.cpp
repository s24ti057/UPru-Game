#include "Stage.h"
#include <cstdio>

// 壁の初期化処理
void Stage::initialize()
{
    walls.clear(); // 既存の壁を削除

    // 壁の追加
    addWall(Wall(0, 0, 40, 300, WALL_NORMAL)
        .setTexture(WALL_TEXTURE_TREE, 40.0f));

    addWall(Wall(760, 0, 40, 770, WALL_NORMAL)
        .setTexture(WALL_TEXTURE_TREE, 50.0f)); 

    // 特殊壁（例：中央に配置）
    addWall(Wall(0, 300, 40, 1050, WALL_NO_STICK));

    addWall(Wall(0, 1350, 40, 240, WALL_NORMAL)
        .setTexture(WALL_TEXTURE_TREE1));

    addWall(Wall(300, 200, 200, 20, WALL_NORMAL)
        .setTexture(WALL_TEXTURE_GRASS, 20.0f));

    addWall(Wall(100, 400, 150, 20, WALL_NORMAL)
        .setTexture(WALL_TEXTURE_GRASS, 20.0f));

    addWall(Wall(660, 400, 100, 20, WALL_NORMAL)
        .setTexture(WALL_TEXTURE_GRASS1, 20.0f));

    addWall(Wall(500, 720, 40, 200, WALL_NORMAL));
    addWall(Wall(500, 920, 40, 200, WALL_NO_STICK));
    addWall(Wall(760, 770, 40, 300, WALL_NO_STICK));

    addWall(Wall(760, 1070, 40, 460, WALL_NORMAL)
        .setTexture(WALL_TEXTURE_TREE2, 40.0f));

    addWall(Wall(540, 1120, 40, 40, WALL_NO_STICK));

    addWall(Wall(500, 1300, 100, 20, WALL_NORMAL));

    addWall(Wall(240, 1450, 520, 80, WALL_NORMAL)
        .setTexture(WALL_TEXTURE_STONE1, 80.0f));

    addWall(Wall(300, 1350, 100, 20, WALL_NORMAL)
        .setTexture(WALL_TEXTURE_STONE1, 20.0f));

    addWall(Wall(100, 1200, 70, 20, WALL_NORMAL));

    addWall(Wall(250, 800, 90, 90, WALL_NORMAL)
        .setTexture(WALL_TEXTURE_GRASS1, 90.0f));

    addWall(Wall(200, 1450, 40, 80, WALL_NO_STICK));
    addWall(Wall(200, 1600, 40, 200, WALL_NO_STICK));
    addWall(Wall(180, 1590, 20, 10, WALL_NO_STICK));
    addWall(Wall(0, 1590, 40, 410, WALL_NO_STICK));

    addWall(Wall(0, 2000, 40, 870, WALL_NORMAL)
        .setTexture(WALL_TEXTURE_TREE));

    addWall(Wall(180, 1800, 20, 20, WALL_NO_STICK));
    addWall(Wall(160, 1820, 20, 20, WALL_NO_STICK));

    addWall(Wall(660, 1650, 30, 200, WALL_BOOST)
        .setTexture(BOOST_WALL_NORMAL, 30.0f));
    addWall(Wall(390, 1850, 30, 200, WALL_BOOST)
        .setTexture(BOOST_WALL_NORMAL, 30.0f));
    addWall(Wall(640, 2250, 40, 100, WALL_BOOST));
    addWall(Wall(380, 2500, 40, 50, WALL_BOOST));

    addWall(Wall(760, 1530, 40, 1470, WALL_NO_STICK));
    addWall(Wall(760, 3000, 40, 950, WALL_NORMAL));

    addWall(Wall(150, 2600, 20, 100, WALL_NORMAL));

    addWall(Wall(400, 3050, 20, 100, WALL_NORMAL)
        .setMovement(5.0f, 2950.0f, 3150.0f));

    addWall(Wall(0, 2870, 40, 1830, WALL_NO_STICK));
    addWall(Wall(0, 4700, 40, 50, WALL_NORMAL));

    addWall(Wall(5, 2870, 40, 130, WALL_REFLECT));
    addWall(Wall(140, 2950, 30, 20, WALL_NORMAL));
    addWall(Wall(170, 2930, 20, 20, WALL_NO_STICK));
    addWall(Wall(190, 2910, 20, 20, WALL_NO_STICK));
    addWall(Wall(120, 2950, 20, 20, WALL_NO_STICK));
    addWall(Wall(720, 2670, 40, 20, WALL_NORMAL));

    addWall(Wall(650, 3300, 20, 100, WALL_NORMAL)
        .setMovement(4.0f, 3200.0f, 3350.0f));

    addWall(Wall(540, 3500, 30, 250, WALL_REFLECT)
        .setTexture(REFLECT_WALL_NORMAL, 30.0f));
    addWall(Wall(755, 3650, 30, 150, WALL_REFLECT)
        .setTexture(REFLECT_WALL_NORMAL, 30.0f));
    addWall(Wall(755, 3550, 30, 50, WALL_REFLECT)
        .setTexture(REFLECT_WALL_NORMAL, 30.0f));

    addWall(Wall(300, 3490, 150, 30, WALL_BOOST_RX)
        .setBoostX(-20.0f)
        .setTexture(BOOST_RX_WALL_NORMAL, 30.0f));

    addWall(Wall(5, 3450, 40, 400, WALL_BOOST));
    addWall(Wall(200, 4100, 30, 30, WALL_NO_STICK));
    addWall(Wall(430, 4300, 30, 30, WALL_NO_STICK));

    addWall(Wall(540, 3950, 220, 30, WALL_BOOST_RX)
        .setBoostX(-25.0f)
        .setTexture(BOOST_RX_WALL_NORMAL, 30.0f));

    addWall(Wall(260, 4400, 30, 30, WALL_NO_STICK));
    addWall(Wall(130, 4550, 30, 30, WALL_NO_STICK));
    addWall(Wall(150, 4640, 130, 20, WALL_NO_STICK));
    addWall(Wall(650, 4500, 60, 30, WALL_NORMAL));

    addWall(Wall(760, 3600, 40, 50, WALL_NO_STICK));
    addWall(Wall(0, 4750, 40, 250, WALL_NO_STICK));

    addWall(Wall(150, 4660, 20, 240, WALL_NO_STICK));
    addWall(Wall(150, 4900, 20, 50, WALL_NORMAL));
    addWall(Wall(150, 4950, 20, 300, WALL_NO_STICK));
    addWall(Wall(150, 5250, 20, 40, WALL_NORMAL));
    addWall(Wall(150, 5290, 20, 110, WALL_NO_STICK));

    addWall(Wall(0, 5000, 40, 20, WALL_NORMAL));
    addWall(Wall(0, 5020, 40, 980, WALL_NO_STICK));

    addWall(Wall(300, 5600, 60, 20, WALL_NORMAL));

    addWall(Wall(200, 5800, 380, 100, WALL_NORMAL)
        .setTexture(WALL_TEXTURE_STONE, 100.0f));
    addWall(Wall(200, 5900, 380, 100, WALL_NORMAL)
        .setTexture(WALL_SNOW_GRASS1, 100.0f));

    addWall(Wall(580, 5800, 20, 200, WALL_NO_STICK));
    addWall(Wall(380, 5700, 60, 20, WALL_NORMAL));
    addWall(Wall(560, 5700, 60, 20, WALL_NORMAL));

    addWall(Wall(760, 3950, 40, 2050, WALL_NO_STICK));


    // 氷ステージ
    addWall(Wall(0, 6000, 40, 14000, WALL_SLIPPERY));
    addWall(Wall(760, 6000, 40, 14000, WALL_SLIPPERY));
    addWall(Wall(755, 5900, 30, 200, WALL_REFLECT)
        .setTexture(REFLECT_WALL_NORMAL, 30.0f));

    addWall(Wall(300, 6100, 20, 200, WALL_SLIPPERY));
    addWall(Wall(710, 6150, 50, 30, WALL_NORMAL));
    addWall(Wall(40, 6400, 80, 80, WALL_SLIPPERY));
    addWall(Wall(120, 6410, 70, 70, WALL_SLIPPERY));
    addWall(Wall(190, 6420, 60, 60, WALL_SLIPPERY));
    addWall(Wall(40, 6480, 210, 20, WALL_NORMAL)
        .setTexture(WALL_SNOW_GRASS, 20.0f));
    addWall(Wall(500, 6250, 20, 200, WALL_SLIPPERY));
    addWall(Wall(755, 6180, 5, 3820, WALL_NO_STICK));

    addWall(Wall(190, 6780, 60, 20, WALL_NORMAL)
        .setTexture(WALL_SNOW_GRASS, 20.0f));

    addWall(Wall(500, 6850, 20, 200, WALL_SLIPPERY)
        .setMovement(4.0f, 6750.0f, 6950.0f));

    addWall(Wall(300, 7100, 20, 200, WALL_SLIPPERY)
        .setMovement(4.0f, 7000.0f, 7200.0f));

    addWall(Wall(520, 7300, 40, 250, WALL_SLIPPERY)
        .setMovement(3.0f, 7250.0f, 7400.0f));

    addWall(Wall(650, 7600, 20, 230, WALL_SLIPPERY)
        .setMovement(4.0f, 7500.0f, 7850.0f));

    addWall(Wall(710, 7100, 50, 20, WALL_NORMAL)
        .setTexture(WALL_SNOW_GRASS, 20.0f));
    addWall(Wall(400, 7900, 60, 20, WALL_NORMAL)
        .setTexture(WALL_SNOW_STONE, 20.0f));
    addWall(Wall(40, 7000, 5, 13000, WALL_NO_STICK));
    addWall(Wall(395, 8100, 20, 400, WALL_SLIPPERY));
    addWall(Wall(395, 8500, 20, 10, WALL_NORMAL)
        .setTexture(WALL_SNOW_STONE, 10.0f));

    addWall(Wall(290, 8650, 30, 100, WALL_REFLECT)
        .setTexture(REFLECT_WALL_NORMAL, 30.0f));
    addWall(Wall(500, 8750, 30, 100, WALL_REFLECT)
        .setTexture(REFLECT_WALL_NORMAL, 30.0f));
    addWall(Wall(290, 8800, 30, 100, WALL_REFLECT)
        .setTexture(REFLECT_WALL_NORMAL, 30.0f));
    addWall(Wall(500, 8900, 30, 100, WALL_REFLECT)
        .setTexture(REFLECT_WALL_NORMAL, 30.0f));
    addWall(Wall(290, 8950, 30, 100, WALL_REFLECT)
        .setTexture(REFLECT_WALL_NORMAL, 30.0f));
    addWall(Wall(500, 9050, 30, 100, WALL_REFLECT)
        .setTexture(REFLECT_WALL_NORMAL, 30.0f));

    addWall(Wall(170, 9150, 190, 20, WALL_NORMAL)
        .setTexture(WALL_SNOW_STONE, 20.0f));
    addWall(Wall(310, 9350, 50, 20, WALL_NORMAL)
        .setTexture(WALL_SNOW_STONE, 20.0f));
    addWall(Wall(220, 9450, 40, 20, WALL_NORMAL)
        .setTexture(WALL_SNOW_STONE, 20.0f));

    addWall(Wall(50, 9650, 30, 150, WALL_REFLECT)
        .setMovement(4.0f, 9550.0f, 9750.0f)
        .setTexture(REFLECT_WALL_NORMAL, 30.0f));

    addWall(Wall(290, 9770, 30, 30, WALL_NORMAL)
        .setTexture(WALL_SNOW_STONE, 30.0f));
    addWall(Wall(350, 9970, 30, 150, WALL_BOOST)
        .setTexture(BOOST_WALL_ICE, 30.0f));

    addWall(Wall(320, 10500, 40, 40, WALL_BOOST_FX)
        .setBoostX(12.0f)
        .setTexture(BOOST_FX_WALL_ICE));

    addWall(Wall(440, 10500, 40, 40, WALL_BOOST_RX)
        .setBoostX(-12.0f)
        .setTexture(BOOST_RX_WALL_ICE));

    addWall(Wall(280, 10540, 40, 40, WALL_BOOST_FX)
        .setBoostX(12.0f)
        .setTexture(BOOST_FX_WALL_ICE));

    addWall(Wall(480, 10540, 40, 40, WALL_BOOST_RX)
        .setBoostX(-12.0f)
        .setTexture(BOOST_RX_WALL_ICE));

    addWall(Wall(240, 10580, 40, 40, WALL_BOOST_FX)
        .setBoostX(12.0f)
        .setTexture(BOOST_FX_WALL_ICE));

    addWall(Wall(520, 10580, 40, 40, WALL_BOOST_RX)
        .setBoostX(-12.0f)
        .setTexture(BOOST_RX_WALL_ICE));

    addWall(Wall(200, 10620, 40, 40, WALL_BOOST_FX)
        .setBoostX(12.0f)
        .setTexture(BOOST_FX_WALL_ICE));

    addWall(Wall(560, 10620, 40, 40, WALL_BOOST_RX)
        .setBoostX(-12.0f)
        .setTexture(BOOST_RX_WALL_ICE));

    addWall(Wall(160, 10660, 40, 40, WALL_BOOST_FX)
        .setBoostX(12.0f)
        .setTexture(BOOST_FX_WALL_ICE));

    addWall(Wall(600, 10660, 40, 40, WALL_BOOST_RX)
        .setBoostX(-12.0f)
        .setTexture(BOOST_RX_WALL_ICE));

    addWall(Wall(440, 10240, 40, 100, WALL_BOOST)
        .setTexture(BOOST_WALL_ICE, 40.0f));
    addWall(Wall(450, 10620, 40, 10, WALL_NO_STICK));
    addWall(Wall(210, 10750, 5, 20, WALL_NO_STICK));
    addWall(Wall(140, 10900, 10, 150, WALL_SLIPPERY));

    addWall(Wall(390, 11000, 30, 30, WALL_NO_STICK)
        .setMovement(5.0f, 10920.0f, 11150.0f));

    addWall(Wall(550, 11420, 40, 40, WALL_NORMAL)
        .setTexture(WALL_SNOW_STONE));
    addWall(Wall(590, 11440, 40, 40, WALL_NORMAL)
        .setTexture(WALL_SNOW_STONE));
    addWall(Wall(630, 11460, 40, 40, WALL_NORMAL)
        .setTexture(WALL_SNOW_STONE));
    addWall(Wall(670, 11480, 40, 20, WALL_NORMAL)
        .setTexture(WALL_SNOW_STONE, 20.0f));
    addWall(Wall(710, 11490, 50, 10, WALL_NORMAL)
        .setTexture(WALL_SNOW_STONE, 10.0f));

    // ステージ補強
    addWall(Wall(150, 1840, 10, 25, WALL_NO_STICK));
    addWall(Wall(35, 2200, 10, 670, WALL_NO_STICK));
    addWall(Wall(45, 2200, 17, 10, WALL_NO_STICK));
    addWall(Wall(40, 2190, 10, 10, WALL_NO_STICK));
    addWall(Wall(40, 2210, 30, 10, WALL_NO_STICK));

    addWall(Wall(45, 2220, 25, 3, WALL_NORMAL)
        .setColor(0.0f, 0.4f, 0.0f));

    addWall(Wall(40, 750, 40, 20, WALL_NORMAL)
        .setColor(0.0f, 0.4f, 0.0f));

    // 追加ステージ
    addWall(Wall(350, 11650, 20, 200, WALL_SLIPPERY)
        .setMovement(3.0f, 11520.0f, 11800.0f));

    addWall(Wall(100, 11940, 20, 170, WALL_SLIPPERY));
    addWall(Wall(120, 11580, 40, 40, WALL_NORMAL)
        .setTexture(WALL_SNOW_STONE));

    addWall(Wall(100, 12220, 20, 70, WALL_SLIPPERY));

    addWall(Wall(100, 12380, 20, 70, WALL_SLIPPERY));

    addWall(Wall(100, 12540, 20, 60, WALL_SLIPPERY));

    addWall(Wall(140, 12710, 20, 70, WALL_SLIPPERY));
    addWall(Wall(170, 12930, 20, 180, WALL_SLIPPERY));

    addWall(Wall(350, 13010, 150, 30, WALL_NORMAL)
        .setTexture(WALL_SNOW_STONE, 30.0f));

    addWall(Wall(755, 11450, 5, 5500, WALL_NO_STICK));

    
    printf("%d", getWallCount());


    for (int i = 0; i < getWallCount(); i++) {
        walls[i].resetPreviousY();
    }
}

// 各壁に更新を指示する
void Stage::update()
{
    // 個々の壁の更新処理を行う
    for (std::size_t i = 0; i < walls.size(); i++)
    {
        walls[i].update(); // 更新処理
    }
}

// 全壁を描画
void Stage::draw()
{
    for (std::size_t i = 0; i < walls.size(); i++)
    {
        walls[i].draw();
    }
}

// 壁の追加処理
bool Stage::addWall(const Wall& wall)
{
    walls.push_back(wall); // wallsに要素を追加
    return true;
}

// 指定した壁を取得
Wall& Stage::getWall(int index)
{
    return walls[index];
}

// 壁の数を取得
int Stage::getWallCount() const
{
    return static_cast<int>(walls.size());
}




