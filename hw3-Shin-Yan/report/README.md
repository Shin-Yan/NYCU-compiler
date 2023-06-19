# hw3 report

|||
|-:|:-|
|Name|yourname|
|ID|your student id|

## How much time did you spend on this project

> e.g. 2 hours.

3 days (Implement) + 2 days (Read Spec)

## Project overview

> Please describe the structure of your code and the ideas behind your implementation in an organized way.
> The point is to show us how you deal with the problems. It is not necessary to write a lot of words or paste all of your code here.

首先, 要修改scanner.l並設定yylval給parser.y使用, 因為會需要使用到attribute的部分

接下來要修改parser.y, 也是整個project比較麻煩的部分, 要先定義好需要用到的yylval,
並需要小心每個attribute pass到上一層的時候有沒有出問題, 並且要製造出對應的Node,
而這次使用OOP的概念, 每個Node都繼承自AstNode並實作自己的accept visitChild 等等,
助教很貼心地寫好大部分模板我們只需要修改一些constructor跟getMember之類的function即可

最後就是利用vistor pattern去遍歷所有的node, 並要注意在AstDumper裡面要修改一些簡單的TODO,
然後流程大致如下:
1. 由main function 中 call root->accept(*visitor)
2. pass visitor by reference, 然後Astnode accept裡面會call visitor.visit(*this), 此行為允許visitor訪問 *this, 而非只是AstNode(真正的型別)
3. 接下來 visit 由AstDumper實作 印出重要信息並call visitChildNode
4. 重複2~4的步驟

## What is the hardest you think in this project

> Not required, but bonus point may be given.
最難的地方是看Spec要看很久, 然後要注意到parser.y 中attribute是無法使用std::string的, 所以要用char* 去做一些比較麻煩的字串處理
還有OOP的概念不是那麼熟, 花了點時間才回憶起來怎麼使用, 並且對於多型有比較多認識, visitor patter其實寫起來還滿舒服的
最後應該就是debug了, 這code沒寫好很容易會有segmentation fault, 我有一個bug雖然很智障但最後是用gdb debug出來的, 畢竟print 大法對seg fault沒用QQ


## Feedback to T.A.s

> Not required, but bonus point may be given.
謝謝助教給出這個作業, 雖然重複的地方很多, 但這也才更了解AST Tree, 還有使用OOP的重要性
再來其實更多的是(雖然前兩個作業也是) 助教把很雜的事情也都處理掉了 像是Makefile, docker等等的 
還出了不錯的test case讓我們能發現自己的程式可能錯在哪裡 真的很謝謝助教