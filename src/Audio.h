#pragma once

class Audio
{
public:
    // 初期化処理
    void initialize();

    // ステージBGM
    void startStageBGM();
    void stopStageBGM();
    void updateStageBGM();

    // タイトルBGM
    void startTitleBGM();
    void stopTitleBGM();
    void updateTitleBGM();

    // ゴールBGM
    void startGoalBGM();
    void stopGoalBGM();

    // SE
    void playJumpSound();      // ジャンプSE
    void playChargeMaxSound(); // 最大チャージSE
};