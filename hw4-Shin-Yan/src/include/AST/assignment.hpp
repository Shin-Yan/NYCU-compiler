#ifndef AST_ASSIGNMENT_NODE_H
#define AST_ASSIGNMENT_NODE_H

#include "AST/ast.hpp"
#include "AST/expression.hpp"
#include "AST/VariableReference.hpp"

#include <memory>

class AssignmentNode final : public AstNode {
  private:
    std::unique_ptr<VariableReferenceNode> m_lvalue;
    std::unique_ptr<ExpressionNode> m_expr;

  public:
    ~AssignmentNode() = default;
    AssignmentNode(const uint32_t line, const uint32_t col,
                   VariableReferenceNode *p_var_ref, ExpressionNode *p_expr)
        : AstNode{line, col}, m_lvalue(p_var_ref), m_expr(p_expr){}
    const char *getConstantValueCString();
    const char *getLvalueNameCString();
    const char *getLvalueTypeCString();
    const char *getRvalueTypeCString();
    const uint32_t getLvalueLocationCol() const;
    const uint32_t getRvalueLocationCol() const;
    const int checkInvalidLvalue() const;
    const int checkInvalidRvalue() const;
    const int checkLvalueScalarType() const;
    const int checkRvalueScalarType() const;
    const int checkCompatibleLRvalueType() const;
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
};

#endif
