#include "XmlNode.h"
#include "XmlParser.h"
#include <iostream>
#include <fstream>
#include <exception>

void modifyXmlFile(const std::string& filename, const XmlNode& node) {
    std::ofstream outFile(filename, std::ios::out);
    if (!outFile.is_open()) {
        throw std::runtime_error("�� ������� ������� ���� ��� ������: " + filename);
    }

    outFile << node.toString();
    if (!outFile.good()) {
        throw std::runtime_error("��������� ������ ��� ������ � ����: " + filename);
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

        // ������������� ������ add ��� ���������� ����
        XmlNode::XmlNodeIterator addIt = root.add("newChild", "newText");
        if (addIt.currentNode != nullptr) {
            std::cout << "���� ��������: " << (*addIt).toString() << std::endl;
        }

        // ������������� ������ find ��� ������ ������������ ����
        XmlNode::XmlNodeIterator foundIt = root.find("newChild", "newText");
        if (foundIt.currentNode != nullptr) {
            std::cout << "��������� ����: " << (*foundIt).toString() << std::endl;
        }

        // ������������� ������ Erase ��� �������� ����
        bool erased = root.Erase(foundIt);
        std::cout << "���� ��� " << (erased ? "�����" : "�� ������") << std::endl;

        modifyXmlFile(outputFilePath, root); // ������ ��������� ����������� � ����� ����
    }
    catch (const std::exception& e) {
        std::cerr << "����������: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
