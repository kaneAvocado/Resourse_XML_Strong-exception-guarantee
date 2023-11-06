#ifndef XMLPARSER_H
#define XMLPARSER_H

#include "XmlNode.h"
#include <string>

class XmlParser {
public:
    XmlNode parse(const std::string& xml);
};

#endif // XMLPARSER_H
