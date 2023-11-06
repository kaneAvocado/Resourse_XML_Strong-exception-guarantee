#include "XmlNode.h"
#include "XmlParser.h"
#include <iostream>
#include <fstream>
#include <exception>

void modifyXmlFile(const std::string& filename, const XmlNode& node) {
    std::ofstream outFile(filename, std::ios::out);
    if (!outFile.is_open()) {
        throw std::runtime_error("Не удалось открыть файл для записи: " + filename);
    }

    outFile << node.toString();
    if (!outFile.good()) {
        throw std::runtime_error("Произошла ошибка при записи в файл: " + filename);
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

        // Использование метода add для добавления узла
        XmlNode::XmlNodeIterator addIt = root.add("newChild", "newText");
        if (addIt.currentNode != nullptr) {
            std::cout << "Узел добавлен: " << (*addIt).toString() << std::endl;
        }

        // Использование метода find для поиска добавленного узла
        XmlNode::XmlNodeIterator foundIt = root.find("newChild", "newText");
        if (foundIt.currentNode != nullptr) {
            std::cout << "Найденный узел: " << (*foundIt).toString() << std::endl;
        }

        // Использование метода Erase для удаления узла
        bool erased = root.Erase(foundIt);
        std::cout << "Узел был " << (erased ? "удалён" : "не найден") << std::endl;

        modifyXmlFile(outputFilePath, root); // Теперь изменения сохраняются в новый файл
    }
    catch (const std::exception& e) {
        std::cerr << "Исключение: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
