# hw2 report

|||
|-:|:-|
|Name|yourname|
|ID|your student id|

## How much time did you spend on this project

> e.g. 2 hours.
> 6 hours

## Project overview

> Please describe the structure of your code and the ideas behind your implementation in an organized way. \
> The point is to show us how you deal with the problems. It is not necessary to write a lot of words or paste all of your code here. 
> 首先, parser這邊要定義token, 並且修改scanner得以return token給parser.
> 接下來, 使用left 來處理operator associative (照著spec上的precedence去排)
> 最後, 就是production rule, 透過收斂到 start symbol (program name) 來判定是否成功, 也是此次作業的重點 

## What is the hardest you think in this project

> 想symbol的名字, 並且建立dependency, yacc 語法熟悉

## Feedback to T.A.s

> 我測試的時候只看了前面三個testcase, 改一改就全對了, 建議後面的advanced 可以改更奇怪(? 不然可能不知道自己的grammar有沒有什麼漏洞
