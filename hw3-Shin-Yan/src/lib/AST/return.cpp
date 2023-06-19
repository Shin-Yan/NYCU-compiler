#include "AST/return.hpp"

// TODO
ReturnNode::ReturnNode(const uint32_t line, const uint32_t col, AstNode *p_ret_val)
    : AstNode{line, col}, ret_val(p_ret_val) {}

// TODO: You may use code snippets in AstDumper.cpp
void ReturnNode::print() {}

void ReturnNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void ReturnNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    if(ret_val != NULL)
        ret_val->accept(p_visitor);
}
