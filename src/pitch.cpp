#include "pitch.h"

#include <utility>
#include <stdexcept>
#include <algorithm>
#include <cctype>

namespace luteconv
{

Pitch::Pitch(char step, int alter, int octave)
: m_step{step}, m_alter{alter}, m_octave{octave}
{
    
}

Pitch::Pitch(int midi)
{
    const int semitones = midi - 12; // C0 = 0
    
    // 12 notes in an octave.  Ignore enharmonics, prefer B flat over A sharp.
    static const std::pair<char, int> octaveNotes[] =
        {{'C', 0}, {'C', 1}, {'D', 0}, {'D', 1},
         {'E', 0}, {'F', 0}, {'F', 1},
         {'G', 0}, {'G', 1}, {'A', 0},
         {'B', -1}, {'B', 0}
        };
    
    m_octave = semitones / 12;
    m_step = octaveNotes[semitones % 12].first;
    m_alter = octaveNotes[semitones % 12].second;
}

bool Pitch::operator ==(const Pitch & rhs) const
{
    return m_step == rhs.m_step &&
            m_alter == rhs.m_alter &&
            m_octave == rhs.m_octave;
}

int Pitch::Midi() const
{
    // Distance in semitones from the octave's starting C to the given step
    //                                A   B  C  D  E  F  G
    static const int octaveStart[] = {9, 11, 0, 2, 4, 5, 7};
    const int semitones = m_octave * 12 + octaveStart[m_step - 'A'] + m_alter; // semitones from C0
    return semitones + 12; // MIDI note C4 = 60
}

std::string Pitch::ToString() const
{
    std::string result;
    
    // step
    result += m_step;
    
    // alter
    if (m_alter > 0)
        result += std::string(m_alter, '#');
    if (m_alter < 0)
        result += std::string(-(m_alter), 'b');
    
    // octave
    result += std::to_string(m_octave);
    return result;
}

bool Pitch::operator !=(const Pitch & rhs) const
{
    return !(operator ==(rhs));
}

Pitch& Pitch::operator +=(int rhs)
{
    *this = *this + rhs;
    return *this;
}

Pitch Pitch::operator +(int rhs) const
{
    return Pitch(Midi() + rhs);
}

int Pitch::operator -(const Pitch& rhs) const
{
    return Midi() - rhs.Midi();
}

void Pitch::SetTuning(int courses, std::vector<Pitch>& tuning)
{
    if (courses == 8)
    {
        SetTuning("G4 D4 A3 F3 C3 G2 F2 D2", tuning);
    }
    else if (courses < 11)
    {
        SetTuning("G4 D4 A3 F3 C3 G2 F2 Eb2 D2 C2", tuning);
    }
    else
    {
        SetTuning("F4 D4 A3 F3 D3 A2 G2 F2 E2 D2 C2 B1 A1", tuning);
    }
}

void Pitch::SetTuning(const char * s, std::vector<Pitch>& tuning)
{
    tuning.clear();
    
    Pitch p;
    while (*s)
    {
        if (*s == ' ')
        {
            ++s;
            continue;
        }
        
        if (*s < 'A' || *s > 'G')
            throw std::runtime_error(std::string("Error: SPN note syntax"));
        p.m_step = *s++;
        
        p.m_alter = 0;
        while (*s == 'b')
        {
            --p.m_alter;
            ++s;
        }
        while (*s == '#')
        {
            ++p.m_alter;
            ++s;
        }
        
        if (*s < '0' || *s > '9')
            throw std::runtime_error(std::string("Error: SPN octave syntax"));
        
        p.m_octave = *s - '0';
        ++s;
        
        tuning.push_back(p);
    }
}

std::string Pitch::GetTuning(const std::vector<Pitch>& tuning)
{
    std::string result;

    for (const auto& pitch : tuning)
        result += pitch.ToString();
    
    return result;
}

void Pitch::SetTuningTab(const char * s, std::vector<Pitch>& tuning)
{
    tuning.clear();
    
    Pitch p;
    while (*s)
    {
        //  Tab allows both upper and lower case
        const char c = std::toupper(*s);
        if (c < 'A' || c > 'G')
            throw std::runtime_error(std::string("Error: Tab note syntax"));
        p.m_step = c;
        ++s;
        
        p.m_alter = 0;
        while (*s == '-')
        {
            --p.m_alter;
            ++s;
        }
        while (*s == '+')
        {
            ++p.m_alter;
            ++s;
        }
        
        if (*s < '0' || *s > '9')
            throw std::runtime_error(std::string("Error: Tab octave syntax"));
        
        // Tab octaves decrease with increasing pitch, and run A..G
        p.m_octave = 6 - (*s - '0');
        if (p.m_step == 'A' || p.m_step == 'B')
            --p.m_octave;
        
        ++s;
        
        // Tab lowest to highest
        tuning.insert(tuning.begin(), p);
    }
}

std::string Pitch::GetTuningTab(const std::vector<Pitch>& tuning)
{
    std::string result;

    for (std::vector<Pitch>::const_reverse_iterator p = tuning.rbegin(); 
            p != tuning.rend(); ++p )
    {
        // step
        result += std::tolower(p->m_step);
        
        // alter
        if (p->m_alter > 0)
            result += std::string(p->m_alter, '+');
        if (p->m_alter < 0)
            result += std::string(-(p->m_alter), '-');
        
        // octave
        // Tab octaves decrease with increasing pitch, and run A..G
        int octave = 6 - p->m_octave;
        if (p->m_step == 'A' || p->m_step == 'B')
                --octave;
        result += std::to_string(octave);
    }
    
    return result;
}
    
} // namespace luteconv

