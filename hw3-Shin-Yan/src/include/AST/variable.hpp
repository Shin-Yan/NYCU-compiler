#ifndef __AST_VARIABLE_NODE_H
#define __AST_VARIABLE_NODE_H

#include "AST/ast.hpp"

class VariableNode : public AstNode {
  public:
    VariableNode(const uint32_t line, const uint32_t col, const char *p_name, 
                  const char *p_type, AstNode *p_constant
                 /* TODO: variable name, type, constant value */);
    ~VariableNode() = default;

    void print() override;
    void accept(AstNodeVisitor &p_visitor) override;
    void visitChildNodes(AstNodeVisitor &p_visitor);
    std::string getType();
    const char *getTypeCString();
    const char *getNameCString();

  private:
    // TODO: variable name, type, constant value
    const std::string name;
    const std::string type;
    AstNode *constant;
};

#endif
