/*
本文以itop4412 支持设备树Linux4.14版本源码分析其启动流程。（uboot分析在Linux-Ubootanalys中）

内核映像被加载到内存并获得控制权之后，内核启动流程开始。通常，内核映像以压缩形式存储，并不是一个可以执行的内核。因此，内核阶段的首要工作是自解压内核映像。
 
内核编译生成vmliunx后，通常会对其进行压缩，得到zImage（小内核，小于512KB）或bzImage（大内核，大于512KB）。在它们的头部嵌有解压缩程序。

通过linux/arch/arm/boot/compressed目录下的Makefile寻找到vmlinux文件的链接脚本（vmlinux.lds），从中查找系统启动入口函数
*/
