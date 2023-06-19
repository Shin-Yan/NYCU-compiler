#ifndef __AST_COMPOUND_STATEMENT_NODE_H
#define __AST_COMPOUND_STATEMENT_NODE_H

#include "AST/ast.hpp"

class CompoundStatementNode : public AstNode {
  public:
    CompoundStatementNode(const uint32_t line, const uint32_t col,
                          std::vector<AstNode*> *p_decl_list,
                          std::vector<AstNode*> *p_state_list
                          /* TODO: declarations, statements */);
    ~CompoundStatementNode() = default;
    
    void print() override;
    void accept(AstNodeVisitor &p_visitor) override;
    void visitChildNodes(AstNodeVisitor &p_visitor);
    int getDeclListLen();
    int getStateListLen();

  private:
    // TODO: declarations, statements
    std::vector<AstNode*> *decl_list;
    std::vector<AstNode*> *state_list;
};

#endif
