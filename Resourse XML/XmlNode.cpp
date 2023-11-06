#include "XmlNode.h"
#include <sstream>

XmlNode::XmlNode(const std::string& nodeName) : name(nodeName) {}

XmlNode::XmlNode() = default;

XmlNode::XmlNodeIterator::XmlNodeIterator(const XmlNode* node)
    : currentNode(node), mapIt(node ? node->children.cbegin() : std::map<std::string, XmlNode>::const_iterator()) {}

bool XmlNode::XmlNodeIterator::operator!=(const XmlNodeIterator& other) const {
    return mapIt != other.mapIt;
}

const XmlNode& XmlNode::XmlNodeIterator::operator*() const {
    static const XmlNode endNode;
    return mapIt == currentNode->children.cend() ? endNode : mapIt->second;
}

XmlNode::XmlNodeIterator& XmlNode::XmlNodeIterator::operator++() {
    if (currentNode && mapIt != currentNode->children.cend()) {
        ++mapIt;
    }
    return *this;
}

XmlNode::XmlNodeIterator XmlNode::begin() const {
    return XmlNodeIterator(this);
}

XmlNode::XmlNodeIterator XmlNode::end() const {
    return XmlNodeIterator(nullptr);
}

XmlNode::XmlNodeIterator XmlNode::find(const std::string& nodeName, const std::string& nodeText) const {
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

XmlNode::XmlNodeIterator XmlNode::add(const std::string& nodeName, const std::string& nodeValue) {
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

bool XmlNode::Erase(const XmlNodeIterator& iterator) {
    if (!iterator.currentNode || iterator.mapIt == children.cend()) {
        return false;
    }
    children.erase(iterator.mapIt->first);
    return true;
}

std::string XmlNode::toString(const std::string& indent) const {
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