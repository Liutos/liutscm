# 毕业设计

## 简介

Scheme方言子集的解释器。开发流程和一些实现思路参考了[Scheme From Scratch](http://michaux.ca/articles/scheme-from-scratch-introduction)和[chibi-scheme-0.2](http://synthcode.com/wiki/chibi-scheme)。

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

### 函数库

参见src/proc.c文件中的init_environment函数和.liut.scm文件

### 安装与使用

编译得到可执行的REPL，产生文件./liutscm：

    make liutscm

编译得到可执行的测试程序，产生文件./run-test：

    make run-test

## 作者

Liutos(<mat.liutos@gmail.com>)

## License

Copyright (C) 2013 Liutos, MIT License
