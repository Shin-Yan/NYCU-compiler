#include "AST/read.hpp"

const int ReadNode::checkInvalidChildren() const{
    std::string type = m_target->getPTypeCString();
    if(type == "null")
        return 1;
    return 0;
}

const int ReadNode::checkTargetScalarType() const{
    std::string type = m_target->getPTypeCString();
    if(type == "integer" || type == "real" || type == "string" || type == "boolean")
        return 1;
    else
        return 0;
}

const char *ReadNode::getTargetNameCString() const{
    return m_target->getNameCString();
}

const uint32_t ReadNode::getTargetLocationCol() const{
    return m_target->getLocation().col;
} 

void ReadNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    m_target->accept(p_visitor);
}
