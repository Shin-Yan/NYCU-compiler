#include "AST/VariableReference.hpp"

// TODO
VariableReferenceNode::VariableReferenceNode(const uint32_t line,const uint32_t col,
                                                const char *p_name, std::vector<AstNode*> *p_expr_list)
    : ExpressionNode{line, col}, name(p_name), expr_list(p_expr_list) {}

// TODO
// VariableReferenceNode::VariableReferenceNode(const uint32_t line,
//                                              const uint32_t col)
//     : ExpressionNode{line, col} {}

// TODO: You may use code snippets in AstDumper.cpp
void VariableReferenceNode::print() {}

void VariableReferenceNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void VariableReferenceNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    if(expr_list != NULL){
        for(auto &expr:*expr_list){
            expr->accept(p_visitor);
        }
    }
}

const char *VariableReferenceNode::getNameCString() const{
    return name.c_str();
}