1 项目简介
 Gs9330项目的编码子模块。

2 代码目录结构
 2.1）所有代码都存在于Codes目录下。
 2.2）Codes/Include/Functions 用于存放功能模块需要被其他模块引用的.h文件，本目录所有代码都属于Gs9330命名空间。
 2.3）Codes/Include/Public 用于存放其他项目也可能用到的通用库的.h文件，本目录所有代码都属于缺省命名空间。
 2.4）Codes/Src/Functions 用于存放功能模块.cpp文件，本目录所有代码都属于Gs9330命名空间。
 2.5）Codes/Src/Public 用于存放其他项目也可能用到的通用库的.cpp文件，本目录所有代码都属于缺省命名空间。

3 提交策略
 为了避免浪费过多的时间和项目组其他成员讨论代码和Project的层次结构。以下目录/文件不提交到项目的的svn server:
 3.1) Gs9330.sln
 3.2) VcProject, VcUnitTestProject
 3.3) Documents