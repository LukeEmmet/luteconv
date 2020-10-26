#include <gtest/gtest.h>
#include <converter.h>

#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <regex>

class LuteConvFixture: public ::testing::Test
{
public:
    void SetUp() override
    {
#define XSTRINGIFY(s) STRINGIFY(s)
#define STRINGIFY(s) #s
        m_binaryDir = XSTRINGIFY(BINARY_DIR);
        m_sourceDir = XSTRINGIFY(SOURCE_DIR);
#undef STRINGIFY
#undef XSTRINGIFY
    
        std::cout << "BIN_DIRECTORY=" << m_binaryDir << std::endl;
        std::cout << "SOURCE_DIRECTORY=" << m_sourceDir << std::endl;
    }
    
    void ConvertOneTest(const std::string& originalDir, const std::string& dstDir,
            const std::string& convertedDir, const std::string& filename);

    void Diff(const std::string& lhs, const std::string& rhs);
    
    std::string m_binaryDir;
    std::string m_sourceDir;
};

TEST_F(LuteConvFixture, FixtureTest)
{

}

TEST_F(LuteConvFixture, ConvertTest)
{
    const std::string originalDir = m_sourceDir + "/examples/original";
    const std::string convertedDir = m_sourceDir + "/examples/converted";
    const std::string dstDir = m_binaryDir + "/converter_test";
    
    mkdir(dstDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    
    struct dirent* entry{nullptr};
    DIR* dir = opendir(originalDir.c_str());
    EXPECT_NE(nullptr, dir);
    
    while ((entry = readdir(dir)) != nullptr)
    {
        if (entry->d_type == DT_REG)
            ConvertOneTest(originalDir, dstDir, convertedDir, entry->d_name);
    }
    closedir(dir);
}

void LuteConvFixture::ConvertOneTest(const std::string& originalDir, const std::string& dstDir,
        const std::string& convertedDir, const std::string& filename)
{
    using namespace luteconv;
    
    for (auto filetype : {".tab", ".musicxml", ".tc"})
    {
        Options options;
        options.m_srcFilename = originalDir + "/" + filename;
        options.m_dstFilename = dstDir + "/" + filename + filetype;
        options.SetFormatFilename();
        
        // convert original into dstDir
        Converter converter;
        EXPECT_NO_THROW(converter.Convert(options));
        
        // compare convert with expected
        Diff(convertedDir + "/" + filename + filetype, options.m_dstFilename);
    }
}

void LuteConvFixture::Diff(const std::string& lhs, const std::string& rhs)
{
    std::fstream lhsStream(lhs.c_str(), std::fstream::in);
    std::fstream rhsStream(rhs.c_str(), std::fstream::in);
    
    std::string lhsLine;
    std::string rhsLine;
    
    while (!lhsStream.eof() && !rhsStream.eof())
    {
        getline(lhsStream, lhsLine);
        getline(rhsStream, rhsLine);
        
        if (lhsLine != rhsLine)
        {
            // allow dates and our version number to differ
            const std::regex re{R"((2\d\d\d|luteconv))"};
            std::cmatch results;
            if (!std::regex_search(lhsLine.c_str(), results, re))
            {
                std::cout << "Compare " << lhs << " with " << rhs << std::endl;
                EXPECT_EQ(lhsLine, rhsLine);
                break;
            }
        }
    }
    
    EXPECT_EQ(lhsStream.eof(), rhsStream.eof());
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
