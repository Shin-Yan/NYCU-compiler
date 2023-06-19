#ifndef __AST_UNARY_OPERATOR_NODE_H
#define __AST_UNARY_OPERATOR_NODE_H

#include "AST/expression.hpp"

class UnaryOperatorNode : public ExpressionNode {
  public:
    UnaryOperatorNode(const uint32_t line, const uint32_t col, Unary_Op p_op, AstNode *p_operand
                      /* TODO: operator, expression */);
    ~UnaryOperatorNode() = default;
    
    void print() override;
    void accept(AstNodeVisitor &p_visitor) override;
    void visitChildNodes(AstNodeVisitor &p_visitor);
    const char *getOperatorCString() const;

  private:
    // TODO: operator, expression
    Unary_Op op;
    AstNode *operand;
};

#endif
