#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define NOMINMAX
#include <Windows.h>
#include <GL/gl.h>

#include "Texture.h"
#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>

// 背景画像のテクスチャ番号
static GLuint forestTexture1 = 0;
static GLuint forestTexture2 = 0;
static GLuint forestTexture3 = 0;
static GLuint forestTexture4 = 0;
static GLuint iceTexture = 0;
static GLuint snowTexture1 = 0;
static GLuint snowTexture2 = 0;
static GLuint snowTexture3 = 0;
static GLuint snowTexture4 = 0;
static GLuint windTexture1 = 0;
static GLuint windTexture2 = 0;

static GLuint playerWalkTexture = 0;   // 中央
static GLuint playerWalkLTexture = 0;  // 左足前
static GLuint playerWalkRTexture = 0;  // 右足前
static GLuint playerUpTexture = 0;     // 上昇
static GLuint playerDownTexture = 0;   // 落下
static GLuint playerWallTexture = 0;   // 壁張り付き中
static GLuint playerKickTexture = 0;   // 壁キック時

static GLuint titleTexture = 0;
static GLuint titleIdle1Texture = 0;
static GLuint titleStepLTexture = 0;
static GLuint titleIdle2Texture = 0;
static GLuint titleStepRTexture = 0;

static GLuint howtoplayTexture = 0;
static GLuint timeTexture = 0;

static GLuint goal1Texture = 0;
static GLuint goal2Texture = 0;
static GLuint goal3Texture = 0;
static GLuint goal4Texture = 0;
static GLuint goalflagTexture = 0;

static GLuint clearPlayerTexture = 0;
static GLuint resultTexture = 0;

using namespace std;

// 画像の外周につながっている白背景だけを透明にする
static void MakeConnectedWhiteTransparent(unsigned char* image, int width, int height) {
	if (image == nullptr || width <= 0 || height <= 0) return;

	const int channels = 4;
	std::vector<bool> visited(width * height, false);
	std::queue<std::pair<int, int>> pixels;

	// 白に近い色か判定する
	auto isWhite = [&](int x, int y)
		{
			const int index = (y * width + x) * channels;

			const int r = static_cast<int>(image[index + 0]);
			const int g = static_cast<int>(image[index + 1]);
			const int b = static_cast<int>(image[index + 2]);
			const int a = static_cast<int>(image[index + 3]);

			const int maxColor =
				std::max(r, std::max(g, b));

			const int minColor =
				std::min(r, std::min(g, b));

			// 明るく、RGB差が小さい色のみ背景とみなす
			return a > 0 &&
				minColor >= 210 &&
				(maxColor - minColor) <= 25;
		};

	auto addPixel = [&](int x, int y)
		{
			if (x < 0 || x >= width || y < 0 || y >= height) return;
			
			const int visitedIndex = y * width + x;

			if (visited[visitedIndex] || !isWhite(x, y)) return;

			visited[visitedIndex] = true;
			pixels.push({ x, y });
		};

	// 画像の四辺から白背景を探索開始
	for (int x = 0; x < width; ++x)
	{
		addPixel(x, 0);
		addPixel(x, height - 1);
	}

	for (int y = 0; y < height; ++y)
	{
		addPixel(0, y);
		addPixel(width - 1, y);
	}

	const int dx[4] = { 1, -1, 0, 0 };
	const int dy[4] = { 0, 0, 1, -1 };

	while (!pixels.empty())
	{
		const auto [x, y] = pixels.front();
		pixels.pop();

		const int index = (y * width + x) * channels;

		// RGBも0にして完全な透明黒にする
		image[index + 0] = 0;
		image[index + 1] = 0;
		image[index + 2] = 0;
		image[index + 3] = 0;

		for (int i = 0; i < 4; ++i)
		{
			addPixel(x + dx[i], y + dy[i]);
		}
	}

}


// 画像ファイルを読み込み、OpenGLのテクスチャとして登録する
static GLuint LoadTexture(const char* filename, bool removeWhiteBackground = false) {
	
	int width = 0;
	int height = 0;
	int channels = 0;

	stbi_set_flip_vertically_on_load(true);

	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&channels,
		4);

	if (image == nullptr)
	{
		cout << "画像を読み込めませんでした\n";
		cout << stbi_failure_reason() << "\n";
		return 0;
	}

	if (removeWhiteBackground){
		MakeConnectedWhiteTransparent(image, width, height);
	}
	
	GLuint texture = 0;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,image);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(image);

	cout << "画像読込成功！\n";
	cout << "幅:" << width << "\n";
	cout << "高さ:" << height << "\n";
	cout << "チャンネル:" << channels << "\n";

	return texture;
	
}

// 指定されたテクスチャを四角形として描画する
static void DrawTexture(GLuint texture, float x, float y, float width, float height) {
	if (texture == 0)
	{
		return;
	}

	glEnable(GL_TEXTURE_2D);
	// 透明を有効化
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindTexture(GL_TEXTURE_2D, texture);

	// 元の画像色をそのまま表示する
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(x, y);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(x + width, y);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(x + width, y + height);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(x, y + height);

	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);
	// 透明を無効化
	glDisable(GL_BLEND);

	glDisable(GL_TEXTURE_2D);
}

static void DrawTextureFlipped(GLuint texture, float x, float y, float width, float height, bool flipX) {
	if (texture == 0) return;

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindTexture(GL_TEXTURE_2D, texture);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// 通常表示か左右反転かを決める
	const float leftU = flipX ? 1.0f : 0.0f;
	const float rightU = flipX ? 0.0f : 1.0f;

	glBegin(GL_QUADS);

	glTexCoord2f(leftU, 0.0f);
	glVertex2f(x, y);

	glTexCoord2f(rightU, 0.0f);
	glVertex2f(x + width, y);

	glTexCoord2f(rightU, 1.0f);
	glVertex2f(x + width, y + height);

	glTexCoord2f(leftU, 1.0f);
	glVertex2f(x, y + height);

	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

// ゲーム開始時に必要な画像を読み込む
void InitTexture(){
	cout << "Texture System Initialized!\n";

	forestTexture1 = LoadTexture(
		"Assets/Background/forest1.png"
	);

	forestTexture2 = LoadTexture(
		"Assets/Background/forest2.png"
	);

	forestTexture3 = LoadTexture(
		"Assets/Background/forest3.png"
	);

	forestTexture4 = LoadTexture(
		"Assets/Background/forest4.png"
	);

	snowTexture1 = LoadTexture(
		"Assets/Background/snow1.png"
	);

	snowTexture2 = LoadTexture(
		"Assets/Background/snow2.png"
	);

	snowTexture3 = LoadTexture(
		"Assets/Background/snow3.png"
	);

	snowTexture4 = LoadTexture(
		"Assets/Background/snow4.png"
	);
	
	windTexture1 = LoadTexture(
		"Assets/Background/wind1.png"
	);

	windTexture2 = LoadTexture(
		"Assets/Background/wind2.png"
	);


	playerWalkTexture = LoadTexture(
		"Assets/Player/walk.png",
		true
	);

	playerWalkLTexture = LoadTexture(
		"Assets/Player/walkL.png",
		true
	);

	playerWalkRTexture = LoadTexture(
		"Assets/Player/walkR.png",
		true
	);

	playerUpTexture = LoadTexture(
		"Assets/Player/up.png",
		true
	);

	playerDownTexture = LoadTexture(
		"Assets/Player/down.png",
		true
	);

	playerWallTexture = LoadTexture(
		"Assets/Player/wall.png",
		true
	);

	playerKickTexture = LoadTexture(
		"Assets/Player/kick.png",
		true
	);

	titleTexture = LoadTexture(
		"Assets/Background/title1.png"
	);

	titleIdle1Texture = LoadTexture(
		"Assets/Player/player1.png",
		true
	);

	titleStepLTexture = LoadTexture(
		"Assets/Player/player2.png",
		true
	);

	titleIdle2Texture = LoadTexture(
		"Assets/Player/player3.png",
		true
	);

	titleStepRTexture = LoadTexture(
		"Assets/Player/player4.png",
		true
	);

	goal1Texture = LoadTexture(
		"Assets/Player/goal1.png",
		true
	);

	goal2Texture = LoadTexture(
		"Assets/Player/goal2.png",
		true
	);

	goal3Texture = LoadTexture(
		"Assets/Player/goal3.png",
		true
	);

	goal4Texture = LoadTexture(
		"Assets/Player/goal4.png",
		true
	);

	clearPlayerTexture = LoadTexture(
		"Assets/Player/clear.png",
		true
	);

	resultTexture = LoadTexture(
		"Assets/Background/result.png"
	);

	howtoplayTexture = LoadTexture(
		"Assets/Background/how to play1.png"
	);

	timeTexture = LoadTexture(
		"Assets/Background/time.png"
	);

	goalflagTexture = LoadTexture(
		"Assets/Background/goalflag.png",
		true
	);
}

// cameraYに応じて背景画像を選択する
void DrawBackground(float screenWidth){

	
	DrawTexture(forestTexture1, 0.0f, 0.0f, screenWidth, 1500.0f);

	DrawTexture(forestTexture2, 0.0f, 1450.0f, screenWidth, 1500.0f);

	DrawTexture(forestTexture3, 0.0f, 2900.0f, screenWidth, 1500.0f);

	DrawTexture(forestTexture4, 0.0f, 4350.0f, screenWidth, 1500.0f);
	
	DrawTexture(snowTexture1, 0.0f, 5820.0f, screenWidth, 1500.0f);

	DrawTexture(snowTexture2, 0.0f, 7300.0f, screenWidth, 1500.0f);

	DrawTexture(snowTexture3, 0.0f, 8800.0f, screenWidth, 1500.0f);

	DrawTexture(snowTexture4, 0.0f, 10300.0f, screenWidth, 1500.0f);

	DrawTexture(windTexture1, 0.0f, 11800.0f, screenWidth, 1500.0f);

	DrawTexture(windTexture2, 0.0f, 13300.0f, screenWidth, 1500.0f);
	
	
}

void DrawPlayerTexture(float x, float y, float width, float height, int animationStep, int direction, bool isAirborne, float velocityY, bool isWallSticking, bool isWallRight, bool isWallKick) {
	GLuint texture = playerWalkTexture;
	 bool flipX = direction > 0;
	
	 if (isWallKick) { // 壁キック
		 texture = playerKickTexture;
		 flipX = (direction < 0); // 右へ飛ぶときはそのまま、左へ飛ぶときだけ反転
	} else if (isWallSticking) { // 壁張り付き時
		texture = playerWallTexture;
		flipX = !isWallRight; // 元画像が左壁に張り付いている向きの場合、右壁では左右反転する
	} else if (isAirborne) {
		if (velocityY >= 0.0f) { // 空中
			texture = playerUpTexture;
		} else {
			texture = playerDownTexture;
		}
		flipX = direction > 0;
	}
	else { // 地上
		// 左足前 → 中央 → 右足前 → 中央
		switch (animationStep) {
		case 0:
			texture = playerWalkLTexture;
			break;

		case 1:
			texture = playerWalkTexture;
			break;

		case 2:
			texture = playerWalkRTexture;
			break;

		case 3:
			texture = playerWalkTexture;
			break;
        
		default:
			texture = playerWalkTexture;
			break;
		}
		flipX = direction > 0;
	}
	/*
		元画像は左向き。
		directionが1なら右向きに反転する。
		directionが-1ならそのまま左向き。
	*/

	DrawTextureFlipped(texture, x, y, width, height, flipX);
}

// タイトル背景画像描画関数
void DrawTitleBackground(float x, float y, float width, float height) {
	DrawTexture(titleTexture, x, y, width, height);
}

// タイトル画面キャラクターアニメーション描画関数
void DrawTitlePlayerTexture(float x, float y, float width, float height, int animationStep) {
	GLuint texture = titleIdle1Texture;

	switch (animationStep)
	{
	case 0:
		texture = titleIdle1Texture;
		break;

	case 1:
		texture = titleStepLTexture;
		break;

	case 2:
		texture = titleIdle2Texture;
		break;

	case 3:
		texture = titleStepRTexture;
		break;

	default:
		texture = titleIdle1Texture;
		break;
	}

	DrawTexture(texture, x, y, width, height);

}

// ゴール演出キャラクターアニメーション描画関数
void DrawGoalPlayerTexture(float x, float y, float width, float height, int animationStep)
{
	GLuint texture = goal1Texture;

	switch (animationStep)
	{
	case 0:
		texture = goal1Texture;
		break;

	case 1:
		texture = goal2Texture;
		break;

	case 2:
		texture = goal3Texture;
		break;

	case 3:
		texture = goal4Texture;
		break;

	default:
		texture = goal4Texture;
		break;
	}

	DrawTexture(texture, x, y, width, height);
}

// クリア画面描画関数
void DrawClearPlayerTexture(float x, float y, float width, float height)
{
	DrawTexture(clearPlayerTexture, x, y, width, height);
}

// リザルト画像描画関数
void DrawResultTexture(float x, float y, float width, float height)
{
	DrawTexture(resultTexture, x, y, width, height);
}

// 遊び方画像描画関数
void DrawHowToPlayTexture(float x, float y, float width, float height)
{
	DrawTexture(howtoplayTexture, x, y, width, height);
}

// タイム画像描画関数
void TimeTexture(float x, float y, float width, float height)
{
	DrawTexture(timeTexture, x, y, width, height);
}

// ゴール旗画像描画関数
void GoalFlagTexture(float x, float y, float width, float height)
{
	DrawTexture(goalflagTexture, x, y, width, height);
}


