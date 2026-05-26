# ChatServer Code Wiki

## 1. 项目概述

**ChatServer** 是一个基于 **Qt5 (C++20)** 开发的即时通讯后端服务器程序。它通过 TCP 协议与客户端通信，提供用户注册、登录认证、用户在线状态管理以及消息转发等核心功能。

| 项目         | 详情                                 |
| ------------ | ------------------------------------ |
| **项目名称** | ChatServer                           |
| **编程语言** | C++20                                |
| **框架依赖** | Qt5 (Core, Network, Sql)             |
| **数据库**   | MySQL (通过 QMYSQL 驱动)             |
| **构建工具** | CMake (≥ 3.31)                       |
| **设计模式** | 单例模式 (ConnectionPool)、观察者模式 (信号槽)、命令模式 (QRunnable 任务) |
| **IDE**      | CLion                                |

---

## 2. 项目目录结构

```
ChatServer/
├── CMakeLists.txt              # CMake 构建配置文件
├── LICENSE                     # 开源许可证
├── README.md                   # 项目简介
├── .gitignore                  # Git 忽略规则
├── .clang-tidy                 # Clang-Tidy 静态检查配置
│
├── include/                    # 头文件目录
│   ├── tcpserver.h             # TCP 服务器
│   ├── deschandle.h            # 描述符处理器 / 会话管理器 (核心枢纽)
│   ├── transmit.h              # 消息转发模块
│   ├── login.h                 # 登录业务 (QRunnable)
│   ├── register.h              # 注册业务 (QRunnable)
│   ├── connectionpool.h        # 数据库连接池 (单例)
│   └── connectsql.h            # 简单数据库连接封装 (早期版本)
│
├── src/                        # 源文件目录
│   ├── main.cpp                # 程序入口
│   ├── tcpserver.cpp           # TCP 服务器实现
│   ├── deschandle.cpp          # 描述符处理器实现 (核心枢纽)
│   ├── transmit.cpp            # 消息转发实现
│   ├── login.cpp               # 登录业务实现
│   ├── register.cpp            # 注册业务实现
│   ├── connectionpool.cpp      # 数据库连接池实现
│   └── connectsql.cpp          # 简单数据库连接实现
│
└── .idea/                      # CLion IDE 配置 (自动生成)
```

---

## 3. 整体架构

### 3.1 分层架构图

```
┌──────────────────────────────────────────────────────────┐
│                   网络接入层 (Network Layer)                │
│                    TcpServer (QTcpServer)                │
│                   监听 8111 端口                         │
├──────────────────────────────────────────────────────────┤
│                   会话调度层 (Session Layer)               │
│                    DescHandle (QObject)                   │
│         用户池管理 / 协议解析 / 任务分发 / 消息发送         │
├───────────┬──────────────────────────┬───────────────────┤
│           │                          │                   │
│   登录业务层     │       注册业务层       │   消息转发层      │
│ Login(QRunnable)│ Register(QRunnable)  │ Transmit(QObject)│
│   线程池执行     │     线程池执行        │   独立 QThread   │
├───────────┴──────────────────────────┴───────────────────┤
│                     数据访问层 (Data Layer)                │
│                  ConnectionPool (单例)                    │
│                   MySQL 连接池管理                        │
└──────────────────────────────────────────────────────────┘
```

### 3.2 请求处理流程

```
客户端连接 → TcpServer::incomingConnection()
                  ↓
        DescHandle::recvDescriptor()
                  ↓
        DescHandle::actionCheck()  [创建 User, 绑定 QTcpSocket]
                  ↓
        DescHandle::onReadyRead()  [读取数据, 解析二进制协议]
                  ↓
        DescHandle::taskAssign()   [按 COMMAND_TYPE 分发]
              ↙        ↓        ↘
        0x001(登录)  0x002(注册)  0x003(消息转发)
           ↓           ↓           ↓
      Login::run()  Register::run()  Transmit::working()
       (线程池)      (线程池)          (独立线程)
           ↓           ↓           ↓
    登录成功/失败  注册结果回调   taskFinish 信号
           ↓           ↓           ↓
      DescHandle::loginSuccess/loginFailed/registerHandle/sendMsg
                  ↓
            向客户端 socket 写入二进制响应
```

---

## 4. 核心数据结构

### 4.1 User 结构体 ([deschandle.h](file:///D:/ProjectFiles/ClionProject/ChatServer/include/deschandle.h#L23-L33))

表示一个已连接的客户端用户（可能尚未登录）。

| 字段        | 类型                          | 说明                     |
|------------|-------------------------------|--------------------------|
| `uid`      | `qint32`                      | 用户数据库 ID（登录后才赋值） |
| `userInfo` | `QJsonObject`                 | 用户详细信息 JSON            |
| `tcpSocket` | `QSharedPointer<QTcpSocket>` | 客户端的 TCP socket 智能指针 |
| `buffer`   | `QByteArray`                  | 未解析完全的累积数据缓冲区     |

### 4.2 Task 结构体 ([transmit.h](file:///D:/ProjectFiles/ClionProject/ChatServer/include/transmit.h#L16-L46))

表示一条待转发的聊天消息。

| 字段           | 类型        | 说明                          |
|---------------|-------------|-------------------------------|
| `action`      | `QString`   | 动作类型 ("send" / "recipient") |
| `sender`      | `QString`   | 发送者 uid                     |
| `senderName`  | `QString`   | 发送者用户名                    |
| `recipient`   | `QString`   | 接收者 uid                     |
| `msgType`     | `qint32`    | 消息类型 (0=文本等)              |
| `content`     | `QString`   | 消息内容                       |
| `outgoing`    | `qint32`    | 方向标识 (1=发出的)              |
| `sendTimeStamp` | `qint64`  | 发送时间戳 (Unix 秒)            |

---

## 5. 模块详解

### 5.1 网络接入层 — TcpServer

**文件**: [tcpserver.h](file:///D:/ProjectFiles/ClionProject/ChatServer/include/tcpserver.h) / [tcpserver.cpp](file:///D:/ProjectFiles/ClionProject/ChatServer/src/tcpserver.cpp)

| 项目       | 内容                               |
|-----------|------------------------------------|
| **基类**  | `QTcpServer`                       |
| **端口**  | 8111 (默认)                         |
| **职责**  | 监听端口, 接收客户端连接, 将描述符传递给 DescHandle |

**关键方法**:

| 方法                     | 说明                              |
|--------------------------|-----------------------------------|
| `TcpServer(parent, port)` | 构造函数：调用 `initHandle()` 并开始监听 |
| `incomingConnection(qintptr)` | 重写 QTcpServer 虚方法，发射 `newDescriptor` 信号 |
| `initHandle()`             | 创建 DescHandle，连接 `newDescriptor` 和 `startWorker` 信号 |

**信号**:

| 信号              | 说明                           |
|------------------|--------------------------------|
| `newDescriptor(qintptr)` | 通知 DescHandle 有新连接描述符 |
| `startWorker()`          | 触发 DescHandle 工作循环    |

---

### 5.2 会话调度层 — DescHandle (核心枢纽)

**文件**: [deschandle.h](file:///D:/ProjectFiles/ClionProject/ChatServer/include/deschandle.h) / [deschandle.cpp](file:///D:/ProjectFiles/ClionProject/ChatServer/src/deschandle.cpp)

这是整个服务器的**核心模块**，管理用户生命周期、协议解析、任务分发、消息发送。

**成员变量**:

| 变量             | 类型                        | 说明                              |
|-----------------|-----------------------------|-----------------------------------|
| `transmiter`    | `Transmit*`                 | 消息转发模块指针                     |
| `transferStation` | `QThread*`                 | 消息转发模块的独立工作线程             |
| `descQue`       | `QQueue<qintptr>`           | 待处理的套接字描述符队列               |
| `tempId`        | `qint32`                    | 自增的临时 ID 生成器                  |
| `userPool`      | `QHash<qint32, User>`       | 用户池 (key=tempId, 登录前暂存)       |
| `tcpPool`       | `QHash<qint32, User>`       | 连接池 (key=uid, 已登录的持久连接)     |
| `queLock`       | `QMutex`                    | 描述符队列锁                         |
| `userLock`      | `QMutex`                    | 用户池锁                            |
| `tcpLock`       | `QMutex`                    | 连接池锁                            |

**核心方法**:

| 方法                              | 说明                                                   |
|-----------------------------------|--------------------------------------------------------|
| `recvDescriptor(qintptr)`          | 将新描述符入队，调用 `startWorker`                     |
| `actionCheck()`                    | 创建 User 对象、绑定描述符、连接 readyRead/disconnected 信号 |
| `onReadyRead(qint32 tid)`          | 读取 socket 数据，解析二进制协议头部，提取完整数据包      |
| `taskAssign(cmd, tid, packet)`     | 根据 COMMAND_TYPE 分发给 Login / Register / Transmit    |
| `loginFailed(json)`               | 登录失败时发送错误响应并移除临时用户                      |
| `loginSuccess(rootJson, result)`  | 登录成功时将用户从 userPool 移至 tcpPool，发送结果       |
| `registerHandle(json)`            | 注册结果回调，发送结果给客户端                             |
| `sendMsg(recipientUid, json)`     | 向指定 uid 的在线用户发送消息                             |
| `buildStream(cmd, json)`          | 构造二进制协议数据包                                     |
| `onDisconnect(qint32 poolId)`     | 客户端断开时清理 userPool                                 |

**协议解析逻辑** (`onReadyRead`):

1. 累积数据至 `buffer`
2. 检查是否 >= 20 字节（协议头最小长度）
3. 用 `QDataStream` (BigEndian) 依次读取：MAGIC_NUMBER → COMMAND_TYPE → TIMESTAMP → DATALENGTH
4. 验证 MAGIC_NUMBER == `0x4A3B2C1D`，否则断开连接
5. 计算完整包大小 `20 + DATALENGTH`，提取完整包，剩余数据保留
6. 发射 `readComplete` 信号交由 `taskAssign` 处理

---

### 5.3 消息转发层 — Transmit

**文件**: [transmit.h](file:///D:/ProjectFiles/ClionProject/ChatServer/include/transmit.h) / [transmit.cpp](file:///D:/ProjectFiles/ClionProject/ChatServer/src/transmit.cpp)

| 项目      | 内容                                 |
|----------|--------------------------------------|
| **基类** | `QObject`                            |
| **线程** | 在 `transferStation` (独立 QThread) 中运行 |
| **职责** | 将 "send" 消息转化为 "receive" 消息, 触发投递 |

**核心方法**:

| 方法                    | 说明                          |
|------------------------|-------------------------------|
| `working(json)`        | 将消息入队并调用 `processTask()` |
| `processTask()`        | 出队, 将 action 从 "send" 改为 "receive", 发射 `taskFinish(recipient, json)` |

**信号**:

| 信号                              | 说明                     |
|-----------------------------------|--------------------------|
| `taskFinish(qint32, QJsonObject)` | 通知 DescHandle 向接收者发送消息 |

---

### 5.4 登录业务 — Login

**文件**: [login.h](file:///D:/ProjectFiles/ClionProject/ChatServer/include/login.h) / [login.cpp](file:///D:/ProjectFiles/ClionProject/ChatServer/src/login.cpp)

| 项目       | 内容                                     |
|-----------|------------------------------------------|
| **基类**  | `QRunnable`                                |
| **执行**  | `QThreadPool` 全局线程池 (最大 4 线程)       |
| **职责**  | 验证账号密码, 查询用户信息和好友列表           |

**核心方法**:

| 方法                             | 说明                                                |
|----------------------------------|-----------------------------------------------------|
| `run()`                          | 线程入口, 调用 `loginResult()`                        |
| `loginResult(id, json)`          | 执行两次数据库查询：①验证用户 ②查询好友关系            |
| `buildJsonMsg(code, msg)`        | 构造登录结果 JSON                                     |

**数据库查询逻辑 (`loginResult`)**:

- **第一次查询**: `SELECT users.uid, phone, password_hash, salt, account_status, username, avatar FROM users NATURAL JOIN user_profiles WHERE phone = ?`
  - 验证 `password_hash == 客户端密码 + salt`
  - 成功后读取 uid / username / avatar
- **第二次查询**: 查询好友关系 (双向 `friendships` 表 `UNION`)
  - 填充 `friendUid` JSON 数组
- 通过 `QMetaObject::invokeMethod` (QueuedConnection) 安全地回调 `DescHandle::loginSuccess` 或 `DescHandle::loginFailed`

---

### 5.5 注册业务 — Register

**文件**: [register.h](file:///D:/ProjectFiles/ClionProject/ChatServer/include/register.h) / [register.cpp](file:///D:/ProjectFiles/ClionProject/ChatServer/src/register.cpp)

| 项目       | 内容                                |
|-----------|-------------------------------------|
| **基类**  | `QRunnable`                          |
| **执行**  | `QThreadPool` 全局线程池             |
| **职责**  | 新建用户账号, 写入数据库              |

**核心方法**:

| 方法                   | 说明                                          |
|------------------------|-----------------------------------------------|
| `run()`                | 线程入口: 检查账号存在 → 插入新用户 → 回调结果    |
| `acIsExists()`         | 查询 `users` 表 `phone` 字段, 检查账号是否已存在 |
| `salt(int len)`        | 使用 `QRandomGenerator::system()` 生成密码学安全随机盐值 |
| `insertInfoDB()`       | 执行 INSERT, 存储 `phone`, `password_hash`, `salt` |
| `buildJsonMsg(code, msg)` | 构造注册结果 JSON                              |

**密码存储**:
- 生成 32 字节随机盐 (`salt(32)`)
- 存入 `password_hash = 客户端密码 + 盐值` (明文拼接, 未使用标准哈希算法)
- 盐值单独存储于 `salt` 字段

---

### 5.6 数据库连接池 — ConnectionPool

**文件**: [connectionpool.h](file:///D:/ProjectFiles/ClionProject/ChatServer/include/connectionpool.h) / [connectionpool.cpp](file:///D:/ProjectFiles/ClionProject/ChatServer/src/connectionpool.cpp)

| 项目       | 内容                                    |
|-----------|-----------------------------------------|
| **模式**  | 单例 (Meyer's Singleton)                 |
| **数据库** | MySQL (QMYSQL 驱动)                     |
| **最大连接** | 默认 10                                |
| **预创建** | 初始化时创建 `maxConnections / 2` 个连接   |
| **有效性验证** | 归还连接时执行 `SELECT 1` 验证          |

**核心方法**:

| 方法                          | 说明                                             |
|------------------------------|--------------------------------------------------|
| `instance()`                 | 返回全局唯一单例实例                                |
| `init(host, port, db, user, pwd, maxConn=10)` | 初始化连接池, 预创建半量连接 |
| `getConnection()`            | 从队列获取连接, 队列空且未达上限时创建新连接            |
| `releaseConnection(conn)`    | 归还连接; 失效连接则关闭并从驱动中移除                 |
| `destroy()`                  | 关闭所有连接, 清空队列                              |
| `createConnection()`         | 创建单个新数据库连接                                |

**连接管理策略**:
- 线程安全: 所有公共方法均使用 `QMutexLocker` 加锁
- 延迟创建: 初始化时仅创建一半连接 (`maxConnections / 2`), 剩余按需创建
- 验证复用: 归还时执行 `SELECT 1`, 通过则入队复用, 不通过则关闭重建
- 连接耗尽: 当队列为空且已达上限时返回无效 `QSqlDatabase`

---

### 5.7 简单数据库连接 — ConnectSql

**文件**: [connectsql.h](file:///D:/ProjectFiles/ClionProject/ChatServer/include/connectsql.h) / [connectsql.cpp](file:///D:/ProjectFiles/ClionProject/ChatServer/src/connectsql.cpp)

这是一个**早期版本**的数据库连接封装，提供基础的单连接管理功能。当前版本中已被 `ConnectionPool` 取代，该文件仍保留在项目中但**未在 CMakeLists.txt 中引用**。

---

## 6. 通信协议

### 6.1 二进制协议格式

所有客户端 ↔ 服务器通信均采用自定义二进制协议，大端字节序 (BigEndian):

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
│                                                               │
│                       JSON PAYLOAD (变长)                       │
│                    QJsonDocument::Compact 格式                   │
│                                                               │
└─────────────────────────────────────────────────────────────────┘
```

**协议头部共 20 字节**, 之后跟随 JSON 数据体。

### 6.2 COMMAND_TYPE 定义

| 值      | 含义   | 处理模块      |
|---------|--------|-------------|
| `0x001` | 登录   | Login       |
| `0x002` | 注册   | Register    |
| `0x003` | 消息转发 | Transmit    |

### 6.3 协议构造方法

数据包的构造由 `DescHandle::buildStream()` 完成：

```cpp
QByteArray DescHandle::buildStream(const quint32 COMMAND_TYPE, const QJsonObject &jsonObject) {
    QByteArray data;
    QByteArray jsonData = QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);
    QDataStream buffer(&data, QIODevice::WriteOnly);
    buffer.setByteOrder(QDataStream::BigEndian);

    buffer << MAGIC_NUMBER << COMMAND_TYPE << TIMESTAMPS << jsonData.size();
    data.append(jsonData);

    return data;
}
```

### 6.4 协议验证

- 服务器读取前 4 字节验证 `MAGIC_NUMBER == 0x4A3B2C1D`
- 不匹配则调用 `QTcpSocket::abort()` 断开连接

---

## 7. 数据库设计

项目使用 MySQL, 数据库中至少包含以下表（表结构由代码推理得出）：

### 7.1 `users` 表

| 字段            | 类型        | 说明                |
|----------------|-------------|---------------------|
| `uid`          | INT (PK)    | 用户 ID, 自增主键     |
| `phone`        | VARCHAR     | 手机号 / 账号         |
| `password_hash` | VARCHAR   | 密码哈希 (密码+盐)   |
| `salt`         | VARCHAR     | 密码盐值              |
| `account_status` | INT/TINYINT | 账号状态             |
| `username`     | VARCHAR     | 用户名                |

### 7.2 `user_profiles` 表

| 字段     | 类型     | 说明     |
|----------|----------|---------|
| `uid`    | INT (FK) | 用户 ID  |
| `avatar` | VARCHAR  | 头像 URL |

### 7.3 `friendships` 表

| 字段       | 类型     | 说明       |
|------------|----------|-----------|
| `user1_id` | INT (FK) | 用户 A ID  |
| `user2_id` | INT (FK) | 用户 B ID  |

注: 好友关系存储双向记录, 查询时使用 `UNION` 获取完整好友列表。

---

## 8. 构建与运行

### 8.1 环境要求

| 依赖      | 版本要求     |
|-----------|-------------|
| CMake     | ≥ 3.31      |
| Qt5       | Core / Network / Sql 模块 |
| MySQL     | 8.x (推荐)   |
| 编译器    | 支持 C++20 (MSVC / Clang / GCC) |

### 8.2 构建步骤

```bash
# 1. 克隆项目
git clone <repository-url>
cd ChatServer

# 2. 配置 CMake (确保 CMAKE_PREFIX_PATH 指向 Qt5 安装目录)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 3. 编译
cmake --build build

# 4. 运行 (需确保 MySQL 服务可用)
./build/ChatServer
```

### 8.3 MySQL 配置

在 `main.cpp` 中配置数据库连接参数：

```cpp
const QString hostName = "8.148.211.115";  // 数据库主机
const QString dbName   = "chat_app";       // 数据库名
const QString userName = "root";           // 用户名
const QString pwd      = "Wwz0530.";       // 密码
quint16 port           = 3306;             // 端口
```

### 8.4 CMake 构建配置要点

[CMakeLists.txt](file:///D:/ProjectFiles/ClionProject/ChatServer/CMakeLists.txt) 关键配置：

- **C++ 标准**: C++20
- **Qt5 模块**: Core, Network, Sql
- **Qt 特性**: 启用 `AUTOMOC`, `AUTORCC`, `AUTOUIC`
- **Windows 部署**: 自动复制 Qt5 DLL 和 `qwindows.dll` 平台插件到输出目录
- **编译器标志**: Clang/GCC 启用 `-Wall -Wextra -Werror=missing-field-initializers`

---

## 9. 依赖关系图

```
TcpServer
    │
    ├──> DescHandle
    │       ├──> Login (QRunnable → QThreadPool)
    │       │       └──> ConnectionPool (singleton)
    │       ├──> Register (QRunnable → QThreadPool)
    │       │       └──> ConnectionPool (singleton)
    │       ├──> Transmit (QThread)
    │       └──> ConnectionPool (singleton) [indirect via Login/Register]
    │
    └──> ConnectionPool (singleton) [via main.cpp initialization]
```

---

## 10. 线程模型

```
┌─────────────────┐
│  主线程 (main)   │
│  QCoreApplication│
│  TcpServer       │
│  DescHandle      │  ← 信号槽通信 (QueuedConnection)
│  ConnectionPool  │
└────────┬────────┘
         │
    ┌────┴────┐
    │ QThreadPool │  (最大 4 线程)
    │  ┌───────┐  │
    │  │ Login │  │  ← QRunnable, 自动删除
    │  ├───────┤  │
    │  │Register│  │  ← QRunnable, 自动删除
    │  └───────┘  │
    └────┬───────┘
         │
    ┌────┴────┐
    │ transferStation │  (独立 QThread)
    │  ┌──────────┐   │
    │  │ Transmit │   │  ← 消息转发
    │  └──────────┘   │
    └─────────────────┘
```

- **主线程**: 运行 `QCoreApplication` 事件循环, 处理 TCP 连接和信号槽调度
- **线程池**: 执行登录和注册等数据库密集型任务 (最大 4 并发)
- **独立线程**: `Transmit` 消息转发模块在独立线程中处理, 避免阻塞主线程
- **跨线程通信**: 全部使用 `QMetaObject::invokeMethod` + `Qt::QueuedConnection` 确保线程安全
- **线程安全**: `ConnectionPool`、`DescHandle` 中的队列/哈希表均使用 `QMutex` 保护

---

## 11. 安全机制

| 机制         | 说明                                        |
|-------------|---------------------------------------------|
| **协议魔数** | 每个数据包以固定 `MAGIC_NUMBER` 开头, 无效数据直接断开 |
| **密码盐值** | 注册时生成密码学安全随机盐值, 密码存储为 `明文+盐值`    |
| **自动清理** | 登录失败 / 断开连接后立即从用户池移除, 释放资源         |
| **连接验证** | 数据库连接归还时执行 `SELECT 1` 健康检查             |
| **连接上限** | 连接池最大连接数限制, 防止数据库过载                   |

> **注意**: 当前密码存储仅使用"密码明文+盐值"的拼接方式, 未使用标准哈希算法(如 bcrypt / SHA-256), 生产环境中建议升级。

---

## 12. 关键设计决策

1. **为什么选择单一线程的 DescHandle?**
   DescHandle 运行在主线程, 通过信号槽异步回调处理耗时操作的结果。这种设计简化了并发管理, 所有对 userPool / tcpPool 的访问都受互斥锁保护。

2. **为什么 Login 和 Register 使用 QRunnable + 线程池?**
   数据库查询可能耗时较长, 使用线程池可以并行处理多个用户的登录/注册请求, 避免阻塞主线程的事件循环。

3. **为什么 Transmit 在独立线程运行?**
   消息转发是一个持续进行的任务队列, 独立线程可以保证消息转发不会受到其他操作的干扰, 同时也分散了主线程的负载。

4. **为什么使用自定义二进制协议而非纯 JSON/Protobuf?**
   二进制协议头部紧凑(20 字节), 通过魔数快速验证数据有效性, 适合高并发通信场景。

---

## 13. 开发环境

- **IDE**: CLion (`.idea/` 目录为 CLion 项目配置)
- **编译器**: 支持 C++20 的任意编译器 (MSVC / Clang / GCC)
- **代码检查**: 配置了 `.clang-tidy` 用于静态分析
- **版本控制**: Git (已配置 `.gitignore`)