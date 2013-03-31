# 毕业设计

## 简介

Scheme方言子集的解释器。开发流程和一些实现思路参考了[Scheme From Scratch](http://michaux.ca/articles/scheme-from-scratch-introduction)和[chibi-scheme-0.2](http://synthcode.com/wiki/chibi-scheme)。编译器的实现（compile.c）和虚拟机的实现（vm.c）参考了Peter Norvig写的[《Paradigms of Artificial Intelligence Programming: Case Study in Common Lisp》](http://norvig.com/paip.html)的第23章。

## 特性

### 数据类型

1. 定长数
2. 布尔类型
3. 字符类型
4. 字符串类型
5. 空表
6. 点对类型
7. 符号类型
8. 原语和用户自定义的函数
9. 输入与输出文件流
10. 一维向量
11. 编译过的函数
12. 单精度浮点数

### 函数库

参见src/proc.c文件中的init\_environment函数和.liut.scm文件

### 安装与使用

编译得到可执行的REPL，产生文件./liutscm：

    make liutscm

编译得到可执行的REPL测试程序，产生文件./run-repl-test：

    make run-repl-test

编译得到可执行的编译器测试程序，产生文件./run-compiler-test：

    make run-compiler-test

编译得到可执行的虚拟机测试程序，产生文件./run-vm-test：

    make run-vm-test

编译得到可执行的汇编函数测试程序，产生文件./run-asm-test：

    make run-asm-test

## 作者

Liutos(<mat.liutos@gmail.com>)

## License

Copyright (C) 2013 Liutos, MIT License
