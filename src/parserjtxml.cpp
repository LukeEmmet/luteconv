#include "parserjtxml.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "pitch.h"
#include "logger.h"

namespace luteconv
{

using namespace pugi;

void ParserJtxml::Parse(const std::string& filename, void* contents, size_t size, const Options& options, Piece& piece)
{
    xml_document doc;
    xml_parse_result result = doc.load_buffer_inplace(contents, size);
    Parse(filename, doc, result, options, piece);
}

void ParserJtxml::Parse(const Options& options, Piece& piece)
{
    xml_document doc;
    xml_parse_result result = doc.load_file(options.m_srcFilename.c_str());
    Parse(options.m_srcFilename, doc, result, options, piece);
}
  
void ParserJtxml::Parse(const std::string& filename, xml_document& doc, xml_parse_result& result, const Options& options, Piece& piece)
{
    if (!result)
    {
        std::ostringstream ss;
        ss << "Error: XML parse error: " << filename
                    << ". Description: " << result.description() 
                    << " Offset: " << result.offset;
        throw std::runtime_error(ss.str());
    }
    
    xml_node xmlsection = doc.child("DjangoTabXML").child("sections").find_child_by_attribute("section", "index", options.m_index.c_str());
    if (!xmlsection)
        throw std::runtime_error("Error: Can't find <DjangoTabXML><sections><section> index=\"" + options.m_index + "\"");
    
    piece.m_title = xmlsection.child("section-texts").find_child_by_attribute("track-text", "descriptor", "section-name").child_value();
    piece.m_composer = xmlsection.child("section-texts").find_child_by_attribute("track-text", "descriptor", "section-author").child_value();
    Credit credit;
    credit.m_left = xmlsection.child("section-texts").find_child_by_attribute("track-text", "descriptor", "section-comments").child_value();
    if (!credit.m_left.empty())
        piece.m_credits.push_back(credit);
        
    LOGGER << "title=" << piece.m_title;
    LOGGER << "composer=" << piece.m_composer;
    
    ParseTuning(xmlsection, piece);
    
    xml_node xmlsystems = xmlsection.child("systems");
    for (xml_node xmlsystem = xmlsystems.child("system"); xmlsystem;
            xmlsystem = xmlsystem.next_sibling("system"))
    {
        xml_node xmlstaff = xmlsystem.child("instruments").child("instrument").child("staff");
        if (!xmlstaff)
        {
            LOGGER << "No xmlstaff";
        }
        
        piece.m_bars.push_back(Bar());
        
        for (xml_node xmlevent = xmlstaff.child("event"); xmlevent;
                xmlevent = xmlevent.next_sibling("event"))
        {
            ParseEvent(xmlevent, piece);
        }
        
        if (piece.m_bars.back().m_chords.empty())
            piece.m_bars.pop_back();
    }
    
    piece.SetTuning(options);
}

void ParserJtxml::ParseTuning(xml_node& xmlsection, Piece& piece)
{
    xml_node xmlstrings = xmlsection.child("instruments").child("instrument").child("tablature-definition").child("tuning").child("strings");
    const int numCourses = xmlstrings.attribute("count").as_int();
    piece.m_tuning.resize(numCourses);
    
    for (xml_node xmlstring = xmlstrings.child("string"); xmlstring;
            xmlstring = xmlstring.next_sibling("string"))
    {
        const int index = xmlstring.attribute("index").as_int();
        const int midiPitch = xmlstring.attribute("midi-pitch").as_int();
        
        // Don't know why midi-pitch is 23 less than midi note
        piece.m_tuning[index] = Pitch(midiPitch + 23);
    }
    LOGGER << "Tuning=" << Pitch::GetTuning(piece.m_tuning);
}

void ParserJtxml::ParseEvent(xml_node& xmlevent, Piece& piece)
{
    Bar& bar = piece.m_bars.back();
    
    const std::string type{xmlevent.attribute("type").value()};
    
    if (type == "chord")
    {
        bar.m_chords.push_back(Chord());
        Chord& chord = bar.m_chords.back();
        
        ParseFlag(xmlevent.attribute("flag").as_int(), chord);
        
        xml_node xmlnotes = xmlevent.child("notes");
        if (!xmlnotes)
        {
            LOGGER << "No notes";
        }
        for (xml_node xmlnote = xmlnotes.child("note"); xmlnote;
                xmlnote = xmlnote.next_sibling("note"))
        {
            chord.m_notes.push_back(Note());
            Note& note = chord.m_notes.back();
            
            note.m_string = xmlnote.attribute("string").as_int() + 1;
            // jtxml doesn't store the fret but rather the pitch as a MIDI note.  Calculate the fret from the tuning.
            note.m_fret = Pitch(xmlnote.attribute("pitch").as_int()) - piece.m_tuning[note.m_string - 1];
            // TODO fingering
            // TODO ornaments
        }
    }
    else if (type == "bar")
    {
        const std::string barType = xmlevent.attribute("bar-type").value();
        if (barType == "repeat")
        {
            const std::string repeatType = xmlevent.attribute("repeat-type").value();
            if (repeatType== "left")
                bar.m_repeat = RepBackward;
            else if (repeatType== "right")
                bar.m_repeat = RepForward;
            if (repeatType== "both")
                bar.m_repeat = RepJanus;
        }
        piece.m_bars.push_back(Bar());
        // TODO bar-style
        // TODO time signature
    }
    else
    {
        LOGGER << "Unprocessed event=" << type;
    }
}

void ParserJtxml::ParseFlag(int flagNum, Chord& chord)
{
    switch(flagNum)
    {
        case 4096:
        case 0:
            chord.m_noteType = NoteTypeWhole;
            break;
        case 4097:
        case 1:
            chord.m_noteType = NoteTypeHalf;
            break;
        case 4098:
        case 2: 
            chord.m_noteType = NoteTypeQuarter;
            break;
        case 4099 :
        case 3: 
            chord.m_noteType = NoteTypeEighth;
            break;
        case 4100:
        case 4:
            chord.m_noteType = NoteType16th;
            break;
        case 4101:
        case 5:
            chord.m_noteType = NoteType32nd;
            break;
        case 4102:
        case 6:
            chord.m_noteType = NoteType64th;
            break;
        case 7:
        case 4103: 
            chord.m_noteType = NoteType128th;
            break;

        case 4104:
        case 8:
            chord.m_noteType = NoteTypeWhole;
            chord.m_dotted = true;
            break;
        case 4105:
        case 9:
            chord.m_noteType = NoteTypeHalf;
            chord.m_dotted = true;
            break;
        case 4106:
        case 10:
            chord.m_noteType = NoteTypeQuarter;
            chord.m_dotted = true;
            break;
        case 4107:
        case 11:
            chord.m_noteType = NoteTypeEighth;
            chord.m_dotted = true;
            break;
        case 4108:
        case 12:
            chord.m_noteType = NoteType16th;
            chord.m_dotted = true;
            break;
        case 4109:
        case 13:
            chord.m_noteType = NoteType32nd;
            chord.m_dotted = true;
            break;
        case 4110:
        case 14:
            chord.m_noteType = NoteType64th;
            chord.m_dotted = true;
            break;
        case 4111:
        case 15:
            chord.m_noteType = NoteType128th;
            chord.m_dotted = true;
            break;

        default:
            LOGGER << "Unprocessed flag=" << flagNum;
            break;
    }
}


} // namespace luteconv
