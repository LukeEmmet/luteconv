#ifndef _XMLWRITER_H_
#define _XMLWRITER_H_

#include <string>
#include <vector>
#include <memory>
#include <ostream>


namespace luteconv
{

/**
 * An XML attribute
 */
class XMLAttrib
{
public:
    /**
     * Constructor
     * 
     * @param[in] name
     * @param[in] value
     */
    XMLAttrib(const char* name, const char* value);
    
    /**
     * Constructor
     * 
     * @param[in] name
     * @param[in] value
     */
    XMLAttrib(const char* name, int value);
    
    /**
     * Destructor
     */
    ~XMLAttrib() = default;
    
    /**
     * Print
     * 
     * @param[in] s - stream
     */
    void Print(std::ostream& s) const;
    
private:
    std::string m_name;
    std::string m_value;
};

/**
 * Baseclass
 */
class XMLObject
{
public:
    /**
     * Destructor
     */
    virtual ~XMLObject() = default;
    
    /**
     * Print
     * 
     * @param[in] s - stream
     * @param[in] level - indendation level
     */
    virtual void Print(std::ostream& s, int level) const = 0;

    /**
     * Is this content?
     * 
     * @return true <=> content
     */
    virtual bool IsContent() const
    {
        return false;
    }
    
protected:
    /**
     * Constructor
     */
    XMLObject() = default;
};

/**
 * Content
 */
class XMLContent: public XMLObject
{
public:
    /**
     * Constructor
     * 
     * @param[in] value
     */
    explicit XMLContent(const char* value);
    
    /**
     * Constructor
     * 
     * @param[in] value
     */
    explicit XMLContent(int value);
    
    /**
     * Destructor
     */
    ~XMLContent() = default;
    
    /**
     * Print
     * 
     * @param[in] s - stream
     * @param[in] level - indendation level
     */
    void Print(std::ostream& s, int level) const override;

    /**
     * Is this content?
     * 
     * @return true <=> content
     */
    bool IsContent() const override
    {
        return true;
    }
    
private:
    std::string m_content;
};

/**
 * Comment
 */
class XMLComment: public XMLObject
{
public:
    /**
     * Constructor
     * 
     * @param[in] comment
     */
    explicit XMLComment(const char* comment);
    
    /**
     * Destructor
     */
    ~XMLComment() = default;
    
    /**
     * Print
     * 
     * @param[in] s - stream
     * @param[in] level - indendation level
     */
    void Print(std::ostream& s, int level) const override;

private:
    std::string m_comment;
};

/**
 * An element
 */
class XMLElement: public XMLObject
{
public:
    /**
     * Constructor
     * 
     * @param[in] name
     */
    explicit XMLElement(const char* name);
    
    /**
     * Constructor with content
     * 
     * @param[in] name
     * @param[in] content
     */
    XMLElement(const char* name, const char* content);

    /**
     * Constructor with content
     * 
     * @param[in] name
     * @param[in] content
     */
    XMLElement(const char* name, int content);

    /**
     * Destructor
     */
    ~XMLElement() override;
    
    /**
     * Add child
     * 
     * @param[in] child
     */
    void Add(XMLObject* child);
    
    /**
     * Add an attribute
     * 
     * @param[in] name
     * @param[in] value
     */
    void AddAttrib(const char* name, const char* value);
    
    /**
     * Add an attribute
     * 
     * @param[in] name
     * @param[in] value
     */
    void AddAttrib(const char* name, int value);

    /**
     * Add a comment
     * 
     * @param[in] comment
     */
    void AddComment(const char* comment);
    
    /**
     * Add content
     * 
     * @param[in] content
     */
    void AddContent(const char* content);
    
    /**
     * Print
     * 
     * @param[in] s - stream
     * @param[in] level - indendation level
     */
    void Print(std::ostream& s, int level) const override;
    
private:
    std::string m_name;
    std::vector<XMLAttrib> m_attribs;
    std::vector<XMLObject*> m_children;
};

/**
 * Very simple XML writer
 */
class XMLWriter
{
public:
    /**
     * Constructor
     */
    XMLWriter() = default;
    
    /**
     * Destructor
     */
    ~XMLWriter() = default;
    
    /**
     * DOCTYPE
     * 
     * @param[in] doctype
     */
    void AddDoctype(const char* doctype);

    /**
     * Set the root element
     * 
     * @param[in] root
     */
    void SetRoot(XMLElement* root);
    
    /**
     * Get the root element
     * 
     * @return root
     */
    XMLElement* Root() const;
    
    /**
     * Add XML escapes as necessary
     * 
     * @param[in] text - non-escaped text
     * @return escaped text 
     */
    static std::string Escape(const char* text);
    
    /**
     * Indent size
     */
    static const int indent = 4; // spaces
    
    /**
     * Stream output XMLWriter
     */
    friend std::ostream& operator<<(std::ostream& s, const XMLWriter& xmlwriter);
    
private:
    std::string m_doctype;
    std::unique_ptr<XMLElement> m_root;
};


} // namespace luteconv

#endif // _XMLWRITER_H_

