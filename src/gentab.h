#ifndef _GENTAB_H_
#define _GENTAB_H_

#include <iostream>
#include <string>

#include "piece.h"
#include "options.h"

namespace luteconv
{

/**
 * Generate .tab
 */
class GenTab
{
public:
    /**
     * Constructor
     */
    GenTab() = default;

    /**
     * Destructor
     */
    ~GenTab() = default;
    
    /**
     * Generate .tab from destination file in options
     * 
     * @param[in] options
     * @param[in] piece
     */
    void Generate(const Options& options, const Piece& piece);

    /**
     * Generate .tab
     * 
     * @param[in] options
     * @param[in] piece
     * @param[out] dst destination
     */
    void Generate(const Options& options, const Piece& piece, std::ostream& dst);
    
private:
    static std::string GetTimeSignature(const Bar & bar);
    static std::string GetFlagInfo(const std::vector<Chord> & chords, const Chord & our);
    static std::string GetRightFingering(const Note & note);
    static std::string GetLeftFingering(const Note & note);
    static std::string GetRightOrnament(const Note & note);
    static std::string GetLeftOrnament(const Note & note);
    static std::string GetFret(const Note & note, const Options& options);
};

} // namespace luteconv

#endif // _GENTAB_H_

