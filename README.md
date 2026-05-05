# PersonalStudy

存放个人学习文件

---

```
D:.
│  .gitignore
│  README.md
└─cpp
    ├─异步编程 - 包含有适用于CPU密集型的线程池代码
    └─设计模式
```

---

# 待办清单

- [ ] 生产者-消费者队列

- [ ] 无锁队列

- [ ] 中间件
  
  - [ ] 消息队列中间件：用于实现应用程序之间的异步消息传递
    
    - Kafka：librdkafka
    
    - RocketMQ：阿里开源，经历过双十一的考验，主要为Java
    
    - ActiveMQ：老牌，不要用
  
  - [ ] 缓存中间件：用于临时存储频繁访问的数据到内存，加快系统访问速度，以提高系统性能
    
    - Redis
    
    - Memcached
  
  - [ ] Web服务器中间件：用于接受和处理HTTP请求，提供Web服务
    
    - Nginx：高性能的Web服务器和反向代理服务器
  
  - [ ] 通信中间件
    
    - RPC远程过程调用
      
      - gRPC -  Google开源
      
      - brpc - 百度开源
    
    - DDS（Data Distribution Service数据分发服务）
  
  - [ ] 应用服务器中间件：用于托管和管理应用程序的执行环境
    
    - Tomcat - Java
    
    - IIS(Internet Information Services) - .Net

- [ ] Linux Epoll网络编程/Windows IOCP

- [ ] C++事件驱动网络库
  
  - muduo - C++11多线程Linux服务器网络库
  
  - Boost.Asio - 跨平台的网络与I/O库

- [ ] [数据库连接池](https://www.bilibili.com/video/BV17PoRBtECc?spm_id_from=333.788.videopod.episodes&vd_source=4bdb551b219dd3278ebb7f1179036dc4&p=4)
  
  - 用于直观展示程序在运行时的函数调用栈及其资源占用情况（如CPU时间、内存分配等）。它通过层次化的图形结构，帮助开发者快速定位性能瓶颈。

- [ ] 内存泄漏检测工具
  
  - Valgrind
  
  - Windbg
  
  - MTrace

- [ ] 性能分析工具
  
  - perf - Linux下的一个性能分析工具
  
  - 火焰图（Flame Graph） - 本质上是将perf采集的文本数据渲染成了SVG图形

- [ ] C++ 20 coroutine

- [ ] 设计模式

- [ ] UE5
  
  - [ ] 棋牌游戏开发
  - [ ] [全网最好的UE5 3A级多人射击游戏C++开发课程（1-9章）_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1Le411w7FC/?spm_id_from=333.1245.0.0&vd_source=4bdb551b219dd3278ebb7f1179036dc4)
