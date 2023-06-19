#include "AST/BinaryOperator.hpp"

// TODO
BinaryOperatorNode::BinaryOperatorNode(const uint32_t line, const uint32_t col, Binary_Op p_op, 
                                        AstNode *p_left, AstNode *p_right)
    : ExpressionNode{line, col}, op(p_op), left(p_left), right(p_right) {}

// TODO: You may use code snippets in AstDumper.cpp
void BinaryOperatorNode::print() {}

void BinaryOperatorNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void BinaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    if(left != NULL)
        left->accept(p_visitor);

    if(right != NULL)
        right->accept(p_visitor);
}

const char *BinaryOperatorNode::getOperatorCString() const{
    if(op.mul)
        return "*";
    else if (op.div)
        return "/";
    else if(op.mod)
        return "mod";
    else if(op.add)
        return "+";
    else if(op.sub)
        return "-";
    else if(op.lt)
        return "<";
    else if(op.le)
        return "<=";
    else if(op.eq)
        return "=";
    else if(op.ne)
        return "<>";
    else if(op.ge)
        return ">=";
    else if(op.gt)
        return ">";
    else if(op.AND)
        return "and";
    else if(op.OR)
        return "or";
}