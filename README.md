**Irono** 为现代C++编写的高性能服务端框架，主要采用reactor模式，
能帮助使用者快速搭建网络应用程序。

`example` 提供了一些具体应用事例，均已提供Makefile文件。于其中提供了一些利用irono框架实现一些具体功能的事例，如时间轮 Timing wheel、借 shared_ptr 实现copy-on-write等。


其中`base`为基础封装库, `net`为网络库，`others`为额外增加的供用户使用的网络功能。

详细文档见： 核心文档.md

开发文档见： 临时笔记.md

其余文档均已提供，如性能测试等，读者可自行查看。


编译需要boost库：

$ sudo apt install g++ make libboost-dev
