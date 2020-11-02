#ifndef _GENMEI_H_
#define _GENMEI_H_

#include <iostream>

#include "xmlwriter.h"
#include "piece.h"
#include "options.h"

namespace luteconv
{

/**
 * Generate .mei
 */
class GenMei
{
public:
    /**
     * Constructor
     */
    GenMei() = default;

    /**
     * Destructor
     */
    ~GenMei() = default;
    
    /**
     * Generate .mei to file specified in options
     * 
     * @param[in] options
     * @param[in] piece
     */
    void Generate(const Options& options, const Piece& piece);
    
    /**
     * Generate .mei
     * 
     * @param[in] options
     * @param[in] piece
     * @param[out] dst destination
     */
    void Generate(const Options& options, const Piece& piece, std::ostream& dst);
    
private:
    void FileDesc(XMLElement* xmlfileDesc, const Piece& piece);
    void EncodingDesc(XMLElement* xmlEncodingDesc, const Options& options);
    void Work(XMLElement* xmlwork, const Piece& piece);
    void Body(XMLElement* xmlbody, const Options& options, const Piece& piece);
    void Section(XMLElement* xmlsection, const Options& options, const Piece& piece);
    std::string MakeId();
    void AddTimeSignature(XMLElement* xmlmensur, const TimeSig& timeSig);
    
    int m_nextId{0};
};



} // namespace luteconv

#endif // _GENMEI_H_
