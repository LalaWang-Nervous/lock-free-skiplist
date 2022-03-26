# lock-free Skiplist

## Quickly Start

```shell
# 编译
sh run.sh 
# 单元测试
./output/unit_test
# 性能测试
./output/performance
# kv服务模拟, 单写进程，随机写入、删除，100个读进程，随机读取
# 此进程无限循环
./output/kv_service
```

## 目录结构说明

```
.
├── CMakeLists.txt
├── demo
│   ├── kv_service.cpp        // 模拟KV服务实现
│   └── performance.cpp       // 性能测试
├── output                    // 编译脚本生成的可执行文件
│   ├── kv_service
│   ├── performance_test
│   └── unit_test
├── README.md
├── run.sh                    // 编译脚本
├── src
│   ├── Serializers.hpp       // 序列化相关实现
│   └── Skiplist.hpp          // 跳表实现
├── thirdparty
│   ├── googletest            // googletest测试框架
│   └── nlohmann_json         // json解析库
└── utest
    └── Skiplist_utest.cpp    // 跳表实现的单元测试
```

## 特性

- 支持CRUD

- 支持dump/load，将内存中数据存储为json文件

- 模板实现，支持自定义键值类型（如需dump/load， 需要实现自定义类型的序列化方法）

- 基于memory order语义无锁化实现，支持单写多读并发

## 性能测试

测试机器1硬件状况：

| 属性         | 值                                         |
|:---------- | ----------------------------------------- |
| OS         | Deepin20.4 community                      |
| Linux 内核版本 | 5.10.101-amd64-desktop                    |
| CPU        | Intel(R) Core(TM) i5-7300HQ CPU @ 2.50GHz |
| RAM        | 8GB                                       |

测试结果：

| 测试基础信息                                                        | 操作数             | 耗时        | QPS   |
| ------------------------------------------------------------- | --------------- | --------- | ----- |
| key type : int, value : std::string, kv值为有序值（1，2，3....），单线程写入 | 100W            | 1.05818s  | 94.5w |
| key type : int, value : std::string, kv值为随机生成值，单线程写入          | 100W            | 2.29917s  | 43.5w |
| key type : int, value : std::string, kv值为随机生成值，单线程读取          | 100W            | 2.228174s | 43.8w |
| key type : int, value : std::string, kv值为随机生成值，100线程并发读取      | 100$\times$100W | 68.8386s  | 145w  |

测试机器2硬件状况：

| 属性  | 值              |
| --- | -------------- |
| OS  | MacOS Monterey |
| CPU | Apple M1       |
| RAM | 8GB            |

测试结果

| 测试基础信息                                                        | 操作数             | 耗时       | QPS   |
| ------------------------------------------------------------- | --------------- | -------- | ----- |
| key type : int, value : std::string, kv值为有序值（1，2，3....），单线程写入 | 100W            | 1.15124s | 86.8w |
| key type : int, value : std::string, kv值为随机生成值，单线程写入          | 100W            | 2.15202s | 46.4w |
| key type : int, value : std::string, kv值为随机生成值，单线程读取          | 100W            | 2.08174s | 48w   |
| key type : int, value : std::string, kv值为随机生成值，100线程并发读取      | 100$\times$100W | 34.5938s | 289w  |

## 单元测试覆盖情况

| 覆盖类型  | 覆盖率  |
| ----- | ---- |
| 函数覆盖率 | 100% |
| 分支覆盖率 | 80%  |

## 设计思路

- 基本的跳表原理实现不再赘述，详情看代码实现细节及注释；

- 支持增删改查，但不支持相同键的键值对插入

- 最高高度默认值为32，默认设置下生成随机节点高度时以1/4概率升高

- 考虑到跳表所能提供的KV本身的通用性，因此需要模板实现，可以根据需要进行特化

- 考虑到希望支持dump/load，那么就需要有相应的序列化反序列化手段，由于自存在定义类型，直接实现一个覆盖各种可能的序列化、反序列化方法是不合理的，应当由对应的自定义类型定义方提供序列化反序列化方法，具体地，这里定义了将对象转为json格式字符串和反向操作的接口，因此若有dump/load需求，构造跳表时要实现对应接口

- 考虑到希望实现无锁化的单写并发读，这里使用了memory order语义，将跳表中一些读写操作存在竞态可能的属性，如跳表高度、节点的next指针等设置为std::atomic类型，读场景使用memory order acquire语义，写场景使用memory order release语义

- 所有节点的内存在CRUD过程中动态申请，均不释放，直至跳表对象被销毁
