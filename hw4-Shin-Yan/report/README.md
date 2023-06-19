# hw4 report

|||
|-:|:-|
|Name|曾信彥|
|ID|310555008|

## How much time did you spend on this project

> e.g. 2 hours.
一週

## Project overview

> Please describe the structure of your code and the ideas behind your implementation in an organized way.
> The point is to show us how you deal with the problems. It is not necessary to write a lot of words or paste all of your code here.

預備:
更改parser.y: 
    1. 新增 error_happen flag
更改scanner.l:
    1. 新增 //&D-功能, opt_dump flag
    2. 紀錄source code (semantic analyzer報錯時用到)

這次利用了原本建好的AST tree, 並新增SemanticAnalyzer, 去traverse tree, 做symbol table建立以及semantic analysis
首先, 實作 Symbol Entry, Symbol Table, Symbol Manager

Symbol Entry -> 一個entry, 包含一個symbol的 name, kind, level, type, attribute
Symbol Table -> 包含許多entry, 同一個scope的所有entry會被放在 SymbolTable::std::vector<SymbolEntry *> entries底下
Symbol manager -> 包含許多table, 所有個scope會根據level被放在stack上, 除此之外也記錄loop variable 以及 constant 

Visit 實作大致流程:

prgramNode -> new scope -> new symbol entry (program attribute) -> visit child -> dump symbol -> pop scope
declNode -> visit child
variableNode -> get topScope -> check redeclaration -> set type -> visit child -> check dimensions
constantValueNode -> set type
functionNode -> get topScope -> check redeclaration -> new entry -> new scope -> level++ -> visit child -> dump symbol -> pop scope -> level--
compoundStatementNode -> new scope -> visit child -> dump symbol -> pop scope
print -> visit children -> check children -> check scalar
bin_op -> visit children -> check children -> check type
un_op -> visit children -> check children -> check type
functionInvocation -> visit children -> check undeclared -> check non function -> check too many args -> check children -> set type
variable reference -> visit children -> check children -> check error declared -> check invalid symbol -> check dimension -> check index type -> set type and dimension
assignment -> visit children -> check lvalue -> check declaration -> check lvalue constant or loop_var -> check rvalue
read -> visit children -> check children -> check scalar -> check constant or loop_var
if while -> visit children -> check children -> check condition type
for -> new scope -> level++ -> visit children -> check condition -> dump symbol -> level--
return -> visit children -> check function -> check return type

## What is the hardest you think in this project

> Not required, but bonus point may be given.
實際上若需要到outer scope尋找declaration是最麻煩的, 我的實作使用temp_manager去handle 暫時pop出來的scope, 到後來檢查完之後再push回原本的symbol_manager.

再者, 有許多小細節需要注意, 像是constantValueNode在建立時並沒有設定自己的PTypeSharedPtr, 會導致在分析variable reference時出錯, constructor需要加上
```
{type = p_constant->getTypeSharedPtr();}
```
另外, 在語意分析時要case by case的注意出錯點是哪裡, 並不可以一味的使用 node.getLocation().line, node.getLocation().col就好
然後, 整個程式也需要一些flag來維持狀態, 比如說進入for迴圈中的第一個variable declaration就要特別標註, 不然本身會被誤認成loop variable assignment

## Feedback to T.A.s

> Not required, but bonus point may be given.
