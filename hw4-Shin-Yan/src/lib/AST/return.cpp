#include "AST/return.hpp"

const int ReturnNode::checkInvalidRetrunType() const{
    std::string ret = m_ret_val->getPTypeCString();
    if(ret == "null")
        return 1;
    return 0;
}

const char *ReturnNode::getReturnTypeCString() const{
    return m_ret_val->getPTypeCString();
}

const uint32_t ReturnNode::getReturnLocationCol() const{
    return m_ret_val->getLocation().col;
}

void ReturnNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    m_ret_val->accept(p_visitor);
}
