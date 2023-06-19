#ifndef __AST_ASSIGNMENT_NODE_H
#define __AST_ASSIGNMENT_NODE_H

#include "AST/ast.hpp"

class AssignmentNode : public AstNode {
  public:
    AssignmentNode(const uint32_t line, const uint32_t col, AstNode *p_var_ref, AstNode *p_expr
                   /* TODO: variable reference, expression */);
    ~AssignmentNode() = default;
    
    void print() override;
    void accept(AstNodeVisitor &p_visitor) override;
    void visitChildNodes(AstNodeVisitor &p_visitor);

  private:
    // TODO: variable reference, expression
    AstNode *var_ref;
    AstNode *expr;
};

#endif
