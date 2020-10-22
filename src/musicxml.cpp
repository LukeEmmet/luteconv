#include "musicxml.h"

namespace luteconv
{

const char* const MusicXml::noteType[] = {"long", "breve", "whole", "half",
        "quarter", "eighth", "16th", "32nd", "64th", "128th", "256th", nullptr};

const char* const MusicXml::barStyle[] = {"regular", "dotted", "dashed", "heavy", "light-light", "light-heavy",
        "heavy-light", "heavy-heavy", "tick", "short", nullptr};
    
const char* const MusicXml::timeSymbol[] = {"none", "common", "cut", "single-number", "note", "dotted-note", "normal", nullptr};

const char* const MusicXml::pluck[] = {"", "i", "m", "a", "", "p", nullptr};

} // namespace luteconv