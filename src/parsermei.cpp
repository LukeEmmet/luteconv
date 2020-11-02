#include "parsermei.h"

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdexcept>

#include "mei.h"
#include "pitch.h"
#include "logger.h"

namespace luteconv
{

using namespace pugi;

void ParserMei::Parse(const std::string& filename, void* contents, size_t size, const Options& options, Piece& piece)
{
    xml_document doc;
    xml_parse_result result = doc.load_buffer_inplace(contents, size);
    Parse(filename, doc, result, options, piece);
}

void ParserMei::Parse(const Options& options, Piece& piece)
{
    xml_document doc;
    xml_parse_result result = doc.load_file(options.m_srcFilename.c_str());
    Parse(options.m_srcFilename, doc, result, options, piece);
}
  
void ParserMei::Parse(const std::string& filename, xml_document& doc, xml_parse_result& result, const Options& options, Piece& piece)
{
    if (!result)
    {
        std::ostringstream ss;
        ss << "Error: XML [" << filename << "] parsed with errors."
                    << " Description: " << result.description() 
                    << " Offset: " << result.offset;
        throw std::runtime_error(ss.str());
    }
    
    xml_node xmlmei = doc.child("mei");
    if (!xmlmei)
        throw std::runtime_error("Error: Can't find <mei>");

    xml_node xmlwork = xmlmei.child("meiHead").child("workDesc").child("work");
    xml_node xmltileStmt = xmlwork.child("titleStmt");
    piece.m_title = xmltileStmt.child("title").child_value();
    piece.m_composer = xmltileStmt.child("composer").child("name").child("persName").child_value();
    
    LOGGER << "title=" << piece.m_title;
    LOGGER << "composer=" << piece.m_composer;
    
    xml_node xmlscore = xmlmei.child("music").child("body").child("mdiv").child("score");
    if (!xmlscore)
        throw std::runtime_error("Error: Can't find <xmlscore>");
    
    TimeSig timeSig;
    xml_node xmlmensur = xmlscore.child("scoreDef").child("staffGrp").child("staffDef").child("mensur");
    if (xmlmensur)
        timeSig = ParseTimeSignature(xmlmensur);
    
    xml_node xmlcourseTuning = xmlwork.child("perfMedium").child("perfResList").child("perfRes").child("instrConfig").child("courseTuning");
    if (xmlcourseTuning)
        ParseCourseTuning(xmlcourseTuning, piece);
    
    xml_node xmlsection = xmlscore.child("section");
    if (!xmlsection)
        throw std::runtime_error("Error: Can't find <section>");

    bool firstBar{true};
    for (xml_node xmlmeasure = xmlsection.child("measure"); xmlmeasure; xmlmeasure = xmlmeasure.next_sibling("measure"))
    {
        const int measureNo{xmlmeasure.attribute("n").as_int()};
        piece.m_bars.push_back(Bar());
        Bar& bar = piece.m_bars.back();
        
        if (firstBar && timeSig.m_timeSymbol != TimeSyNone)
            bar.m_timeSig = timeSig;

        xml_node xmllayer = xmlmeasure.child("staff").child("layer");
        if (!xmllayer)
        {
            LOGGER << "measure " << measureNo << ": can't find <layer>";
            continue;
        }
        
        ParseTabGrpList(xmlmeasure, xmllayer, GridNone, bar);
        firstBar = false;
    }
    piece.SetTuning(options);
}

void ParserMei::ParseTabGrpList(xml_node& xmlmeasure, xml_node& xmlparent, Grid grid, Bar& bar)
{
    for (xml_node xmlchild = xmlparent.first_child(); xmlchild; xmlchild = xmlchild.next_sibling())
    {
        const std::string childName{xmlchild.name()};
        if (childName == "tabGrp")
        {
            ParseTabGrp(xmlmeasure, xmlchild, grid, bar);
            if (grid == GridStart)
                grid = GridMid;
        }
        else if (childName == "beam")
        {
            ParseTabGrpList(xmlmeasure, xmlchild, GridStart, bar);
            if (!bar.m_chords.empty())
                bar.m_chords.back().m_grid = GridEnd;
        }
        else if (childName == "choice" || childName == "corr")
        {
            ParseTabGrpList(xmlmeasure, xmlchild, grid, bar);
        }
        else if (childName == "sic")
        {
            // ignore original uncorrected version
        }
        else
        {
            LOGGER << "ParseTabGrpList element " << childName << " ignored";
        }
    }
}

void ParserMei::ParseTabGrp(xml_node& xmlmeasure, xml_node& xmltabGrp, Grid grid, Bar& bar)
{
    bar.m_chords.push_back(Chord());
    Chord& chord = bar.m_chords.back();
    
    // dur.ges is 1 whole, 2 half, 4 quarter, 8 eighth ... 
    // But quater note is 1 flag, reduce to 0
    int dur = xmltabGrp.attribute("dur.ges").as_int();
    int noteType{NoteTypeWhole};
    while (dur > 0)
    {
        ++noteType;
        dur >>= 1;
    }
    
    chord.m_noteType = static_cast<NoteType>(noteType);
    chord.m_dotted = xmltabGrp.attribute("dots").as_int() == 1;
    chord.m_grid = grid;
    
    ParseNoteList(xmlmeasure, xmltabGrp, chord);
}

void ParserMei::ParseNoteList(xml_node& xmlmeasure, xml_node& xmlparent, Chord& chord)
{
    for (xml_node xmlchild = xmlparent.first_child(); xmlchild; xmlchild = xmlchild.next_sibling())
    {
        const std::string childName{xmlchild.name()};
        if (childName == "note")
        {
            chord.m_notes.push_back(Note());
            Note& note = chord.m_notes.back();
            
            // string & fret
            note.m_string = xmlchild.attribute("tab.course").as_int();
            note.m_fret = xmlchild.attribute("tab.fret").as_int();
            const std::string xmlid{xmlchild.attribute("xml:id").value()};
            
            if (!xmlid.empty())
                ParseFingering(xmlmeasure, xmlid, note);
        }
        else if (childName == "choice" || childName == "corr")
        {
            ParseNoteList(xmlmeasure, xmlchild, chord);
        }
        else if (childName == "sic")
        {
            // ignore original uncorrected version
        }
        else if (childName == "tabRhythm")
        {
            // TODO tabRhythm
        }
        else
        {
            LOGGER << "ParseNoteList element " << childName << " ignored";
        }
    }
}

void ParserMei::ParseFingering(xml_node& xmlmeasure, const std::string& xmlid, Note& note)
{
    // TODO fingering may apply to a range of ids
    // fingering <fing playingHand='right' playingFinger='1' startid='m3.n8'/>
    // Why is <fing> not a child of <note>?
    for (xml_node xmlchild = xmlmeasure.first_child(); xmlchild; xmlchild = xmlchild.next_sibling())
    {
        const std::string childName{xmlchild.name()};
        if (childName == "fing")
        {
            // FIXME startid should contain #id not id but example da_crema-1546_10-no_6.mei doesn't have #
            if (xmlchild.attribute("startid").value() == xmlid || xmlchild.attribute("startid").value() == ("#" + xmlid))
            {
                const std::string playingHand{xmlchild.attribute("playingHand").value()};
                const std::string playingFinger{xmlchild.attribute("playingFinger").value()};
    
                if (!playingFinger.empty())
                {
                    for (int i = 1; Mei::fingering[i]; ++i)
                    {
                        if (playingFinger == Mei::fingering[i])
                        {
                            if (playingHand == "left")
                            {
                                note.m_leftFingering = static_cast<Fingering>(i);
                            }
                            else if (playingHand == "right")
                            {
                                note.m_rightFingering = static_cast<Fingering>(i);
                            }
                            else
                            {
                                LOGGER << "Unknown playingHand: " << playingHand;
                            }
                            break;
                        }
                    }
                }
            }
        }
        else if (childName == "choice" || childName == "corr")
        {
            ParseFingering(xmlchild, xmlid, note);
        }
        else if (childName == "sic")
        {
            // ignore original uncorrected version
        }
    }
}

TimeSig ParserMei::ParseTimeSignature(xml_node& xmlmensur)
{
    TimeSig timeSig;
                  
    if (xmlmensur)
    {
        if (xmlmensur.attribute("sign").value() == std::string("C"))
        {
            timeSig.m_timeSymbol = (xmlmensur.attribute("slash").as_int() == 1) ? TimeSyCut : TimeSyCommon;
        }
        else
        {
            const int beats = xmlmensur.attribute("num").as_int();
            const int beatType = xmlmensur.attribute("numbase").as_int();
            if (beats > 0 && beatType == 0)
            {
                timeSig.m_beats = beats;
                timeSig.m_beatType = 4;
                timeSig.m_timeSymbol = TimeSySingleNumber;
            }
            else if (beats > 0 && beatType > 0)
            {
                timeSig.m_beats = beats;
                timeSig.m_beatType = beatType;
                timeSig.m_timeSymbol = TimeSyNormal;
            }
        }
    }
    return timeSig;
}

void ParserMei::ParseCourseTuning(xml_node& xmlcourseTuning, Piece& piece)
{
    // tuning
    piece.m_tuning.reserve(20);
    for (xml_node xmlcourse = xmlcourseTuning.child("course"); xmlcourse; xmlcourse = xmlcourse.next_sibling("course"))
    {
        const int course{xmlcourse.attribute("n").as_int()};
        const std::string step{xmlcourse.attribute("pname").value()};
        const int octave{xmlcourse.attribute("oct").as_int()};
        const std::string accid{xmlcourse.attribute("accid").value()};
        
        int alter = 0;
        if (accid.size() == 1)
            alter = (accid[0] == 's')
                    ? 1
                    : (accid[0] == 'f')
                    ? - 1
                    : 0;
        
        if (step.size() == 1 && course >= 1 && octave >= 0)
        {
            if (piece.m_tuning.size() < static_cast<size_t>(course))
                piece.m_tuning.resize(course);
            
            piece.m_tuning[course - 1] = Pitch(toupper(step[0]), alter, octave);
        }
    }
    LOGGER << "tuning=" << Pitch::GetTuning(piece.m_tuning);
}
        
} // namespace luteconv
