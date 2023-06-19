#include "AST/print.hpp"

const int PrintNode::checkInvalidChildren() const{
    std::string type = m_target->getPTypeCString();
    if(type == "null")
        return 1;
    return 0;
}

const int PrintNode::checkTargetScalarType() const{
    std::string type = m_target->getPTypeCString();
    if(type == "integer" || type == "real" || type == "string" || type == "boolean")
        return 1;
    else
        return 0;
}

const uint32_t PrintNode::getTargetLocationCol() const{
    return m_target->getLocation().col;
}

void PrintNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    m_target->accept(p_visitor);
}
