#include "genmxl.h"

#include <sstream>

#include <zip.h>

#include "genmusicxml.h"

namespace luteconv
{

void GenMxl::Generate(const Options& options, const Piece& piece)
{
    // The archive's contents must exist until after the archive is closed
    std::string mimetype;
    std::string musicxmlFilename;
    std::string container;
    std::string musicxml;

    // generate MusicXML image
    {
        std::ostringstream ss;
        GenMusicXml genMusicXml;
        genMusicXml.Generate(options, piece, ss);
        musicxml = ss.str();
    }
    
    // create zip archive
    
    int errorp{0};
    zip_t* zipper = zip_open(options.m_dstFilename.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &errorp);
    if (zipper == nullptr)
    {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, errorp);
        throw std::runtime_error("Error: failed to open output file " + options.m_dstFilename + ": " + zip_error_strerror(&ziperror));
    }

    try
    {
        // file: mimetype
        mimetype = "application/vnd.recordare.musicxml";
        zip_source_t* mimetypeSource = zip_source_buffer(zipper, mimetype.c_str(), mimetype.size(), 0);
        if (mimetypeSource == nullptr)
            throw std::runtime_error(std::string("Error: failed to add mimetype source: ") + zip_strerror(zipper));
        
        if (zip_file_add(zipper, "mimetype", mimetypeSource, ZIP_FL_ENC_UTF_8) < 0)
        {
            zip_source_free(mimetypeSource);
            throw std::runtime_error(std::string("Error: failed to add file mimetype: ") + zip_strerror(zipper));
        }
        
        // file: META-INF/container.xml
        if (zip_dir_add(zipper, "META-INF", ZIP_FL_ENC_UTF_8) < 0)
            throw std::runtime_error(std::string("Error: failed to add directory META-INF: ") + zip_strerror(zipper));
        
        // construct inner filename from archive name bar/foo.mxl -> foo.xml
        musicxmlFilename = options.m_dstFilename;
        const size_t slash = musicxmlFilename.find_last_of("/");
        if (slash != std::string::npos)
            musicxmlFilename = musicxmlFilename.substr(slash + 1);

        const size_t dot = musicxmlFilename.find_last_of(".");
        if (dot != std::string::npos)
            musicxmlFilename = musicxmlFilename.substr(0, dot);

        musicxmlFilename = musicxmlFilename + ".xml";
            
        container =
                R"(<?xml version="1.0" encoding="UTF-8"?>)" "\n"
                R"(<container>)" "\n"
                R"(  <rootfiles>)" "\n"
                R"(    <rootfile full-path=")" + musicxmlFilename + R"(")" "\n"
                R"(              media-type="application/vnd.recordare.musicxml+xml"/>)" "\n"
                R"(  </rootfiles>)" "\n"
                R"(</container>)" "\n";
        
        zip_source_t* containerSource = zip_source_buffer(zipper, container.c_str(), container.size(), 0);
        if (containerSource == nullptr)
            throw std::runtime_error(std::string("Error: failed to add container.xml source: ") + zip_strerror(zipper));

        if (zip_file_add(zipper, "META-INF/container.xml", containerSource, ZIP_FL_ENC_UTF_8) < 0)
        {
            zip_source_free(containerSource);
            throw std::runtime_error(std::string("Error: failed to add file META-INF/container.xml: ") + zip_strerror(zipper));
        }
        
        // MusicXML
        zip_source_t* musicXmlSource = zip_source_buffer(zipper, musicxml.c_str(), musicxml.size(), 0);
        if (musicXmlSource == nullptr)
            throw std::runtime_error(std::string("Error: failed to add ") + musicxmlFilename + " source: " + zip_strerror(zipper));

        if (zip_file_add(zipper, musicxmlFilename.c_str(), musicXmlSource, ZIP_FL_ENC_UTF_8) < 0)
        {
            zip_source_free(musicXmlSource);
            throw std::runtime_error(std::string("Error: failed to add file ") + musicxmlFilename + ": " + zip_strerror(zipper));
        }
    }
    catch (...)
    {
        zip_close(zipper);
        throw;
    }
    
    zip_close(zipper);
}

} // namespace luteconv
