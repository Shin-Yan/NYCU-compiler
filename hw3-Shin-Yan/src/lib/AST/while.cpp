#include "AST/while.hpp"

// TODO
WhileNode::WhileNode(const uint32_t line, const uint32_t col, AstNode *p_expr, AstNode *p_compound)
    : AstNode{line, col}, expr(p_expr), compound(p_compound) {}

// TODO: You may use code snippets in AstDumper.cpp
void WhileNode::print() {}

void WhileNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void WhileNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    if(expr != NULL)
        expr->accept(p_visitor);
    if(compound != NULL)
        compound->accept(p_visitor);
}
