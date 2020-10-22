#include "parsermusicxml.h"

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdexcept>

#include "musicxml.h"
#include "pitch.h"

namespace luteconv
{

using namespace pugi;

void ParserMusicXml::Parse(const std::string& filename, void* contents, size_t size, const Options& options, Piece& piece)
{
    xml_document doc;
    xml_parse_result result = doc.load_buffer_inplace(contents, size);
    Parse(filename, doc, result, options, piece);
}

void ParserMusicXml::Parse(const Options& options, Piece& piece)
{
    xml_document doc;
    xml_parse_result result = doc.load_file(options.m_srcFilename.c_str());
    Parse(options.m_srcFilename, doc, result, options, piece);
}
  
void ParserMusicXml::Parse(const std::string& filename, xml_document& doc, xml_parse_result& result, const Options& options, Piece& piece)
{
    if (!result)
    {
        std::ostringstream ss;
        ss << "Error: XML [" << filename << "] parsed with errors."
                    << " Description: " << result.description() 
                    << " Offset: " << result.offset;
        throw std::runtime_error(ss.str());
    }
    
    xml_node xmlscorePartwise = doc.child("score-partwise");
    if (!xmlscorePartwise)
        throw std::runtime_error("Error: Can't find <score-partwise>");

    piece.m_title = xmlscorePartwise.child("work").child("work-title").child_value();
    piece.m_composer = xmlscorePartwise.child("identification").find_child_by_attribute("creator", "type", "Composer").child_value();
    
    piece.m_copyright = xmlscorePartwise.child("identification").child("rights").child_value();
    piece.m_copyrightEnabled = !piece.m_copyright.empty();
    
    ParseCredit(xmlscorePartwise, piece);
    
    xml_node xmlpart = xmlscorePartwise.child("part");
    if (!xmlpart)
        throw std::runtime_error("Error: Can't find <score-partwise><part>");
    
    ParseStaffTuning(xmlpart, piece);

    for (xml_node xmlmeasure = xmlpart.child("measure"); xmlmeasure; xmlmeasure = xmlmeasure.next_sibling("measure"))
    {
        piece.m_bars.push_back(Bar());

        ParseTimeSignature(xmlmeasure, piece);
        ParseBarline(xmlmeasure, piece);
        
        Bar& bar = piece.m_bars.back();
        bool firstNote{true};
        for (xml_node xmlnote = xmlmeasure.child("note"); xmlnote; xmlnote = xmlnote.next_sibling("note"))
        {
            // do we need a new chord?
            if (!xmlnote.child("chord"))
            {
                bar.m_chords.push_back(Chord());
                firstNote = true;
            }
            
            Chord& chord = bar.m_chords.back();
            
            if (firstNote)
            {
                const std::string xmltype = xmlnote.child("type").child_value();
                for (int i = 0; MusicXml::noteType[i]; ++i)
                {
                    if (xmltype == MusicXml::noteType[i])
                    {
                        chord.m_noteType = static_cast<NoteType>(i);
                        break;
                    }
                }
                
                chord.m_dotted = !!xmlnote.child("dot");
                chord.m_fermata = !!xmlnote.child("notations").child("fermata");
                firstNote = false;
            }
            
            xml_node xmltechnical = xmlnote.child("notations").child("technical");
            if (xmltechnical)
            {
                chord.m_notes.push_back(Note());
                Note& note = chord.m_notes.back();
                // string & fret
                note.m_string = xmltechnical.child("string").text().as_int();
                note.m_fret = xmltechnical.child("fret").text().as_int();
                
                // fingering
                xml_node xmlfingering = xmltechnical.child("fingering");
                if (xmlfingering)
                    note.m_leftFingering = static_cast<Fingering>(xmlfingering.text().as_int());
                
                // pluck
                const std::string pluck = xmltechnical.child("pluck").child_value();
                if (!pluck.empty())
                {
                    for (int i = 1; MusicXml::pluck[i]; ++i)
                    {
                        if (pluck == MusicXml::pluck[i])
                        {
                            note.m_rightFingering = static_cast<Fingering>(i);
                            break;
                        }
                    }
                }
            }
        }
    }
    piece.SetTuning(options);
}

void ParserMusicXml::ParseStaffTuning(xml_node& xmlpart, Piece& piece)
{
    // tuning
    xml_node xmlstaffDetails = xmlpart.child("measure").child("attributes").child("staff-details");
    if (xmlstaffDetails)
    {
        // Specifies tuning of the staff - implicitly gives tuning of the lute.
        // Except may only give 6 courses.  Worst still MuseScore import of MusicXML 
        // only imports 6 courses, and its export is broken: always puts lowest
        // course on line 1, so with more than 6 courses puts the highest courses
        // on line 7 ...
        // The following attempts to compensate.
        
        // find maximum and minimum staff lines.
        int maxLine{1};
        int minLine{1};
        for (xml_node xmlstaffTuning = xmlstaffDetails.child("staff-tuning"); xmlstaffTuning;
                xmlstaffTuning = xmlstaffTuning.next_sibling("staff-tuning"))
        {
           const int line = xmlstaffTuning.attribute("line").as_int();
           maxLine = std::max(maxLine, line);
           minLine = std::min(minLine, line);
        }
    
        const int numCourses = maxLine - minLine + 1;
        piece.m_tuning.resize(numCourses);
        for (xml_node xmlstaffTuning = xmlstaffDetails.child("staff-tuning"); xmlstaffTuning;
                xmlstaffTuning = xmlstaffTuning.next_sibling("staff-tuning"))
        {
            const int line = xmlstaffTuning.attribute("line").as_int();
            Pitch& pitch = piece.m_tuning[maxLine - line];
            pitch.m_step = (xmlstaffTuning.child("tuning-step").child_value())[0];
            pitch.m_alter = xmlstaffTuning.child("tuning-alter").text().as_int();
            pitch.m_octave = xmlstaffTuning.child("tuning-octave").text().as_int();
        }
        
        // if source is in italian tablature then will need to reverse piece.m_tuning.
        // Beware of re-entrant tuning!  Look at all pairs to get most common ordering.
        int direction{0};
        for (size_t i = 0; i < piece.m_tuning.size() - 1; ++i)
        {
            if (piece.m_tuning[i].Midi() >= piece.m_tuning[i + 1].Midi())
                ++direction;
            else
                --direction;
        }
        if (direction < 0)
        {
            // italian tablature, reverse order
            std::reverse(piece.m_tuning.begin(), piece.m_tuning.end());
        }
    }
    else
    {
        throw std::runtime_error("Can't find staff-tuning, perhaps no tablature in this MusicXML file");
    }
}

void ParserMusicXml::ParseBarline(xml_node& xmlmeasure, Piece& piece)
{
    Bar& bar = piece.m_bars.back();
    
    xml_node xmlleftBarline = xmlmeasure.find_child_by_attribute("barline", "location", "left");
    if (xmlleftBarline)
    {
        if (xmlleftBarline.find_child_by_attribute("repeat", "direction", "forward"))
        {
            // update previous bar
            const int ourIndex = std::distance(piece.m_bars.data(), &bar);
            if (ourIndex > 0)
            {
                Bar& prev = piece.m_bars[ourIndex - 1];
                if (prev.m_repeat == RepNone)
                    prev.m_repeat = RepForward;
                else if (prev.m_repeat == RepBackward)
                    prev.m_repeat = RepJanus;
            }
        }
    }
    
    xml_node xmlrightBarline = xmlmeasure.find_child_by_attribute("barline", "location", "right");
    if (xmlrightBarline)
    {
        const std::string barStyle = xmlrightBarline.child("bar-style").child_value();
        if (!barStyle.empty())
        {
            for (int i = 0; MusicXml::barStyle[i]; ++i)
            {
                if (barStyle == MusicXml::barStyle[i])
                {
                    bar.m_barStyle = static_cast<BarStyle>(i);
                    break;
                }
            }
        }
        
        bar.m_fermata = !!xmlrightBarline.child("fermata");
        
        if (xmlrightBarline.find_child_by_attribute("repeat", "direction", "backward"))
            bar.m_repeat = RepBackward;
    }
}

void ParserMusicXml::ParseTimeSignature(xml_node& xmlmeasure, Piece& piece)
{
    Bar& bar = piece.m_bars.back();
    xml_node xmltime = xmlmeasure.child("attributes").child("time");
    if (xmltime)
    {
        const std::string timeSymbol = xmltime.attribute("symbol").value();
        if (!timeSymbol.empty())
        {
            for (int i = 1; MusicXml::timeSymbol[i]; ++i)
            {
                if (timeSymbol == MusicXml::timeSymbol[i])
                {
                    bar.m_timeSymbol = static_cast<TimeSymbol>(i);
                    break;
                }
            }
        }
        bar.m_beats = xmltime.child("beats").text().as_int();
        bar.m_beatType = xmltime.child("beat-type").text().as_int();
    }
}

void ParserMusicXml::ParseCredit(xml_node& xmlscorePartwise, Piece& piece)
{
    for (xml_node xmlcredit = xmlscorePartwise.child("credit"); xmlcredit; xmlcredit = xmlcredit.next_sibling("credit"))
    {
        piece.m_credits.push_back(Credit());
        Credit & credit = piece.m_credits.back();
        for (xml_node xmlcreditWords = xmlcredit.child("credit-words"); xmlcreditWords; xmlcreditWords = xmlcreditWords.next_sibling("credit-words"))
        {
            const std::string halign = xmlcreditWords.attribute("halign").value();
            
            if (halign == "right")
            {
                credit.m_right = xmlcreditWords.child_value();
                credit.m_align = AlignRight;
            }
            else
            {
                credit.m_left = xmlcreditWords.child_value();
            }
            
            if (halign == "center")
                credit.m_align = AlignCenter;
        }
        
        if (!credit.m_left.empty() && !credit.m_right.empty())
            credit.m_align = AlignLeftRight;
    }
}


} // namespace luteconv
