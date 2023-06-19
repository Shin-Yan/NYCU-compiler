#include "AST/while.hpp"
#include <string.h>
const int WhileNode::checkInvalidCondition() const{

    if(strcmp(m_condition->getPTypeCString(), "null")==0)
        return 1;

    return 0;
}

const int WhileNode::checkConditionBoolType() const{
    if(strcmp(m_condition->getPTypeCString(), "boolean")==0)
        return 1;
    else
        return 0;
}

void WhileNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    m_condition->accept(p_visitor);
    m_body->accept(p_visitor);
}
