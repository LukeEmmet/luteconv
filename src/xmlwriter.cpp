#include "xmlwriter.h"

namespace luteconv
{


// class XMLAttrib

XMLAttrib::XMLAttrib(const char* name, const char* value)
: m_name{name}, m_value{XMLWriter::Escape(value)}
{
}

XMLAttrib::XMLAttrib(const char* name, int value)
: m_name{name}, m_value{std::to_string(value)}
{
}

void XMLAttrib::Print(std::ostream& s) const
{
    s << m_name << "=\"" << m_value << "\"";
}

// class XMLContent

XMLContent::XMLContent(const char* content)
: m_content{XMLWriter::Escape(content)}
{
    
}

XMLContent::XMLContent(int content)
: m_content{std::to_string(content)}
{
    
}

void XMLContent::Print(std::ostream& s, int level) const
{
    s << std::string(XMLWriter::indent * level, ' ') << m_content;
}

// class XMLComment

XMLComment::XMLComment(const char* comment)
: m_comment{comment}
{
    
}

void XMLComment::Print(std::ostream& s, int level) const
{
    s << std::string(XMLWriter::indent * level, ' ') << "<!-- " << m_comment << " -->";
}

// class XMLElement

XMLElement::XMLElement(const char* name)
: m_name{name}
{
    
}

XMLElement::XMLElement(const char* name, const char* content)
: m_name{name}
{
    Add(new XMLContent(content));
}

XMLElement::XMLElement(const char* name, int content)
: m_name{name}
{
    Add(new XMLContent(content));
}

XMLElement::~XMLElement()
{
    for (auto child : m_children)
        delete child;
}

void XMLElement::AddAttrib(const char* name, const char* value)
{
    m_attribs.emplace_back(name, value);
}

void XMLElement::AddAttrib(const char* name, int value)
{
    m_attribs.emplace_back(name, value);
}

void XMLElement::AddContent(const char* content)
{
    Add(new XMLContent(content));
}

void XMLElement::AddComment(const char* comment)
{
    Add(new XMLComment(comment));
}

void XMLElement::Add(XMLObject* child)
{
    m_children.push_back(child);
}

void XMLElement::Print(std::ostream& s, int level) const
{
    // <tag
    s << std::string(XMLWriter::indent * level, ' ') << "<" << m_name;
    
    // attributes
    if (!m_attribs.empty())
    {
        for (const auto& attrib : m_attribs)
        {
            s << " ";
            attrib.Print(s);
        }
    }
    
    // don't print final std::endl
    
    if (m_children.empty())
    {
        // empty element
        s << " />";
    }
    else if (m_children.size() == 1 && m_children[0]->IsContent())
    {
        // Single content, tags inline.
        // <tag>content</tag>
        s << ">";
        m_children[0]->Print(s, 0);
        s << "</" << m_name << ">";
    }
    else
    {
        // indented block of children
        // <tag>
        //     <child1>
        //     ...
        // </tag>
        
        s << ">" << std::endl; 
        
        for (auto child : m_children)
        {
            child->Print(s, level + 1);
            s << std::endl;
        }
        
        s << std::string(XMLWriter::indent * level, ' ') << "</" << m_name << ">";
    }
}

// class XMLWriter

void XMLWriter::AddDoctype(const char* doctype)
{
    m_doctype = doctype;
}

void XMLWriter::SetRoot(XMLElement* root)
{
    m_root.reset(root);
}

XMLElement* XMLWriter::Root() const
{
    return m_root.get();
}

std::string XMLWriter::Escape(const char* text)
{
    std::string result;
    for (const char* p = text; *p; ++p)
    {
        switch (*p)
        {
        case '&':
            result += "&amp;";
            break;
        case '<':
            result += "&lt;";
            break;
        case '>':
            result += "&gt;";
            break;
        case '\'':
            result += "&apos;";
            break;
        case '\"':
            result += "&quot;";
            break;
        default:
            result += *p;
            break;
        }
    }
    
    return result;
}

std::ostream& operator<<(std::ostream& s, const XMLWriter& xmlwriter)
{
    s << R"(<?xml version="1.0" standalone="no"?>)" << std::endl;

    if (!xmlwriter.m_doctype.empty())
        s << xmlwriter.m_doctype << std::endl;
    
    xmlwriter.m_root->Print(s, 0);
    s << std::endl;

    return s;
}

} // namespace luteconv
