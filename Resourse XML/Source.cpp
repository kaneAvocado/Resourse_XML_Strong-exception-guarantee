#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>

class XmlResource {
private:
    struct Node {
        std::string name;
        std::string value;
        std::map<std::string, Node> children;
    };

    Node root;

    Node parseNode(std::istringstream& iss) {
        Node node;
        std::string line;
        while (std::getline(iss, line)) {
            std::istringstream lineStream(line);
            std::string keyword;
            if (line.find("<") != std::string::npos && line.find(">") != std::string::npos) {
                size_t startPos = line.find("<") + 1;
                size_t endPos = line.find(">");
                keyword = line.substr(startPos, endPos - startPos);
                if (keyword[0] == '/') {
                    break;
                }
                node.name = keyword;
                std::string restOfLine = line.substr(endPos + 1);
                if (!restOfLine.empty() && restOfLine.find("<") != std::string::npos) {
                    node.value = restOfLine.substr(0, restOfLine.find("<"));
                    iss.str(restOfLine);
                    iss.clear();
                }
                else {
                    node.children[keyword] = parseNode(iss);
                }
            }
            else {
                node.value += line + "\n";
                std::cout << 45;
            }
        }
        return node;
    }

    // В этом методе мы изменяем логику сохранения, чтобы соответствовать новому формату
    void saveNode(std::ofstream& ofs, const Node& node, const std::string& indent) {
        ofs << indent << "<" << node.name << ">\n";
        if (!node.value.empty()) {
            ofs << indent << "  " << node.value << "\n";
        }
        for (const auto& child : node.children) {
            saveNode(ofs, child.second, indent + "  ");
        }
        ofs << indent << "</" << node.name << ">\n";
    }

public:
    XmlResource() = default;

    void loadFromFile(const std::string& filename) {
        std::ifstream ifs(filename);
        if (!ifs) {
            throw std::runtime_error("Cannot open file for reading: " + filename);
        }
        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        std::istringstream iss(content);
        root = parseNode(iss);
    }

    void saveToFile(const std::string& filename) {
        std::ofstream ofs(filename);
        if (!ofs) {
            throw std::runtime_error("Cannot open file for writing: " + filename);
        }
        saveNode(ofs, root, "");
    }

    using NodePtr = Node*;

    std::string trim(const std::string& str) {
        auto start = str.begin();
        while (start != str.end() && std::isspace(*start)) {
            start++;
        }

        auto end = str.end();
        do {
            end--;
        } while (std::distance(start, end) > 0 && std::isspace(*end));

        return std::string(start, end + 1);
    }


    NodePtr find(const std::string& name, const std::string& value, NodePtr node = nullptr) {
        if (node == nullptr) {
            node = &root;
        }
        if (node->name == name && trim(node->value) == trim(value)) {
            return node;
        }
        for (auto& child : node->children) {
            NodePtr result = find(name, value, &(child.second));
            if (result != nullptr) {
                return result;
            }
        }
        return nullptr;
    }

    NodePtr add(const std::string& name, const std::string& value, NodePtr parent) {
        if (parent == nullptr) {
            throw std::invalid_argument("Parent node is null");
        }
        Node& newNode = parent->children[name];
        newNode.name = name;
        newNode.value = value;
        return &newNode;
    }

    bool eraseHelper(NodePtr parentNode, NodePtr nodeToDelete) {
        if (parentNode == nullptr || nodeToDelete == nullptr) {
            return false;
        }

        for (auto it = parentNode->children.begin(); it != parentNode->children.end(); ++it) {
            if (&(it->second) == nodeToDelete) {
                parentNode->children.erase(it);
                return true;
            }
            else {
                if (eraseHelper(&(it->second), nodeToDelete)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool erase(NodePtr nodeToDelete) {
        if (nodeToDelete == nullptr || nodeToDelete == &root) return false;
        return eraseHelper(&root, nodeToDelete);
    }
};

int main() {
    XmlResource xmlRes;

    try {
        xmlRes.loadFromFile("example.xml");

        XmlResource::NodePtr foundNode = xmlRes.find("first.potomok", "my value of first.potomok");
        if (foundNode) {
            std::cout << "Found node: " << foundNode->name << " with value: " << foundNode->value << std::endl;
        }
        else {
            std::cout << "Node not found." << std::endl;
        }

        XmlResource::NodePtr addedNode = xmlRes.add("new.child", "new value", foundNode);
        if (addedNode) {
            std::cout << "Added node: " << addedNode->name << " with value: " << addedNode->value << std::endl;
        }

        bool erased = xmlRes.erase(addedNode);
        std::cout << "Node " << (erased ? "erased successfully." : "was not erased.") << std::endl;

        xmlRes.saveToFile("example_modified.xml");
        std::cout << "XML file was saved successfully." << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
