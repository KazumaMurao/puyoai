#include "field.h"

#include <gtest/gtest.h>
#include <string>

#include "../../core/constant.h"
#include "decision.h"
#include "field_column_bit_field.h"
#include "rensainfo.h" 

using namespace std;

void testUrl(string url, int expected_chains, int expected_score)
{
    Field f(url);
    BasicRensaInfo rensaInfo;
    f.simulate(rensaInfo);
    EXPECT_EQ(expected_chains, rensaInfo.chains);
    EXPECT_EQ(expected_score, rensaInfo.score);
}

TEST(FieldTest, ChainAndScoreTest1)
{
    testUrl("http://www.inosendo.com/puyo/rensim/??444400", 1, 40);
}

TEST(FieldTest, ChainAndScoreTest2)
{
    testUrl("http://www.inosendo.com/puyo/rensim/??5000005565444455", 2, 700);
}

TEST(FieldTest, ChainAndScoreTest3)
{
    testUrl("http://www.inosendo.com/puyo/rensim/??50745574464446676456474656476657564547564747676466766747674757644657575475755", 19, 175080);
    testUrl("http://www.inosendo.com/puyo/rensim/??500467767675744454754657447767667644674545455767477644457474656455446775455646", 19, 175080);
    testUrl("http://www.inosendo.com/puyo/rensim/??550050455045451045745074745074645067674067674056567056567515167444416555155", 2, 38540);
    testUrl("http://www.inosendo.com/puyo/rensim/??50550040455075451075745064745064645067674057674747574776567675156644415555155", 3, 43260);
    testUrl("http://www.inosendo.com/puyo/rensim/??550040455075451775745464745464645467674457674147574776567675156644415555155", 4, 50140);
    testUrl("http://www.inosendo.com/puyo/rensim/??745550576455666451175745564745564745567674157674747574776566615156644415555155", 5, 68700);
    testUrl("http://www.inosendo.com/puyo/rensim/??444411114141414114114111414144411114414111114414411114441114111141444141111141", 4, 4840);
    testUrl("http://www.inosendo.com/puyo/rensim/??545544544454454545454545454545545454445544554455454545545454554544445455455445", 9, 49950);
    testUrl("http://www.inosendo.com/puyo/rensim/??444446544611446164564441546166565615454551441444111111111111111111111111111111", 9, 32760);
    testUrl("http://www.inosendo.com/puyo/rensim/??667547466555661677471475451447461666661547457477556446776555744646476466744555", 18, 155980);
    testUrl("http://www.inosendo.com/puyo/rensim/??444044144414114144411411414144141414414141441414114411441144414141141414144144", 11, 47080);
    testUrl("http://www.inosendo.com/puyo/rensim/??444444444444444444444444444444444444444444444444444444444444444444444444", 1, 7200);
}

TEST(FieldTest, FramesTest) {
    BasicRensaInfo info;
    {
        // 1 Rensa, no drop.
        Field f("444400");
        f.simulate(info);
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_AFTER_NO_DROP, info.frames);
    }
    {
        Field f("500000"
                "444400");
        f.simulate(info);
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE + FRAMES_AFTER_DROP, info.frames);
    }
    {
        Field f("500000"
                "400000"
                "444000");
        f.simulate(info);
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP,
                  info.frames);
    }
    {
        Field f("500000"
                "450000"
                "444000");
        f.simulate(info);
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP,
                  info.frames);
    }
    {
        Field f("500000"
                "455000"
                "444500");
        f.simulate(info);
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP +
                  FRAMES_AFTER_VANISH + FRAMES_AFTER_NO_DROP,
                  info.frames);
    }
    {
        Field f("560000"
                "455000"
                "444500");
        f.simulate(info);
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP +
                  FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE + FRAMES_AFTER_DROP,
                  info.frames);
    }
}

TEST(FieldTest, ConnectedPuyoNumsTest)
{
    Field f("004455"
            "045465");

    EXPECT_EQ(f.connectedPuyoNums(3, 2), 3);
    EXPECT_EQ(f.connectedPuyoNums(5, 2), 3);
    EXPECT_EQ(f.connectedPuyoNums(5, 1), 1);
}

TEST(FieldTest, Height)
{
    Field f("004100"
            "004040"
            "004707"
            "014040");

    EXPECT_EQ(f.height(1), 0);
    EXPECT_EQ(f.height(2), 1);
    EXPECT_EQ(f.height(3), 4);
    EXPECT_EQ(f.height(4), 4);
    EXPECT_EQ(f.height(5), 3);
    EXPECT_EQ(f.height(6), 2);
}

TEST(FieldTest, HeightShouldBeCopied)
{
    Field f("004100"
            "004040"
            "004707"
            "014040");

    Field g(f);

    EXPECT_EQ(g.height(1), 0);
    EXPECT_EQ(g.height(2), 1);
    EXPECT_EQ(g.height(3), 4);
    EXPECT_EQ(g.height(4), 4);
    EXPECT_EQ(g.height(5), 3);
    EXPECT_EQ(g.height(6), 2);
}

TEST(FieldTest, HeightAfterSimulate)
{
    Field f("050005"
            "050055"
            "445644"
            "445644");

    BasicRensaInfo info;
    f.simulate(info);

    EXPECT_EQ(f.height(1), 0);
    EXPECT_EQ(f.height(2), 0);
    EXPECT_EQ(f.height(3), 0);
    EXPECT_EQ(f.height(4), 2);
    EXPECT_EQ(f.height(5), 3);
    EXPECT_EQ(f.height(6), 4);
}

TEST(FieldTest, DropPuyoOn)
{
    Field f("050005"
            "050055"
            "445644"
            "445644");

    f.dropPuyoOn(1, RED);

    EXPECT_EQ(f.color(1, 3), RED);
    EXPECT_EQ(f.height(1), 3);
}

TEST(FieldTest, RemoveTopPuyoFrom)
{
    Field f("456756");

    f.removeTopPuyoFrom(1);
    EXPECT_EQ(f.color(1, 1), EMPTY);
    EXPECT_EQ(f.height(1), 0);

    f.removeTopPuyoFrom(1);
    EXPECT_EQ(f.color(1, 1), EMPTY);
    EXPECT_EQ(f.height(1), 0);
}

TEST(FieldTest, CountPuyos)
{
    Field f("050015"
            "050055"
            "445644"
            "445644");

    EXPECT_EQ(f.countPuyos(), 18);
    EXPECT_EQ(f.countColorPuyos(), 17);
}

TEST(FieldTest, TrackedFieldSimulation)
{
    Field f("400040"
            "456474"
            "445667"
            "556774");

    
    BasicRensaInfo rensaInfo;
    TrackResult trackResult;
    f.simulateAndTrack(rensaInfo, trackResult);

    EXPECT_EQ(rensaInfo.chains, 5);
    EXPECT_EQ(trackResult.erasedAt(1, 2), 1);
    EXPECT_EQ(trackResult.erasedAt(1, 1), 2);
    EXPECT_EQ(trackResult.erasedAt(3, 3), 3);
    EXPECT_EQ(trackResult.erasedAt(5, 3), 4);
    EXPECT_EQ(trackResult.erasedAt(5, 4), 5);
}

TEST(FieldTest, DropKumiPuyoExtreme)
{
    Field f1("http://www.inosendo.com/puyo/rensim/??110111550477450455745466754655664576755776666564777644555455666545775454444554");
    Field f2("http://www.inosendo.com/puyo/rensim/??110111550477451455745466754655664576755776666564777644555455666545775454444554");

    f1.dropKumiPuyoSafely(Decision(3, 0), KumiPuyo(RED, RED));
    f1.dropKumiPuyoSafely(Decision(3, 0), KumiPuyo(RED, RED));

    f2.dropKumiPuyoSafely(Decision(3, 0), KumiPuyo(RED, RED));
    f2.dropKumiPuyoSafely(Decision(3, 0), KumiPuyo(RED, RED));
}

TEST(FieldTest, FramesToDropNextWithoutChigiri)
{
    // TODO(mayah): We have to confirm this.
    Field f;

    EXPECT_EQ(f.framesToDropNext(Decision(3, 0)), Field::HEIGHT * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI);
    EXPECT_EQ(f.framesToDropNext(Decision(3, 1)), Field::HEIGHT * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI);
    EXPECT_EQ(f.framesToDropNext(Decision(3, 2)), (Field::HEIGHT - 1) * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI);
    EXPECT_EQ(f.framesToDropNext(Decision(3, 3)), Field::HEIGHT * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI);

    EXPECT_EQ(f.framesToDropNext(Decision(1, 0)), Field::HEIGHT * FRAMES_DROP_1_LINE + FRAMES_HORIZONTAL_MOVE * 2 + FRAMES_AFTER_NO_CHIGIRI);
}

TEST(FieldTest, FramesToDropNextWithChigiri)
{
    Field f("004000"
            "005000"
            "006000"
            "007000");

    EXPECT_EQ(f.framesToDropNext(Decision(3, 1)), (Field::HEIGHT - 4) * FRAMES_DROP_1_LINE + FRAMES_AFTER_CHIGIRI + FRAMES_CHIGIRI_1_LINE_1 + FRAMES_CHIGIRI_1_LINE_2 + 2 * FRAMES_CHIGIRI_1_LINE_3);    
}

struct ContainsRensa {
    ContainsRensa(int chains, PuyoSet set) : chains(chains), set(set) {}
    
    bool operator()(const PossibleRensaInfo& info) const {
        return chains == info.rensaInfo.chains && info.necessaryPuyoSet == set;
    }
    
    int chains;
    PuyoSet set;
};

TEST(FieldTest, FindRensaTest)
{
    Field f("054400"
            "045400"
            "556660");

    vector<PossibleRensaInfo> result;
    f.findRensas(result, PuyoSet());

    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsRensa(1, PuyoSet(1, 0, 0, 0))));
    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsRensa(2, PuyoSet(3, 0, 0, 0))));
    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsRensa(3, PuyoSet(0, 0, 1, 0))));
}

TEST(FieldTest, FindPossibleRensas)
{
    Field f("450000"
            "445000"
            "556000");

    vector<PossibleRensaInfo> result;
    f.findPossibleRensas(result, 3);

    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsRensa(1, PuyoSet(0, 2, 0, 0))));
    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsRensa(2, PuyoSet(1, 0, 0, 0))));
    EXPECT_TRUE(std::count_if(result.begin(), result.end(), ContainsRensa(3, PuyoSet(1, 0, 3, 0))));
}
