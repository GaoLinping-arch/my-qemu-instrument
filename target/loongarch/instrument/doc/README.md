# README

本项目是一个开源的龙架构平台（LoongArch）的用户态的动态二进制插桩框架。

该软件基于QEMU开发，并且实现了插桩框架
[Intel Pin](https://www.intel.com/content/www/us/en/developer/articles/tool/pin-a-dynamic-binary-instrumentation-tool.html)
的（部分）插桩API，你可以按照Pintool的写法编写插桩工具，在LoongArch平台上对指令、基本块、轨迹、系统调用、函数、镜像进行插桩。

本插桩框架基于QEMU开发，移除了TCG代码生成组件，重新实现了代码生成部分，相比QEMU有更高的执行效率（尤其是浮点运算）。缺点是其只能在LoongArch平台上运行，而不能像QEMU一样跨平台运行。

如果你想直接把Intel Pin中给出的pintool示例直接搬过来使用，这样做大概率是不行的。如果你想要使用Intel Pin给出的示例代码，需要进行修改，但工作量并不是很大，请参照本框架实现的API来修改原始示例代码。此外，本项目由于指令集之间的差异，无法实现全部的Pin API，也会导致无法直接使用Pin的官方代码示例。

如果你不太了解如何编写插桩工具，这里有一份 [简单易懂的pintool教程](./eazy_pintool.md)。

## 代码架构

请参考 [代码架构](./code_structure.md)

## 当前状态

该软件目前仍处在开发阶段，INS、BBL、Trace、RTN、IMG中大部分插桩功能均已实现，可以运行 SPEC 2006 及一些小型程序，更复杂的大型程序还待测试。

## 后续目标

该软件由于开发过程中的一些因素，性能并非十分高效。在本项目开发的下一阶段，会将主要精力用于提高本框架的运行效率之上。与此同时，本团队会继续将Pin的API进行完善，同时开发Pintool，以展示本软件在Loongarch平台上的一些应用。

## 快速上手

请参考[编译文档](./how_to_build.md)

## 插桩工具编写注意事项

- 插桩位置 `IPOINT` 目前只支持 `IPOINT_BEFORE`，即只支持在某条指令INS或某个基本块BBL前插入函数调用（这已经能应付大多数插桩场景）。（对于系统调用和函数插桩，允许使用 `IPONT_AFTER` 在系统调用返回和函数返回时插桩）
- 插桩函数没有内联优化，因此对每条指令INS都插桩会导致程序性能大幅下降，请尽量采用对基本块BBL插桩的方式收集信息。
- 建议使用 `BBL_InsertInlineAdd` 和 `INS_InsertInlineAdd` 这两个插桩API，它们的效果是令某个内存地址中的值加上指定的值，该插桩接口不会进行函数调用，可以编写出高性能的插桩工具。