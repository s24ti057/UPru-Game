#include "Ranking.h"
#include <fstream>

// Ranking()というコンストラクタの実装
Ranking::Ranking()
{
	for (int i = 0; i < RANKING_SIZE; i++)
	{
		bestTimes[i] = -1;
	}
}

// ランキング保存関数
void Ranking::save()
{
	std::ofstream file("ranking.txt"); // ファイルへ書き込み(ファイルがない場合、新しく作成)

	// エラー処理
	if (!file)
	{
		return;
	}

	for (int i = 0; i < RANKING_SIZE; i++)
	{
		file << bestTimes[i] << "\n";
	}

	file.close();
}

// 読み込み関数
void Ranking::load()
{
	std::ifstream file("ranking.txt"); // ファイルからの読み込み

	// ファイルがまだ存在しない(一度もクリアしていない)
	if (!file)
	{
		return;
	}

	// ファイルから過去のランキングデータを一行ずつ読み込む
	for (int i = 0; i < RANKING_SIZE; i++)
	{
		file >> bestTimes[i];

		if (!file) {
			bestTimes[i] = -1;
		}
	}

	file.close();

}

// クリアタイムをランキングに入れる関数(引数は今回クリアしたタイム)
void Ranking::registerTime(int newTime)
{
	for (int i = 0; i < RANKING_SIZE; i++)
	{
		// 記録なし、または新しいタイムの方が速い
		if (bestTimes[i] == -1 || newTime < bestTimes[i])
		{
			// 下の順位を1つずつ後ろへずらす
			for (int j = RANKING_SIZE - 1; j > i; j--)
			{
				bestTimes[j] = bestTimes[j - 1];
			}

			bestTimes[i] = newTime;

			// ランキングをファイル保存(ranking.txtへ書き込み)
			save();

			break;
		}
	}
}

// 任意の順位のタイムを外部から取得
int Ranking::getTime(int index) const
{
	return bestTimes[index];
}

// ランキングサイズを外部から取得
int Ranking::getSize() const
{
	return RANKING_SIZE;
}
