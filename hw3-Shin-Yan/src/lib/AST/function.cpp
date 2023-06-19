#include "AST/function.hpp"

// TODO
FunctionNode::FunctionNode(const uint32_t line, const uint32_t col, const char *p_name, const char *p_ret_type,
                            std::vector<AstNode*> *p_decl_list, AstNode* p_compound)
    : AstNode{line, col}, name(p_name), ret_type(p_ret_type), decl_list(p_decl_list), compound(p_compound) {}

// TODO: You may use code snippets in AstDumper.cpp
void FunctionNode::print() {}

void FunctionNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void FunctionNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    // TODO
    if(decl_list != NULL){
        for(auto &decl: *decl_list){
            decl->accept(p_visitor);
        }
    }

    if(compound != NULL)
        compound->accept(p_visitor);
}

const char *FunctionNode::getNameCStirng() const{return name.c_str(); }
const char *FunctionNode::getReturnTypeCString() const{return ret_type.c_str(); }
std::string FunctionNode::getPrototypeString() {
    DeclNode *dn;
    VariableNode *vn;
    std::string ret = "(";
    if(decl_list != NULL){
        for(int i = 0 ; i < decl_list->size(); i++){
            dn = dynamic_cast<DeclNode*>(decl_list->at(i));
            std::vector<AstNode*> *var_list = dn->getVarList();
            if(i != 0)
                ret+=", ";
            if(var_list != NULL){
                for(int j = 0 ; j < var_list->size() ;j++){
                    vn = dynamic_cast<VariableNode*>(var_list->at(j));
                    ret+= vn->getType();
                    if(j != var_list->size()-1)
                        ret+=", ";
                }
            }
        }
    }
    ret+=")";
    // printf("Final prototype is %s!\n", ret.c_str());
    return ret;
}