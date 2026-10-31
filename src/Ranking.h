#pragma once

class Ranking
{
private:
    static const int RANKING_SIZE = 5;
    int bestTimes[RANKING_SIZE];

public:
    Ranking();

    void load();
    void save();
    void registerTime(int newTime);

    int getTime(int index) const; // 外部から記録を見る
    int getSize() const; // ランキング数を調べる
};