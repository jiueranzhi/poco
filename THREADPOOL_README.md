# POCO ThreadPool 线程池使用指南

## 📖 什么是线程池？

**线程池（ThreadPool）** 就像一个"工人团队"：
- 提前雇佣一批工人（线程）待命
- 有任务时，分配给空闲工人
- 工人完成后，继续等待下一个任务
- 避免频繁招聘/解雇（创建/销毁线程）

---

## 🎯 为什么需要线程池？

### ❌ 不使用线程池的问题

```cpp
// 每个任务创建一个新线程
for (int i = 0; i < 1000; ++i)
{
    Thread t;
    MyTask task;
    t.start(task);  // 创建 1000 个线程！
}
```

**问题**：
- 🔴 创建/销毁线程开销大（每次都要调用系统）
- 🔴 线程数量无限制（可能耗尽系统资源）
- 🔴 难以管理和监控
- 🔴 上下文切换开销大（太多线程竞争 CPU）

---

### ✅ 使用线程池的优势

```cpp
// 使用线程池
ThreadPool pool(4, 16);  // 最少4个，最多16个线程

for (int i = 0; i < 1000; ++i)
{
    MyTask task;
    pool.start(task);  // 复用线程！
}
```

**好处**：
- ✅ **高性能**：线程复用，避免频繁创建/销毁
- ✅ **资源可控**：限制最大线程数
- ✅ **简化代码**：不用手动管理线程生命周期
- ✅ **易于监控**：统一的管理接口

---

## 🚀 快速开始

### 步骤 1：创建任务类

```cpp
#include "Poco/Runnable.h"
#include <iostream>

class MyTask : public Poco::Runnable
{
public:
    MyTask(int id) : _id(id) {}

    void run() override  // 实现 run() 方法
    {
        std::cout << "任务 " << _id << " 执行中..." << std::endl;
        // 执行实际工作
    }

private:
    int _id;
};
```

---

### 步骤 2：创建线程池并提交任务

```cpp
#include "Poco/ThreadPool.h"

using namespace Poco;

int main()
{
    // 创建线程池：最少2个线程，最多8个线程
    ThreadPool pool(2, 8);

    // 提交任务
    for (int i = 0; i < 10; ++i)
    {
        MyTask task(i);
        pool.start(task);  // 在线程池中执行
    }

    // 等待所有任务完成
    pool.joinAll();

    return 0;
}
```

---

### 步骤 3：或者使用默认线程池（更简单）

```cpp
// 使用 POCO 提供的默认全局线程池
MyTask task(1);
ThreadPool::defaultPool().start(task);

ThreadPool::defaultPool().joinAll();
```

---

## 📝 核心 API

### ThreadPool 构造函数

```cpp
ThreadPool pool(minCapacity, maxCapacity, idleTime, stackSize);
```

| 参数 | 说明 | 默认值 |
|------|------|--------|
| **minCapacity** | 最小线程数（始终保持） | 2 |
| **maxCapacity** | 最大线程数（需要时创建） | 16 |
| **idleTime** | 空闲线程存活时间（秒） | 60 |
| **stackSize** | 线程栈大小 | 系统默认 |

**示例**：
```cpp
// 最少 4 个线程，最多 10 个线程
ThreadPool pool(4, 10);

// 空闲线程 30 秒后自动销毁（节省资源）
ThreadPool pool(2, 8, 30);
```

---

### 主要方法

#### 1. 提交任务

```cpp
void start(Runnable& target);
void start(Runnable& target, const std::string& name);
```

**示例**：
```cpp
MyTask task(1);
pool.start(task);              // 提交任务
pool.start(task, "任务1");     // 提交并命名
```

---

#### 2. 等待完成

```cpp
void joinAll();  // 等待所有任务完成
```

**示例**：
```cpp
// 提交多个任务
for (int i = 0; i < 10; ++i)
{
    MyTask task(i);
    pool.start(task);
}

// 阻塞，直到所有任务完成
pool.joinAll();
```

---

#### 3. 查询状态

```cpp
int capacity() const;   // 最大容量
int available() const;  // 可用线程数
int used() const;       // 正在使用的线程数
int allocated() const;  // 已分配的线程数
```

**示例**：
```cpp
std::cout << "线程池容量: " << pool.capacity() << std::endl;
std::cout << "可用线程: " << pool.available() << std::endl;
std::cout << "使用中: " << pool.used() << std::endl;
```

---

#### 4. 管理线程池

```cpp
void addCapacity(int n);  // 增加容量
void stopAll();           // 停止所有线程
void collect();           // 清理空闲线程
```

---

### 默认线程池

```cpp
static ThreadPool& defaultPool();
```

POCO 提供了一个全局默认线程池，推荐使用：

```cpp
// 直接使用，不需要创建
MyTask task;
ThreadPool::defaultPool().start(task);
```

---

## 🎨 实用示例

### 示例 1：批量处理文件

```cpp
class FileProcessor : public Runnable
{
public:
    FileProcessor(const std::string& filename)
        : _filename(filename) {}

    void run() override
    {
        // 处理文件
        std::cout << "处理文件: " << _filename << std::endl;
        // ... 实际处理逻辑
    }

private:
    std::string _filename;
};

int main()
{
    ThreadPool pool(4, 8);  // 4-8 个线程

    std::vector<std::string> files = {
        "file1.txt", "file2.txt", "file3.txt", /* ... */
    };

    // 批量提交
    for (const auto& file : files)
    {
        FileProcessor task(file);
        pool.start(task);
    }

    pool.joinAll();  // 等待所有文件处理完成
    return 0;
}
```

---

### 示例 2：并行计算

```cpp
class Calculator : public Runnable
{
public:
    Calculator(int start, int end, int& result, Mutex& mutex)
        : _start(start), _end(end), _result(result), _mutex(mutex) {}

    void run() override
    {
        int sum = 0;
        for (int i = _start; i <= _end; ++i)
        {
            sum += i;
        }

        // 保护共享数据
        Mutex::ScopedLock lock(_mutex);
        _result += sum;
    }

private:
    int _start, _end;
    int& _result;
    Mutex& _mutex;
};

int main()
{
    ThreadPool pool(4, 4);
    int result = 0;
    Mutex mutex;

    // 将 1-1000 分成 4 份并行计算
    Calculator task1(1, 250, result, mutex);
    Calculator task2(251, 500, result, mutex);
    Calculator task3(501, 750, result, mutex);
    Calculator task4(751, 1000, result, mutex);

    pool.start(task1);
    pool.start(task2);
    pool.start(task3);
    pool.start(task4);

    pool.joinAll();

    std::cout << "1+2+...+1000 = " << result << std::endl;
    // 输出: 500500

    return 0;
}
```

---

### 示例 3：Web 服务器请求处理

```cpp
class RequestHandler : public Runnable
{
public:
    RequestHandler(const HTTPRequest& request)
        : _request(request) {}

    void run() override
    {
        // 处理 HTTP 请求
        processRequest(_request);
    }

private:
    HTTPRequest _request;

    void processRequest(const HTTPRequest& req)
    {
        // 处理逻辑...
    }
};

// 在服务器中使用
void handleConnection(Socket& socket)
{
    HTTPRequest request = parseRequest(socket);

    // 提交到线程池处理
    RequestHandler handler(request);
    ThreadPool::defaultPool().start(handler);
}
```

---

### 示例 4：共享数据（线程安全）

```cpp
class Counter
{
public:
    Counter() : _count(0) {}

    void increment()
    {
        Mutex::ScopedLock lock(_mutex);  // 加锁
        _count++;
    }

    int getCount() const { return _count; }

private:
    int _count;
    mutable Mutex _mutex;
};

class CounterTask : public Runnable
{
public:
    CounterTask(Counter& counter, int times)
        : _counter(counter), _times(times) {}

    void run() override
    {
        for (int i = 0; i < _times; ++i)
        {
            _counter.increment();
        }
    }

private:
    Counter& _counter;
    int _times;
};

int main()
{
    ThreadPool pool(4, 4);
    Counter counter;

    // 10 个任务，每个增加 1000 次
    for (int i = 0; i < 10; ++i)
    {
        CounterTask task(counter, 1000);
        pool.start(task);
    }

    pool.joinAll();

    std::cout << "计数结果: " << counter.getCount() << std::endl;
    // 输出: 10000（线程安全）

    return 0;
}
```

---

## 🎓 最佳实践

### ✅ 推荐做法

1. **使用默认线程池**（除非有特殊需求）
   ```cpp
   ThreadPool::defaultPool().start(task);
   ```

2. **合理设置线程数**
   - CPU 密集型任务：`线程数 = CPU 核心数 + 1`
   - I/O 密集型任务：`线程数 = CPU 核心数 * 2`
   ```cpp
   int cores = Thread::getNumberOfCPUs();
   ThreadPool pool(cores, cores * 2);
   ```

3. **保护共享数据**
   ```cpp
   Mutex::ScopedLock lock(mutex);
   // 访问共享数据
   ```

4. **始终调用 joinAll()**
   ```cpp
   pool.joinAll();  // 确保任务完成后再退出
   ```

5. **任务粒度适中**
   - 太小：线程池开销大于收益
   - 太大：无法充分并行
   - 建议：单个任务耗时 10ms - 1s

---

### ❌ 避免的做法

1. ❌ **在任务中创建新线程**（破坏线程池目的）
2. ❌ **任务互相等待**（可能死锁）
3. ❌ **不保护共享数据**（数据竞争）
4. ❌ **线程数过多**（浪费资源，增加切换开销）
5. ❌ **忘记 joinAll()**（程序可能提前退出）

---

## 📊 线程池参数调优

### 如何选择线程数？

```cpp
// 获取 CPU 核心数
int cores = Poco::Thread::getNumberOfCPUs();

// CPU 密集型（计算为主）
ThreadPool cpuPool(cores, cores + 1);

// I/O 密集型（网络、文件操作为主）
ThreadPool ioPool(cores * 2, cores * 4);

// 混合型
ThreadPool mixedPool(cores, cores * 2);
```

---

### 监控线程池状态

```cpp
void printPoolStatus(ThreadPool& pool)
{
    std::cout << "线程池状态:" << std::endl;
    std::cout << "  容量: " << pool.capacity() << std::endl;
    std::cout << "  已分配: " << pool.allocated() << std::endl;
    std::cout << "  使用中: " << pool.used() << std::endl;
    std::cout << "  可用: " << pool.available() << std::endl;
}
```

---

## 🔍 常见问题

### Q: 线程池中的线程会自动销毁吗？
**A**: 会的。如果空闲时间超过 `idleTime` 且线程数 > `minCapacity`，多余的线程会被销毁。

---

### Q: 可以动态调整线程池大小吗？
**A**: 可以。使用 `addCapacity(n)` 增加容量：
```cpp
pool.addCapacity(4);  // 增加 4 个线程的容量
```

---

### Q: 任务抛出异常会怎样？
**A**: 线程会捕获异常，线程本身不会崩溃，可以继续处理下一个任务。

---

### Q: ThreadPool 线程安全吗？
**A**: 是的。可以从多个线程安全地调用 `start()`。

---

### Q: 如何设置线程优先级？
**A**: 使用 `startWithPriority()`：
```cpp
pool.startWithPriority(Thread::PRIO_HIGH, task);
```

---

## 🎯 总结

### 核心概念
1. **ThreadPool** = 线程的集合，复用线程
2. **Runnable** = 任务接口，实现 `run()` 方法
3. **start()** = 提交任务到线程池
4. **joinAll()** = 等待所有任务完成

### 使用流程
```
1. 创建 Runnable 任务类
   ↓
2. 创建 ThreadPool（或使用 defaultPool）
   ↓
3. 提交任务（start）
   ↓
4. 等待完成（joinAll）
```

### 适用场景
- ✅ 批量数据处理
- ✅ 并发网络请求
- ✅ 文件/图片处理
- ✅ Web 服务器请求处理
- ✅ 并行计算

---

## 📚 学习资源

### 示例程序（本目录下）

1. **threadpool_quick_start.cpp** - 最简单入门
2. **threadpool_demo_simple.cpp** - 完整功能演示
3. **threadpool_poco_demo.cpp** - POCO 原生 API

### 编译运行

```bash
# 简化版（立即可运行）
g++ -std=c++17 threadpool_quick_start.cpp -o quick_threadpool -lpthread
./quick_threadpool

# 完整演示
g++ -std=c++17 threadpool_demo_simple.cpp -o threadpool_demo -lpthread
./threadpool_demo
```

---

## 💡 实用技巧

### 1. 任务队列模式

配合 `NotificationQueue` 使用：

```cpp
NotificationQueue queue;
ThreadPool pool(4, 8);

// Worker 从队列获取任务
class Worker : public Runnable {
    void run() {
        while (auto notification = queue.waitDequeue()) {
            // 处理任务
        }
    }
};

// 启动工作线程
Worker worker;
for (int i = 0; i < 4; ++i)
    pool.start(worker);

// 生产者添加任务到队列
queue.enqueue(new MyNotification());
```

---

### 2. 任务超时处理

```cpp
class TimeoutTask : public Runnable {
    void run() {
        Stopwatch sw;
        sw.start();

        // 执行任务
        doWork();

        if (sw.elapsed() > 5000000)  // 5 秒
            std::cout << "警告：任务超时" << std::endl;
    }
};
```

---

### 3. 异常处理

```cpp
class SafeTask : public Runnable {
    void run() {
        try {
            // 可能抛异常的代码
            riskyOperation();
        }
        catch (Exception& e) {
            std::cerr << "任务异常: " << e.displayText() << std::endl;
        }
    }
};
```

---

线程池是高性能并发编程的基石，掌握它会让您的程序性能大幅提升！🚀
