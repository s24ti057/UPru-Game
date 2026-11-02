#pragma once

class Audio
{
private:
    bool iceSoundPlaying = false; // 氷壁SEの再生管理

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

    void playReflectSound(); // 反射壁SE
    void playBoostSound(); // 加速壁SE
    void playBoostXSound(); // 加速床SE

    // 氷壁SE
    void updateIceSound(bool isSlidingIce);
    void stopIceSound();
};
