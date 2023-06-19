#ifndef AST_PROGRAM_NODE_H
#define AST_PROGRAM_NODE_H

#include "AST/ast.hpp"

#include <string>

class ProgramNode final : public AstNode {
  public:
    ~ProgramNode() = default;
    ProgramNode(const uint32_t line, const uint32_t col,
                const char *p_name, const char *p_ret_type,
                std::vector<AstNode*> *p_decl_list, std::vector<AstNode*> *p_func_list,
                AstNode *p_compound
                /* TODO: return type, declarations, functions,
                 *       compound statement */);

    // visitor pattern version: const char *getNameCString() const;
    const char *getNameCString() const;
    void print() override;
    void accept(AstNodeVisitor &p_visitor) override;
    void visitChildNodes(AstNodeVisitor &p_visitor);

  private:
    const std::string name;
    // TODO: return type, declarations, functions, compound statement
    const std::string ret_type;
    std::vector<AstNode*> *decl_list;
    std::vector<AstNode*> *func_list;
    AstNode *compound;
};

#endif
