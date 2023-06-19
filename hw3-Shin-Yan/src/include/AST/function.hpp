#ifndef __AST_FUNCTION_NODE_H
#define __AST_FUNCTION_NODE_H

#include "AST/ast.hpp"
#include "decl.hpp"
#include "variable.hpp"
class FunctionNode : public AstNode {
  public:
    FunctionNode(const uint32_t line, const uint32_t col, const char *p_name, const char *p_ret_type,
                  std::vector<AstNode*> *p_decl_list,  AstNode *p_compound);
    ~FunctionNode() = default;
    void print() override;
    void accept(AstNodeVisitor &p_visitor) override;
    void visitChildNodes(AstNodeVisitor &p_visitor);
    const char *getNameCStirng() const;
    const char *getReturnTypeCString() const;
    std::string getPrototypeString() ;

  private:
    // TODO: name, declarations, return type, compound statement
    const std::string name;
    const std::string ret_type;
    std::vector<AstNode*> *decl_list;
    AstNode* compound;
};

#endif
