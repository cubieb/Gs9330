1 项目简介
 Gs9330项目的编码子模块。

2 代码目录结构
 2.1）所有代码都存在于Codes目录下。
 2.2）Codes/Include 用于存放功能模块需要被其他模块引用的.h文件，本目录所有代码都属于Gs9330命名空间。
 2.4）Codes/Src 用于存放功能模块.cpp文件，本目录所有代码都属于Gs9330命名空间。

3 提交策略
 为了避免浪费过多的时间和项目组其他成员讨论代码和Project的层次结构。以下目录/文件不提交到项目的的svn server:
 3.1) Gs9330.sln
 3.2) VcProject, VcUnitTestProject
 3.3) Documents

ver.2.2:  采用最新的构架设计, 并且修改了所有已知的bug.
ver.2.5:  支持将单个table分为多个section的功能, 拆分时 nit 和 bat 将多个ts放入不同的section中. sdt 将多个service详细信息放入不同的section中. 
          eit将多个event信息放入不同的section中.
ver.2.6:  正式开始提交测试部测试, 从本版本开始, 之后所有代码的bug都统计为Coding Bug.
          添加了一些有用的unit test的代码.  并根据unit test的结果对代码做了一些修改.
ver.2.7:  为了做性能优化, 特地创建一个branch,  避免性能优化的代码影响了 ver.2.6 稳定的代码.
          根据iso13818-1的描述， 修改类名 TsPacketInterface => TransportPacketInterface, TsPacketsInterface => TransportPacketsInterface.
ver.2.8:  对读取xml文件的效率做了优化, 优化后读取32个ts 一周的eit信息只需要3~4秒(优化前大约需要50秒).
          增加了一些单元测试用例.
          在ver.2.8的基础上, 创建了一个ver.Debug.2.8. 用于测试多线程是否对读取xml文件的效率有明显的提升。 效果不理想。
          Ver.Debug.2.8 可用于将来做其他性能测试的基础版本。
ver.2.9:  增加对中心频点的支持, receiver.xml每个网络的第一条receiver 就是中心频点.
          新增模板类 SiTableTemplate 做为sdt, bat, nit, eit的基类, 利用SiTableTemplate, 4张表可以在GetCodeSize(), GetSecNumber()上清除掉很多冗余的代码.
ver.3.0:  通过环境变量读取发送配置的路径.
          修改SiTableXmlWrapperInterface::Select()的申明, 新的函数定义通过return 返回xml中读取到的SiTable。
          发布给"凌海"的稳定版本.
ver.3.1:  修改在凌海部署时发现的需求上的bug. 
          1 新增功能, 支持组播地址  做为EpgSender的目的地址. 
          2 新增功能, 支持根据不同的tsid,将eit的pid映射为不同的pid.
          
4 实测数据
    983,116字节的xml编码后的Ts为178,788字节.
    
5 备注
    在实测过程中,偶尔会发现EpgSender.exe 发出的包在wireshark里面显示为Malformed Packet. 这有可能是wireshark的bug, 保存当前captured包到文件, 然后再用wireshark打开保存的文件, 如果不再显示Malformed Packet, 则说明EgpSender.exe发出的包是没有问题的. 为了避免再浪费更多的时间来找Malformed Packet的原因, 特将wirechar的bug的特殊情况记录在此。