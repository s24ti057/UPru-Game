#pragma once

#include "Wall.h"
#include <vector>

class Stage
{
private:
	std::vector<Wall> walls; // 壁を動的配列で管理

public:
	void initialize(); // 壁の初期化処理

	void update(); // 壁の更新を指示

	void draw(); // 全壁を描画

	bool addWall(const Wall &wall); // 壁の追加処理 

	Wall &getWall(int index); // 指定した壁を取得 
	
	int getWallCount() const; // 壁の数を取得


};