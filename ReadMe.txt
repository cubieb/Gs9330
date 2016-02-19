1 项目简介
 Gs9330项目的编码子模块。

2 代码目录结构
 2.1）所有代码都存在于Codes目录下。
 2.2）Codes/Include/Functions 用于存放功能模块需要被其他模块引用的.h文件，本目录所有代码都属于Gs9330命名空间。
 2.3）Codes/Include/Public 用于存放其他项目也可能用到的通用库的.h文件，本目录所有代码都属于缺省命名空间。
 2.4）Codes/Src/Functions 用于存放功能模块.cpp文件，本目录所有代码都属于Gs9330命名空间。
 2.5）Codes/Src/Public 用于存放其他项目也可能用到的通用库的.cpp文件， 本目录所有代码都属于缺省命名空间。

3 提交策略
 为了避免浪费过多的时间和项目组其他成员讨论代码和Project的层次结构。以下目录/文件不提交到项目的的svn server:
 3.1) Gs9330.sln
 3.2) VcProject, VcUnitTestProject
 3.3) Documents

ver.2.2:  采用最新的构架设计, 并且修改了所有已知的bug.
ver.2.5:  支持将单个table分为多个section的功能, 拆分时 nit 和 bat 将多个ts放入不同的section中. sdt 将多个service详细信息放入不同的section中. 
          eit将多个event信息放入不同的section中.
ver.2.6:  添加了一些有用的unit test的代码.  并根据unit test的结果对代码做了一些修改.
ver.2.7:  为了做性能优化, 特地创建一个branch,  避免性能优化的代码影响了 ver.2.6 稳定的代码.
          根据iso13818-1的描述， 修改类名 TsPacketInterface => TransportPacketInterface, TsPacketsInterface => TransportPacketsInterface.
ver.2.8:  对读取xml文件的效率做了优化, 优化后读取32个ts 一周的eit信息只需要3~4秒(优化前大约需要50秒).