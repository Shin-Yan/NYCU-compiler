#ifndef AST_FUNCTION_INVOCATION_NODE_H
#define AST_FUNCTION_INVOCATION_NODE_H

#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>
#include <string>
#include <vector>

struct checkArgReturn{
  int wrong_idx;
  std::string para;
  std::string arg;
}typedef checkArgReturn;

class FunctionInvocationNode final : public ExpressionNode {
  public:
    using ExprNodes = std::vector<std::unique_ptr<ExpressionNode>>;

  private:
    std::string m_name;
    ExprNodes m_args;

  public:
    ~FunctionInvocationNode() = default;
    FunctionInvocationNode(const uint32_t line, const uint32_t col,
                           const char *const p_name, ExprNodes &p_args)
        : ExpressionNode{line, col}, m_name(p_name), m_args(std::move(p_args)){}

    const char *getNameCString() const { return m_name.c_str(); }
    const int getArgumentsCount() const;
    const uint32_t getArgumentLocationCol(const int id) const;
    const char *getArgumentTypeCString(const int id) const;
    const int checkInvalidChildren() const;
    checkArgReturn *checkArgumentsType(const char *attribute) const;
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
};

#endif
