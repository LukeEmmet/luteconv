#include "unzipper.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <zip.h>

#include "logger.h"

namespace luteconv
{

void Unzipper::Unzip(const std::string& filename, std::vector<char>& image, std::string& zipFilename)
{
    int err{0};
    zip_file* zipFile{nullptr};
    zip* zipArchive = zip_open(filename.c_str(), 0, &err);
        
    try
    {
        if (zipArchive == nullptr)
        {
            char buf[100];
            zip_error_to_str(buf, sizeof(buf), err, errno);
            
            std::ostringstream ss;
            ss << "Error: Can't open zip archive " << filename << " " << buf;
            throw std::runtime_error(ss.str());
        }
    

        std::vector<std::pair<std::string, zip_uint64_t>> index;
        for (int i = 0; i < zip_get_num_entries(zipArchive, 0); i++)
        {
            struct zip_stat sb;
    
            if (zip_stat_index(zipArchive, i, 0, &sb) == 0)
            {
                LOGGER << "name=" << sb.name << " size=" << sb.size;
                index.emplace_back(sb.name, sb.size);
            }
        }
                 
        // zip files are archives, find our particular file
        size_t entry = 0;
        if (index.size() > 1)
        {
            // look for our candidate
            for (size_t i = 0; i < index.size(); ++i)
            {
                if (index[i].first != "mimetype" && index[i].first != "META-INF/container.xml")
                {
                    entry = i;
                    break;
                }
            }
        }

        zipFilename = index[entry].first;
        zip_file* zipFile = zip_fopen_index(zipArchive, entry, 0);
        if (!zipFile)
        {
            std::ostringstream ss;
            ss << "Error: Can't open zip file " << filename << ":" << entry << ":" << index[entry].first;
            throw std::runtime_error(ss.str());
        }
        
        // decompress and read all the file into memory
        const zip_uint64_t blockInc = 4096;
        zip_uint64_t blockSize = index[0].second + blockInc;

        zip_uint64_t imageSize{0};
        image.resize(static_cast<size_t>(blockSize));
        
        for (;;)
        {
            const zip_int64_t nRead = zip_fread(zipFile, image.data() + imageSize, blockSize);
            if (nRead < 0)
            {
                std::ostringstream ss;
                ss << "Error: Reading " << filename << ":" << entry << ":" << index[entry].first;
                throw std::runtime_error(ss.str());
            }
            
            imageSize += static_cast<unsigned int>(nRead);
            if (nRead < static_cast<int>(blockSize))
                break;
        
            blockSize = blockInc;
            image.resize(static_cast<size_t>(imageSize + blockSize));
        }
        
        image.resize(static_cast<size_t>(imageSize));
    }
    catch (...)
    {
        if (zipFile)
            zip_fclose(zipFile);
        if (zipArchive)
            zip_close(zipArchive);
        throw;
    }
    
    if (zipFile)
        zip_fclose(zipFile);
    if (zipArchive)
        zip_close(zipArchive);
}

} // namespace luteconv
