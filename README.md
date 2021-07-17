## 软件简介
- intercept 是应用透明链路追踪工具。

- intercept 追踪php核心调用库运行时信息并且记录到日志里。

- 依赖于intercept 很容易构建基于php语言的无浸入分布式全链路追踪系统。

## 安装
- 以下是你需要做的安装intercept在你的系统上。

1. phpize
2. ./configure
3. make && make install
4. make install 复制 pid.so 到确切的位置, 但是你还需要开启模块在php配置中,编辑你自己的php.ini, 并且添加如下内容:extension=pid.so。

## 基础配置
- pid.log_path 日志目录。
- pid.function 要拦截的函数多个逗号分割。

1. pid.log_path=/tmp/
2. pid.function=date,aa

## TODO
- 支持zipkin/optracing格式,真正的链路追踪