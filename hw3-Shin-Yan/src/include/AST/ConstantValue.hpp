#ifndef __AST_CONSTANT_VALUE_NODE_H
#define __AST_CONSTANT_VALUE_NODE_H

#include "AST/expression.hpp"

class ConstantValueNode : public ExpressionNode {
  public:
    ConstantValueNode(const uint32_t line, const uint32_t col, Constant_Value p_const_val
                      /* TODO: constant value */);
    ~ConstantValueNode() = default;

    void print() override;
    void accept(AstNodeVisitor &p_visitor) override; 
    const char *getCstringValue()const;

  private:
    // TODO: constant value
    Constant_Value const_val;
};

#endif
