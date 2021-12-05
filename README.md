**Irono** 为现代C++编写的高性能服务端框架，主要采用reactor模式，
能帮助使用者快速搭建网络应用程序。

编译需要boost库：

#Debian, Ubuntu, etc.

$ sudo apt install g++ cmake make libboost-dev

#CentOS

$ sudo yum install gcc-c++ cmake make boost-devel

/irono/example  提供了一些具体应用事例，均已提供Makefile文件。

其中`base`为基础封装库, `net`为网络库，`others`为额外增加的供用户使用的网络功能(解编码器、时间轮等)。

详细文档见： 核心文档.md

开发文档见： 临时笔记.md

其余文档均已提供，如性能测试等，读者可自行查看。