#include "genmusicxml.h"

#include <string>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>

#include "musicxml.h"

namespace luteconv
{

const char doctype[] = R"(<!DOCTYPE score-partwise PUBLIC "-//Recordare//DTD MusicXML 3.1 Partwise//EN")" "\n"
                       R"(            "http://www.musicxml.org/dtds/partwise.dtd">)";

void GenMusicXml::Generate(const Options& options, const Piece& piece)
{
    std::fstream dst;
    dst.open(options.m_dstFilename.c_str(), std::fstream::out | std::fstream::trunc);
    if (!dst.is_open())
        throw std::runtime_error(std::string("Error: Can't open ") + options.m_dstFilename);
    
    Generate(options, piece, dst);
}

void GenMusicXml::Generate(const Options& options, const Piece& piece, std::ostream& dst)
{
    std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct std::tm * ptm = std::gmtime(&tt);

    XMLWriter xmlwriter;
    xmlwriter.AddDoctype(doctype);
    xmlwriter.SetRoot(new XMLElement("score-partwise"));
    
    const std::string title = !piece.m_title.empty()
                              ? piece.m_title
                              : !piece.m_credits.empty()
                              ? piece.m_credits[0].m_left
                              : "";
    const std::string composer = !piece.m_composer.empty()
                                 ? piece.m_composer
                                 : (!piece.m_credits.empty() && !piece.m_credits[0].m_left.empty())
                                 ? piece.m_credits[0].m_right
                                 : "anonymous";

    // work
    XMLElement* work = new XMLElement("work");
    work->Add(new XMLElement("work-title", title.c_str()));
    xmlwriter.Root()->Add(work);
    
    // identification
    XMLElement* identification = new XMLElement("identification");
    
    XMLElement* creator = new XMLElement("creator", composer.c_str());
    identification->Add(creator);
    creator->AddAttrib("type", "Composer");
    
    // copyright
    if (!piece.m_copyright.empty() && piece.m_copyrightEnabled)
    {
        identification->Add(new XMLElement("rights", piece.m_copyright.c_str()));
    }
    
    XMLElement* encoding = new XMLElement("encoding");
    encoding->Add(new XMLElement("software", ("luteconv " + options.m_version).c_str()));
    
    // encoding-date
    {
        std::ostringstream ss;
        ss << std::put_time(ptm,"%F");
        encoding->Add(new XMLElement("encoding-date", ss.str().c_str()));
    }
    identification->Add(encoding);
    xmlwriter.Root()->Add(identification);
    
    // credit
    if (!piece.m_credits.empty())
    {
        for (const auto & credit : piece.m_credits)
        {
            XMLElement* xmlcredit = new XMLElement("credit");
            xmlcredit->AddAttrib("page", 1);

            switch (credit.m_align)
            {
            case AlignLeft:
            {
                XMLElement* creditWords = new XMLElement("credit-words", credit.m_left.c_str());
                creditWords->AddAttrib("halign", "left");
                xmlcredit->Add(creditWords);

                break;
            }
            case AlignRight:
            {
                XMLElement* creditWords = new XMLElement("credit-words", credit.m_right.c_str());
                creditWords->AddAttrib("halign", "right");
                xmlcredit->Add(creditWords);
                break;
            }
            case AlignCenter:
            {
                XMLElement* creditWords = new XMLElement("credit-words", credit.m_left.c_str());
                creditWords->AddAttrib("halign", "center");
                xmlcredit->Add(creditWords);
                break;
            }
            case AlignLeftRight:
            {
                XMLElement* creditWords = new XMLElement("credit-words", credit.m_left.c_str());
                creditWords->AddAttrib("halign", "left");
                xmlcredit->Add(creditWords);

                creditWords = new XMLElement("credit-words", credit.m_right.c_str());
                creditWords->AddAttrib("halign", "right");
                xmlcredit->Add(creditWords);
                break;
            }
            default:
                break;
            }
            xmlwriter.Root()->Add(xmlcredit);
        }
    }
    
    // part-list
    XMLElement* partList = new XMLElement("part-list");
    
    XMLElement* scorePart = new XMLElement("score-part");
    scorePart->AddAttrib("id", "P1");
    scorePart->Add(new XMLElement("part-name", "Lute"));
    scorePart->Add(new XMLElement("part-abbreviation", "Lute"));
    partList->Add(scorePart);
    xmlwriter.Root()->Add(partList);
    
    // part
    XMLElement* part = new XMLElement("part");

    part->AddAttrib("id", "P1");
        
    const int count = piece.m_bars.size();
    bool repForward{false};
    for (int i = 0; i < count; i++)
    {
        part->Add(Measure(piece, piece.m_bars[i], i + 1, repForward, options));
    }
    xmlwriter.Root()->Add(part);
    
    dst << xmlwriter;
}

int GenMusicXml::Duration(NoteType noteType)
{
    // Durations are chosen to be divisable by 3 and 2 for triplets and dotted notes
    return (1 << (NoteType256th - noteType)) * 3;
}

XMLElement* GenMusicXml::Measure(const Piece& piece, const Bar& bar, int n, bool& repForward, const Options& options)
{
    XMLElement* measure = new XMLElement("measure");
    measure->AddAttrib("number", n);
    
    //  forward repeat carried over from previous bar
    if (repForward)
    {
        XMLElement* barline = new XMLElement("barline");
        barline->AddAttrib("location", "left");
        barline->Add(new XMLElement("bar-style", MusicXml::barStyle[BarStyleHeavyLight]));
        XMLElement* repeat = new XMLElement("repeat");
        repeat->AddAttrib("direction", "forward");
        barline->Add(repeat);
        measure->Add(barline);
        repForward = false;
    }
    
    if (n == 1)
    {
        measure->Add(FirstMeasureAttributes(piece, bar, options));
    }
    else if (bar.m_timeSymbol != TimeSyNone)
    {
        XMLElement* attributes = new XMLElement("attributes");
        AddTimeSignature(attributes, bar);
        measure->Add(attributes);
    }
    
    for (const auto & luteChord : bar.m_chords)
    {
        // TODO Musecore has crotchet == 0 flags
        const NoteType noteType2 = static_cast<NoteType>(luteChord.m_noteType + 0);
        const int duration = luteChord.m_dotted ? (3 * Duration(noteType2)) / 2 : Duration(noteType2);

        if (luteChord.m_notes.empty())
        {
            XMLElement* note = new XMLElement("note");
            
            note->Add(new XMLElement("rest"));
            note->Add(new XMLElement("duration", duration));
            note->Add(new XMLElement("type", MusicXml::noteType[luteChord.m_noteType]));
            
            if (luteChord.m_dotted)
                note->Add(new XMLElement("dot"));
            
            measure->Add(note);
        }
        else
        {
            bool fermata{luteChord.m_fermata};
            bool firstNote{true};
            for (const auto & luteNote : luteChord.m_notes)
            {
                const Pitch pitch{piece.m_tuning[luteNote.m_string - 1] + luteNote.m_fret};
                const char step[] = {pitch.m_step, '\0'};
                // TODO adjust for triplets
                // TODO rests
                // TODO ornaments
                
                XMLElement* note = new XMLElement("note");
                
                if (firstNote)
                {
                    firstNote = false;
                }
                else
                {
                    note->Add(new XMLElement("chord"));
                }
                
                XMLElement* xmlpitch = new XMLElement("pitch");
                xmlpitch->Add(new XMLElement("step", step));
                if (pitch.m_alter != 0)
                    xmlpitch->Add(new XMLElement("alter", pitch.m_alter));
                xmlpitch->Add(new XMLElement("octave", pitch.m_octave));
                note->Add(xmlpitch);
                
                note->Add(new XMLElement("duration", duration));
                note->Add(new XMLElement("type", MusicXml::noteType[luteChord.m_noteType]));

                if (luteChord.m_dotted)
                    note->Add(new XMLElement("dot"));
    
                XMLElement* notations = new XMLElement("notations");

                if (fermata)
                {
                    notations->Add(new XMLElement("fermata"));
                    fermata = false; // only on first note of chord
                }

                XMLElement* technical = new XMLElement("technical");
                technical->Add(new XMLElement("string", luteNote.m_string));
                technical->Add(new XMLElement("fret", luteNote.m_fret));
          
                if (luteNote.m_leftFingering != FingerNone)
                    technical->Add(new XMLElement("fingering", luteNote.m_leftFingering));
                
                if (luteNote.m_rightFingering != FingerNone)
                    technical->Add(new XMLElement("pluck", MusicXml::pluck[luteNote.m_rightFingering]));
                
                notations->Add(technical);
                note->Add(notations);
                measure->Add(note);
            }
        }
    }
    
    if (bar.m_barStyle != BarStyleRegular || bar.m_repeat != RepNone)
    {
        // forward repeat carried to next bar
        repForward = (bar.m_repeat == RepForward || bar.m_repeat == RepJanus);
        const bool repBackward = (bar.m_repeat == RepBackward || bar.m_repeat == RepJanus);

        const char * const direction = repBackward ? "backward" : nullptr;
        
        const BarStyle style = repBackward ? BarStyleLightHeavy : bar.m_barStyle;
        
        XMLElement* barline = new XMLElement("barline");

        barline->AddAttrib("location", "right");
        barline->Add(new XMLElement("bar-style", MusicXml::barStyle[style]));

        if (bar.m_fermata)
            barline->Add(new XMLElement("fermata"));

        if (direction)
        {
            XMLElement* repeat = new XMLElement("repeat");
            repeat->AddAttrib("direction", direction);
            barline->Add(repeat);
        }
        
        measure->Add(barline);
    }
    
    return measure;
}

XMLElement* GenMusicXml::FirstMeasureAttributes(const Piece& piece, const Bar& bar, const Options& options)
{
    XMLElement* attributes = new XMLElement("attributes");
    
    // divisions
    attributes->Add(new XMLElement("divisions", Duration(NoteTypeQuarter)));
    
    // key
    XMLElement* key = new XMLElement("key");
    key->AddAttrib("print-object", "no");
    key->Add(new XMLElement("fifths", 0));
    attributes->Add(key);
    
    // time signature
    AddTimeSignature(attributes, bar);

    // clef
    XMLElement* clef = new XMLElement("clef");
    clef->AddAttrib("print-object", "no");
    clef->Add(new XMLElement("sign", "TAB"));
    clef->Add(new XMLElement("line", 5));
    attributes->Add(clef);
    
    // staff-details
    XMLElement* staffDetails = new XMLElement("staff-details");
    const int staffLines = 6;
    staffDetails->Add(new XMLElement("staff-lines", staffLines));
    if (options.m_dstTabType == TabFrench)
        staffDetails->AddAttrib("show-frets", "letters");
    
    // lines are numbered 1 at bottom of stave to 6 at the top.
    // French tablature: courses are numbered 1 at the top of the stave to 6 the bottom,
    // then contine 0, -1, -2 ... below the stave.
    // Italian tablature: courses are numbered 1 at the bottom of the stave to 6 the top,
    // then contine 7, 8, 9 ... above the stave.
    const int numCourses = piece.m_tuning.size();
    for (int course = 1; course <= numCourses; ++course)
    {
        const int line = (options.m_dstTabType == TabItalian) ? course : staffLines - (course - 1);
        
        XMLElement* staffTuning = new XMLElement("staff-tuning");
        staffTuning->AddAttrib("line", line);
        
        const char step[] = {piece.m_tuning[course - 1].m_step, '\0'};
        staffTuning->Add(new XMLElement("tuning-step", step));
        if (piece.m_tuning[course - 1].m_alter != 0)
            staffTuning->Add(new XMLElement("tuning-alter", piece.m_tuning[course - 1].m_alter));
        staffTuning->Add(new XMLElement("tuning-octave", piece.m_tuning[course - 1].m_octave));
        staffDetails->Add(staffTuning);
    }
    
    attributes->Add(staffDetails);
    
    return attributes;
}

void GenMusicXml::AddTimeSignature(XMLElement* attributes, const Bar& bar)
{
    if (bar.m_timeSymbol == TimeSyNone)
        return;
    
    XMLElement* time = new XMLElement("time");
    
    if (bar.m_timeSymbol != TimeSyNormal)
    {
        time->AddAttrib("symbol", MusicXml::timeSymbol[bar.m_timeSymbol]);
    }
    time->Add(new XMLElement("beats", bar.m_beats));
    time->Add(new XMLElement("beat-type", bar.m_beatType));
    
    attributes->Add(time);
}


} // namespace luteconv
