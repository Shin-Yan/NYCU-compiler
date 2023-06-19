#include "AST/assignment.hpp"
#include "AST/ConstantValue.hpp"

const char *AssignmentNode::getConstantValueCString(){
    std::unique_ptr<ConstantValueNode> expr_const(dynamic_cast<ConstantValueNode *>(m_expr.get()));
    if(expr_const)
        m_expr.release();

    return expr_const->getConstantValueCString();
}

const char *AssignmentNode::getLvalueNameCString(){
    return m_lvalue->getNameCString();
}

const char *AssignmentNode::getLvalueTypeCString(){
    return m_lvalue->getPTypeCString();
}

const char *AssignmentNode::getRvalueTypeCString(){
    return m_expr->getPTypeCString();
}

const uint32_t AssignmentNode::getLvalueLocationCol() const{
    return m_lvalue->getLocation().col;
};

const uint32_t AssignmentNode::getRvalueLocationCol() const{
    return m_expr->getLocation().col;
}

const int AssignmentNode::checkInvalidLvalue() const{
    std::string lvalue = m_lvalue->getPTypeCString();
    if(lvalue == "null")
        return 1;
    return 0;
}

const int AssignmentNode::checkInvalidRvalue() const{
    std::string rvalue = m_expr->getPTypeCString();
    if(rvalue == "null")
        return 1;
    return 0;
}

const int AssignmentNode::checkLvalueScalarType() const{
    std::string lvalue = m_lvalue->getPTypeCString();
    if(lvalue == "integer" || lvalue == "real" || lvalue == "string" || lvalue == "boolean")
        return 1;
    return 0;
}

const int AssignmentNode::checkRvalueScalarType() const{
    std::string rvalue = m_expr->getPTypeCString();
    if(rvalue == "integer" || rvalue == "real" || rvalue == "string" || rvalue == "boolean")
        return 1;
    return 0;
}

const int AssignmentNode::checkCompatibleLRvalueType() const{
    std::string lvalue = m_lvalue->getPTypeCString(), rvalue = m_expr->getPTypeCString();
    if(lvalue == rvalue)
        return 1;
    else if(lvalue == "real" && rvalue == "integer")
        return 1;
    return 0;
}

void AssignmentNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    m_lvalue->accept(p_visitor);
    m_expr->accept(p_visitor);
}
