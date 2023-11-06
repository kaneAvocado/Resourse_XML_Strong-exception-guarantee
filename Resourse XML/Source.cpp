#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <exception>

class XmlNode {
public:
    std::string name;
    std::string text;
    std::map<std::string, XmlNode> children;

    XmlNode() {}
    XmlNode(const std::string& name) : name(name) {}

    // Добавляет потомка к узлу
    void addChild(const XmlNode& child) {
        children[child.name] = child;
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << "<" + name + "<";
        if (!text.empty()) {
            oss << text;
        }
        for (const auto& child : children) {
            oss << child.second.toString();
        }
        oss << "</" + name + ">";
        return oss.str();
     }
};

class XmlParser {
public:
    XmlNode parse(const std::string& xml) {
        std::vector<XmlNode> nodeStack;
        std::string currentText;
        XmlNode rootNode;

        for (size_t i = 0; i < xml.size(); ++i) {
            if (xml[i] == '<') {
                if (!currentText.empty()) {
                    if (nodeStack.empty()) {
                        throw std::runtime_error("No opening tag for text content");
                    }
                    nodeStack.back().text = currentText;
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
                    XmlNode completedNode = nodeStack.back();
                    nodeStack.pop_back();
                    if (nodeStack.empty()) {
                        rootNode = completedNode;
                    }
                    else {
                        nodeStack.back().addChild(completedNode);
                    }
                }
                else {
                    nodeStack.emplace_back(tagName);
                }
                i = closeTag;
            }
            else {
                currentText += xml[i];
            }
        }
        if (!nodeStack.empty()) {
            throw std::runtime_error("Malformed XML: Tags left open");
        }
        return rootNode;
    }
};


void modifyXmlFile(const std::string& filePath, const XmlNode& rootNode) {
    std::ofstream outFile(filePath);
    outFile << rootNode.toString();
    if (!outFile.good()) {
        throw std::runtime_error("Error occurred while writing to file: " + filePath);
    }
}

int main() {
    const std::string filePath = "example.xml";
    try {
        std::ifstream xmlFile(filePath);
        std::string xmlContent((std::istreambuf_iterator<char>(xmlFile)), std::istreambuf_iterator<char>());

        XmlParser parser;
        XmlNode root = parser.parse(xmlContent);

        modifyXmlFile(filePath, root);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
