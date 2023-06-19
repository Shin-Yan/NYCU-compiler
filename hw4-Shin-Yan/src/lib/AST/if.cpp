#include "AST/if.hpp"

const int IfNode::checkInvalidCondition() const{
    std::string cond = m_condition->getPTypeCString();
    if(cond == "null")
        return 1;
    return 0;
}

const int IfNode::checkConditionBoolType() const{
    std::string cond = m_condition->getPTypeCString();
    if(cond == "boolean")
        return 1;
    return 0;
}

const uint32_t IfNode::getConditionLocationCol() const{
    return m_condition->getLocation().col;
}

void IfNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    m_condition->accept(p_visitor);  
    m_body->accept(p_visitor);
    if (m_else_body) {
        m_else_body->accept(p_visitor);
    }
}
