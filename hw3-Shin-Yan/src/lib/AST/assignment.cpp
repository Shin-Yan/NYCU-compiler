#include "AST/assignment.hpp"

// TODO
AssignmentNode::AssignmentNode(const uint32_t line, const uint32_t col, AstNode *p_var_ref, AstNode *p_expr)
    : AstNode{line, col}, var_ref(p_var_ref), expr(p_expr) {}

// TODO: You may use code snippets in AstDumper.cpp
void AssignmentNode::print() {}

void AssignmentNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void AssignmentNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    if(var_ref!=NULL)
        var_ref->accept(p_visitor);

    if(expr!=NULL)
        expr->accept(p_visitor);
}
