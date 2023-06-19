#ifndef __AST_DECL_NODE_H
#define __AST_DECL_NODE_H

#include "AST/ast.hpp"

class DeclNode : public AstNode {
  public:
    // variable declaration
    DeclNode(const uint32_t line, const uint32_t col, std::vector<AstNode*> *p_var_list
             /* TODO: identifiers, type */);

    // constant variable declaration
    //DeclNode(const uint32_t, const uint32_t col
    //         /* TODO: identifiers, constant */);

    ~DeclNode() = default;
    
    void print() override;
    void accept(AstNodeVisitor &p_visitor) override;
    void visitChildNodes(AstNodeVisitor &p_visitor);
    std::vector<AstNode*> *getVarList();

  private:
    // TODO: variables
    std::vector<AstNode*> *var_list;
};

#endif
