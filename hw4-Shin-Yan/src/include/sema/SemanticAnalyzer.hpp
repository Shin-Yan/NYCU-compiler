#ifndef SEMA_SEMANTIC_ANALYZER_H
#define SEMA_SEMANTIC_ANALYZER_H

#include "visitor/AstNodeVisitor.hpp"
#include <vector>
#include <stack>
#include <string>

#define DUMPFORMAT          "%-33s%-11s%-11s%-17s%-11s\n"
#define DUMPENTRY           "%-33s%-11s%d%-10s%-17s%-11s\n"
#define MAX_LINE_NUM        512
extern uint32_t opt_dump;
extern char *source_code[MAX_LINE_NUM];
extern bool error_happen;

class SymbolEntry{
  public:
    SymbolEntry(const char *p_variableName, const char* p_kind, const int p_level, const char *p_type, const char *p_attribute)
    : variableName(p_variableName), kind(p_kind), level(p_level), type(p_type), attribute(p_attribute){}
    void dumpEntry(void);
    const char *getNameCString() const;
    const char *getTypeCString() const;
    const char *getKindCString() const;
    const char *getAttributeCString() const;
    int getTypeDimension() const;
    void setKindString(const char *p_kind);
    void setAttributeString(const char *p_attribute);
    void getNewTypeDimensions(std::vector<uint64_t> &dims, int ignore_before) const;
    int getFunctionParameterCount() const;
  private:
    std::string variableName;
    std::string kind;
    int level;
    std::string type;
    std::string attribute;
};
class SymbolTable{
  public:
    SymbolTable(){}
    void addSymbol(SymbolEntry *entry);
    void dumpSymbol(void);
    int checkRedeclaration(const char *p_name) const;
    void addErrorSymbol(const char *p_name);
    int checkErrorDeclare(const char *p_name) const;
    SymbolEntry *getEntry(const char *p_name);
  private:
    std::vector<SymbolEntry *> entries;
    std::vector<const char *> error_declaration;
};

class SymbolManager{
  public:
    SymbolManager(){}
    void pushScope(SymbolTable *new_scope);
    void popScope(void);
    SymbolTable* topScope(void);
    int checkLoopVariableRedeclaration(const char *);
    int checkConstant(const char *);
    void push_loop_var(const char *p_name);
    void pop_loop_var(void);
    void push_constant(const char *p_name);
    void pop_constant(void);
    int getScopeSize(void);
  private:
    std::stack <SymbolTable *> tables;
    std::vector <const char *> loop_var;
    std::vector <const char *> constants;
};

class SemanticAnalyzer final : public AstNodeVisitor {
  private:
    // TODO: something like symbol manager (manage symbol tables)
    //       context manager, return type manager
    bool flag_for = false;
    bool flag_loop_var = false;
    bool flag_function = false;
    bool flag_in_function = false;
    bool flag_var = false;
    // bool flag_parameter = false;
    int m_level = 0;
    
    std::string current_function_name;
    SymbolManager *symbol_manager;
    SymbolManager *temp_manager;
    SymbolTable *current_table;
    SymbolEntry *current_entry;

  public:
    ~SemanticAnalyzer() = default;
    SemanticAnalyzer() = default;

    void visit(ProgramNode &p_program) override;
    void visit(DeclNode &p_decl) override;
    void visit(VariableNode &p_variable) override;
    void visit(ConstantValueNode &p_constant_value) override;
    void visit(FunctionNode &p_function) override;
    void visit(CompoundStatementNode &p_compound_statement) override;
    void visit(PrintNode &p_print) override;
    void visit(BinaryOperatorNode &p_bin_op) override;
    void visit(UnaryOperatorNode &p_un_op) override;
    void visit(FunctionInvocationNode &p_func_invocation) override;
    void visit(VariableReferenceNode &p_variable_ref) override;
    void visit(AssignmentNode &p_assignment) override;
    void visit(ReadNode &p_read) override;
    void visit(IfNode &p_if) override;
    void visit(WhileNode &p_while) override;
    void visit(ForNode &p_for) override;
    void visit(ReturnNode &p_return) override;
};

#endif
