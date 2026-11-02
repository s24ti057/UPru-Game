#include "Audio.h"

#include <Windows.h>
#include <mmsystem.h>
#include <tchar.h>

#pragma comment(lib, "winmm.lib")

// 最大チャージSEの初期化処理
void Audio::initialize()
{
	mciSendString(TEXT("open \"charge_max1.wav\" type waveaudio alias charge"), NULL, 0, NULL);
}

// ステージBGM開始
void Audio::startStageBGM()
{
	mciSendString(TEXT("close bgm"), NULL, 0, NULL); // 前回のステージBGMを閉じる

	// WAVファイルの"stage_bgm.wav"を開くという操作をbgmとする 
	mciSendString(TEXT("open \"stage_bgm1.wav\" type waveaudio alias bgm"), NULL, 0, NULL);

	mciSendString(TEXT("setaudio bgm volume to 300"), NULL, 0, NULL); // 音量調整

	mciSendString(TEXT("play bgm"), NULL, 0, NULL); // bgmを再生する
}

// ステージBGM停止
void Audio::stopStageBGM()
{
	mciSendString(TEXT("stop bgm"), NULL, 0, NULL);
	mciSendString(TEXT("close bgm"), NULL, 0, NULL);
}

// ステージBGMのループ確認
void Audio::updateStageBGM()
{
	TCHAR mode[32];

	mciSendString(TEXT("status bgm mode"), mode, 32, NULL);

	if (_tcscmp(mode, TEXT("stopped")) == 0)
	{
		mciSendString(TEXT("play bgm from 0"), NULL, 0, NULL);
	}
}


// タイトルBGM開始
void Audio::startTitleBGM()
{
	mciSendString(TEXT("close title"), NULL, 0, NULL); // 前回のタイトルBGMを閉じる

	// WAVファイルの"8bit_Far-Horizon.wav"を開くという操作をtitleとする
	mciSendString(TEXT("open \"8bit_Far-Horizon.wav\" type waveaudio alias title"), NULL, 0, NULL);

	mciSendString(TEXT("play title"), NULL, 0, NULL); // titleを再生する
}

// タイトルBGM停止
void Audio::stopTitleBGM()
{
	mciSendString(TEXT("stop title"), NULL, 0, NULL);
	mciSendString(TEXT("close title"), NULL, 0, NULL);
}

// タイトルBGMのループ確認
void Audio::updateTitleBGM()
{
	TCHAR mode[32]; // MCIから返ってきたBGMの状態を入れておく文字列用の箱

	mciSendString(TEXT("status title mode"), mode, 32, NULL); // titleの現在のmodeをMCIに聞く

	// BGMが終了していたらはじめから再生する
	if (_tcscmp(mode, TEXT("stopped")) == 0)
	{
		mciSendString(TEXT("play title from 0"), NULL, 0, NULL);
	}
}


// ゴールBGM開始
void Audio::startGoalBGM()
{
	mciSendString(TEXT("close goal"), NULL, 0, NULL);

	mciSendString(TEXT("open \"goal_bgm.wav\" type waveaudio alias goal"), NULL, 0, NULL);

	mciSendString(TEXT("play goal"), NULL, 0, NULL);
}

// ゴールBGM停止
void Audio::stopGoalBGM()
{
	mciSendString(TEXT("stop goal"), NULL, 0, NULL);
	mciSendString(TEXT("close goal"), NULL, 0, NULL);
}

// ジャンプSE
void Audio::playJumpSound()
{
	PlaySound(TEXT("jump.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}

// 最大ジャンプSE
void Audio::playChargeMaxSound()
{
	mciSendString(TEXT("play charge from 0"), NULL, 0, NULL);
}

// 反射壁SE
void Audio::playReflectSound()
{
	PlaySound(TEXT("reflect.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}

// 加速壁SE
void Audio::playBoostSound()
{
	PlaySound(TEXT("boost.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}

// 加速床SE
void Audio::playBoostXSound()
{
	PlaySound(TEXT("boostX.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}

// 氷壁SE再生・ループ
void Audio::updateIceSound(bool isSlidingIce)
{
    // 氷壁を滑り続けているか
    if (isSlidingIce)
    {
        // 再生していなければ開始
        if (!iceSoundPlaying)
        {
            MCIERROR err = mciSendString(TEXT("open \"ice.wav\" type waveaudio alias ice"),NULL, 0, NULL);

            if (err != 0)
                return;

            err = mciSendString(TEXT("play ice"), NULL, 0, NULL);

            if (err != 0)
            {
                mciSendString(TEXT("close ice"), NULL, 0, NULL);
                return;
            }

            iceSoundPlaying = true;
        }
        else
        {
            // 再生終了後にループ
            TCHAR mode[32] = {};

            if (mciSendString(TEXT("status ice mode"), mode, 32, NULL) == 0)
            {
                if (_tcscmp(mode, TEXT("stopped")) == 0)
                {
                    mciSendString(TEXT("play ice from 0"), NULL, 0, NULL);
                }
            }
        }
    }
    else
     // 氷壁から離れたとき
    {
        stopIceSound();
    }
}

// 氷壁SE停止
void Audio::stopIceSound()
{
    if (!iceSoundPlaying)
        return;

    mciSendString(TEXT("stop ice"), NULL, 0, NULL);
    mciSendString(TEXT("close ice"), NULL, 0, NULL);

    iceSoundPlaying = false;
}
