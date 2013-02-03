#include "field.h"

#include <cstdlib>
#include <glog/logging.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cassert>

#include "../../core/constant.h"
#include "ctrl.h"
#include "decision.h"
#include "field_bit_field.h"
#include "field_column_bit_field.h"
#include "score.h"
#include "plan.h"
#include "rensainfo.h"

using namespace std;

class NonTrackingStrategy {
public:
    void colorPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) { }
    void ojamaPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) { }
    void puyoIsDropped(int /*x*/, int /*fromY*/, int /*toY*/) { }
};

class RensaTrackingStrategy {
public:
    RensaTrackingStrategy(TrackResult& trackResult)
        : m_result(trackResult)
    {
        for (int x = 0; x < Field::MAP_WIDTH; ++x) {
            for (int y = 0; y < Field::MAP_HEIGHT; ++y) {
                m_originalY[x][y] = y;
                m_result.m_erasedAt[x][y] = 0;
            }
        }
    }

    void colorPuyoIsVanished(int x, int y, int nthChain) {
        m_result.m_erasedAt[x][m_originalY[x][y]] = nthChain;
    }

    void ojamaPuyoIsVanished(int x, int y, int nthChain) {
        m_result.m_erasedAt[x][m_originalY[x][y]] = nthChain;
    }

    void puyoIsDropped(int x, int fromY, int toY) {
        m_originalY[x][toY] = m_originalY[x][fromY];        
    }

private:
    int m_originalY[Field::MAP_WIDTH][Field::MAP_HEIGHT];
    TrackResult& m_result;
};

void Field::initialize()
{
    // Initialize field information.
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y)
            m_field[x][y] = EMPTY;
    }

    for (int x = 0; x < MAP_WIDTH; ++x)
        m_field[x][0] = m_field[x][MAP_HEIGHT - 1] = WALL;

    for (int y = 0; y < MAP_HEIGHT; ++y)
        m_field[0][y] = m_field[MAP_WIDTH - 1][y] = WALL;

    for (int x = 0; x < MAP_WIDTH; ++x)
        m_heights[x] = 0;
}

Field::Field()
{
    initialize();
}

Field::Field(const std::string& url)
{
    initialize();

    std::string prefix = "http://www.inosendo.com/puyo/rensim/??";
    int data_starts_at = url.find(prefix) == 0 ? prefix.length() : 0;

    int counter = 0;
    for (int i = url.length() - 1; i >= data_starts_at; --i) {
        int x = 6 - (counter % 6);
        int y = counter / 6 + 1;
        PuyoColor c = puyoColorOf(url[i]);        
        m_field[x][y] = c;
        if (c != EMPTY)
            m_heights[x] = y;
        counter++;
    }
}

Field::Field(const Field& f)
{
    for (int x = 0; x < MAP_WIDTH; x++) {
        m_heights[x] = f.m_heights[x];
        for (int y = 0; y < MAP_HEIGHT; y++) {
            m_field[x][y] = f.m_field[x][y];
        }
    }
}

void Field::dropKumiPuyo(const Decision& decision, const KumiPuyo& kumiPuyo)
{
    int x1 = decision.x;
    int x2 = decision.x + (decision.r == 1) - (decision.r == 3);

    PuyoColor c1 = kumiPuyo.axis;
    PuyoColor c2 = kumiPuyo.child;
    if (decision.r == 2)
        swap(c1, c2);

    dropPuyoOn(x1, c1);
    dropPuyoOn(x2, c2);
}

void Field::dropKumiPuyoSafely(const Decision& decision, const KumiPuyo& kumiPuyo)
{
    int x1 = decision.x;
    int x2 = decision.x + (decision.r == 1) - (decision.r == 3);

    PuyoColor c1 = kumiPuyo.axis;
    PuyoColor c2 = kumiPuyo.child;
    if (decision.r == 2)
        swap(c1, c2);

    if (height(x1) < 14)
        dropPuyoOn(x1, c1);
    if (height(x2) < 14)
        dropPuyoOn(x2, c2);
}

int Field::framesToDropNext(const Decision& decision) const
{
    // TODO: This calculation should be more accurate.
    int x1 = decision.x;
    int x2 = decision.x + (decision.r == 1) - (decision.r == 3);

    int dropFrames = abs(3 - x1) * FRAMES_HORIZONTAL_MOVE;

    if (decision.r == 0)
        dropFrames += (HEIGHT - height(x1)) * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI;
    else if (decision.r == 2) {
        // TODO: If puyo lines are high enough, rotation might take time. We should measure this later.
        if (height(x1) > 6)
            dropFrames += (HEIGHT - height(x1) + 1) * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI;
        else
            dropFrames += (HEIGHT - height(x1) - 1) * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI;
    } else {
        if (height(x1) == height(x2))
            dropFrames += (HEIGHT - height(x1)) * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI;
        else {
            int minHeight = min(height(x1), height(x2));
            int maxHeight = max(height(x1), height(x2));
            int diff = maxHeight - minHeight;
            dropFrames += (HEIGHT - maxHeight) * FRAMES_DROP_1_LINE;
            dropFrames += FRAMES_AFTER_CHIGIRI;
            if (diff == 1)
                dropFrames += FRAMES_CHIGIRI_1_LINE_1;
            else if (diff == 2)
                dropFrames += FRAMES_CHIGIRI_1_LINE_1 + FRAMES_CHIGIRI_1_LINE_2;
            else
                dropFrames += FRAMES_CHIGIRI_1_LINE_1 + FRAMES_CHIGIRI_1_LINE_2 + (diff - 2) * FRAMES_CHIGIRI_1_LINE_3;
        }
    }

    if (dropFrames < 0)
        dropFrames = 0;

    return dropFrames;
}

void Field::dropPuyoOn(int x, PuyoColor c)
{
    DCHECK(c != EMPTY);
    DCHECK(height(x) < 14);
    DCHECK(color(x, height(x) + 1) == EMPTY) << " (x, height, color) = " << x << " : " << height(x) << " : " << color(x, height(x + 1));

    m_field[x][++m_heights[x]] = c;
}

int Field::connectedPuyoNums(int x, int y) const
{
    DCHECK(isColorPuyo(color(x, y)));

    Position positions[WIDTH * HEIGHT];
    FieldBitField checked;

    Position* filledHead = fillSameColorPosition(x, y, color(x, y), positions, checked);
    return filledHead - positions;
}

int Field::countColorPuyos() const
{
    int cnt = 0;
    for (int x = 1; x <= WIDTH; ++x) {
        for (int y = 1; color(x, y) != EMPTY; ++y) {
            if (isColorPuyo(color(x, y)))
                ++cnt;
        }
    }

    return cnt;
}

int Field::countPuyos() const
{
    int count = 0;
    for (int x = 1; x <= WIDTH; ++x)
        count += height(x);

    return count;
}

inline Position* Field::checkCell(PuyoColor c, FieldBitField& checked, Position* writeHead, int x, int y) const
{
    if (checked(x, y))
        return writeHead;

    if (y <= HEIGHT && c == color(x, y)) {
        writeHead->x = x;
        writeHead->y = y;
        checked.set(x, y);

        return writeHead + 1;
    }

    return writeHead;
}

Position* Field::fillSameColorPosition(int x, int y, PuyoColor color, Position* positionQueueHead, FieldBitField& checked) const
{
    DCHECK(!checked(x, y));

    Position* writeHead = positionQueueHead;
    Position* readHead = positionQueueHead;

    *writeHead++ = Position(x, y);
    checked.set(x, y);
    
    while (readHead != writeHead) {
        Position p = *readHead++;
        
        writeHead = checkCell(color, checked, writeHead, p.x + 1, p.y);
        writeHead = checkCell(color, checked, writeHead, p.x - 1, p.y);
        writeHead = checkCell(color, checked, writeHead, p.x, p.y + 1);
        writeHead = checkCell(color, checked, writeHead, p.x, p.y - 1);
    }

    return writeHead;
}

template<typename Strategy>
bool Field::vanish(int nthChain, int* score, int minHeights[], Strategy& strategy)
{
    FieldBitField checked;
    Position eraseQueue[WIDTH * HEIGHT]; // All the positions of erased puyos will be stored here.
    Position* eraseQueueHead = eraseQueue;

    bool usedColors[COLORS + 1] = { 0 };
    int numUsedColors = 0;
    int longBonusCoef = 0;

    for (int x = 1; x <= WIDTH; ++x) {
        for (int y = minHeights[x]; y <= HEIGHT; ++y) {
            if (color(x, y) == EMPTY)
                break;

            if (checked(x, y) || color(x, y) == OJAMA)
                continue;

            PuyoColor c = color(x, y);
            Position* head = fillSameColorPosition(x, y, c, eraseQueueHead, checked);

            int connectedPuyoNum = head - eraseQueueHead;
            if (connectedPuyoNum < ERASE_NUM)
                continue;

            eraseQueueHead = head;
            longBonusCoef += longBonus(connectedPuyoNum);
            if (!usedColors[c]) {
                ++numUsedColors;
                usedColors[c] = true;
            }
        }
    }

    int numErasedPuyos = eraseQueueHead - eraseQueue;
    if (numErasedPuyos == 0)
        return false;

    // --- Actually erase the Puyos to be vanished.
    erase(nthChain, eraseQueue, eraseQueueHead, minHeights, strategy);

    int rensaBonusCoef = calculateRensaBonusCoef(chainBonus(nthChain), longBonusCoef, colorBonus(numUsedColors));
    *score += 10 * numErasedPuyos * rensaBonusCoef;
    return true;
}

template<typename Strategy>
void Field::erase(int nthChain, Position* eraseQueue, Position* eraseQueueHead, int minHeights[], Strategy& strategy)
{
    for (int i = 1; i <= WIDTH; i++)
        minHeights[i] = 100;

    for (Position* head = eraseQueue; head != eraseQueueHead; ++head) {
        int x = head->x;
        int y = head->y;

        m_field[x][y] = EMPTY;
        strategy.colorPuyoIsVanished(x, y, nthChain);
        minHeights[x] = std::min(minHeights[x], y);

        // Check OJAMA puyos erased
        if (color(x + 1, y) == OJAMA) {
            m_field[x + 1][y] = EMPTY;
            strategy.ojamaPuyoIsVanished(x + 1, y, nthChain);
            minHeights[x + 1] = std::min(minHeights[x + 1], y);
        }

        if (color(x - 1, y) == OJAMA) {
            m_field[x - 1][y] = EMPTY;
            strategy.ojamaPuyoIsVanished(x - 1, y, nthChain);
            minHeights[x - 1] = std::min(minHeights[x - 1], y);
        }

        // We don't need to update minHeights here.
        if (color(x, y + 1) == OJAMA && y + 1 <= HEIGHT) {
            m_field[x][y + 1] = EMPTY;
            strategy.ojamaPuyoIsVanished(x, y + 1, nthChain);
        }

        if (color(x, y - 1) == OJAMA) {
            m_field[x][y - 1] = EMPTY;
            strategy.ojamaPuyoIsVanished(x, y - 1, nthChain);
            minHeights[x] = std::min(minHeights[x], y - 1);
        }
    }
}

template<typename Strategy>
int Field::drop(int minHeights[], Strategy& strategy)
{
    int maxDrops = 0;
    for (int x = 1; x <= WIDTH; x++) {
        if (minHeights[x] >= MAP_HEIGHT)
            continue;
        int maxHeight = height(x);

        int writeAt = minHeights[x];
        m_heights[x] = writeAt - 1;

        DCHECK(color(x, writeAt) == EMPTY) << writeAt << ' ' << color(x, writeAt);
        for (int y = writeAt + 1; y <= maxHeight; ++y) {
            if (color(x, y) == EMPTY)
                continue;

            m_heights[x] = y;
            maxDrops = max(maxDrops, y - writeAt);
            m_field[x][writeAt] = m_field[x][y];
            m_field[x][y] = EMPTY;
            strategy.puyoIsDropped(x, y, writeAt++);
        }
    }

    if (maxDrops == 0)
        return FRAMES_AFTER_NO_DROP;
    else
        return FRAMES_DROP_1_LINE * maxDrops + FRAMES_AFTER_DROP;
}

bool Field::isZenkeshi() const
{
    for (int x = 1; x <= WIDTH; ++x) {
        if (color(x, 1) != EMPTY)
            return false;
    }

    return true;
}

void Field::simulate(BasicRensaInfo& rensaInfo, int additionalChain)
{
    rensaInfo.score = 0;
    rensaInfo.chains = 1 + additionalChain;
    rensaInfo.frames = 0;

    NonTrackingStrategy strategy;
    simulateWithStrategy(rensaInfo, strategy);
}

void Field::simulateAndTrack(BasicRensaInfo& rensaInfo, TrackResult& trackResult)
{
    rensaInfo.score = 0;
    rensaInfo.chains = 1;
    rensaInfo.frames = 0;

    RensaTrackingStrategy strategy(trackResult);
    simulateWithStrategy(rensaInfo, strategy);
}

template<typename Strategy>
void Field::simulateWithStrategy(BasicRensaInfo& rensaInfo, Strategy& strategy)
{
    int minHeights[MAP_WIDTH] = { 100, 1, 1, 1, 1, 1, 1, 100 };

    while (vanish(rensaInfo.chains, &rensaInfo.score, minHeights, strategy)) {
        rensaInfo.frames += drop(minHeights, strategy);
        rensaInfo.frames += FRAMES_AFTER_VANISH;
        rensaInfo.chains += 1;
    }

    rensaInfo.chains -= 1;
}

std::string Field::getDebugOutput() const
{
    std::ostringstream s;
    for (int y = MAP_HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            s << toChar(color(x, y)) << ' ';
        }
        s << std::endl;
    }
    s << ' ';
    for (int x = 1; x <= WIDTH; ++x)
        s << setw(2) << height(x);
    s << std::endl;
    return s.str();
}

bool Field::hasSameField(const Field& field) const
{
    for (int x = 1; x <= WIDTH; ++x) {
        for (int y = 1; y < MAP_HEIGHT; ++y) {
            if (color(x, y) != field.color(x, y))
                return false;
        }
    }

    return true;
}

void Field::findAvailablePlans(int depth, const vector<KumiPuyo>& kumiPuyos, vector<Plan>& plans) const
{
    DCHECK(1 <= depth && depth <= 3);

    plans.clear();
    plans.reserve(22 + 22*22 + 22*22*22);
    findAvailablePlansInternal(NULL, depth, 0, kumiPuyos, plans);
}

void Field::findAvailablePlansInternal(const Plan* previousPlan, int restDepth, int nth, const vector<KumiPuyo>& kumiPuyos, vector<Plan>& plans) const
{
    static const Decision decisions[22] = {
        Decision(2, 3), Decision(3, 3), Decision(3, 1), Decision(4, 1), Decision(5, 1), 
        Decision(1, 2), Decision(2, 2), Decision(3, 2), Decision(4, 2), Decision(5, 2), Decision(6, 2),         
        Decision(1, 1), Decision(2, 1), Decision(4, 3), Decision(5, 3), Decision(6, 3), 
        Decision(1, 0), Decision(2, 0), Decision(3, 0), Decision(4, 0), Decision(5, 0), Decision(6, 0),         
    };

    PuyoColor c1 = kumiPuyos[nth].axis;
    PuyoColor c2 = kumiPuyos[nth].child;
    int num_decisions = (c1 == c2) ? 11 : 22;
    
    for (int i = 0; i < num_decisions; i++) {
        const Decision& decision = decisions[i];
        if (!Ctrl::isReachable(*this, decision))
            continue;

        Field nextField(*this);
        int dropFrames = nextField.framesToDropNext(decision);
        nextField.dropKumiPuyo(decision, kumiPuyos[nth]);

        BasicRensaInfo rensaInfo;
        nextField.simulate(rensaInfo);
        if (nextField.color(3, 12) != EMPTY)
            continue;
        
        // Add a new plan.
        Plan plan = previousPlan ? 
            Plan(*previousPlan, decision, nextField, rensaInfo.score + previousPlan->totalScore(),
                 rensaInfo.chains, previousPlan->totalFrames(), dropFrames + rensaInfo.frames + previousPlan->totalFrames(), rensaInfo.chains > 0) :                 
            Plan(decision, nextField, rensaInfo.score, rensaInfo.chains, 0, dropFrames + rensaInfo.frames, rensaInfo.chains > 0);            
        // If we have fired our rensa, we don't think any more in this time.
        // TODO: We have to think sooner or later.
        if (restDepth > 1 && rensaInfo.chains == 0)
            nextField.findAvailablePlansInternal(&plan, restDepth - 1, nth + 1, kumiPuyos, plans);
        else
            plans.push_back(plan);
    }
}

void Field::findRensas(vector<PossibleRensaInfo>& result, const PuyoSet& puyoSet) const
{
    for (int x = 1; x <= Field::WIDTH; ++x) {
        for (int y = height(x); y >= 1; --y) {
            PuyoColor c = color(x, y);

            DCHECK(c != EMPTY);
            if (c == OJAMA)
                continue;

            // Drop puyo on
            for (int d = -1; d <= 1; ++d) {
                if (x + d <= 0 || Field::WIDTH < x + d)
                    continue;
                if (d == 0) {
                    if (color(x, y + 1) != EMPTY)
                        continue;
                } else {
                    if (color(x + d, y) != EMPTY)
                        continue;
                }

                Field f(*this);
                int necessaryPuyos = 0;
                while (necessaryPuyos <= 4 && f.connectedPuyoNums(x, y) < 4 && f.height(x + d) <= 13) {
                    f.dropPuyoOn(x + d, c);
                    ++necessaryPuyos;
                }

                if (necessaryPuyos > 4)
                    continue;

                PossibleRensaInfo info;
                info.necessaryPuyoSet.add(puyoSet);
                info.necessaryPuyoSet.add(c, necessaryPuyos);
                f.simulate(info.rensaInfo);

                result.push_back(info);
            }            
        }
    }
}

void Field::findPossibleRensas(std::vector<PossibleRensaInfo>& result, int numMaxAddedPuyo) const
{
    findPossibleRensasInternal(result, PuyoSet(), 1, numMaxAddedPuyo);
}

void Field::findPossibleRensasInternal(std::vector<PossibleRensaInfo>& result, PuyoSet addedSet, int leftX, int restAdded) const
{
    findRensas(result, addedSet);

    if (restAdded <= 0)
        return;

    Field f(*this);
    for (int x = leftX; x <= Field::WIDTH; ++x) {
        if (f.height(x) >= 13)
            continue;

        for (int i = 0; i < NUM_NORMAL_PUYO_COLORS; ++i) {
            PuyoColor c = normalPuyoColorOf(i);

            f.dropPuyoOn(x, c);
            addedSet.add(c, 1);

            if (f.connectedPuyoNums(x, f.height(x)) < 4)
                f.findPossibleRensasInternal(result, addedSet, x, restAdded - 1);

            f.removeTopPuyoFrom(x);
            addedSet.add(c, -1);
        }
    }
}

void Field::findFeasibleRensas(vector<FeasibleRensaInfo>& result, int numKumiPuyo, const vector<KumiPuyo>& kumiPuyos) const
{
    vector<Plan> plans;
    plans.reserve(22 + 22*22 + 22*22*22);
    findAvailablePlans(numKumiPuyo, kumiPuyos, plans);

    for (vector<Plan>::iterator it = plans.begin(); it != plans.end(); ++it) {
        if (!it->isRensaPlan())
            continue;

        result.push_back(FeasibleRensaInfo(BasicRensaInfo(it->totalChains(),
                                                          it->totalScore(),
                                                          it->totalFrames() - it->initiatingFrames()),
                                           it->initiatingFrames()));
    }
}