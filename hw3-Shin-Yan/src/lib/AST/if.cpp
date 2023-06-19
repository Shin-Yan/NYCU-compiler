#include "AST/if.hpp"

// TODO
IfNode::IfNode(const uint32_t line, const uint32_t col, AstNode *p_expr, 
                AstNode *p_comp1, AstNode *p_comp2)
    : AstNode{line, col}, expr(p_expr), comp1(p_comp1), comp2(p_comp2) {}

// TODO: You may use code snippets in AstDumper.cpp
void IfNode::print() {}

void IfNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void IfNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    if(expr != NULL)
        expr->accept(p_visitor);
    if(comp1 != NULL)
        comp1->accept(p_visitor);
    if(comp2 != NULL)
        comp2->accept(p_visitor);
}
