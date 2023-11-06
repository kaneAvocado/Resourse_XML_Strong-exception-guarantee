#include "XmlParser.h"
#include "XmlParser.h"
#include <vector>
#include <string>
#include <stdexcept>


XmlNode XmlParser::parse(const std::string& xml) {
    std::vector<XmlNode*> nodeStack;
    std::string currentText;
    XmlNode* rootNode = nullptr;

    for (size_t i = 0; i < xml.size(); ++i) {
        if (xml[i] == '<') {
            if (!currentText.empty()) {
                if (nodeStack.empty()) {
                    throw std::runtime_error("No opening tag for text content");
                }
                nodeStack.back()->text = currentText;
                currentText.clear();
            }
            size_t closeTag = xml.find('>', i);
            if (closeTag == std::string::npos) {
                throw std::runtime_error("Malformed XML: No closing tag found");
            }

            std::string tagName = xml.substr(i + 1, closeTag - i - 1);
            if (tagName[0] == '/') {
                if (nodeStack.empty()) {
                    throw std::runtime_error("No opening tag for closing tag");
                }
                if (tagName.substr(1) != nodeStack.back()->name) {
                    throw std::runtime_error("Mismatched tag name");
                }
                XmlNode* completedNode = nodeStack.back();
                nodeStack.pop_back();
                if (nodeStack.empty()) {
                    rootNode = completedNode;
                }
                else {
                    nodeStack.back()->children[completedNode->name] = *completedNode;
                    delete completedNode; // Delete node after adding to children
                }
            }
            else {
                nodeStack.push_back(new XmlNode(tagName));
            }
            i = closeTag;
        }
        else if (xml[i] != '\n' && xml[i] != '\r' && xml[i] != '\t' && xml[i] != ' ') { // Skipping whitespaces outside tags
            currentText += xml[i];
        }
    }

    if (!nodeStack.empty()) {
        throw std::runtime_error("Malformed XML: Tags left open");
    }

    if (!rootNode) {
        throw std::runtime_error("No root node found");
    }

    XmlNode rootCopy = *rootNode;
    delete rootNode;
    return rootCopy;
}
