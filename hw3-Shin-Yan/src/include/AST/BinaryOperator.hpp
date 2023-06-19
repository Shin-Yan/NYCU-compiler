#ifndef __AST_BINARY_OPERATOR_NODE_H
#define __AST_BINARY_OPERATOR_NODE_H

#include "AST/expression.hpp"

#include <memory>

class BinaryOperatorNode : public ExpressionNode {
  public:
    BinaryOperatorNode(const uint32_t line, const uint32_t col, Binary_Op p_op, 
                        AstNode *p_left, AstNode *p_right/* TODO: operator, expressions */);
    ~BinaryOperatorNode() = default;
    
    void print() override;
    void accept(AstNodeVisitor &p_visitor) override;
    void visitChildNodes(AstNodeVisitor &p_visitor);
    const char *getOperatorCString() const;

  private:
    // TODO: operator, expressions
    Binary_Op op;
    AstNode *left;
    AstNode *right;
};

#endif
