#ifndef _PITCH_H_
#define _PITCH_H_

#include <string>
#include <vector>

namespace luteconv
{

/**
 * Scientific Pitch Notation
 */
class Pitch
{
public:
    
    /**
     * Constructor
     * 
     * Middle C is C4
     * 
     * @param[in] step
     * @param[in] alter
     * @param[in] octave
     */
    Pitch(char step, int alter, int octave);
    
    /**
     * Constructor
     * 
     * Middle C is 60
     * 
     * @param[in] midi note
     */
    Pitch(int midi);
        
    /**
     * Get MIDI note. Middle C is 60
     * 
     * @return MIDI note
     */
    int Midi() const;
    
    Pitch() = default;
    ~Pitch() = default;
    
    /**
     * String representation, e.g. C#3
     */
    std::string ToString() const;
    
    /**
     * Equality.  Note that enharmonics are not equal.
     * If enharmonic equality is required then use lhs.Midi() == rhs.Midi()
     * 
     * @param[in] rhs
     * @return true <=> equal
     */
    bool operator ==(const Pitch & rhs) const;
    
    /**
     * Inequality.  Note that enharmonics are not equal.
     * If enharmonic equality is required then use lhs.Midi() != rhs.Midi()
     * 
     * @param[in] rhs
     * @return true <=> not equal
     */
    bool operator !=(const Pitch & rhs) const;
    
    /**
     * Add semitones to a pitch
     * 
     * @param[in] rhs semitones to add, may be negative
     * @return *this
     */
    Pitch& operator +=(int rhs);
    
    /**
     * Add semitones to a pitch.
     * 
     * @param[in] rhs semitones to add, may be negative
     * @return pitch
     */
    Pitch operator +(int rhs) const;

    /**
     * Difference in semitones between two pitches
     * 
     * @param rhs
     * @return difference in semitones
     */
    int operator -(const Pitch & rhs) const;

    /**
     * Set the lute tuning
     * 
     * @param[in] courses number of courses
     * @param[out] tuning
     */
    static void SetTuning(int courses, std::vector<Pitch>& tuning);
    
    /**
     * Set the lute tuning from Scientific Pitch Notation
     * e.g for 10 course lute "G4 D4 A3 F3 C3 G2 F2 Eb2 D2 C2"
     * Spaces between notes are optional.
     * 
     * @param[in] spn scientific pitch notation, first string ... last string
     * @param[out] tuning
     */
    static void SetTuning(const char * s, std::vector<Pitch>& tuning);
    
    /**
     * Get the lute tuning in Scientific Pitch Notation
     * e.g for 10 course lute "G4D4A3F3C3G2F2Eb2D2C2"
     * 
     * @param[in] tuning
     * @return tuning
     */
    static std::string GetTuning(const std::vector<Pitch>& tuning);

    /**
     * Set the lute tuning from Wayne Cripps' Tab program
     * Octaves are numbered in decreasing pitch, octaves run from A..G
     * e.g for 10 course lute "C4D4E4F4G4c3f3a2d2g2"
     * 
     * @param[in] Wayne Cripps Tab, last string ... first string
     * @param[out] tuning
     */
    static void SetTuningTab(const char * s, std::vector<Pitch>& tuning);
    
    /**
     * Get the lute tuning from Wayne Cripps' Tab program
     * Octaves are numbered in decreasing pitch, octaves run from A..G
     * e.g for 10 course lute "C4D4E4F4G4c3f3a2d2g2"
     * 
     * @param[in] tuning
     * @return Wayne Cripps Tab, last string ... first string
     */
    static std::string GetTuningTab(const std::vector<Pitch>& tuning);
    
    char m_step{'\0'};
    int m_alter{0};
    int m_octave{0};
};

} // namespace luteconv

#endif // _PITCH_H_