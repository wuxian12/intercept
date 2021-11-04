## 软件简介
- performance 是应用透明链路追踪工具。

- 该扩展主要是收集数据,处理数据有其它语言完成,例如php脚本处理数据

- performance 追踪php核心调用库运行时信息并且记录到日志里。

- performance 很容易构建基于php语言的无浸入分布式全链路追踪系统。

## 安装
- 以下是你需要做的安装performance在你的系统上。

1. phpize
2. ./configure
3. make && make install
4. make install 复制 performance.so 到确切的位置, 但是你还需要开启模块在php配置中,编辑你自己的php.ini, 并且添加如下内容:extension=performance.so。


## TODO
- 支持php7.0以上版本
- 拦截更多函数
- 支持swoole