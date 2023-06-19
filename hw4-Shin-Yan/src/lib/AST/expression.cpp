#include "AST/expression.hpp"

const char *ExpressionNode::getPTypeCString(){
    if(type != nullptr)
        return type->getPTypeCString();
    else
        return "null";
}

void ExpressionNode::setNodeType(std::string type_string){
    if(type_string == "void")
        type = std::make_shared<PType>(PType::PrimitiveTypeEnum::kVoidType);
    else if(type_string == "boolean")
       type = std::make_shared<PType>(PType::PrimitiveTypeEnum::kBoolType);
    else if(type_string == "string")
        type = std::make_shared<PType>(PType::PrimitiveTypeEnum::kStringType);
    else if(type_string == "integer")
        type = std::make_shared<PType>(PType::PrimitiveTypeEnum::kIntegerType);
    else if(type_string == "real")
        type = std::make_shared<PType>(PType::PrimitiveTypeEnum::kRealType);
}

void ExpressionNode::setNodeTypeDimensions(std::vector<uint64_t> dims){
    type->setDimensions(dims);
}