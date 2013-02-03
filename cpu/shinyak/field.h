#ifndef __FIELD_H__
#define __FIELD_H__

#include <string>
#include <vector>

#include "puyo.h"
#include "puyo_set.h"
#include "util.h"

class Decision;
class FieldBitField;
class FieldColumnBitField;
class NonTrackingStrategy;
class Plan;
class TrackResult;
class TrackingStrategy;
struct BasicRensaInfo;
struct FeasibleRensaInfo;
struct PossibleRensaInfo;

struct Position {
    Position() {}
    Position(int x, int y) : x(x), y(y) {}

    int x;
    int y;
};

class Field {
public:
    static const int WIDTH = 6;
    static const int HEIGHT = 12;
    static const int MAP_WIDTH = 1 + WIDTH + 1;
    static const int MAP_HEIGHT = 1 + HEIGHT + 3;
    static const int ERASE_NUM = 4;
    static const int COLORS = 8;

    Field();
    Field(const std::string& url);    
    Field(const Field&);

    // Get a color of puyo at a specified position.
    PuyoColor color(int x, int y) const { return m_field[x][y].color(); }

    // Do not use not in PlayerInfo.
    void setColor(int x, int y, PuyoColor c) { m_field[x][y] = c; }
    void recalcHeightOn(int x) {
        m_heights[x] = 0;
        for (int y = 1; color(x, y) != EMPTY; ++y)
            m_heights[x] = y;
    }

    // Returns the height of the specified column.
    int height(int x) const { return m_heights[x]; }

    // Check if the field is in Zenkeshi
    bool isZenkeshi() const;
    int countColorPuyos() const;
    int countPuyos() const;
    int connectedPuyoNums(int x, int y) const;

    // True if EMPTY exists around (x, y)
    bool hasEmptyAround(int x, int y) const {
        return color(x - 1, y) == EMPTY || color(x + 1, y) == EMPTY || color(x, y + 1) == EMPTY;
    }

    // Drop kumipuyo with decision.
    void dropKumiPuyo(const Decision&, const KumiPuyo&);
    void dropKumiPuyoSafely(const Decision&, const KumiPuyo&);
    // Returns #frame to drop the next Haipuyo with decision. This function do not drop the puyo.
    int framesToDropNext(const Decision&) const;

    void dropPuyoOn(int x, PuyoColor);
    void removeTopPuyoFrom(int x) {
        if (height(x) > 0)
            m_field[x][m_heights[x]--] = EMPTY;
    }

    void simulate(BasicRensaInfo& rensaInfo, int additionalChain = 0);
    void simulateAndTrack(BasicRensaInfo& rensaInfo, TrackResult& trackResult);
    
    // Normal print for debugging purpose.
    std::string getDebugOutput() const;

    bool hasSameField(const Field&) const;

public:
    void findAvailablePlans(int depth, const std::vector<KumiPuyo>& kumiPuyos, std::vector<Plan>& plans) const;

    // 現在のフィールドから発火可能な連鎖を列挙する。
    void findRensas(std::vector<PossibleRensaInfo>& result, const PuyoSet& additionalPuyoSet = PuyoSet()) const;

    // Before calling findRensa, adding some puyos (numMaxAddedPuyo) is allowed. 
    void findPossibleRensas(std::vector<PossibleRensaInfo>& result, int numMaxAddedPuyo) const;

    // 与えられた組ぷよだけを使って発火可能な連鎖を求める
    void findFeasibleRensas(std::vector<FeasibleRensaInfo>& result, int numKumiPuyo, const std::vector<KumiPuyo>& kumiPuyos) const;

private:
    // Crears every data this class has.
    void initialize();

    int calculateRensaBonusCoef(int chainBonusCoef, int longBonusCoef, int colorBonusCoef) const {
        int coef = chainBonusCoef + longBonusCoef + colorBonusCoef;
        if (coef == 0)
            return 1;
        if (coef > 999)
            return 999;
        return coef;
    }
    
    // Simulate chains until the end, and returns chains, score, and frames
    // before finishing the chain.
    template<typename Strategy>
    void simulateWithStrategy(BasicRensaInfo&, Strategy&);

    // Vanish puyos, and adds score. The argument "chains" is used to calculate score.
    template<typename Strategy>
    bool vanish(int nthChain, int* score, int minHeights[], Strategy&);
    template<typename Strategy>
    void erase(int nthChain, Position* eraseQueue, Position* eraseQueueHead, int minHeights[], Strategy&);
    template<typename Strategy>
    int drop(int minHeights[], Strategy&);

    Position* checkCell(PuyoColor, FieldBitField& checked, Position* writeHead, int x, int y) const;

    Position* fillSameColorPosition(int x, int y, PuyoColor, Position* positionQueueHead, FieldBitField& checked) const;

    void findAvailablePlansInternal(const Plan* previousPlan, int restDepth, int nth, const std::vector<KumiPuyo>& kumiPuyos, std::vector<Plan>& plans) const;
    void findPossibleRensasInternal(std::vector<PossibleRensaInfo>& result, PuyoSet addedSet, int leftX, int restAdd) const;

private:
    Puyo m_field[MAP_WIDTH][MAP_HEIGHT];
    byte m_heights[MAP_WIDTH];
};

#endif  // __FIELD_H__