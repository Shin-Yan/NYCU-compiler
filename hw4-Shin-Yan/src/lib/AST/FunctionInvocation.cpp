#include "AST/FunctionInvocation.hpp"

#include <algorithm>
#include <string.h>

const int FunctionInvocationNode::getArgumentsCount() const{
    return m_args.size();
}

const uint32_t FunctionInvocationNode::getArgumentLocationCol(const int id) const{
    return m_args[id]->getLocation().col;
}

const char *FunctionInvocationNode::getArgumentTypeCString(const int id) const{
    return m_args[id]->getPTypeCString();
}

const int FunctionInvocationNode::checkInvalidChildren() const{
    for(auto &i:m_args){
        if(strcmp(i->getPTypeCString(), "null") == 0)
            return 1;
    }
    return 0;
}

checkArgReturn *FunctionInvocationNode::checkArgumentsType(const char *attribute) const{
    std::string attri_string(attribute);
    checkArgReturn *ret = new checkArgReturn;
    bool flag_wrong = false;
    size_t left = 0, right = attri_string.find(',');
    for(int i = 0 ; i < m_args.size() ; i++){
        if(right == std::string::npos)
            right = attri_string.size();
        std::string arg = m_args[i]->getPTypeCString(), para = attri_string.substr(left, right-left);
        
        if(para == "real"){
            if(arg != "real" && arg != "integer"){
                ret->wrong_idx = i;
                ret->para = para;
                ret->arg = arg;
                flag_wrong = true;
                break;
            }
        }
        else if(para != arg){
            ret->wrong_idx = i;
            ret->para = para;
            ret->arg = arg;
            flag_wrong = true;
            break;
        }
        if(i != m_args.size()-1){
            attri_string = attri_string.substr(right+2);
            right = attri_string.find(',');
        }
    }
    if(!flag_wrong)
        ret->wrong_idx = -1;
    return ret;
}

void FunctionInvocationNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    auto visit_ast_node = [&](auto &ast_node) { ast_node->accept(p_visitor); };

    for_each(m_args.begin(), m_args.end(), visit_ast_node);
}
