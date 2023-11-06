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

    friend class XmlParser; // XmlParser может создавать и изменять XmlNode

public:
    XmlNode() = default;

    struct XmlNodeIterator {
        const XmlNode* currentNode;
        std::map<std::string, XmlNode>::const_iterator mapIt;

        explicit XmlNodeIterator(const XmlNode* node)
            : currentNode(node), mapIt(node ? node->children.cbegin() : std::map<std::string, XmlNode>::const_iterator()) {}

        bool operator!=(const XmlNodeIterator& other) const {
            // Проверка на то, что итераторы не равны, не требует проверки на end
            return mapIt != other.mapIt;
        }

        const XmlNode& operator*() const {
        if (!currentNode || mapIt == currentNode->children.cend()) {
            static const XmlNode endNode; // Специальный узел для конца
            return endNode;
        }
        return mapIt->second;
        }

       XmlNodeIterator& operator++() {
        if (!currentNode || mapIt == currentNode->children.cend()) {
            return *this; // Просто возвращаем итератор, если он уже в состоянии end
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
        // Проверяем, что currentNode не nullptr и что итератор mapIt не указывает на end
        if (!iterator.currentNode || iterator.mapIt == children.cend()) {
            return false;
        }

        // Удаляем узел по ключу
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
                        delete completedNode; // Удаляем узел после добавления в детей
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
        return *rootNode; // Возвращаем копию корня
    }

};

void modifyXmlFile(const std::string& outputPath, const XmlNode& rootNode) {
    std::ofstream outFile(outputPath, std::ios::out);
    if (!outFile.is_open()) {
        throw std::runtime_error("Не удалось открыть файл для записи: " + outputPath);
    }

    outFile << rootNode.toString();
    if (!outFile.good()) {
        throw std::runtime_error("Произошла ошибка при записи в файл: " + outputPath);
    }
}


int main() {
    setlocale(LC_ALL, "RUSSIAN");
    const std::string inputFilePath = "example.xml"; // Путь к исходному файлу
    const std::string outputFilePath = "modified_example.xml"; // Путь к файлу результатов

    try {
        std::ifstream xmlFile(inputFilePath);
        if (!xmlFile.is_open()) {
            throw std::runtime_error("Не удалось открыть файл: " + inputFilePath);
        }

        std::string xmlContent((std::istreambuf_iterator<char>(xmlFile)),
            std::istreambuf_iterator<char>());

        XmlParser parser;
        XmlNode root = parser.parse(xmlContent);

        XmlNode::XmlNodeIterator addIt = root.add("newChild", "newText");
        if (addIt.currentNode != nullptr) {
            std::cout << "Узел добавлен: " << addIt.currentNode->toString() << std::endl;
        }

        // Демонстрация работы метода Erase
        XmlNode::XmlNodeIterator addIt = root.add("ChildToDelete", "TextToDelete");
        if (addIt.currentNode != nullptr) {
            std::cout << "Добавленный узел: " << (*addIt).toString() << std::endl;
        }

        bool erased = root.Erase(addIt);
        std::cout << "Узел был " << (erased ? "удалён" : "не найден") << std::endl;

        modifyXmlFile(outputFilePath, root); // Теперь изменения сохраняются в новый файл
    }
    catch (const std::exception& e) {
        std::cerr << "Исключение: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

