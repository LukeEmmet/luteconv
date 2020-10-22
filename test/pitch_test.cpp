#include <gtest/gtest.h>
#include <pitch.h>
#include <cstring>
#include <iostream>
#include <sstream>

class LuteConvFixture: public ::testing::Test
{
};

TEST_F(LuteConvFixture, FixtureTest)
{
    
}

TEST_F(LuteConvFixture, Midi)
{
    EXPECT_EQ(12, luteconv::Pitch('C', 0, 0).Midi());
    EXPECT_EQ(59, luteconv::Pitch('C', -1, 4).Midi());
    EXPECT_EQ(60, luteconv::Pitch('C', 0, 4).Midi());
    EXPECT_EQ(61, luteconv::Pitch('C', 1, 4).Midi());
    
    EXPECT_EQ(luteconv::Pitch(12), luteconv::Pitch('C', 0, 0));
    EXPECT_EQ(luteconv::Pitch(59), luteconv::Pitch('B', 0, 3));
    EXPECT_EQ(luteconv::Pitch(60), luteconv::Pitch('C', 0, 4));
    EXPECT_EQ(luteconv::Pitch(61), luteconv::Pitch('C', 1, 4));
}

TEST_F(LuteConvFixture, AddSemitones)
{
    luteconv::Pitch p('A', 0, 4);
    
    EXPECT_EQ(luteconv::Pitch('A', 0, 4), p + 0);
    EXPECT_EQ(luteconv::Pitch('B', -1, 4), p + 1);
    EXPECT_EQ(luteconv::Pitch('B', 0, 4), p + 2);
    EXPECT_EQ(luteconv::Pitch('C', 0, 5), p + 3);
    EXPECT_EQ(luteconv::Pitch('C', 1, 5), p + 4);
    EXPECT_EQ(luteconv::Pitch('D', 0, 5), p + 5);
    EXPECT_EQ(luteconv::Pitch('D', 1, 5), p + 6);
    EXPECT_EQ(luteconv::Pitch('E', 0, 5), p + 7);
    EXPECT_EQ(luteconv::Pitch('F', 0, 5), p + 8);
    EXPECT_EQ(luteconv::Pitch('F', 1, 5), p + 9);
    EXPECT_EQ(luteconv::Pitch('G', 0, 5), p + 10);
    EXPECT_EQ(luteconv::Pitch('G', 1, 5), p + 11);
    EXPECT_EQ(luteconv::Pitch('A', 0, 5), p + 12);
    EXPECT_EQ(luteconv::Pitch('B', -1, 5), p + 13);
}

TEST_F(LuteConvFixture, SetTuningByCourses)
{
    std::vector<luteconv::Pitch> tuning;
    luteconv::Pitch::SetTuning(6, tuning);
    EXPECT_EQ(luteconv::Pitch('G', 0, 4), tuning[0]);
    EXPECT_EQ(luteconv::Pitch('D', 0, 4), tuning[1]);
    EXPECT_EQ(luteconv::Pitch('A', 0, 3), tuning[2]);
    EXPECT_EQ(luteconv::Pitch('F', 0, 3), tuning[3]);
    EXPECT_EQ(luteconv::Pitch('C', 0, 3), tuning[4]);
    EXPECT_EQ(luteconv::Pitch('G', 0, 2), tuning[5]);
}

TEST_F(LuteConvFixture, SetTuningByString)
{
    std::vector<luteconv::Pitch> tuning;
    luteconv::Pitch::SetTuning("G4 D4 A3 F3 C3 G2", tuning);
    EXPECT_EQ(luteconv::Pitch('G', 0, 4), tuning[0]);
    EXPECT_EQ(luteconv::Pitch('D', 0, 4), tuning[1]);
    EXPECT_EQ(luteconv::Pitch('A', 0, 3), tuning[2]);
    EXPECT_EQ(luteconv::Pitch('F', 0, 3), tuning[3]);
    EXPECT_EQ(luteconv::Pitch('C', 0, 3), tuning[4]);
    EXPECT_EQ(luteconv::Pitch('G', 0, 2), tuning[5]);
}

TEST_F(LuteConvFixture, SetTuningTab)
{
    std::vector<luteconv::Pitch> tuning;
    luteconv::Pitch::SetTuningTab("G4c3f3a2d2g2", tuning);
    EXPECT_EQ(luteconv::Pitch('G', 0, 4), tuning[0]);
    EXPECT_EQ(luteconv::Pitch('D', 0, 4), tuning[1]);
    EXPECT_EQ(luteconv::Pitch('A', 0, 3), tuning[2]);
    EXPECT_EQ(luteconv::Pitch('F', 0, 3), tuning[3]);
    EXPECT_EQ(luteconv::Pitch('C', 0, 3), tuning[4]);
    EXPECT_EQ(luteconv::Pitch('G', 0, 2), tuning[5]);
}

TEST_F(LuteConvFixture, GetTuning)
{
    std::vector<luteconv::Pitch> tuning;
    luteconv::Pitch::SetTuning("G4 D4 A3 F3 C3 G2", tuning);
    EXPECT_EQ("G4D4A3F3C3G2", luteconv::Pitch::GetTuning(tuning));
}

TEST_F(LuteConvFixture, GetTuningTab)
{
    std::vector<luteconv::Pitch> tuning;
    luteconv::Pitch::SetTuning("G4 D4 A3 F3 C3 G2", tuning);
    EXPECT_EQ("g4c3f3a2d2g2", luteconv::Pitch::GetTuningTab(tuning));
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
