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
class TrackingStrategy;
class TrackResult;
struct BasicRensaInfo;
struct TrackedRensaInfo;
struct Position;

class Field {
public:
    static const int WIDTH = 6;
    static const int HEIGHT = 12;
    static const int MAP_WIDTH = 1 + WIDTH + 1;
    static const int MAP_HEIGHT = 1 + HEIGHT + 3;

    Field();
    Field(const std::string& url);    
    Field(const Field&);

    // Get a color of puyo at a specified position.
    PuyoColor color(int x, int y) const { return static_cast<PuyoColor>(m_field[x][y]); }
    // Returns the height of the specified column.
    int height(int x) const { return m_heights[x]; }

    // Check if the field is in Zenkeshi
    bool isZenkeshi() const;
    int countColorPuyos() const;
    int countPuyos() const;
    int connectedPuyoNums(int x, int y) const;
    int connectedPuyoNums(int x, int y, FieldBitField& checked) const;
    std::pair<int, int> connectedPuyoNumsWithAllowingOnePointJump(int x, int y) const;
    std::pair<int, int> connectedPuyoNumsWithAllowingOnePointJump(int x, int y, FieldBitField& checked) const;

    bool findBestBreathingSpace(int& breathingX, int& breathingY, int x, int y) const;
    
    // Drop kumipuyo with decision.
    void dropKumiPuyo(const Decision&, const KumiPuyo&);
    void dropKumiPuyoSafely(const Decision&, const KumiPuyo&);
    // Returns #frame to drop the next KumiPuyo with decision. This function do not drop the puyo.
    int framesToDropNext(const Decision&) const;

    void dropPuyoOn(int x, PuyoColor);
    void removeTopPuyoFrom(int x) {
        if (height(x) > 0)
            m_field[x][m_heights[x]--] = EMPTY;
    }

    void forceDrop();

    void simulate(BasicRensaInfo& rensaInfo, int additionalChains = 0);
    void simulateAndTrack(BasicRensaInfo& rensaInfo, TrackResult& trackResult, int additionalChains = 0);
    
    // Normal print for debugging purpose.
    std::string getDebugOutput() const;
    void showDebugOutput() const;

    friend bool operator==(const Field&, const Field&);

public:
    // Compatibility interface for Ctrl
    PuyoColor Get(int x, int y) const { return color(x, y); }

protected:
    void set(int x, int y, PuyoColor c) {
        m_field[x][y] = static_cast<Puyo>(c);
    }

private:
    // Crears every data this class has.
    void initialize();
    
    // Simulate chains until the end, and returns chains, score, and frames
    // before finishing the chain.
    template<typename Strategy>
    void simulateWithStrategy(BasicRensaInfo&, Strategy&);

    // Vanish puyos, and adds score. The argument "chains" is used to calculate score.
    template<typename Strategy>
    bool vanish(int nthChain, int* score, int minHeights[], Strategy&);
    template<typename Strategy>
    void eraseQueuedPuyos(int nthChain, Position* eraseQueue, Position* eraseQueueHead, int minHeights[], Strategy&);
    template<typename Strategy>
    int dropAfterVanish(int minHeights[], Strategy&);

    Position* checkCell(PuyoColor, FieldBitField& checked, Position* writeHead, int x, int y) const;

    Position* fillSameColorPosition(int x, int y, PuyoColor, Position* positionQueueHead, FieldBitField& checked) const;

protected:
    Puyo m_field[MAP_WIDTH][MAP_HEIGHT];
    byte m_heights[MAP_WIDTH];
};

class ArbitrarilyModifiableField : public Field {
public:
    ArbitrarilyModifiableField() {}
    ArbitrarilyModifiableField(const Field& field) :
        Field(field) {}

    // Do not use not in PlayerInfo.
    void setColor(int x, int y, PuyoColor c) { set(x, y, c); }
    void recalcHeightOn(int x) {
        m_heights[x] = 0;
        for (int y = 1; color(x, y) != EMPTY; ++y)
            m_heights[x] = y;
    }    
};

#endif  // __FIELD_H__
