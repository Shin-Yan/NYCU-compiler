#ifndef AST_PRINT_NODE_H
#define AST_PRINT_NODE_H

#include "AST/ast.hpp"
#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>

class PrintNode final : public AstNode {
  private:
    std::unique_ptr<ExpressionNode> m_target;

  public:
    ~PrintNode() = default;
    PrintNode(const uint32_t line, const uint32_t col,
              ExpressionNode *p_target)
        : AstNode{line, col}, m_target(p_target){}
    const int checkInvalidChildren() const;
    const int checkTargetScalarType() const;
    const uint32_t getTargetLocationCol() const;
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
};

#endif
