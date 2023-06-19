#ifndef AST_EXPRESSION_NODE_H
#define AST_EXPRESSION_NODE_H

#include "AST/ast.hpp"
#include "AST/PType.hpp"

class ExpressionNode : public AstNode {
  public:
    ~ExpressionNode() = default;
    ExpressionNode(const uint32_t line, const uint32_t col)
        : AstNode{line, col} {}
    const char *getPTypeCString();
    void setNodeType(std::string type_string);
    void setNodeTypeDimensions(std::vector<uint64_t> dims);
  protected:
    // for carrying type of result of an expression
    // TODO: for next assignment
    PTypeSharedPtr type = nullptr;
};

#endif
