#ifndef _MUSICXML_H_
#define _MUSICXML_H_

namespace luteconv
{

class MusicXml
{
public:
    
    /**
     * Strings for enum NoteType
     */
    static const char* const noteType[];

    /**
     * Strings for enum BarStyle
     */
    static const char* const barStyle[];
    
    /**
     * Strings for enum TimeSymbol
     */
    static const char* const timeSymbol[];
    
    /**
     * Strings for right hand fingering, enum Fingering
     */
    static const char* const pluck[];
};

} // namespace luteconv

#endif // _MUSICXML_H_
