# POCO Logger 日志系统使用指南

## 📖 什么是 Logger？

Logger（日志记录器）就像程序的"日记本"，记录程序运行时发生的事情：
- ✅ 程序启动、关闭
- ⚠️ 警告和错误
- 🔍 调试信息
- 📊 运行状态

---

## 🎯 核心概念

### 1. **日志级别**（从高到低）

| 级别 | 说明 | 何时使用 |
|------|------|---------|
| **FATAL** | 致命错误 | 程序崩溃 |
| **CRITICAL** | 严重错误 | 关键功能失败 |
| **ERROR** | 普通错误 | 操作失败（如连接数据库失败） |
| **WARNING** | 警告 | 潜在问题（如内存使用率高） |
| **NOTICE** | 通知 | 重要但正常的事件 |
| **INFORMATION** | 信息 | 常规操作（如用户登录） |
| **DEBUG** | 调试 | 开发时的详细信息 |
| **TRACE** | 跟踪 | 最详细的调试信息 |

**过滤规则**：只显示 >= 设定级别的日志

例如：级别设为 WARNING，则只显示 WARNING、ERROR、CRITICAL、FATAL

---

### 2. **Logger 的组成部分**

```
┌─────────────────────────────────────────┐
│           Your Application              │
│                                         │
│  logger.information("用户登录");        │
│              ↓                          │
│         [ Logger ]                      │
│              ↓                          │
│      [ Level Filter ]                   │
│       (过滤低级别日志)                   │
│              ↓                          │
│      [ Formatter ]                      │
│    (格式化：加时间戳等)                  │
│              ↓                          │
│       [ Channel ]                       │
│     (输出到哪里？)                       │
│         ↙     ↘                         │
│   Console   File   Syslog  Network     │
│   (控制台) (文件) (系统日志) (网络)      │
└─────────────────────────────────────────┘
```

---

## 🚀 快速开始

### 最简单的用法

```cpp
#include "Poco/Logger.h"
#include "Poco/ConsoleChannel.h"

using namespace Poco;

int main()
{
    // 1. 创建输出到控制台的 Logger
    Logger& logger = Logger::create(
        "MyApp",                    // 名称
        new ConsoleChannel,         // 输出到控制台
        Message::PRIO_INFORMATION   // 级别
    );

    // 2. 记录日志
    logger.information("程序启动");
    logger.warning("内存使用率 80%");
    logger.error("连接失败");

    return 0;
}
```

---

## 📝 常用示例

### 示例1: 输出到文件

```cpp
#include "Poco/Logger.h"
#include "Poco/FileChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FormattingChannel.h"

// 创建文件 Channel
AutoPtr<FileChannel> pFileChannel(new FileChannel);
pFileChannel->setProperty("path", "app.log");
pFileChannel->setProperty("rotation", "10 M");  // 10MB 轮转

// 格式化
AutoPtr<PatternFormatter> pFormatter(new PatternFormatter);
pFormatter->setProperty("pattern", "%Y-%m-%d %H:%M:%S [%p] %t");

// 组合
AutoPtr<FormattingChannel> pFC(
    new FormattingChannel(pFormatter, pFileChannel)
);

// 创建 Logger
Logger& logger = Logger::create("FileLogger", pFC, Message::PRIO_DEBUG);

// 使用
logger.information("日志写入文件");
```

### 示例2: 同时输出到控制台和文件

```cpp
#include "Poco/SplitterChannel.h"

// 创建分流器
AutoPtr<SplitterChannel> pSplitter(new SplitterChannel);

// 添加控制台
pSplitter->addChannel(new ConsoleChannel);

// 添加文件
AutoPtr<FileChannel> pFile(new FileChannel);
pFile->setProperty("path", "app.log");
pSplitter->addChannel(pFile);

// 创建 Logger
Logger& logger = Logger::create("MultiLogger", pSplitter, Message::PRIO_INFO);

logger.information("同时输出到控制台和文件");
```

### 示例3: 使用日志宏（推荐）

```cpp
#include "Poco/Logger.h"

Logger& logger = Logger::get("MyApp");

// 基础宏
poco_information(logger, "这是信息");
poco_warning(logger, "这是警告");
poco_error(logger, "这是错误");
poco_debug(logger, "这是调试信息");

// 带参数的宏
int userId = 123;
std::string userName = "张三";
poco_information_f2(logger, "用户 %s (ID=%d) 登录", userName, userId);

// 带条件的宏（只有条件为真才记录）
if (someCondition)
    poco_warning(logger, "条件满足时才记录");
```

---

## 🎨 日志格式化

### PatternFormatter 常用占位符

| 占位符 | 说明 | 示例 |
|--------|------|------|
| `%Y-%m-%d` | 日期 | 2024-01-07 |
| `%H:%M:%S` | 时间 | 14:30:25 |
| `%i` | 毫秒 | 123 |
| `%p` | 优先级 | Information |
| `%t` | 消息文本 | 用户登录 |
| `%s` | 日志来源 | MyApp |
| `%P` | 进程ID | 12345 |
| `%I` | 线程ID | 67890 |
| `%N` | 节点名 | localhost |
| `%U` | 源文件 | main.cpp |
| `%u` | 行号 | 42 |

### 常用格式示例

```cpp
// 简洁格式
"%H:%M:%S [%p] %t"
// 输出: 14:30:25 [Information] 用户登录

// 详细格式
"%Y-%m-%d %H:%M:%S.%i [%p] %s: %t"
// 输出: 2024-01-07 14:30:25.123 [Information] MyApp: 用户登录

// 完整格式（用于调试）
"%Y-%m-%d %H:%M:%S [%p] %s (%U:%u) %t"
// 输出: 2024-01-07 14:30:25 [Information] MyApp (main.cpp:42) 用户登录
```

---

## ⚙️ 高级功能

### 文件轮转

```cpp
AutoPtr<FileChannel> pFile(new FileChannel);
pFile->setProperty("path", "app.log");
pFile->setProperty("rotation", "10 M");        // 按大小：10MB
// 或
pFile->setProperty("rotation", "daily");       // 按时间：每天
pFile->setProperty("archive", "timestamp");    // 归档方式
pFile->setProperty("purgeAge", "7 days");      // 保留7天
```

### 层级 Logger

```cpp
// 创建层级结构
Logger& root = Logger::create("App", channel, PRIO_INFO);
Logger& db = Logger::create("App.Database", channel, PRIO_DEBUG);
Logger& web = Logger::create("App.WebServer", channel, PRIO_WARNING);

// 使用
Logger::get("App").information("应用启动");
Logger::get("App.Database").debug("查询执行");
Logger::get("App.WebServer").warning("响应慢");
```

---

## 💡 最佳实践

### ✅ 推荐做法

1. **使用宏而不是直接调用方法**
   ```cpp
   // 好
   poco_information(logger, "消息");

   // 不好
   logger.information("消息");
   ```

2. **不同模块使用不同 Logger**
   ```cpp
   Logger& dbLogger = Logger::get("Database");
   Logger& authLogger = Logger::get("Auth");
   Logger& cacheLogger = Logger::get("Cache");
   ```

3. **生产环境使用合适的级别**
   ```cpp
   // 开发环境
   Logger::create("App", channel, Message::PRIO_DEBUG);

   // 生产环境
   Logger::create("App", channel, Message::PRIO_INFORMATION);
   ```

4. **文件日志要设置轮转**
   ```cpp
   pFile->setProperty("rotation", "10 M");
   pFile->setProperty("purgeAge", "30 days");
   ```

### ❌ 避免的做法

1. ❌ 在循环中记录大量低级别日志
2. ❌ 在日志中输出敏感信息（密码、密钥）
3. ❌ 忘记设置日志文件轮转（文件会无限增长）
4. ❌ 生产环境使用 TRACE 级别

---

## 🎓 学习资源

### 示例程序（本目录下）

1. **logger_quick_start.cpp** - 最简单入门
2. **logger_demo_simple.cpp** - 完整功能演示
3. **logger_demo.cpp** - POCO 原生示例

### 编译运行

```bash
# 简化版（立即可运行）
g++ -std=c++17 logger_quick_start.cpp -o quick_log
./quick_log

# 完整演示
g++ -std=c++17 logger_demo_simple.cpp -o logger_demo_log
./logger_demo_log

# 查看日志文件
cat app.log
cat MyApp.log
cat WebServer.log
```

---

## 📊 日志级别选择指南

| 场景 | 推荐级别 | 原因 |
|------|---------|------|
| 开发调试 | DEBUG | 看到详细信息 |
| 测试环境 | INFORMATION | 记录主要流程 |
| 生产环境 | WARNING | 只记录问题 |
| 排查问题 | DEBUG/TRACE | 临时开启详细日志 |

---

## 🔍 常见问题

### Q: 日志不显示？
**A**: 检查日志级别，可能消息级别低于 Logger 设定级别

### Q: 日志文件太大？
**A**: 设置文件轮转：`rotation` 和 `purgeAge`

### Q: 如何在多线程中使用？
**A**: Logger 是线程安全的，可以直接在多线程中使用

### Q: 如何获取已创建的 Logger？
**A**: 使用 `Logger::get("名称")`

---

## 🎯 总结

1. **Logger** = 日志记录器
2. **Level** = 控制哪些日志显示
3. **Channel** = 输出到哪里（控制台、文件等）
4. **Formatter** = 日志格式
5. **使用宏** = 推荐方式，自动添加文件名和行号

日志是程序的眼睛，用好日志可以：
- ✅ 快速定位问题
- ✅ 监控程序运行状态
- ✅ 分析用户行为
- ✅ 审计系统操作
