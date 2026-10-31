#pragma once

class Ranking;

// スタート画面
void drawStartScreen(int menuIndex, int titleAnimationStep);

// タイム画面の描画
void drawTimeScreen(Ranking& ranking);

// 遊び方画面の描画
void drawHowToPlayScreen();

// クリア画面
void drawClearScreen(int clearPage, int clearTime, int jumpCount, int fallCount);

// プレイタイムの描画
void drawPlayTime(int playStartTime, int totalPauseTime);

// ポーズ画面の描画
void drawPauseScreen(int pauseMenuIndex);