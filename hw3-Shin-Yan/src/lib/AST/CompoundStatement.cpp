#include "AST/CompoundStatement.hpp"

// TODO
CompoundStatementNode::CompoundStatementNode(const uint32_t line,
                                             const uint32_t col,
                                             std::vector<AstNode*> *p_decl_list,
                                             std::vector<AstNode*> *p_state_list)
    : AstNode{line, col}, decl_list(p_decl_list), state_list(p_state_list) {}

// TODO: You may use code snippets in AstDumper.cpp
void CompoundStatementNode::print() {}

void CompoundStatementNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void CompoundStatementNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    if(decl_list != NULL){
        for(auto &decl:*decl_list){
            decl->accept(p_visitor);
        }
    }
    if(state_list != NULL){
        for(auto &state:*state_list){
            state->accept(p_visitor);
        }
    }
}

int CompoundStatementNode::getDeclListLen(){
    return decl_list->size();
}

int CompoundStatementNode::getStateListLen(){
    return state_list->size();
}