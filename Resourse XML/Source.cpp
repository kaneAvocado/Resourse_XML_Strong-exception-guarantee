#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <exception>

class XmlNode {
private:
    std::string name;
    std::string text;
    std::map<std::string, XmlNode> children;

    XmlNode(const std::string& nodeName) : name(nodeName) {}

    friend class XmlParser; // XmlParser ����� ��������� � �������� XmlNode

public:
    XmlNode() = default;

    struct XmlNodeIterator {
        const XmlNode* currentNode;
        std::map<std::string, XmlNode>::const_iterator mapIt;

        explicit XmlNodeIterator(const XmlNode* node)
            : currentNode(node), mapIt(node ? node->children.cbegin() : std::map<std::string, XmlNode>::const_iterator()) {}

        bool operator!=(const XmlNodeIterator& other) const {
            // �������� �� ��, ��� ��������� �� �����, �� ������� �������� �� end
            return mapIt != other.mapIt;
        }

        const XmlNode& operator*() const {
        if (!currentNode || mapIt == currentNode->children.cend()) {
            static const XmlNode endNode; // ����������� ���� ��� �����
            return endNode;
        }
        return mapIt->second;
        }

       XmlNodeIterator& operator++() {
        if (!currentNode || mapIt == currentNode->children.cend()) {
            return *this; // ������ ���������� ��������, ���� �� ��� � ��������� end
        }
        ++mapIt;
        return *this;
       }
    };



    XmlNodeIterator begin() const {
        return XmlNodeIterator(this);
    }

    XmlNodeIterator end() const {
        return XmlNodeIterator(nullptr);
    }

    XmlNodeIterator find(const std::string& nodeName, const std::string& nodeText) const {
        for (auto it = children.begin(); it != children.end(); ++it) {
            if (it->second.name == nodeName && (nodeText.empty() || it->second.text == nodeText)) {
                return XmlNodeIterator(&(it->second));
            }
            auto found = it->second.find(nodeName, nodeText);
            if (found.currentNode) {
                return found;
            }
        }
        return XmlNodeIterator(nullptr);
    }

    XmlNodeIterator add(const std::string& nodeName, const std::string& nodeValue) {
        auto it = children.find(nodeName);
        if (it != children.end()) {
            it->second.text = nodeValue;
        }
        else {
            it = children.insert({ nodeName, XmlNode(nodeName) }).first;
            it->second.text = nodeValue;
        }
        return XmlNodeIterator(&(it->second));
    }

    bool Erase(const XmlNodeIterator& iterator) {
        // ���������, ��� currentNode �� nullptr � ��� �������� mapIt �� ��������� �� end
        if (!iterator.currentNode || iterator.mapIt == children.cend()) {
            return false;
        }

        // ������� ���� �� �����
        children.erase(iterator.mapIt->first);
        return true;
    }



    std::string toString(const std::string& indent = "") const {
        std::ostringstream oss;
        std::string newIndent = indent + "  ";
        oss << indent << "<" + name + ">" << std::endl;
        if (!text.empty()) {
            oss << newIndent << text << std::endl;
        }
        for (const auto& child : children) {
            oss << child.second.toString(newIndent);
        }
        oss << indent << "</" + name + ">" << std::endl;
        return oss.str();
    }
};

class XmlParser {
public:
    XmlNode parse(const std::string& xml) {
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
                    XmlNode* completedNode = nodeStack.back();
                    nodeStack.pop_back();
                    if (nodeStack.empty()) {
                        rootNode = completedNode;
                    }
                    else {
                        nodeStack.back()->children[completedNode->name] = *completedNode;
                        delete completedNode; // ������� ���� ����� ���������� � �����
                    }
                }
                else {
                    nodeStack.push_back(new XmlNode(tagName));
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
        if (!rootNode) {
            throw std::runtime_error("No root node found");
        }
        return *rootNode; // ���������� ����� �����
    }

};

void modifyXmlFile(const std::string& outputPath, const XmlNode& rootNode) {
    std::ofstream outFile(outputPath, std::ios::out);
    if (!outFile.is_open()) {
        throw std::runtime_error("�� ������� ������� ���� ��� ������: " + outputPath);
    }

    outFile << rootNode.toString();
    if (!outFile.good()) {
        throw std::runtime_error("��������� ������ ��� ������ � ����: " + outputPath);
    }
}


int main() {
    setlocale(LC_ALL, "RUSSIAN");
    const std::string inputFilePath = "example.xml"; // ���� � ��������� �����
    const std::string outputFilePath = "modified_example.xml"; // ���� � ����� �����������

    try {
        std::ifstream xmlFile(inputFilePath);
        if (!xmlFile.is_open()) {
            throw std::runtime_error("�� ������� ������� ����: " + inputFilePath);
        }

        std::string xmlContent((std::istreambuf_iterator<char>(xmlFile)),
            std::istreambuf_iterator<char>());

        XmlParser parser;
        XmlNode root = parser.parse(xmlContent);

        XmlNode::XmlNodeIterator addIt = root.add("newChild", "newText");
        if (addIt.currentNode != nullptr) {
            std::cout << "���� ��������: " << addIt.currentNode->toString() << std::endl;
        }

        // ������������ ������ ������ Erase
        XmlNode::XmlNodeIterator addIt = root.add("ChildToDelete", "TextToDelete");
        if (addIt.currentNode != nullptr) {
            std::cout << "����������� ����: " << (*addIt).toString() << std::endl;
        }

        bool erased = root.Erase(addIt);
        std::cout << "���� ��� " << (erased ? "�����" : "�� ������") << std::endl;

        modifyXmlFile(outputFilePath, root); // ������ ��������� ����������� � ����� ����
    }
    catch (const std::exception& e) {
        std::cerr << "����������: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

