# ChatServer

ChatServer 是一个基于 **Qt5 (C++20)** 开发的高性能即时通讯后端服务器。它通过 TCP 协议与客户端通信，提供用户注册、登录认证、好友管理以及消息转发等核心功能，适用于移动端或桌面端即时通讯应用的后端服务。

---

## 核心功能

- **用户注册** — 手机号注册，密码加盐存储
- **登录认证** — 账号密码验证，返回用户信息及好友列表
- **消息转发** — 在线用户间的即时消息投递
- **连接管理** — 用户在线状态追踪与自动清理
- **数据库连接池** — 多线程安全、连接复用的 MySQL 连接管理

---

## 技术栈

| 项目 | 详情 |
|------|------|
| **编程语言** | C++20 |
| **框架** | Qt5 (Core, Network, Sql) |
| **数据库** | MySQL (QMYSQL 驱动) |
| **构建工具** | CMake (≥ 3.31) |
| **通信协议** | 自定义二进制协议 (BigEndian) |
| **许可证** | Apache 2.0 |

---

## 项目结构

```
ChatServer/
├── CMakeLists.txt              # CMake 构建配置
├── LICENSE                     # Apache 2.0 许可证
├── include/                    # 头文件
│   ├── tcpserver.h             # TCP 服务器（连接监听）
│   ├── deschandle.h            # 会话调度核心（协议解析/任务分发）
│   ├── transmit.h              # 消息转发模块
│   ├── login.h                 # 登录业务 (QRunnable)
│   ├── register.h              # 注册业务 (QRunnable)
│   ├── connectionpool.h        # 数据库连接池（单例）
│   └── connectsql.h            # 简单数据库连接封装（已弃用）
├── src/                        # 源文件
│   ├── main.cpp                # 程序入口
│   ├── tcpserver.cpp
│   ├── deschandle.cpp
│   ├── transmit.cpp
│   ├── login.cpp
│   ├── register.cpp
│   ├── connectionpool.cpp
│   └── connectsql.cpp
└── .clang-tidy                 # Clang-Tidy 静态检查配置
```

---

## 快速开始

### 环境要求

| 依赖 | 版本要求 |
|------|----------|
| CMake | ≥ 3.31 |
| Qt5 | Core / Network / Sql 模块 |
| MySQL | 8.x（推荐） |
| 编译器 | 支持 C++20 (MSVC / Clang / GCC) |

### 构建步骤

```bash
# 1. 克隆项目
git clone <repository-url>
cd ChatServer

# 2. 配置 CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 3. 编译
cmake --build build

# 4. 运行（确保 MySQL 服务可用）
./build/ChatServer
```

### MySQL 配置

数据库连接参数在 [main.cpp](file:///D:/ProjectFiles/ClionProject/ChatServer/src/main.cpp) 中配置，请根据实际环境修改：

| 参数 | 说明 |
|------|------|
| 主机 | 数据库服务器地址 |
| 端口 | 数据库端口（默认 3306） |
| 数据库名 | 数据库名称 |
| 用户名 | 数据库用户名 |
| 密码 | 数据库密码 |

---

## 架构概览

### 分层架构

```
┌──────────────────────────────────────────┐
│             网络接入层                     │
│         TcpServer (QTcpServer)           │
│              监听 8111 端口              │
├──────────────────────────────────────────┤
│             会话调度层                     │
│           DescHandle (QObject)           │
│     用户管理 / 协议解析 / 任务分发         │
├──────────┬───────────┬───────────────────┤
│ 登录业务  │  注册业务  │    消息转发       │
│ Login    │ Register  │   Transmit        │
│(线程池)   │ (线程池)   │  (独立线程)       │
├──────────┴───────────┴───────────────────┤
│             数据访问层                     │
│        ConnectionPool (单例)             │
│           MySQL 连接池管理                │
└──────────────────────────────────────────┘
```

### 线程模型

- **主线程** — 运行 QCoreApplication 事件循环，处理 TCP 连接和信号槽调度
- **线程池** (QThreadPool, 最大 4 线程) — 执行登录和注册等数据库密集型任务
- **独立线程** — Transmit 消息转发模块在独立 QThread 中运行，避免阻塞主线程
- **跨线程通信** — 使用 `QMetaObject::invokeMethod` + `Qt::QueuedConnection` 确保线程安全

---

## 通信协议

客户端与服务器之间使用自定义二进制协议，大端字节序 (BigEndian)：

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
├─────────────────────────────────────────────────────────────────┤
│                       MAGIC_NUMBER (4 bytes)                     │
│                          0x4A3B2C1D                              │
├─────────────────────────────────────────────────────────────────┤
│                       COMMAND_TYPE (4 bytes)                     │
│                   0x001=登录 0x002=注册 0x003=消息               │
├─────────────────────────────────────────────────────────────────┤
│                       TIMESTAMP (8 bytes)                        │
│                    Unix 毫秒时间戳                                │
├─────────────────────────────────────────────────────────────────┤
│                       DATALENGTH (4 bytes)                       │
│                    JSON 负载字节长度                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│                       JSON PAYLOAD (变长)                        │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

协议头部固定 **20 字节**，之后跟随 JSON 格式的数据体。服务器通过 MAGIC_NUMBER (`0x4A3B2C1D`) 验证数据有效性，无效数据将直接断开连接。

---

## 开源协议

本项目基于 [Apache License 2.0](LICENSE) 开源。