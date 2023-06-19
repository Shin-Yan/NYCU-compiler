#include "AST/VariableReference.hpp"
#include <string.h>
#include <algorithm>
#include <iostream>

int VariableReferenceNode::checkInvalidChildren(){
    for(auto &indice:m_indices){
        // std::cout << "Indices Type:" << indice->getPTypeCString() << std::endl;
        if(strcmp(indice->getPTypeCString(), "null") == 0)
            return 1;
    }
    return 0;
}

int VariableReferenceNode::getIndicesNum() const{
    return m_indices.size();
}

int VariableReferenceNode::checkNonIntegerIndices(){
    for(auto &i:m_indices){
        if(strcmp(i->getPTypeCString(), "integer")!=0)
            return i->getLocation().col;
    }
    return -1;
}

void VariableReferenceNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    auto visit_ast_node = [&](auto &ast_node) { ast_node->accept(p_visitor); };

    for_each(m_indices.begin(), m_indices.end(), visit_ast_node);
}
