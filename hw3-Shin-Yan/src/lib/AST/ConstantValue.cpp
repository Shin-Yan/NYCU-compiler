#include "AST/ConstantValue.hpp"
#include <string.h>

#define strlength 100
char returnString[strlength];
// TODO
ConstantValueNode::ConstantValueNode(const uint32_t line, const uint32_t col, Constant_Value p_const_val)
    : ExpressionNode{line, col}, const_val(p_const_val) {}

// TODO: You may use code snippets in AstDumper.cpp
void ConstantValueNode::print() {}

void ConstantValueNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

const char* ConstantValueNode::getCstringValue()const{
    memset(returnString, 0 ,strlength);
    if(const_val.bool_type)
        return const_val.str_value;
    if(const_val.real_type)
        sprintf(returnString,"%f",const_val.real_value);
    else if(const_val.int_type)
        sprintf(returnString,"%d",const_val.int_value);
    else if(const_val.str_type)
        return const_val.str_value;

    return returnString;
}