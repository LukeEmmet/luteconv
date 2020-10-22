#include <gtest/gtest.h>
#include <pitch.h>
#include <options.h>

class LuteConvFixture: public ::testing::Test
{
};

TEST_F(LuteConvFixture, FixtureTest)
{
    
}

TEST_F(LuteConvFixture, ProcessArgs)
{
    using namespace luteconv;
    
    const char* argv[] = {"luteconv",
            "-o", "dest",
            "-D", "spanish",
            "-S", "italian",
            "-s", "jtz",
            "-d", "mxl",
            "-7", "D2",
            "-i", "5",
            "src",
            nullptr};
    
    Options options;
    options.ProcessArgs(16, const_cast<char**>(argv));
    
    EXPECT_EQ("dest", options.m_dstFilename);
    EXPECT_EQ(TabSpanish, options.m_dstTabType);
    EXPECT_EQ(TabItalian, options.m_srcTabType);
    EXPECT_EQ(FormatJtz, options.m_srcFormat);
    EXPECT_EQ(FormatMxl, options.m_dstFormat);
    EXPECT_EQ(1, options.m_7tuning.size());
    EXPECT_EQ(Pitch('D', 0, 2), options.m_7tuning[0]);
    EXPECT_EQ("5", options.m_index);
    EXPECT_EQ("src", options.m_srcFilename);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
