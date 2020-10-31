#include "piece.h"

#include <algorithm>
#include <stdexcept>

namespace luteconv
{

void Piece::SetTuning(const Options& options)
{
    // count the numCourses
    int numCourses{0};
    for (const auto & bar : m_bars)
    {
        for (const auto & chord : bar.m_chords)
        {
            for (const auto & note : chord.m_notes)
            {
                numCourses = std::max(numCourses, note.m_string);
            }
        }
    }

    numCourses = std::max(numCourses, 6);
    
    // tuning in the command line takes precedence
    // then tuning from source file
    // then default tuning based on number of numCourses
    
    std::vector<Pitch> defaultTuning;
    Pitch::SetTuning(numCourses, defaultTuning);
    
    m_tuning.resize(numCourses);
    
    for (int i = 0; i < numCourses; ++i)
    {
        if (i < static_cast<int>(options.m_tuning.size()))
            m_tuning[i] = options.m_tuning[i];
        else if (m_tuning[i].m_step == '\0')
            m_tuning[i] = defaultTuning[i];
    }
    
    // optionally modify tuning of 7th ... course
    for (int i = 6; i < numCourses; ++i)    
    {
        if (i < static_cast<int>(6 + options.m_7tuning.size()))
            m_tuning[i] = options.m_7tuning[i - 6];
    }
}

void Bar::Clear()
{
    m_timeSig = TimeSig();
    m_barStyle = BarStyleRegular;
    m_repeat = RepNone;
    m_fermata = false;
    m_eol = false;
    
    m_chords.clear();
}

} // namespace luteconv
