#include <iostream>
#include <functional>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

class XMLNode {
public:
    using XMLNodePtr = std::shared_ptr<XMLNode>;

private:
    std::string name;
    std::string value;
    std::vector<XMLNodePtr> children;
    XMLNodePtr parent;

public:
    XMLNode(const std::string& nodeName, const std::string& nodeValue, XMLNodePtr parentNode = nullptr)
        : name(nodeName), value(nodeValue), parent(std::move(parentNode)) {}

    std::string getName() const { return name; }
    std::string getValue() const { return value; }
    const std::vector<XMLNodePtr>& getChildren() const { return children; }

    XMLNodePtr getParent() const {
        return parent;
    }
};

// Простой класс для парсинга XML
class XMLParser {
public:
    XMLNode::XMLNodePtr parseXML(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file");
        }

        XMLNode::XMLNodePtr root;
        XMLNode::XMLNodePtr currentNode;

        std::string line;
        while (std::getline(file, line)) {
            // Базовая обработка для открытия и закрытия тегов
            if (line.find('<') != std::string::npos && line.find("</") == std::string::npos) {
                size_t startPos = line.find('<') + 1;
                size_t endPos = line.find('>');
                std::string tagName = line.substr(startPos, endPos - startPos);
                // Упрощенная логика: предполагается, что за открывающим тегом следует значение
                std::getline(file, line);
                std::string value = line;
                auto node = std::make_shared<XMLNode>(tagName, value, currentNode);
                if (!currentNode) {  // Это корневой узел
                    root = node;
                }
                currentNode = node;  // Настраиваем текущий узел для добавления детей
            }
            else if (line.find("</") != std::string::npos) {
                currentNode = currentNode->getParent();  // Возвращаемся к родителю
            }
        }

        return root;
    }
};

// Функция для сохранения XML
void saveXML(const XMLNode::XMLNodePtr& root, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing");
    }

    std::function<void(const XMLNode::XMLNodePtr&, size_t)> writeNode;
    writeNode = [&](const XMLNode::XMLNodePtr& node, size_t depth) {
        std::string indent(depth, ' ');
        file << indent << "<" << node->getName() << ">\n";
        file << indent << "  " << node->getValue() << "\n";
        for (const auto& child : node->getChildren()) {
            writeNode(child, depth + 2);
        }
        file << indent << "</" << node->getName() << ">\n";
    };

    writeNode(root, 0);
}

int main() {
    try {
        XMLParser parser;
        auto root = parser.parseXML("xmlData.txt");
        saveXML(root, "output.xml");
    }
    catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << '\n'; // stron exeption garanty
    }

    return 0;
}
