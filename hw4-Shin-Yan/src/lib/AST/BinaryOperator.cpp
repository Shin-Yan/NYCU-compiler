#include "AST/BinaryOperator.hpp"
#include <string.h>

const char *BinaryOperatorNode::getLeftTypeCString() const{
    return m_left_operand->getPTypeCString();
}
const char *BinaryOperatorNode::getRightTypeCString() const{
    return m_right_operand->getPTypeCString();
}

const int BinaryOperatorNode::checkInvalidChildren() const{
    if(strcmp(m_left_operand->getPTypeCString(), "null") == 0)
        return 1;
    if(strcmp(m_right_operand->getPTypeCString(), "null") == 0)
        return 1;
    return 0;
}

void BinaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    auto visit_ast_node = [&](auto &ast_node) { ast_node->accept(p_visitor); };

    visit_ast_node(m_left_operand);
    visit_ast_node(m_right_operand);
}
