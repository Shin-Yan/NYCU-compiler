#include "codegen/CodeGenerator.hpp"
#include "visitor/AstNodeInclude.hpp"

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <iostream>
void CodeGenerator::addrStackPush(const std::string p_name){
    if(addr_stack.find(p_name) != addr_stack.end()){
        addr_stack[p_name].push(local_addr);
    }
    else{
        std::stack<int> tmp;
        tmp.push(local_addr);
        addr_stack[p_name] = tmp;
    }
}

void CodeGenerator::addrStackPop(const std::string p_name){
    if(addr_stack.find(p_name) != addr_stack.end()){
        if(addr_stack[p_name].size() == 1){
            addr_stack.erase(p_name);
        }
        else{
            addr_stack[p_name].pop();
        }
    }
}

CodeGenerator::CodeGenerator(const std::string source_file_name,
                             const std::string save_path,
                             const SymbolManager *const p_symbol_manager)
    : m_symbol_manager_ptr(p_symbol_manager),
      m_source_file_path(source_file_name) {
    // FIXME: assume that the source file is always xxxx.p
    const std::string &real_path =
        (save_path == "") ? std::string{"."} : save_path;
    auto slash_pos = source_file_name.rfind("/");
    auto dot_pos = source_file_name.rfind(".");

    assert(dot_pos != std::string::npos && source_file_name[dot_pos+1]=='p' && "file not recognized");

    if (slash_pos != std::string::npos) {
        ++slash_pos;
    } else {
        slash_pos = 0;
    }
    std::string output_file_path(
        real_path + "/" +
        source_file_name.substr(slash_pos, dot_pos - slash_pos) + ".S");
    m_output_file.reset(fopen(output_file_path.c_str(), "w"));
    assert(m_output_file.get() && "Failed to open output file");
    local_addr = 8;
    parameter_id = 0;
    label = 1;
    flag_main = true;
    flag_glb_const = false;
    flag_lvalue = false;
    flag_funcInvocation = false;
    flag_if = false;
    flag_while = false;
    flag_for = false;
    flag_branch = false;
    flag_for_assign = false;
}

static void dumpInstructions(FILE *p_out_file, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(p_out_file, format, args);
    va_end(args);
}

void CodeGenerator::visit(ProgramNode &p_program) {
    // Generate RISC-V instructions for program header
    // clang-format off
    constexpr const char*const riscv_assembly_file_prologue =
        "    .file \"%s\"\n"
        "    .option nopic\n\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_file_prologue,
                     m_source_file_path.c_str());

    // Reconstruct the hash table for looking up the symbol entry
    // Hint: Use symbol_manager->lookup(symbol_name) to get the symbol entry.
    m_symbol_manager_ptr->reconstructHashTableFromSymbolTable(
        p_program.getSymbolTable());

    p_program.visitChildNodes(*this);

    constexpr const char*const riscv_assembly_function_epilogue =
        "# in the function epilogue\n"
        "   lw ra, 124(sp)      # load return address saved in the current stack\n"
        "   lw s0, 120(sp)      # move frame pointer back to the bottom of the last stack\n"
        "   addi sp, sp, 128    # move stack pointer back to the top of the last stack\n"
        "   jr ra               # jump back to the caller function\n"
        "   .size main, .-main\n\n";

    dumpInstructions(m_output_file.get(), riscv_assembly_function_epilogue);

    // const_cast<CompoundStatementNode &>(p_program.getBody()).accept(*this);
    if(p_program.getSymbolTable() != nullptr){
        const auto &entries = p_program.getSymbolTable()->getEntries();
        for(const auto &entry : entries){
            addrStackPop(entry->getName());
        }
    }

    // Remove the entries in the hash table
    m_symbol_manager_ptr->removeSymbolsFromHashTable(p_program.getSymbolTable());
}

void CodeGenerator::visit(DeclNode &p_decl) {
    p_decl.visitChildNodes(*this);
}

void CodeGenerator::visit(VariableNode &p_variable) {
    const SymbolEntry *entry = m_symbol_manager_ptr->lookup(p_variable.getName());
    if(entry == nullptr)
        return;

    // global variable
    if(entry->getLevel() == 0 && entry->getKind() == SymbolEntry::KindEnum::kVariableKind){
        constexpr const char*const riscv_assembly_global_variable_expr =
        "# global variable declaration: %s\n" 
        ".comm %s, 4, 4\n\n";
        dumpInstructions(m_output_file.get(), riscv_assembly_global_variable_expr, p_variable.getNameCString(), p_variable.getNameCString());
    }

    // global constant
    else if(entry->getLevel() == 0 && entry->getKind() == SymbolEntry::KindEnum::kConstantKind){
        const char* v_name = p_variable.getNameCString();
        constexpr const char*const riscv_assembly_global_const_expr =
        "# global constant declaration: %s\n"
        ".section    .rodata\n"
        "   .align 2\n"
        "   .globl %s\n"
        "   .type %s, @object\n"
        "%s:\n"
        "    .word ";
        dumpInstructions(m_output_file.get(), riscv_assembly_global_const_expr, v_name, v_name, v_name, v_name);
        flag_glb_const = true;
        p_variable.visitChildNodes(*this);
        dumpInstructions(m_output_file.get(), "\n\n");
    }

    // local variable, loop variable
    else if(entry->getKind() == SymbolEntry::KindEnum::kVariableKind || 
            entry->getKind() == SymbolEntry::KindEnum::kLoopVarKind){
        addrStackPush(entry->getName());
        if(entry->getTypePtr()->isScalar()){
            if(!entry->getTypePtr()->isString())
                local_addr+=4;
        }
        else{
            int element_num = 1;
            for(auto dimension : p_variable.getTypePtr()->getDimensions())
                element_num *= dimension;

            local_addr += 4 * element_num;
        }
    }

    // local constant
    else if(entry->getKind() == SymbolEntry::KindEnum::kConstantKind){
        addrStackPush(entry->getName());
        constexpr const char*const riscv_assembly_local_const_expr=
        "# local constant declaration: %s\n"
        "   addi t0, s0, -%d\n"
        "   addi sp, sp, -4\n"
        "   sw t0, 0(sp)        # push the address to the stack\n";
        local_addr += 4;
        dumpInstructions(m_output_file.get(), riscv_assembly_local_const_expr, p_variable.getNameCString(), local_addr);
        

        p_variable.visitChildNodes(*this);

        dumpInstructions(m_output_file.get(), 
                "   lw t0, 0(sp)        # pop the value from the stack\n"
                "   addi sp, sp, 4\n"
                "   lw t1, 0(sp)        # pop the address from the stack\n"
                "   addi sp, sp, 4\n"
                "   sw t0, 0(t1)\n");
    }

    // function parameter
    else if(entry->getKind() == SymbolEntry::KindEnum::kParameterKind){
        addrStackPush(entry->getName());
        constexpr const char*const riscv_assembly_func_para_expr =
        "   sw a%d, -%d(s0)      # save parameter %s in the local stack\n";
        constexpr const char*const riscv_assembly_func_paras_expr =
        "   sw s%d, -%d(s0)      # save parameter %s in the local stack\n";
        if(parameter_id <= 7){
            if(p_variable.getTypePtr()->isScalar()){
                local_addr+=4;
                dumpInstructions(m_output_file.get(), riscv_assembly_func_para_expr,parameter_id,local_addr,p_variable.getNameCString());
            }
            else{
                int element_num = 1;
                for(auto dimension:p_variable.getTypePtr()->getDimensions())
                    element_num *= dimension;
                int addr = addr_stack[entry->getName()].top();
                for(int i = 0 ; i < element_num; ++i){
                    if(i <= 7){
                        dumpInstructions(m_output_file.get(), riscv_assembly_func_para_expr, i, addr+4, p_variable.getNameCString());
                    }
                    else{
                        dumpInstructions(m_output_file.get(), riscv_assembly_func_paras_expr, i-7, addr+4, p_variable.getNameCString());
                    }
                    addr+=4;
                }
            }
        }
        else{
            dumpInstructions(m_output_file.get(), riscv_assembly_func_paras_expr, parameter_id-7, local_addr+4, p_variable.getNameCString());
        }
        parameter_id++;
        if(!p_variable.getTypePtr()->isScalar()){
            int element_num = 1;
            for(auto dimension:p_variable.getTypePtr()->getDimensions())
                element_num *= dimension;

            local_addr += (4*element_num);
        }
    }

}

void CodeGenerator::visit(ConstantValueNode &p_constant_value) {
    if(flag_glb_const){
        flag_glb_const = false;
        dumpInstructions(m_output_file.get(), "%s", p_constant_value.getConstantValueCString());
    }
    else{
        const char* value = p_constant_value.getConstantValueCString();
        if(p_constant_value.getTypePtr()->isBool()){
            if(std::strcmp(value, "true")==0)
                value = "1";
            else
                value = "0";
        }
        if(p_constant_value.getTypePtr()->isBool() || p_constant_value.getTypePtr()->isInteger()){
            constexpr const char*const riscv_assembly_const_expr = 
            "   li t0, %s            # load value to register 't0'\n"
            "   addi sp, sp, -4\n"
            "   sw t0, 0(sp)        # push the value to the stack\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_const_expr, value);
        }
        else if(p_constant_value.getTypePtr()->isString()){
            dumpInstructions(m_output_file.get(), "\"%s\"\n\n", p_constant_value.getConstantValueCString());
        }
    }


}

void CodeGenerator::visit(FunctionNode &p_function) {
    // Reconstruct the hash table for looking up the symbol entry
    m_symbol_manager_ptr->reconstructHashTableFromSymbolTable(
        p_function.getSymbolTable());
    
    local_addr = 8;
    flag_main = false;
    const char *func_name = p_function.getNameCString();
    constexpr const char*const riscv_assembly_func_expr =
    ".section    .text\n"
    "   .align 2\n"
    "   .globl %s\n"
    "   .type %s, @function\n\n"
    "%s:\n"
    "# in the function prologue\n"
    "   addi sp, sp, -128   # move stack pointer to lower address to allocate a new stack\n"
    "   sw ra, 124(sp)      # save return address of the caller function in the current stack\n"
    "   sw s0, 120(sp)      # save frame pointer of the last stack in the current stack\n"
    "   addi s0, sp, 128    # move frame pointer to the bottom of the current stack\n\n";
    dumpInstructions(m_output_file.get(), riscv_assembly_func_expr, func_name, func_name, func_name);

    p_function.visitChildNodes(*this);
    parameter_id = 0;
    flag_main = true;
    local_addr = 8;

    constexpr const char*const riscv_assembly_func_epilogue=
    "# in the function epilogue\n"
    "   lw ra, 124(sp)      # load return address saved in the current stack\n"
    "   lw s0, 120(sp)      # move frame pointer back to the bottom of the last stack\n"
    "   addi sp, sp, 128    # move stack pointer back to the top of the last stack\n"
    "   jr ra               # jump back to the caller function\n"
    "   .size %s, .-%s\n\n";

    dumpInstructions(m_output_file.get(), riscv_assembly_func_epilogue, func_name, func_name);

    if(p_function.getSymbolTable() != nullptr){
        const auto &entries = p_function.getSymbolTable()->getEntries();
        for(const auto &entry : entries){
            addrStackPop(entry->getName());
        }
    }

    // Remove the entries in the hash table
    m_symbol_manager_ptr->removeSymbolsFromHashTable(p_function.getSymbolTable());
}

void CodeGenerator::visit(CompoundStatementNode &p_compound_statement) {
    // Reconstruct the hash table for looking up the symbol entry
    m_symbol_manager_ptr->reconstructHashTableFromSymbolTable(
        p_compound_statement.getSymbolTable());

    if(flag_main){
        constexpr const char*const riscv_assembly_main_func_expr =
        ".section    .text\n"
        "   .align 2\n"
        "   .globl main         # emit symbol 'main' to the global symbol table\n"
        "   .type main, @function\n\n"
        
        "main:\n"
        "# in the function prologue\n"
        "   addi sp, sp, -128   # move stack pointer to lower address to allocate a new stack\n"
        "   sw ra, 124(sp)      # save return address of the caller function in the current stack\n"
        "   sw s0, 120(sp)      # save frame pointer of the last stack in the current stack\n"
        "   addi s0, sp, 128    # move frame pointer to the bottom of the current stack\n\n";

        dumpInstructions(m_output_file.get(), riscv_assembly_main_func_expr);
        flag_main = false;
    }
    if(flag_if)
        dumpInstructions(m_output_file.get(), "L%d:\n", label_id);
    
    if(flag_while)
        dumpInstructions(m_output_file.get(), "L%d:\n", label_id+1);

    if(flag_for){
        constexpr const char*const riscv_assembly_for_prologue_expr =
        "   lw t0, 0(sp)        # pop the value from the stack\n"
        "   addi sp, sp, 4\n"
        "   lw t1, 0(sp)        # pop the value from the stack\n"
        "   addi sp, sp, 4\n"
        "   bge t1, t0, L%d        # if i >= condition value, exit the loop\n"
        "L%d:\n";
        dumpInstructions(m_output_file.get(), riscv_assembly_for_prologue_expr, label_id+2 , label_id+1);
    }

    p_compound_statement.visitChildNodes(*this);

    if(flag_if){
        dumpInstructions(m_output_file.get(), "   j L%d                # jump to L%d\nL%d:\n", label_id +2 , label_id +2, label_id+1);
        flag_if = false;
    }
    if(flag_while){
        dumpInstructions(m_output_file.get(), "   j L%d                # jump to L%d\n", label_id, label_id);
        flag_while = false;
    }

    if(p_compound_statement.getSymbolTable() != nullptr){
        const auto &entries = p_compound_statement.getSymbolTable()->getEntries();
        for(const auto &entry:entries){
            addrStackPop(entry->getName());
        }
    }

    // Remove the entries in the hash table
    m_symbol_manager_ptr->removeSymbolsFromHashTable(
        p_compound_statement.getSymbolTable());
}

void CodeGenerator::visit(PrintNode &p_print) {
    dumpInstructions(m_output_file.get(), "\n# print\n");
    p_print.visitChildNodes(*this);
    if(p_print.getTarget().getInferredType()->isInteger()){
        constexpr const char*const riscv_assembly_print_expr = 
        "   lw a0, 0(sp)        # pop the value from the stack to the first argument register 'a0'\n"
        "   addi sp, sp, 4\n"
        "   jal ra, printInt    # call function 'printInt'\n\n";
        dumpInstructions(m_output_file.get(), riscv_assembly_print_expr);
    }
    else if(p_print.getTarget().getInferredType()->isString()){
        constexpr const char*const riscv_assembly_print_expr = 
        "   lw a0, 0(sp)        # pop the value from the stack to the first argument register 'a0'\n"
        "   addi sp, sp, 4\n"
        "   jal ra, printString    # call function 'printString'\n\n";
        dumpInstructions(m_output_file.get(), riscv_assembly_print_expr);
    }
    
}

void CodeGenerator::visit(BinaryOperatorNode &p_bin_op) {
    dumpInstructions(m_output_file.get(), "\n# binary operator: %s\n", p_bin_op.getOpCString());
    bool branch = flag_branch;
    flag_branch = false;
    p_bin_op.visitChildNodes(*this);
    constexpr const char*const riscv_assembly_pop2 = 
    "   lw t0, 0(sp)        # pop the value from the stack\n"
    "   addi sp, sp, 4\n"
    "   lw t1, 0(sp)        # pop the value from the stack\n"
    "   addi sp, sp, 4\n";

    constexpr const char*const riscv_assembly_push1=
    "   addi sp, sp, -4\n"
    "   sw t0, 0(sp)        # push the value to the stack\n";
    dumpInstructions(m_output_file.get(), riscv_assembly_pop2);
    const char *ops = p_bin_op.getOpCString();
    if(std::strcmp(ops, "+")==0){
        dumpInstructions(m_output_file.get(), "   add t0, t1, t0      # always save the value in a certain register you choose\n");
        dumpInstructions(m_output_file.get(), riscv_assembly_push1);
    }

    else if(std::strcmp(ops, "-")==0){
        dumpInstructions(m_output_file.get(), "   sub t0, t1, t0      # always save the value in a certain register you choose\n");
        dumpInstructions(m_output_file.get(), riscv_assembly_push1);
    }

    else if(std::strcmp(ops, "*")==0){
        dumpInstructions(m_output_file.get(), "   mul t0, t1, t0      # always save the value in a certain register you choose\n");
        dumpInstructions(m_output_file.get(), riscv_assembly_push1);
    }

    else if(std::strcmp(ops, "/")==0){
        dumpInstructions(m_output_file.get(), "   div t0, t1, t0      # always save the value in a certain register you choose\n");
        dumpInstructions(m_output_file.get(), riscv_assembly_push1);
    }

    else if(std::strcmp(ops, "mod")==0){
        dumpInstructions(m_output_file.get(), "   rem t0, t1, t0      # always save the value in a certain register you choose\n");
        dumpInstructions(m_output_file.get(), riscv_assembly_push1);
    }

    else if(std::strcmp(ops, "and")==0){
        dumpInstructions(m_output_file.get(), "   and t0, t1, t0      # always save the value in a certain register you choose\n");
        dumpInstructions(m_output_file.get(), riscv_assembly_push1);
    }

    else if(std::strcmp(ops, "or")==0){
        dumpInstructions(m_output_file.get(), "   or t0, t1, t0      # always save the value in a certain register you choose\n");
        dumpInstructions(m_output_file.get(), riscv_assembly_push1);
    }

    else{
        constexpr const char*const riscv_assembly_branch = 
        "   %s t1, t0, L%d      # if t1 %s t0, jump to L%d\n";
        if(std::strcmp(p_bin_op.getOpCString(), "<=") == 0){
            if(flag_if && branch){
                dumpInstructions(m_output_file.get(), riscv_assembly_branch, "bgt", label_id+1, ">", label_id+1);
            }
            else if(flag_while && branch){
                dumpInstructions(m_output_file.get(), riscv_assembly_branch, "bgt", label_id+2, ">", label_id+2);
            }
            else{
                constexpr const char*const riscv_assembly_bool_res =
                "   sub t0, t1, t0      # always save the value in a certain register you choose\n"
                "   slti t0, t0, 1\n";
                dumpInstructions(m_output_file.get(), riscv_assembly_bool_res);
                dumpInstructions(m_output_file.get(), riscv_assembly_push1);
            }
        }
        if(std::strcmp(p_bin_op.getOpCString(), "<") == 0){
            if(flag_if && branch){
                dumpInstructions(m_output_file.get(), riscv_assembly_branch, "bge", label_id+1, ">=", label_id+1);
            }
            else if(flag_while && branch){
                dumpInstructions(m_output_file.get(), riscv_assembly_branch, "bge", label_id+2, ">=", label_id+2);
            }
            else{
                constexpr const char*const riscv_assembly_bool_res =
                "   sub t0, t1, t0      # always save the value in a certain register you choose\n"
                "   slti t0, t0, 0\n";
                dumpInstructions(m_output_file.get(), riscv_assembly_bool_res);
                dumpInstructions(m_output_file.get(), riscv_assembly_push1);
            }
        }
        if(std::strcmp(p_bin_op.getOpCString(), ">=") == 0){
            if(flag_if && branch){
                dumpInstructions(m_output_file.get(), riscv_assembly_branch, "blt", label_id+1, "<", label_id+1);
            }
            else if(flag_while && branch){
                dumpInstructions(m_output_file.get(), riscv_assembly_branch, "blt", label_id+2, "<", label_id+2);
            }
            else{
                constexpr const char*const riscv_assembly_bool_res =
                "   sub t0, t1, t0      # always save the value in a certain register you choose\n"
                "   slti t0, t0, 0\n"
                "   slti t0, t0, 1\n";
                dumpInstructions(m_output_file.get(), riscv_assembly_bool_res);
                dumpInstructions(m_output_file.get(), riscv_assembly_push1);
            }
        }
        if(std::strcmp(p_bin_op.getOpCString(), ">") == 0){
            if(flag_if && branch){
                dumpInstructions(m_output_file.get(), riscv_assembly_branch, "ble", label_id+1, "<=", label_id+1);
            }
            else if(flag_while && branch){
                dumpInstructions(m_output_file.get(), riscv_assembly_branch, "ble", label_id+2, "<=", label_id+2);
            }
            else{
                constexpr const char*const riscv_assembly_bool_res =
                "   sub t0, t1, t0      # always save the value in a certain register you choose\n"
                "   slti t0, t0, 1\n"
                "   slti t0, t0, 1\n";
                dumpInstructions(m_output_file.get(), riscv_assembly_bool_res);
                dumpInstructions(m_output_file.get(), riscv_assembly_push1);
            }
        }
        if(std::strcmp(p_bin_op.getOpCString(), "=") == 0){
            if(flag_if && branch){
                dumpInstructions(m_output_file.get(), riscv_assembly_branch, "bne", label_id+1, "!=", label_id+1);
            }
            else if(flag_while && branch){
                dumpInstructions(m_output_file.get(), riscv_assembly_branch, "bne", label_id+2, "!=", label_id+2);
            }
            else{
                constexpr const char*const riscv_assembly_bool_res =
                "   sub t0, t1, t0      # always save the value in a certain register you choose\n"
                "   seqz t0, t0\n";
                dumpInstructions(m_output_file.get(), riscv_assembly_bool_res);
                dumpInstructions(m_output_file.get(), riscv_assembly_push1);
            }
        }
        if(std::strcmp(p_bin_op.getOpCString(), "<>") == 0){
            if(flag_if && branch){
                dumpInstructions(m_output_file.get(), riscv_assembly_branch, "beq", label_id+1, "=", label_id+1);
            }
            else if(flag_while && branch){
                dumpInstructions(m_output_file.get(), riscv_assembly_branch, "beq", label_id+2, "=", label_id+2);
            }
            else{
                constexpr const char*const riscv_assembly_bool_res =
                "   sub t0, t1, t0      # always save the value in a certain register you choose\n"
                "   snez t0, t0, 1\n";
                dumpInstructions(m_output_file.get(), riscv_assembly_bool_res);
                dumpInstructions(m_output_file.get(), riscv_assembly_push1);
            }
        }
    }
    dumpInstructions(m_output_file.get(), "\n");
}

void CodeGenerator::visit(UnaryOperatorNode &p_un_op) {
    const char* ops = p_un_op.getOpCString();
    dumpInstructions(m_output_file.get(),"\n# unary operator: %s\n",ops);
    if(std::strcmp(ops, "neg") == 0){
        p_un_op.visitChildNodes(*this);
        constexpr const char*const riscv_assembly_unary_expr=
        "   lw t0, 0(sp)        # pop the value from the stack\n"
        "   addi sp, sp, 4\n"
        "   neg t0, t0\n        # always save the value in a certain register you choose\n"
        "   addi sp, sp, -4\n"
        "   sw t0, 0(sp)        # push the value to the stack\n\n";
        dumpInstructions(m_output_file.get(), riscv_assembly_unary_expr);
    }
    if(std::strcmp(ops, "not") == 0){
        bool branch = flag_branch;
        flag_branch = false;
        p_un_op.visitChildNodes(*this);
        
        constexpr const char*const riscv_assembly_not_expr=
        "   lw t0, 0(sp)        # pop the value from the stack\n"
        "   addi sp, sp, 4\n"
        "   slti t0, t0, 1\n"
        "   addi sp, sp, -4\n"
        "   sw t0, 0(sp)        # push the value to the stack\n\n";
        dumpInstructions(m_output_file.get(), riscv_assembly_not_expr);

        if(branch){
            constexpr const char*const riscv_assembly_not_branch_expr=
            "   lw t1, 0(sp)         # pop the value from the stack\n"
            "   addi sp, sp, 4\n"
            "   li t0, 0\n"
            "   beq t1, t0, L%d      # if t1 == 0, jump to L%d\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_not_branch_expr,label_id+1,label_id+1);
        }
    }
    
}

void CodeGenerator::visit(FunctionInvocationNode &p_func_invocation) {
    dumpInstructions(m_output_file.get(), "\n# function invocation: %s\n", p_func_invocation.getNameCString());
    flag_funcInvocation = true;
    p_func_invocation.visitChildNodes(*this);
    flag_funcInvocation = false;
    constexpr const char*const riscv_assembly_popa = 
    "   lw a%d, 0(sp)        # pop the value from the stack to the argument register a%d\n"
    "   addi sp, sp, 4\n";
    constexpr const char*const riscv_assembly_pops = 
    "   lw s%d, 0(sp)        # pop the value from the stack to the argument register s%d\n"
    "   addi sp, sp, 4\n";
    // std::cout <<"argsize = "<< p_func_invocation.getArguments().size() << std::endl;
    for(int i = p_func_invocation.getArguments().size()-1 ; i>=0 ; --i){
        if(i <= 7){
            if(p_func_invocation.getArguments()[i]->getInferredType()->isScalar()){
                dumpInstructions(m_output_file.get(), riscv_assembly_popa, i, i);
            }
            // array
            else{
                int element_num = 1;
                for(auto dimension:p_func_invocation.getArguments()[i]->getInferredType()->getDimensions()){
                    element_num *= dimension;
                }
                int register_id;
                if(element_num>8)
                    register_id=7;
                else
                    register_id = element_num-1;

                for(int j = 0; j < element_num ;j++){
                    if(j<element_num-8){
                        dumpInstructions(m_output_file.get(), riscv_assembly_pops, element_num-8-j, element_num-8-j);
                    }
                    else{
                        dumpInstructions(m_output_file.get(), riscv_assembly_popa, register_id, register_id);
                        register_id --;
                    }
                }
            }
        }
        // i>=8
        else{
            dumpInstructions(m_output_file.get(), riscv_assembly_pops, i-7, i-7);
        }
    }
    constexpr const char*const riscv_assembly_function_call=
    "   jal ra, %s         # call function %s\n"
    "   mv t0, a0          # always move the return value to a certain register you choose\n"
    "   addi sp, sp, -4\n"
    "   sw t0, 0(sp)       # push the value to the stack\n\n\n";

    dumpInstructions(m_output_file.get(), riscv_assembly_function_call, p_func_invocation.getNameCString(), p_func_invocation.getNameCString());
}

void CodeGenerator::visit(VariableReferenceNode &p_variable_ref) {
    const SymbolEntry *entry = m_symbol_manager_ptr->lookup(p_variable_ref.getName());
    if(entry == nullptr)
        return;
    const char *var_name = p_variable_ref.getNameCString();
    // global variable
    if(entry->getLevel() == 0 && entry->getKind() == SymbolEntry::KindEnum::kVariableKind){
        if(flag_lvalue){
            flag_lvalue = false;
            constexpr const char*const riscv_assembly_glval_ref_expr =
            "   la t0, %s           # load the address of variable %s\n"
            "   addi sp, sp, -4\n"
            "   sw t0, 0(sp)     # push the address to the stack\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_glval_ref_expr, var_name, var_name);
        }
        else{
            constexpr const char*const riscv_assembly_grval_ref_expr =
            "   la t0, %s\n"
            "   lw t1, 0(t0)        # load the value of %s\n"
            "   mv t0, t1\n"
            "   addi sp, sp, -4\n"
            "   sw t0, 0(sp)     # push the address to the stack\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_grval_ref_expr, var_name, var_name);
        }
    }

    // global constant
    else if(entry->getLevel() == 0 && entry->getKind() == SymbolEntry::KindEnum::kConstantKind){
        constexpr const char*const riscv_assembly_gconst_ref_expr =
        "   la t0, %s\n"
        "   lw t1, 0(t0)        # load the value of %s\n"
        "   mv t0, t1\n"
        "   addi sp, sp, -4\n"
        "   sw t0, 0(sp)     # push the value to the stack\n";
        dumpInstructions(m_output_file.get(), riscv_assembly_gconst_ref_expr, var_name, var_name);
    }

    // local variable, function parameter, loop variable
    else if(entry->getLevel() != 0 && (entry->getKind() == SymbolEntry::KindEnum::kVariableKind
        || entry->getKind() == SymbolEntry::KindEnum::kParameterKind
        || entry->getKind() == SymbolEntry::KindEnum::kLoopVarKind)){
        
        int addr = addr_stack[entry->getName()].top();

        if(flag_lvalue){
            flag_lvalue = false;
            // scalar reference lvalue
            if(entry->getTypePtr()->isScalar()){
                if(!entry->getTypePtr()->isString()){
                    constexpr const char*const riscv_assembly_llvalue_ref_expr =
                    "   addi t0, s0, -%d\n"
                    "   addi sp, sp, -4\n"
                    "   sw t0, 0(sp)        # push the address to the stack\n";
                    dumpInstructions(m_output_file.get(), riscv_assembly_llvalue_ref_expr, addr+4);
                }
                else{
                    constexpr const char*const riscv_assembly_const_str_expr = 
                    "    .section    .rodata\n"
                    "    .align 2\n"
                    "%s:\n"
                    "    .string ";
                    dumpInstructions(m_output_file.get(), riscv_assembly_const_str_expr, entry->getNameCString());
                }
            }

            // array reference lvalue
            else{
                dumpInstructions(m_output_file.get(), "# array reference lvalue\n");
                p_variable_ref.visitChildNodes(*this);
                int times = 1;

                dumpInstructions(m_output_file.get(), "\n# count offset----------------\n   li t2, 0\n");
                constexpr const char*const riscv_assembly_count_off_expr =
                "   lw t0, 0(sp)        # pop the value from the stack\n"
                "   addi t0, t0, -1     # index starts from 1\n"
                "   addi sp, sp, 4\n"
                "   li t1, %d\n"
                "   mul t1, t0, t1\n"
                "   add t2, t1, t2\n";
                for(int i = p_variable_ref.getIndices().size()-1; i>=0 ; --i){
                    if(i != p_variable_ref.getIndices().size()-1)
                        times *= entry->getTypePtr()->getDimensions()[i+1];
                    dumpInstructions(m_output_file.get(), riscv_assembly_count_off_expr, times);
                }
                constexpr const char*const riscv_assembly_count_off_end_expr =
                "   li t1, 4\n"
                "   mul t2, t2, t1\n"
                "   addi t2, t2, %d\n"
                "   addi t2, t2, 4\n"
                "# count offset end-----------\n\n"
                "   sub t0, s0, t2\n"
                "   addi sp, sp, -4\n"
                "   sw t0, 0(sp)        # push the address to the stack\n";
                dumpInstructions(m_output_file.get(), riscv_assembly_count_off_end_expr, addr);
            }
        }
        else{
            // scalar reference rvalue
            if(entry->getTypePtr()->isScalar()){
                // string reference
                if(entry->getTypePtr()->isString()){
                    constexpr const char*const riscv_assembly_rvalue_ref_str_expr=
                    "   lui t0, %%hi(%s)\n"
                    "   addi t0, t0, %%lo(%s)\n"
                    "   addi sp, sp, -4\n"
                    "   sw t0, 0(sp)        # push the value to the stack\n";
                    dumpInstructions(m_output_file.get(), riscv_assembly_rvalue_ref_str_expr, entry->getNameCString(), entry->getNameCString());
                }
                else{
                    constexpr const char*const riscv_assembly_lrvalue_ref_expr =
                    "   lw t0, -%d(s0)      # load the value of %s\n"
                    "   addi sp, sp, -4\n"
                    "   sw t0, 0(sp)        # push the value to the stack\n";
                    dumpInstructions(m_output_file.get(), riscv_assembly_lrvalue_ref_expr, addr+4, p_variable_ref.getNameCString());
                }
            }
            // array reference or funcInvocation rvalue
            else{
                // function invocation rvalue, pass the whole array value as parameter
                if(flag_funcInvocation){
                    dumpInstructions(m_output_file.get(), "# array reference rvalue\n");
                    int element_num = 1;
                    for(auto dimension : p_variable_ref.getInferredType()->getDimensions()){
                        element_num *= dimension;
                    }
                    int addr = addr_stack[entry->getName()].top();
                    constexpr const char*const riscv_assembly_larvalue_ref_expr =
                    "   lw t0, -%d(s0)      # load the value of %s\n"
                    "   addi sp, sp, -4\n"
                    "   sw t0, 0(sp)        # push the value to the stack\n";
                    for(int i = 0 ; i < element_num ; ++i){
                        dumpInstructions(m_output_file.get(), riscv_assembly_larvalue_ref_expr, addr+4);
                        addr += 4;
                    }
                }
                // array reference rvalue
                else{
                    dumpInstructions(m_output_file.get(), "# array reference rvalue\n");
                    p_variable_ref.visitChildNodes(*this);
                    int times = 1;
                    dumpInstructions(m_output_file.get(), "\n# count offset----------------\n   li t2, 0\n");
                    constexpr const char*const riscv_assembly_count_off_expr =
                    "   lw t0, 0(sp)        # pop the value from the stack\n"
                    "   addi t0, t0, -1     # index starts from 1\n"
                    "   addi sp, sp, 4\n"
                    "   li t1, %d\n"
                    "   mul t1, t0, t1\n"
                    "   add t2, t1, t2\n";
                    for(int i = p_variable_ref.getIndices().size()-1; i>=0 ; --i){
                        if(i != p_variable_ref.getIndices().size()-1)
                            times *= entry->getTypePtr()->getDimensions()[i+1];
                        dumpInstructions(m_output_file.get(), riscv_assembly_count_off_expr, times);
                    }
                    constexpr const char*const riscv_assembly_count_off_end_expr =
                    "   li t1, 4\n"
                    "   mul t2, t2, t1\n"
                    "   addi t2, t2, %d\n"
                    "   addi t2, t2, 4\n"
                    "# count offset end-----------\n\n"
                    "   sub t0, s0, t2\n"
                    "   lw t0, 0(t0)\n"
                    "   addi sp, sp, -4\n"
                    "   sw t0, 0(sp)        # push the address to the stack\n";
                    dumpInstructions(m_output_file.get(), riscv_assembly_count_off_end_expr, addr);
                }
            }
        }
    }

    // local constant
    else if(entry->getLevel() != 0 && entry->getKind() == SymbolEntry::KindEnum::kConstantKind){
        int addr = addr_stack[entry->getName()].top();
        constexpr const char*const riscv_assembly_lrvalue_const_expr =
        "   lw t0, -%d(s0)       # load the value of %s\n"
        "   addi sp, sp, -4\n"
        "   sw t0, 0(sp)        # push the value to the stack\n";
        dumpInstructions(m_output_file.get(), riscv_assembly_lrvalue_const_expr, addr + 4, p_variable_ref.getNameCString());
    }

    // TODO: consider branch
    if(flag_branch){
        flag_branch = false;
        constexpr const char*const riscv_assembly_var_refb_expr=
        "   lw t1, 0(sp)        # pop the value from the stack\n"
        "   addi sp, sp, 4\n"
        "   li t0, 0\n"
        "   beq t1, t0, L%d      # if t1 == 0, jump to L%d\n";
        dumpInstructions(m_output_file.get(), riscv_assembly_var_refb_expr ,label_id+1, label_id+1);
    }
}

void CodeGenerator::visit(AssignmentNode &p_assignment) {
    flag_lvalue = true;

    dumpInstructions(m_output_file.get(), "\n# variable assignment: %s\n", p_assignment.getLvalue().getNameCString());
    p_assignment.visitChildNodes(*this);

    if(!p_assignment.getLvalue().getInferredType()->isString()){
        constexpr const char*const riscv_assembly_assign_expr =
        "   lw t0, 0(sp)        # pop the value from the stack\n"
        "   addi sp, sp, 4\n"
        "   lw t1, 0(sp)        # pop the address from the stack\n"
        "   addi sp, sp, 4\n"
        "   sw t0, 0(t1)        # save the value to %s\n\n";

        dumpInstructions(m_output_file.get(), riscv_assembly_assign_expr, p_assignment.getLvalue().getNameCString());
    }
    if(flag_for_assign){
        flag_for_assign = false;
        int addr = addr_stack[p_assignment.getLvalue().getName()].top();
        constexpr const char*const riscv_assembly_for_assign_expr=
        "L%d:\n"
        "   lw t0, -%d(s0)      # load the value of %s\n"
        "   addi sp, sp, -4\n"
        "   sw t0, 0(sp)        # push the value to the stack\n";
        dumpInstructions(m_output_file.get(), riscv_assembly_for_assign_expr, label_id ,addr+4, p_assignment.getLvalue().getNameCString());
    }
}

void CodeGenerator::visit(ReadNode &p_read) {
    dumpInstructions(m_output_file.get(), "\n# read\n");
    flag_lvalue = true;
    p_read.visitChildNodes(*this);
    constexpr const char*const riscv_assembly_read=
    "   jal ra, readInt     # call function 'readInt'\n"
    "   lw t0, 0(sp)        # pop the address from the stack\n"
    "   addi sp, sp, 4\n"
    "   sw a0, 0(t0)        # save the return value to %s\n\n";
    dumpInstructions(m_output_file.get(), riscv_assembly_read, p_read.getTarget().getNameCString());
}

void CodeGenerator::visit(IfNode &p_if) {
    flag_if = true;
    flag_branch = true;
    label_base.push(label);
    label += 3;
    label_id = label_base.top();

    p_if.visitChildNodes(*this);

    flag_if = false;
    dumpInstructions(m_output_file.get(), "L%d:\n", label_id + 2);
    label_base.pop();
    if(!label_base.empty())
        label_id = label_base.top();
    else
        label_id = label;
}

void CodeGenerator::visit(WhileNode &p_while) {
    label_base.push(label);
    label += 3;
    label_id = label_base.top();
    dumpInstructions(m_output_file.get() , "L%d:\n", label_id);
    flag_while = true;
    flag_branch = true;
    p_while.visitChildNodes(*this);

    flag_while = false;
    dumpInstructions(m_output_file.get(), "L%d:\n", label_id + 2);
    label_base.pop();
    if(!label_base.empty())
        label_id = label_base.top();
    else
        label_id = label;
}

void CodeGenerator::visit(ForNode &p_for) {
    // Reconstruct the hash table for looking up the symbol entry
    m_symbol_manager_ptr->reconstructHashTableFromSymbolTable(
        p_for.getSymbolTable());

    label_base.push(label);
    label += 3;
    label_id = label_base.top();

    flag_for = true;
    flag_for_assign = true;
    // flag_branch = true;
    p_for.visitChildNodes(*this);

    flag_for = false;
    int addr = addr_stack[p_for.getInitialStatement()->getLvalue().getName()].top();
    constexpr const char*const riscv_assembly_for_expr=
    "   addi t0, s0, -%d      # load the address of loop variable\n"
    "   addi sp, sp, -4\n"
    "   sw t0, 0(sp)        # push the address to the stack\n"

    "   lw t0, -%d(s0)      # load the value of loop variable\n"
    "   addi sp, sp, -4\n"
    "   sw t0, 0(sp)        # push the value to the stack\n"

    "   li t0, 1\n"
    "   addi sp, sp, -4\n"
    "   sw t0, 0(sp)        # push the value to the stack\n"
    
    "   lw t0, 0(sp)        # pop the value from the stack\n"
    "   addi sp, sp, 4\n"
    "   lw t1, 0(sp)        # pop the value from the stack\n"
    "   addi sp, sp, 4\n"
    
    "   add t0, t1, t0      # always save the value in a certain register you choose\n"

    "   addi sp, sp, -4\n"
    "   sw t0, 0(sp)        # push the value to the stack\n"
    "   lw t0, 0(sp)        # pop the value from the stack\n"
    "   addi sp, sp, 4\n"
    "   lw t1, 0(sp)        # pop the address from the stack\n"
    "   addi sp, sp, 4\n"
    "   sw t0, 0(t1)        # save the value to loop variable\n"
    "   j L%d                # jump back to loop condition\n"
    "L%d:\n";
    dumpInstructions(m_output_file.get(), riscv_assembly_for_expr, addr+4, addr+4, label_id, label_id+2);
    label_base.pop();
    if(!label_base.empty())
        label_id = label_base.top();
    else
        label_id = label;

    if(p_for.getSymbolTable() != nullptr){
        const auto &entries = p_for.getSymbolTable()->getEntries();
        for(const auto &entry:entries){
            addrStackPop(entry->getName());
        }
    }
    // Remove the entries in the hash table
    m_symbol_manager_ptr->removeSymbolsFromHashTable(p_for.getSymbolTable());
}

void CodeGenerator::visit(ReturnNode &p_return) {
    p_return.visitChildNodes(*this);

    constexpr const char*const riscv_assembly_return_expr=
    "   lw t0, 0(sp)        # pop the value from the stack\n"
    "   addi sp, sp, 4\n"
    "   mv a0, t0           # load the value to the return value register 'a0'\n\n";
    dumpInstructions(m_output_file.get(), riscv_assembly_return_expr);
}
