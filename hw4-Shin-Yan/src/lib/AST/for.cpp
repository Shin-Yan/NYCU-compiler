#include "AST/for.hpp"

void ForNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    m_loop_var_decl->accept(p_visitor);
    m_init_stmt->accept(p_visitor);
    m_end_condition->accept(p_visitor);
    m_body->accept(p_visitor);
}

const char *ForNode::getInitialValueCString(){
    return m_init_stmt->getConstantValueCString();
}

const char *ForNode::getConditionValueCString(){
    std::unique_ptr<ConstantValueNode> expr_const(dynamic_cast<ConstantValueNode *>(m_end_condition.get()));
    if(expr_const)
        m_end_condition.release();

    return expr_const->getConstantValueCString();
}