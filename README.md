# learn-leveldb
![license](https://img.shields.io/github/license/mashape/apistatus.svg)
![learn-leveldb (Linux)](https://github.com/plantree/learn-leveldb/workflows/learn-leveldb%20(Linux)/badge.svg)

#### 1. 初衷

[LevelDB](https://github.com/google/leveldb)是一个快速的、可持久化存储的key-value数据库，基于LSTM-Tree。和Redis不同，其数据存储在磁盘上而不是内存中，代码实现有很多可取之处。更重要的是，代码行数不大（截止到2020.09.09），阅读起来不是很苦难。

![image-20200909160535066](https://img.plantree.me/image-20200909160535066.png)

目前工作中将倒排索引存储在磁盘上借鉴了其中的某些思想，因此说，无论是提升自己C++开发水平，还是更好的理解存储场景需要，都有必要对源码进行一番研究。当然工作繁忙，剩余的时间也许不多，一点点琢磨吧，过程中记录wiki。

#### 2. 阅读工具

- Visual Sudio Code
- Catch2
- Clang-1001.0.46.4
- CMake

#### 3. 阅读方法

LevelDB中有很多utils工具代码，因此打算采用最笨拙的方式：逐行复制。每个模块都匹配单元测试，以及整个过程的wiki记录。

#### 4. 过程记录







