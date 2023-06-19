#include "AST/UnaryOperator.hpp"
#include <string.h>

const char *UnaryOperatorNode::getOperandTypeCString() const{
    return m_operand->getPTypeCString();
}

const int UnaryOperatorNode::checkInvalidChildren() const{
    if(strcmp(m_operand->getPTypeCString(), "null") == 0)
        return 1;
    return 0;
}

void UnaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    auto visit_ast_node = [&](auto &ast_node) { ast_node->accept(p_visitor); };

    visit_ast_node(m_operand);
}
