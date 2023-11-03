#include <iostream>
#include <fstream>
#include <stack>
#include <vector>
#include <memory>
#include <string>

struct XmlNode {
    std::string name;
    std::string value;
    std::vector<std::shared_ptr<XmlNode>> children;
};

std::shared_ptr<XmlNode> parseXml(std::istream& input) {
    std::stack<std::shared_ptr<XmlNode>> stack;
    std::string line;
    std::shared_ptr<XmlNode> root, currentNode;

    while (getline(input, line)) {
        size_t startTagPos = line.find('<');
        size_t endTagPos = line.find('>', startTagPos);
        if (startTagPos != std::string::npos && endTagPos != std::string::npos) {
            if (line[startTagPos + 1] != '/') { // start tag
                currentNode = std::make_shared<XmlNode>();
                currentNode->name = line.substr(startTagPos + 1, endTagPos - startTagPos - 1);
                if (!stack.empty()) {
                    stack.top()->children.push_back(currentNode);
                }
                stack.push(currentNode);
                if (!root) {
                    root = currentNode;
                }
            }
            else { // end tag
                if (!stack.empty()) {
                    currentNode = stack.top();
                    stack.pop();
                }
            }
        }
        else {
            if (!stack.empty()) {
                currentNode->value += line;
            }
        }
    }
    return root;
}

void printXml(const std::shared_ptr<XmlNode>& node, int depth = 0) {
    std::cout << std::string(depth, ' ') << node->name << ": " << node->value << "\n";
    for (const auto& child : node->children) {
        printXml(child, depth + 2);
    }
}

int main() {
    std::ifstream xmlFile("xmlData.txt");
    if (!xmlFile.is_open()) {
        std::cerr << "Failed to open xmlData.txt\n";
        return 1;
    }

    auto xmlTree = parseXml(xmlFile);

    if (xmlTree) {
        printXml(xmlTree);
    }
    else {
        std::cout << "The XML tree is empty or could not be parsed.\n";
    }

    return 0;
}
