#include "genmei.h"

#include <string>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "mei.h"

namespace luteconv
{

void GenMei::Generate(const Options& options, const Piece& piece)
{
    std::fstream dst;
    dst.open(options.m_dstFilename.c_str(), std::fstream::out | std::fstream::trunc);
    if (!dst.is_open())
        throw std::runtime_error(std::string("Error: Can't open ") + options.m_dstFilename);
    
    Generate(options, piece, dst);
}

void GenMei::Generate(const Options& options, const Piece& piece, std::ostream& dst)
{
    XMLWriter xmlwriter;
    
    // <mei xmlns='http://www.music-encoding.org/ns/mei' meiversion='3.0.0'>
    XMLElement* xmlmei = new XMLElement("mei");
    xmlmei->AddAttrib("xmlns", "http://www.music-encoding.org/ns/mei");
    // TODO what meiversion?
    xmlmei->AddAttrib("meiversion", "3.0.0");
    xmlwriter.SetRoot(xmlmei);
    
    XMLElement* xmlmeiHead = new XMLElement("meiHead");
    xmlwriter.Root()->Add(xmlmeiHead);
    
    XMLElement* xmlfileDesc = new XMLElement("fileDesc");
    xmlmeiHead->Add(xmlfileDesc);
    FileDesc(xmlfileDesc, piece);
    
    XMLElement* xmlencodingDesc = new XMLElement("encodingDesc");
    xmlmeiHead->Add(xmlencodingDesc);
    EncodingDesc(xmlencodingDesc, options);
    
    XMLElement* xmlworkDesc = new XMLElement("workDesc");
    xmlmeiHead->Add(xmlworkDesc);
    XMLElement* xmlwork = new XMLElement("work");
    xmlworkDesc->Add(xmlwork);
    Work(xmlwork, piece);
    
    XMLElement* xmlmusic = new XMLElement("music");
    xmlwriter.Root()->Add(xmlmusic);
    
    XMLElement* xmlBody = new XMLElement("body");
    xmlmusic->Add(xmlBody);
    Body(xmlBody, options, piece);
    
    dst << xmlwriter;
}

void GenMei::FileDesc(XMLElement* xmlfileDesc, const Piece& piece)
{
    // titleStmt and pubStmt are mandatory
    XMLElement* xmltitleStmt = new XMLElement("titleStmt");
    xmlfileDesc->Add(xmltitleStmt);
    if (!piece.m_title.empty())
        xmltitleStmt->Add(new XMLElement("title", piece.m_title.c_str()));

    XMLElement* xmlpubStmt = new XMLElement("pubStmt");
    xmlfileDesc->Add(xmlpubStmt);
}
   
void GenMei::EncodingDesc(XMLElement* xmlEncodingDesc, const Options& options)
{
    XMLElement* xmlappInfo = new XMLElement("appInfo");
    xmlEncodingDesc->Add(xmlappInfo);
    
    XMLElement* xmlapplication = new XMLElement("application");
    xmlappInfo->Add(xmlapplication);
    
    std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct std::tm * ptm = std::gmtime(&tt);
    std::ostringstream ss;
    ss << std::put_time(ptm,"%F");
    
    xmlapplication->AddAttrib("isodate", ss.str().c_str());
    xmlapplication->AddAttrib("version", options.m_version.c_str());
    xmlapplication->Add(new XMLElement("name", "luteconv"));
}

void GenMei::Work(XMLElement* xmlwork, const Piece& piece)
{
    // title and composer
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


    XMLElement* xmltitleStmt = new XMLElement("titleStmt");
    xmlwork->Add(xmltitleStmt);

    xmltitleStmt->Add(new XMLElement("title", title.c_str()));
    
    XMLElement* xmlcomposer = new XMLElement("composer");
    xmltitleStmt->Add(xmlcomposer);
    
    XMLElement* xmlname = new XMLElement("name");
    xmlcomposer->Add(xmlname);
    
    xmlname->Add(new XMLElement("persName", composer.c_str()));
    
    // tuning
    XMLElement* xmlperfMedium = new XMLElement("perfMedium");
    xmlwork->Add(xmlperfMedium);
    
    XMLElement* xmlperfResList = new XMLElement("perfResList");
    xmlperfMedium->Add(xmlperfResList);
    
    XMLElement* xmlperfRes = new XMLElement("perfRes");
    xmlperfResList->Add(xmlperfRes);
    xmlperfRes->AddAttrib("label", "lute");
    xmlperfRes->AddAttrib("solo", "true");
    
    XMLElement* xmlinstrConfig = new XMLElement("instrConfig");
    xmlperfRes->Add(xmlinstrConfig);
    
    XMLElement* xmlcourseTuning = new XMLElement("courseTuning");
    xmlinstrConfig->Add(xmlcourseTuning);

    // <course n='1' pname='g' oct='4'>
    int course{1};
    for (const auto & tuning : piece.m_tuning)
    {
        XMLElement* xmlcourse = new XMLElement("course");
        xmlcourseTuning->Add(xmlcourse);
        xmlcourse->AddAttrib("n", course);
        const char pname[] = {static_cast<char>(tolower(tuning.m_step)), '\0'};
        xmlcourse->AddAttrib("pname", pname);
        xmlcourse->AddAttrib("oct", tuning.m_octave);
        if (tuning.m_alter == 1)
            xmlcourse->AddAttrib("accid", "s");
        else if (tuning.m_alter == -1)
            xmlcourse->AddAttrib("accid", "f");
        ++course;
    }
}
   
void GenMei::Body(XMLElement* xmlbody, const Options& options, const Piece& piece)
{
    XMLElement* xmlmdiv = new XMLElement("mdiv");
    xmlbody->Add(xmlmdiv);
    xmlmdiv->AddAttrib("n", 1);
    
    XMLElement* xmlscore = new XMLElement("score");
    xmlmdiv->Add(xmlscore);
    
    XMLElement* xmlscoreDef = new XMLElement("scoreDef");
    xmlscore->Add(xmlscoreDef);
    
    XMLElement* xmlstaffGrp = new XMLElement("staffGrp");
    xmlscoreDef->Add(xmlstaffGrp);
    
    XMLElement* xmlstaffDef = new XMLElement("staffDef");
    xmlstaffGrp->Add(xmlstaffDef);
    xmlstaffDef->AddAttrib("n", 1);
    xmlstaffDef->AddAttrib("lines", options.m_dstTabType == TabGerman ? 0 : 6);
    xmlstaffDef->AddAttrib("notationtype", Mei::notationtype[options.m_dstTabType]);
    
    // <tuning tuning.standard='lute.renaissance.6'/>
    std::ostringstream ss;
    ss << "lute."
       << (piece.m_tuning.size() >= 11 ? "baroque" : "renaissance")
       << "."
       << piece.m_tuning.size();
    XMLElement* xmltuning = new XMLElement("tuning");
    xmlstaffDef->Add(xmltuning);
    xmltuning->AddAttrib("tuning.standard", ss.str().c_str());
    
    // Time signature from first bar
    // TODO How do we record change of time signature?
    if (!piece.m_bars.empty() && piece.m_bars.front().m_timeSig.m_timeSymbol != TimeSyNone)
    {
        XMLElement* xmlmensur = new XMLElement("mensur");
        xmlstaffDef->Add(xmlmensur);
        AddTimeSignature(xmlmensur, piece.m_bars.front().m_timeSig);
    }
    
    // section
    XMLElement* xmlsection = new XMLElement("section");
    xmlscore->Add(xmlsection);
    xmlsection->AddAttrib("n", 1);
    Section(xmlsection, options, piece);
}
    
void GenMei::Section(XMLElement* xmlsection, const Options& options, const Piece& piece)
{
    for (const auto & bar : piece.m_bars)
    {
        const int barNo = std::distance(&piece.m_bars.front(), &bar) + 1;
        XMLElement* xmlmeasure = new XMLElement("measure");
        xmlsection->Add(xmlmeasure);
        xmlmeasure->AddAttrib("n", barNo);
        
        XMLElement* xmlstaff = new XMLElement("staff");
        xmlmeasure->Add(xmlstaff);
        xmlstaff->AddAttrib("n", 1);
        
        XMLElement* xmllayer = new XMLElement("layer");
        xmlstaff->Add(xmllayer);
        xmllayer->AddAttrib("n", 1);
        
        for (auto & chord : bar.m_chords)
        {
            XMLElement* xmltabGrp = new XMLElement("tabGrp");
            xmllayer->Add(xmltabGrp);
            
            // mei has quater note = 1 flag
            NoteType adjusted{static_cast<NoteType>(chord.m_noteType - 1 + options.m_flags)};
            adjusted = std::max(NoteTypeWhole, adjusted);
            adjusted = std::min(NoteType256th, adjusted);
            const int durGes = (1 << (adjusted - NoteTypeWhole));
            xmltabGrp->AddAttrib("dur.ges", durGes);
            
            if (chord.m_dotted)
                xmltabGrp->AddAttrib("dots", 1);
            
            XMLElement* xmltabRhythm = new XMLElement("tabRhythm");
            xmltabGrp->Add(xmltabRhythm);
            
            for (const auto & note : chord.m_notes)
            {
                XMLElement* xmlnote = new XMLElement("note");
                xmltabGrp->Add(xmlnote);
                
                xmlnote->AddAttrib("tab.course", note.m_string);
                xmlnote->AddAttrib("tab.fret", note.m_fret);
                
                // fingering
                // <fing playingHand='right' playingFinger='1' startid='m3.n6'/>
                if (note.m_leftFingering != FingerNone || note.m_rightFingering != FingerNone)
                {
                    const std::string id = MakeId();
                    xmlnote->AddAttrib("xml:id", id.c_str());
                    
                    if (note.m_leftFingering != FingerNone)
                    {
                        XMLElement* xmlfing = new XMLElement("fing");
                        xmlmeasure->Add(xmlfing);
                        xmlfing->AddAttrib("playingHand", "left");
                        xmlfing->AddAttrib("playingFinger", Mei::fingering[note.m_leftFingering]);
                        xmlfing->AddAttrib("startid", ("#" + id).c_str());
                    }
                    if (note.m_rightFingering != FingerNone)
                    {
                        XMLElement* xmlfing = new XMLElement("fing");
                        xmlmeasure->Add(xmlfing);
                        xmlfing->AddAttrib("playingHand", "right");
                        xmlfing->AddAttrib("playingFinger", Mei::fingering[note.m_rightFingering]);
                        xmlfing->AddAttrib("startid", ("#" + id).c_str());
                    }
                }
            }
        }
    }
}

std::string GenMei::MakeId()
{
    return "id" + std::to_string(m_nextId++); 
}

void GenMei::AddTimeSignature(XMLElement* xmlmensur, const TimeSig& timeSig)
{
    switch (timeSig.m_timeSymbol)
    {
    case TimeSyCommon:
        xmlmensur->AddAttrib("sign", "C");
        break;
    case TimeSyCut:
        xmlmensur->AddAttrib("sign", "C");
        xmlmensur->AddAttrib("slash", 1);
        break;
    case TimeSySingleNumber:
        xmlmensur->AddAttrib("num", timeSig.m_beats);
        break;
    case TimeSyNormal:
        xmlmensur->AddAttrib("num", timeSig.m_beats);
        xmlmensur->AddAttrib("numBase", timeSig.m_beatType);
        break;
    default:
        break;
    }
}

} // namespace luteconv
