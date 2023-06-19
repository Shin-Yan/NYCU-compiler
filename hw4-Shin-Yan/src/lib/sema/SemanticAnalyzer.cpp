#include "sema/SemanticAnalyzer.hpp"
#include "visitor/AstNodeInclude.hpp"
#include "AST/PType.hpp"
#include <iostream>
#include <string.h>

// #define    debug1(x)            std::cout<< "first traverse " << x << " node" <<std::endl;
// #define    debug2(x)            std::cout<< "back to " << x << " node" <<std::endl;

static void dumpDemarcation(const char symb){
    for(int i = 0 ; i < 110 ; ++i){
        std::cout<<symb;
    }
    std::cout<<std::endl;
}

static void showErrorMessage(uint32_t line, uint32_t column, std::string error_message){
    std::cerr << "<Error> Found in line "<< line << ", column " << column << ": " << error_message << std::endl;
    std::cerr <<"    "<<source_code[line] << std::endl;
    for (int i = 1; i < column + 4; i++)
        std::cerr << " ";
    std::cerr << "^" << std::endl;
}

void SymbolEntry::dumpEntry(void){
    std::string level_dump;
    if(level==0) level_dump = "(global)";
    else level_dump = "(local)";
    printf(DUMPENTRY, variableName.c_str(), kind.c_str(), level, level_dump.c_str(), type.c_str(), attribute.c_str());
}

const char *SymbolEntry::getNameCString(void) const{
    return variableName.c_str();
}

const char *SymbolEntry::getTypeCString(void) const{
    return type.c_str();
}

const char *SymbolEntry::getKindCString(void) const{
    return kind.c_str();
}

const char *SymbolEntry::getAttributeCString(void) const{
    return attribute.c_str();
}

int SymbolEntry::getTypeDimension() const{
    int dim = 0;
    for(auto c:type){
        if(c == '[')
            dim++;
    }
    return dim;
}

void SymbolEntry::setKindString(const char *p_kind){
    kind = p_kind;
}

void SymbolEntry::setAttributeString(const char *p_attribute){
    attribute = p_attribute;
}

void SymbolEntry::getNewTypeDimensions(std::vector<uint64_t> &dims ,int ignore_before) const{
    int count = 0;
    for(int i = 0 ; i < type.size() ;i++){
        if(type[i]!='[')
            continue;
        if(count != ignore_before){
            count++;
            continue;
        }
        uint64_t d = 0;
        i++;
        while(type[i] != ']'){
            d*=10;
            d+=type[i]-'0';
            i++;
        }
        dims.push_back(d);
    }
}

int SymbolEntry::getFunctionParameterCount() const{
    int num;
    if(attribute.size() == 0)
        return 0;
    else
        num = 1;
    for(auto c:attribute){
        if(c == ',')
            num++;
    }
    return num;
}

void SymbolTable::addSymbol(SymbolEntry *entry){
    entries.push_back(entry);
}

void SymbolTable::dumpSymbol(void){
    dumpDemarcation('=');
    printf(DUMPFORMAT, "Name", "Kind", "Level", "Type", "Attribute");
    dumpDemarcation('-');
    for(auto it:entries)
        it->dumpEntry();
    dumpDemarcation('-');
}

int SymbolTable::checkRedeclaration(const char *p_name) const{
    // return 1 if redeclaration exist else 0
    for(auto i:entries){
        if(strcmp(i->getNameCString(), p_name)==0)
            return 1;
    }
    return 0;
}

void SymbolTable::addErrorSymbol(const char *p_name) {
    error_declaration.push_back(p_name);
}

int SymbolTable::checkErrorDeclare(const char *p_name) const{
    // return 1 if the name is declared but error else 0
    for(auto err:error_declaration){
        if(strcmp(err, p_name)==0)
            return 1;
    }
    return 0;
}

SymbolEntry *SymbolTable::getEntry(const char *p_name){
    for(auto i:entries){
        if(strcmp(i->getNameCString(), p_name)==0)
            return i;
    }
    return NULL;
}

void SymbolManager::pushScope(SymbolTable *new_scope){
    tables.push(new_scope);
}

void SymbolManager::popScope(void){
    tables.pop();
}

SymbolTable* SymbolManager::topScope(void){
    return tables.top();
}

int SymbolManager::checkLoopVariableRedeclaration(const char *p_name){
    for(auto var:loop_var){
        if(strcmp(var, p_name)==0)
            return 1;
    }
    return 0;
}

int SymbolManager::checkConstant(const char *p_name){
    for(auto cons:constants){
        if(strcmp(cons, p_name) == 0)
            return 1;
    }
    return 0;
}

void SymbolManager::push_loop_var(const char *p_name){
    loop_var.push_back(p_name);
}

void SymbolManager::pop_loop_var(){
    loop_var.pop_back();
}

void SymbolManager::push_constant(const char *p_name){
    constants.push_back(p_name);
}

void SymbolManager::pop_constant(){
    constants.pop_back();
}

int SymbolManager::getScopeSize(){
    return tables.size();
}

void SemanticAnalyzer::visit(ProgramNode &p_program) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // debug1("p_program");
    symbol_manager = new SymbolManager();
    temp_manager = new SymbolManager();
    current_table = new SymbolTable();
    current_entry = new SymbolEntry(p_program.getNameCString(), "program", m_level, "void", "");
    current_table->addSymbol(current_entry);
    symbol_manager->pushScope(current_table);

    p_program.visitChildNodes(*this);
    if(opt_dump){
        current_table = symbol_manager->topScope();
        current_table->dumpSymbol();
    }
    symbol_manager->popScope();
}

void SemanticAnalyzer::visit(DeclNode &p_decl) {
    // debug1("p_decl");
    p_decl.visitChildNodes(*this);
    // debug2("p_decl");
}

void SemanticAnalyzer::visit(VariableNode &p_variable) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // debug1("p_variable");

    current_table = symbol_manager->topScope();
    if(current_table->checkRedeclaration(p_variable.getNameCString()) 
    || symbol_manager->checkLoopVariableRedeclaration(p_variable.getNameCString())){
        error_happen = true;
        std::string error_message = "symbol '" + std::string(p_variable.getNameCString()) + "' is redeclared";
        showErrorMessage(p_variable.getLocation().line, p_variable.getLocation().col, error_message);
    }
    else{
        std::string type;
        if(flag_function)
            type = "parameter";
        else if(flag_for){
            type = "loop_var";
            flag_for = false;
            symbol_manager->push_loop_var(p_variable.getNameCString());
        }
        else
            type = "variable";
        current_entry = new SymbolEntry(p_variable.getNameCString(), type.c_str(), m_level, p_variable.getTypeCString(), "");
        current_table->addSymbol(current_entry);
        flag_var = true;
        p_variable.visitChildNodes(*this);
        if(strcmp(current_entry->getKindCString(),"constant")==0)
            symbol_manager->push_constant(p_variable.getNameCString());
        flag_var = false;
        // debug2("p_variable");
    }
    
    if(p_variable.checkInvalidDimensions()){
        error_happen = true;
        std::string error_message = "'" + std::string(p_variable.getNameCString()) 
                    + "' declared as an array with an index that is not greater than 0";
        showErrorMessage(p_variable.getLocation().line, p_variable.getLocation().col, error_message);
        current_table->addErrorSymbol(p_variable.getNameCString());
    }
    
}

void SemanticAnalyzer::visit(ConstantValueNode &p_constant_value) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    // debug1("p_const");
    if(flag_var){
        current_entry->setKindString("constant");
        current_entry->setAttributeString(p_constant_value.getConstantValueCString());
    }
    // else{
    //     p_constant_value.setNodeType("");
    // }
    // debug2("p_const");
}

void SemanticAnalyzer::visit(FunctionNode &p_function) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // debug1("p_function");

    current_table = symbol_manager->topScope();
    // Redeclaration
    if(current_table->checkRedeclaration(p_function.getNameCString())){
        error_happen = true;
        std::string error_message="symbol '" + std::string(p_function.getNameCString()) + "' is redeclared";
        showErrorMessage(p_function.getLocation().line, p_function.getLocation().col, error_message);
    }
    // No Redeclaration
    else{
        current_entry = new SymbolEntry(p_function.getNameCString(), "function", m_level, p_function.getPTypeCString(), p_function.getArgumentCString());
        current_table->addSymbol(current_entry);
    }
    current_table = new SymbolTable();
    symbol_manager->pushScope(current_table);
    m_level++;
    flag_function = true;
    flag_in_function = true;
    current_function_name = p_function.getNameCString();
    p_function.visitChildNodes(*this);
    // debug2("p_function");
    current_function_name = "";
    flag_in_function = false;
    flag_function = false;
    if(opt_dump){
        current_table = symbol_manager->topScope();
        current_table->dumpSymbol();
    }
    symbol_manager->popScope();
    m_level--;
}

void SemanticAnalyzer::visit(CompoundStatementNode &p_compound_statement) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // debug1("p_compund");
    bool flag_pop = true;
    // The statement is not under a function
    if(!flag_function){
        current_table = new SymbolTable();
        symbol_manager->pushScope(current_table);
        m_level++;
    }
    // The statement is under a function
    else{
        flag_function = false;
        flag_pop = false;
    }

    p_compound_statement.visitChildNodes(*this);
    // debug2("p_compound");

    // if the statement is not under a function, need to dump the scope information
    if(flag_pop){
        if(opt_dump){
            current_table = symbol_manager->topScope();
            current_table->dumpSymbol();
        }
        symbol_manager->popScope();
        m_level--;
    }
}

void SemanticAnalyzer::visit(PrintNode &p_print) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // debug1("p_print");
    p_print.visitChildNodes(*this);

    if(p_print.checkInvalidChildren() == 1)
        return;
    
    if(p_print.checkTargetScalarType() == 0){
        error_happen = true;
        std::string error_message = "expression of print statement must be scalar type";
        showErrorMessage(p_print.getLocation().line, p_print.getTargetLocationCol(), error_message);
    }
    // debug2("p_print");
}

void SemanticAnalyzer::visit(BinaryOperatorNode &p_bin_op) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // debug1("p_binOp");
    p_bin_op.visitChildNodes(*this);

    if(p_bin_op.checkInvalidChildren())
        return;
    
    std::string left_type, right_type, operator_type;
    left_type = p_bin_op.getLeftTypeCString();
    right_type = p_bin_op.getRightTypeCString();
    operator_type = p_bin_op.getOpCString();
    std::string error_message = "invalid operands to binary operator '" + operator_type 
                                        + "' ('" + left_type + "' and '" + right_type + "')";
    // Arithmetic
    if(operator_type == "+" || operator_type == "-" || operator_type == "*" || operator_type == "/"){
        if(operator_type == "+" && left_type == "string" && right_type == "string")
            p_bin_op.setNodeType("string");
        else if(left_type == "integer" && right_type == "integer")
            p_bin_op.setNodeType("integer");
        else if(left_type == "real" && right_type == "integer")
            p_bin_op.setNodeType("real");
        else if(left_type == "integer" && right_type == "real")
            p_bin_op.setNodeType("real");
        else if(left_type == "real" && right_type == "real")
            p_bin_op.setNodeType("real");
        else{
            error_happen = true;
            showErrorMessage(p_bin_op.getLocation().line, p_bin_op.getLocation().col, error_message);
        }
    }
    // Modulo
    else if(operator_type == "mod"){
        if(left_type == "integer" && right_type == "integer")
            p_bin_op.setNodeType("integer");
        else{
            error_happen = true;
            showErrorMessage(p_bin_op.getLocation().line, p_bin_op.getLocation().col, error_message);
        }
    }
    // Logical
    else if(operator_type == "and" || operator_type == "or" ){
        if(left_type == "boolean" && right_type == "boolean")
            p_bin_op.setNodeType("boolean");
        else{
            error_happen = true;
            showErrorMessage(p_bin_op.getLocation().line, p_bin_op.getLocation().col, error_message);
        }
    }
    // Comparison
    else{
        if(left_type == "integer" && right_type == "integer")
            p_bin_op.setNodeType("boolean");
        else if(left_type == "real" && right_type == "integer")
            p_bin_op.setNodeType("boolean");
        else if(left_type == "integer" && right_type == "real")
            p_bin_op.setNodeType("boolean");
        else if(left_type == "real" && right_type == "real")
            p_bin_op.setNodeType("boolean");
        else{
            error_happen = true;
            showErrorMessage(p_bin_op.getLocation().line, p_bin_op.getLocation().col, error_message);
        }
    }
    // debug2("p_binOp");
}

void SemanticAnalyzer::visit(UnaryOperatorNode &p_un_op) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // debug1("p_un_op");
    p_un_op.visitChildNodes(*this);
    if(p_un_op.checkInvalidChildren() == 1)
        return;
    std::string operand_type, op_type, error_message;
    operand_type = p_un_op.getOperandTypeCString();
    op_type = p_un_op.getOpCString();
    error_message = "invalid operand to unary operator '" + op_type + "' ('" + operand_type + "')";
    // neg operator
    if(op_type == "neg"){
        if(operand_type == "integer" || operand_type == "real")
            p_un_op.setNodeType(operand_type);
        else{
            error_happen = true;
            showErrorMessage(p_un_op.getLocation().line, p_un_op.getLocation().col, error_message);
        }
    }
    // not operator
    else{
        if(operand_type == "boolean")
            p_un_op.setNodeType(operand_type);
        else{
            error_happen = true;
            showErrorMessage(p_un_op.getLocation().line, p_un_op.getLocation().col, error_message);
        }
    }
    // debug2("p_un_op");
}

void SemanticAnalyzer::visit(FunctionInvocationNode &p_func_invocation) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // debug1("p_func_invocation");
    p_func_invocation.visitChildNodes(*this);

    while(symbol_manager->getScopeSize() != 0){
        current_table = symbol_manager->topScope();
        symbol_manager->popScope();
        temp_manager->pushScope(current_table);

        if(current_table->checkRedeclaration(p_func_invocation.getNameCString()) == 0 && symbol_manager->getScopeSize() != 0)
            continue;
        else if(current_table->checkRedeclaration(p_func_invocation.getNameCString()) == 0 && symbol_manager->getScopeSize() == 0){
            error_happen = true;
            std::string error_message = "use of undeclared symbol '" + std::string(p_func_invocation.getNameCString()) + "'";
            showErrorMessage(p_func_invocation.getLocation().line, p_func_invocation.getLocation().col, error_message); 
            break;
        }

        current_entry = current_table->getEntry(p_func_invocation.getNameCString());
        if(strcmp(current_entry->getKindCString(), "function") != 0){
            error_happen = true;
            std::string error_message = "call of non-function symbol '" + std::string(p_func_invocation.getNameCString()) +"'";
            showErrorMessage(p_func_invocation.getLocation().line, p_func_invocation.getLocation().col, error_message);
            break;
        }
        if(current_entry->getFunctionParameterCount() != p_func_invocation.getArgumentsCount()){
            error_happen = true;
            std::string error_message = "too few/much arguments provided for function '" + std::string(p_func_invocation.getNameCString()) + "'";
            showErrorMessage(p_func_invocation.getLocation().line, p_func_invocation.getLocation().col, error_message);
            break;
        }
        if(p_func_invocation.checkInvalidChildren() == 1)
            break;
        checkArgReturn *ret = p_func_invocation.checkArgumentsType(current_entry->getAttributeCString());
        if(ret->wrong_idx != -1){
            error_happen = true;
            uint32_t wrong_col = p_func_invocation.getArgumentLocationCol(ret->wrong_idx);
            std::string error_message = "incompatible type passing '" + ret->arg + "' to parameter of type '" + ret->para +"'";
            showErrorMessage(p_func_invocation.getLocation().line, wrong_col, error_message);
            break;
        }

        std::string return_type = current_entry->getTypeCString();
        p_func_invocation.setNodeType(return_type);
        break; 
    }

    while( temp_manager->getScopeSize()!=0){
        symbol_manager->pushScope(temp_manager->topScope());
        temp_manager->popScope();
    }
    // debug2("p_func_invocation");
}

void SemanticAnalyzer::visit(VariableReferenceNode &p_variable_ref) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // debug1("p_var_ref");
    p_variable_ref.visitChildNodes(*this);
    

    // There are some invalid children types
    if(p_variable_ref.checkInvalidChildren()) return;
    // debug2("p_var_ref");
    current_table = symbol_manager->topScope();
    // The variable reference is declared with error
    if(current_table->checkErrorDeclare(p_variable_ref.getNameCString())) return;

    while(symbol_manager->getScopeSize()!=0){
        current_table = symbol_manager->topScope();
        symbol_manager->popScope();
        temp_manager->pushScope(current_table);

        // if var is not defined in this scope, find it in outer scope
        if(current_table->checkRedeclaration(p_variable_ref.getNameCString())==0 && symbol_manager->getScopeSize() != 0)
            continue;
        else if(current_table->checkRedeclaration(p_variable_ref.getNameCString())==0 && symbol_manager->getScopeSize() == 0){
            error_happen = true;
            std::string error_message = "use of undeclared symbol '" + std::string(p_variable_ref.getNameCString()) + "'";
            showErrorMessage(p_variable_ref.getLocation().line, p_variable_ref.getLocation().col, error_message);
            break;
        }
        current_entry = current_table->getEntry(p_variable_ref.getNameCString());
        if(strcmp(current_entry->getKindCString(), "program")==0 || strcmp(current_entry->getKindCString(), "function")==0){
            error_happen = true;
            std::string error_message = "use of non-variable symbol '" + std::string(p_variable_ref.getNameCString()) +"'";
            showErrorMessage(p_variable_ref.getLocation().line, p_variable_ref.getLocation().col, error_message);
            break;
        }
        int dim = p_variable_ref.getIndicesNum();
        if(dim > current_entry->getTypeDimension()){
            error_happen = true;
            std::string error_message = "there is an over array subscript on '" + std::string(p_variable_ref.getNameCString()) + "'";
            showErrorMessage(p_variable_ref.getLocation().line, p_variable_ref.getLocation().col, error_message);
            break;
        }
        int nonIntCol = p_variable_ref.checkNonIntegerIndices();
        // There are some non integer indices
        if(nonIntCol != -1){
            error_happen = true;
            std::string error_message = "index of array reference must be an integer";
            showErrorMessage(p_variable_ref.getLocation().line, (uint32_t)nonIntCol, error_message);
            break;
        }
        // Set the type of variable reference
        PTypeSharedPtr p_type;
        std::string primitive_type, type_string;
        type_string = current_entry->getTypeCString();
        if(type_string.find("[") != std::string::npos){
            if(type_string.substr(0,4) == "void")
                primitive_type = "void";
            if(type_string.substr(0,7) == "boolean")
                primitive_type = "boolean";
            if(type_string.substr(0,6) == "string")
                primitive_type = "string";
            if(type_string.substr(0,7) == "integer")
                primitive_type = "integer";
            if(type_string.substr(0,4) == "real")
                primitive_type = "real";
        }
        else
            primitive_type = type_string;

        p_variable_ref.setNodeType(primitive_type);
        std::vector<uint64_t> dims;
        current_entry->getNewTypeDimensions(dims, p_variable_ref.getIndicesNum());
        p_variable_ref.setNodeTypeDimensions(dims);
        break;
    }
    while( temp_manager->getScopeSize()!=0){
        symbol_manager->pushScope(temp_manager->topScope());
        temp_manager->popScope();
    }
}

void SemanticAnalyzer::visit(AssignmentNode &p_assignment) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // debug1("p_assignment");
    p_assignment.visitChildNodes(*this);

    if(p_assignment.checkInvalidLvalue() == 1){
        flag_loop_var = false;
        return;
    }
    
    if(p_assignment.checkLvalueScalarType() == 0){
        error_happen = true;
        std::string error_message = "array assignment is not allowed";
        showErrorMessage(p_assignment.getLocation().line, p_assignment.getLvalueLocationCol(), error_message);
        flag_loop_var = false;
        return;
    }
    bool check_rval = true;
    while(symbol_manager->getScopeSize() != 0){
        current_table = symbol_manager->topScope();
        symbol_manager->popScope();
        temp_manager->pushScope(current_table);

        if(current_table->checkRedeclaration(p_assignment.getLvalueNameCString()) == 0)
            continue;
        current_entry = current_table->getEntry(p_assignment.getLvalueNameCString());
        std::string lvalue_type = current_entry->getKindCString();
        if(lvalue_type == "constant"){
            error_happen = true;
            std::string error_message = "cannot assign to variable '" + std::string(p_assignment.getLvalueNameCString()) + "' which is a constant";
            showErrorMessage(p_assignment.getLocation().line, p_assignment.getLvalueLocationCol(), error_message);
            check_rval = false;
            break;
        }
        if(lvalue_type == "loop_var" && !flag_loop_var){
            error_happen = true;
            std::string error_message = "the value of loop variable cannot be modified inside the loop body";
            showErrorMessage(p_assignment.getLocation().line, p_assignment.getLvalueLocationCol(), error_message);
            check_rval = false;
            break;
        }
    }
    while(temp_manager->getScopeSize()!=0){
        symbol_manager->pushScope(temp_manager->topScope());
        temp_manager->popScope();
    }

    if(p_assignment.checkInvalidRvalue()){
        flag_loop_var = false;
        return;
    }

    if(check_rval){
        if(p_assignment.checkRvalueScalarType() == 0){
            error_happen = true;
            std::string error_message = "array assignment is not allowed";
            showErrorMessage(p_assignment.getLocation().line, p_assignment.getRvalueLocationCol(), error_message);
        }
        else if(p_assignment.checkCompatibleLRvalueType() == 0){
            error_happen = true;
            std::string error_message, lvalue_type = p_assignment.getLvalueTypeCString(), rvalue_type = p_assignment.getRvalueTypeCString();
            error_message = "assigning to '" + lvalue_type + "' from incompatible type '" + rvalue_type + "'";
            showErrorMessage(p_assignment.getLocation().line, p_assignment.getLocation().col, error_message);
        }
    }
    flag_loop_var = false;
    // debug2("p_assignment");
}

void SemanticAnalyzer::visit(ReadNode &p_read) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // debug1("p_read");
    p_read.visitChildNodes(*this);

    if(p_read.checkInvalidChildren() == 1)
        return;
    if(p_read.checkTargetScalarType() == 0){
        error_happen = true;
        std::string error_message = "variable reference of read statement must be scalar type";
        showErrorMessage(p_read.getLocation().line, p_read.getTargetLocationCol(), error_message);
    }
    if(symbol_manager->checkConstant(p_read.getTargetNameCString()) || symbol_manager->checkLoopVariableRedeclaration(p_read.getTargetNameCString())){
        error_happen = true;
        std::string error_message = "variable reference of read statement cannot be a constant or loop variable";
        showErrorMessage(p_read.getLocation().line, p_read.getTargetLocationCol(), error_message);
    }
    // debug2("p_read");
}

void SemanticAnalyzer::visit(IfNode &p_if) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // debug1("p_if");
    p_if.visitChildNodes(*this);
    if(p_if.checkInvalidCondition() == 1)
        return;
    if(p_if.checkConditionBoolType() == 0){
        error_happen = true;
        std::string error_message = "the expression of condition must be boolean type";
        showErrorMessage(p_if.getLocation().line, p_if.getConditionLocationCol(), error_message);
    }
    
    // debug2("p_if");
}

void SemanticAnalyzer::visit(WhileNode &p_while) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // debug1("p_while");
    p_while.visitChildNodes(*this);
    // debug2("p_while");
    if(p_while.checkInvalidCondition() == 1)
        return;

    if(p_while.checkConditionBoolType() == 0){
        error_happen = true;
        std::string error_message = "the expression of condition must be boolean type";
        showErrorMessage(p_while.getLocation().line, p_while.getLocation().col, error_message);
    }
}

void SemanticAnalyzer::visit(ForNode &p_for) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // debug1("p_for");

    current_table = new SymbolTable();
    symbol_manager->pushScope(current_table);
    m_level++;
    flag_for = true;
    flag_loop_var = true;

    p_for.visitChildNodes(*this);
    // debug2("p_for");

    flag_loop_var = false;
    flag_for = false;

    int init = atoi(p_for.getInitialValueCString());
    int condition = atoi(p_for.getConditionValueCString());
    if(init > condition){
        error_happen = true;
        std::string error_message = "the lower bound and upper bound of iteration count must be in the incremental order";
        showErrorMessage(p_for.getLocation().line, p_for.getLocation().col, error_message);
    }
    symbol_manager->pop_loop_var();
    if(opt_dump){
        current_table = symbol_manager->topScope();
        current_table->dumpSymbol();
    }
    symbol_manager->popScope();
    m_level--;
}

void SemanticAnalyzer::visit(ReturnNode &p_return) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    // debug1("p_return");
    p_return.visitChildNodes(*this);
    // debug2("p_return");
    if(!flag_in_function){
        error_happen = true;
        std::string error_message = "program/procedure should not return a value";
        showErrorMessage(p_return.getLocation().line, p_return.getLocation().col, error_message);
        return;
    }

    while(symbol_manager->getScopeSize() != 0){
        current_table = symbol_manager->topScope();
        symbol_manager->popScope();
        temp_manager->pushScope(current_table);

        if(current_table->checkRedeclaration(current_function_name.c_str()) == 0)
            continue;
        SymbolEntry *function_entry = current_table->getEntry(current_function_name.c_str());
        std::string return_type = function_entry->getTypeCString();
        if(return_type == "void"){
            error_happen = true;
            std::string error_message = "program/procedure should not return a value";
            showErrorMessage(p_return.getLocation().line, p_return.getLocation().col, error_message);
            break;
        }
        if(p_return.checkInvalidRetrunType() == 1)
            break;
        std::string return_value_type = p_return.getReturnTypeCString();
        if(return_value_type != return_type){
            error_happen = true;
            std::string error_message = "return '" + return_value_type + "' from a function with return type '" + return_type + "'";
            showErrorMessage(p_return.getLocation().line, p_return.getReturnLocationCol(), error_message);
            break; 
        }
    }
    while(temp_manager->getScopeSize()!=0){
        symbol_manager->pushScope(temp_manager->topScope());
        temp_manager->popScope();
    }
    
}
