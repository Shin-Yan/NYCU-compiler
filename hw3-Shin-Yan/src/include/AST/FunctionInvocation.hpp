#ifndef __AST_FUNCTION_INVOCATION_NODE_H
#define __AST_FUNCTION_INVOCATION_NODE_H

#include "AST/expression.hpp"

class FunctionInvocationNode : public ExpressionNode {
  public:
    FunctionInvocationNode(const uint32_t line, const uint32_t col,
                            const char* p_name, std::vector<AstNode*> *p_expr_list
                           /* TODO: function name, expressions */);
    ~FunctionInvocationNode() = default;
    
    void print() override;
    void accept(AstNodeVisitor &p_visitor) override;
    void visitChildNodes(AstNodeVisitor &p_visitor);
    const char *getNameCString() const;

  private:
    // TODO: function name, expressions
    const std::string name;
    std::vector<AstNode*> *expr_list;
};

#endif
