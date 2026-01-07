//
// mutex_demo_simple.cpp
//
// 简化版 Mutex 演示（使用标准 C++ 来展示 POCO Mutex 的概念）
// 编译: g++ -std=c++17 mutex_demo_simple.cpp -o mutex_demo -lpthread
// 运行: ./mutex_demo
//

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>

// ========================================
// 示例1：没有锁 - 会出问题！
// ========================================
class BadCounter
{
public:
    BadCounter() : counter(0) {}

    void increase()
    {
        // 每个线程增加10000次
        for (int i = 0; i < 10000; ++i)
        {
            counter++;  // ❌ 危险！多个线程同时修改
        }
    }

    int getCounter() const { return counter; }

private:
    int counter;  // 没有保护的共享变量
};


// ========================================
// 示例2：使用 Mutex - 正确！
// ========================================
class GoodCounter
{
public:
    GoodCounter() : counter(0) {}

    void increase()
    {
        // 每个线程增加10000次
        for (int i = 0; i < 10000; ++i)
        {
            std::lock_guard<std::mutex> lock(mtx);  // ✅ 自动加锁（类似 POCO 的 ScopedLock）
            counter++;  // 现在安全了！
            // lock 在这里自动解锁
        }
    }

    int getCounter() const { return counter; }

private:
    int counter;
    mutable std::mutex mtx;  // 互斥锁保护 counter
};


// ========================================
// 示例3：保护控制台输出
// ========================================
std::mutex outputMutex;  // 全局 mutex 保护输出

void worker(const std::string& name)
{
    for (int i = 0; i < 5; ++i)
    {
        {
            std::lock_guard<std::mutex> lock(outputMutex);  // 加锁保护输出
            std::cout << "[" << name << "] 正在工作... 第 " << (i + 1) << " 次" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 模拟工作
    }

    {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::cout << "[" << name << "] 工作完成！" << std::endl;
    }
}


int main()
{
    std::cout << "\n==================================" << std::endl;
    std::cout << "   Mutex 演示程序" << std::endl;
    std::cout << "   (展示 POCO Mutex 的工作原理)" << std::endl;
    std::cout << "==================================" << std::endl;

    // --------------------------------------
    // 测试1：没有 Mutex - 数据会错乱
    // --------------------------------------
    std::cout << "\n【测试1】没有 Mutex 保护（会出错）" << std::endl;
    std::cout << "启动 5 个线程，每个增加计数器 10000 次..." << std::endl;

    BadCounter badCounter;
    std::vector<std::thread> threads1;

    for (int i = 0; i < 5; ++i)
    {
        threads1.emplace_back([&badCounter]() {
            badCounter.increase();
        });
    }

    for (auto& t : threads1)
    {
        t.join();
    }

    std::cout << "期望结果: 50000" << std::endl;
    std::cout << "实际结果: " << badCounter.getCounter() << std::endl;
    if (badCounter.getCounter() != 50000)
    {
        std::cout << "❌ 数据错乱了！少了 " << (50000 - badCounter.getCounter()) << " 次增加" << std::endl;
        std::cout << "   这就是为什么需要 Mutex！" << std::endl;
    }
    else
    {
        std::cout << "✅ 运气好，这次没出错（但不保证每次都对）" << std::endl;
        std::cout << "   多运行几次，肯定会出错！" << std::endl;
    }


    // --------------------------------------
    // 测试2：使用 Mutex - 数据正确
    // --------------------------------------
    std::cout << "\n【测试2】使用 Mutex 保护（正确）" << std::endl;
    std::cout << "启动 5 个线程，每个增加计数器 10000 次..." << std::endl;

    GoodCounter goodCounter;
    std::vector<std::thread> threads2;

    for (int i = 0; i < 5; ++i)
    {
        threads2.emplace_back([&goodCounter]() {
            goodCounter.increase();
        });
    }

    for (auto& t : threads2)
    {
        t.join();
    }

    std::cout << "期望结果: 50000" << std::endl;
    std::cout << "实际结果: " << goodCounter.getCounter() << std::endl;
    if (goodCounter.getCounter() == 50000)
    {
        std::cout << "✅ 完全正确！Mutex 保护了数据" << std::endl;
        std::cout << "   无论运行多少次，结果都是对的！" << std::endl;
    }
    else
    {
        std::cout << "❌ 出错了（不应该发生）" << std::endl;
    }


    // --------------------------------------
    // 测试3：保护控制台输出
    // --------------------------------------
    std::cout << "\n【测试3】使用 Mutex 保护控制台输出" << std::endl;
    std::cout << "启动 3 个工作线程..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::vector<std::thread> threads3;
    threads3.emplace_back(worker, "工人A");
    threads3.emplace_back(worker, "工人B");
    threads3.emplace_back(worker, "工人C");

    for (auto& t : threads3)
    {
        t.join();
    }

    std::cout << "\n==================================" << std::endl;
    std::cout << "   所有测试完成！" << std::endl;
    std::cout << "==================================" << std::endl;

    std::cout << "\n💡 总结：" << std::endl;
    std::cout << "1. 多线程访问共享数据时，必须使用 Mutex 保护" << std::endl;
    std::cout << "2. POCO 的 Mutex::ScopedLock 就像 C++ 的 lock_guard" << std::endl;
    std::cout << "3. ScopedLock 自动加锁和解锁，防止忘记解锁" << std::endl;
    std::cout << "4. 没有 Mutex 保护会导致数据错乱（竞态条件）" << std::endl;

    return 0;
}
