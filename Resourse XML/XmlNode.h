#ifndef XMLNODE_H
#define XMLNODE_H

#include <string>
#include <map>

class XmlNode {
private:
    std::string name;
    std::string text;
    std::map<std::string, XmlNode> children;

    XmlNode(const std::string& nodeName);

    friend class XmlParser;

public:
    XmlNode();

    struct XmlNodeIterator {
        const XmlNode* currentNode;
        std::map<std::string, XmlNode>::const_iterator mapIt;

        explicit XmlNodeIterator(const XmlNode* node);
        bool operator!=(const XmlNodeIterator& other) const;
        const XmlNode& operator*() const;
        XmlNodeIterator& operator++();
    };

    XmlNodeIterator begin() const;
    XmlNodeIterator end() const;
    XmlNodeIterator find(const std::string& nodeName, const std::string& nodeText) const;
    XmlNodeIterator add(const std::string& nodeName, const std::string& nodeValue);
    bool Erase(const XmlNodeIterator& iterator);
    std::string toString(const std::string& indent = "") const;
};

#endif // XMLNODE_H
