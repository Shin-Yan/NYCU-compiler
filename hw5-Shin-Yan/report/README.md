# hw5 report

|||
|-:|:-|
|Name|曾信彥|
|ID|310555008|

## How much time did you spend on this project

> e.g. 2 hours.
5 days

## Project overview

> Please describe the structure of your code and the ideas behind your implementation in an organized way.
> The point is to show us how you deal with the problems. It is not necessary to write a lot of words or paste all of your code here.

首先 會由parser呼叫visitor開始進行visitor pattern, 在visit的過程中也同時把code dump出來
其中要注意visit順序, 以及是lvalue或者rvalue, lvalue就需要拿address, rvalue需要拿值出來
另外需要注意的還有address Label等等的細節

大致的狀況如下ex:
program-> file prologue-> reconstructhashtable-> visit children-> function epilogue-> clear stack-> removeSymbols
compound-> check main function-> check if for while(dump label)-> visit children-> check if while(dump j Lx)-> clear stack-> removeSymbols

## What is the hardest you think in this project

> Not required, but bonus point may be given.
考慮到每個變數在不同scope中可能會有相同的identifier, 使用了map+stack去實作這部份的想法
    ```
    std::map<std::string, std::stack<int>>
    ```
最難的部分在於要想清楚什麼時候要pop, 什麼時候要push, 什麼時候addr 要+4
如果沒有想清楚這部份的實作 很可能riscv code生的出來 但結果容易不如預期 (segfault or unexpected behavior)
## Feedback to T.A.s

> Not required, but bonus point may be given.
謝謝TA寫了個這麼大的模板讓我們在上面改 省去許多環境設置或是重學OOP的時間
希望以後SPEC可以大概講一下常用的api怎麼call以及代表什麼, 不然要花一些時間去看助教的code怎麼寫 相信這也不是這門課作業的原意
比如說string會被當成一個scalar還是不是scalar, 要用getTypePtr()->isString()去確認type
另外string 方面有點難實作 希望以後提示多一點QQ 我光是放在main function 下的rodata section, assembler就開始抱怨 就不知道怎麼解下去了QQ