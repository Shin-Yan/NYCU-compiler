#include "AST/variable.hpp"

// TODO
VariableNode::VariableNode(const uint32_t line, const uint32_t col, const char *p_name,
                            const char *p_type, AstNode *p_constant)
    : AstNode{line, col}, name(p_name), type(p_type), constant(p_constant) {}

// TODO: You may use code snippets in AstDumper.cpp
void VariableNode::print() {}

void VariableNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void VariableNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    if(constant != NULL)
        constant->accept(p_visitor);
}

std::string VariableNode::getType(){
    return type;
}

const char *VariableNode::getTypeCString(){
    return type.c_str();
}

const char *VariableNode::getNameCString(){
    return name.c_str();
}
