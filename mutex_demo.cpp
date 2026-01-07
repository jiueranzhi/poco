//
// mutex_demo.cpp
//
// 这个程序演示 Mutex 的作用
// 编译: g++ -std=c++17 mutex_demo.cpp -o mutex_demo -I./Foundation/include -L./lib/Linux/x86_64 -lPocoFoundation -lpthread
//

#include "Poco/Thread.h"
#include "Poco/Runnable.h"
#include "Poco/Mutex.h"
#include <iostream>

using namespace Poco;

// ========================================
// 示例1：没有锁 - 会出问题！
// ========================================
class BadCounter : public Runnable
{
public:
    BadCounter() : _counter(0) {}

    void run() override
    {
        // 每个线程增加10000次
        for (int i = 0; i < 10000; ++i)
        {
            _counter++;  // ❌ 危险！多个线程同时修改
        }
    }

    int getCounter() const { return _counter; }

private:
    int _counter;  // 没有保护的共享变量
};


// ========================================
// 示例2：使用 Mutex - 正确！
// ========================================
class GoodCounter : public Runnable
{
public:
    GoodCounter() : _counter(0) {}

    void run() override
    {
        // 每个线程增加10000次
        for (int i = 0; i < 10000; ++i)
        {
            Mutex::ScopedLock lock(_mutex);  // ✅ 自动加锁
            _counter++;  // 现在安全了！
            // lock 在这里自动解锁
        }
    }

    int getCounter() const { return _counter; }

private:
    int _counter;
    mutable Mutex _mutex;  // 互斥锁保护 _counter
};


// ========================================
// 示例3：保护控制台输出
// ========================================
class Worker : public Runnable
{
public:
    Worker(const std::string& name, Mutex& mutex)
        : _name(name), _mutex(mutex) {}

    void run() override
    {
        for (int i = 0; i < 5; ++i)
        {
            {
                Mutex::ScopedLock lock(_mutex);  // 加锁保护输出
                std::cout << "[" << _name << "] 正在工作... 第 " << i + 1 << " 次" << std::endl;
            }
            Thread::sleep(100);  // 模拟工作
        }

        {
            Mutex::ScopedLock lock(_mutex);
            std::cout << "[" << _name << "] 工作完成！" << std::endl;
        }
    }

private:
    std::string _name;
    Mutex& _mutex;
};


int main()
{
    std::cout << "\n==================================" << std::endl;
    std::cout << "   POCO Mutex 演示程序" << std::endl;
    std::cout << "==================================" << std::endl;

    // --------------------------------------
    // 测试1：没有 Mutex - 数据会错乱
    // --------------------------------------
    std::cout << "\n【测试1】没有 Mutex 保护（会出错）" << std::endl;
    std::cout << "启动 5 个线程，每个增加计数器 10000 次..." << std::endl;

    BadCounter badCounter;
    Thread t1, t2, t3, t4, t5;

    t1.start(badCounter);
    t2.start(badCounter);
    t3.start(badCounter);
    t4.start(badCounter);
    t5.start(badCounter);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    std::cout << "期望结果: 50000" << std::endl;
    std::cout << "实际结果: " << badCounter.getCounter() << std::endl;
    if (badCounter.getCounter() != 50000)
        std::cout << "❌ 数据错乱了！少了 " << (50000 - badCounter.getCounter()) << " 次增加" << std::endl;
    else
        std::cout << "✅ 运气好，这次没出错（但不保证每次都对）" << std::endl;


    // --------------------------------------
    // 测试2：使用 Mutex - 数据正确
    // --------------------------------------
    std::cout << "\n【测试2】使用 Mutex 保护（正确）" << std::endl;
    std::cout << "启动 5 个线程，每个增加计数器 10000 次..." << std::endl;

    GoodCounter goodCounter;
    Thread t6, t7, t8, t9, t10;

    t6.start(goodCounter);
    t7.start(goodCounter);
    t8.start(goodCounter);
    t9.start(goodCounter);
    t10.start(goodCounter);

    t6.join();
    t7.join();
    t8.join();
    t9.join();
    t10.join();

    std::cout << "期望结果: 50000" << std::endl;
    std::cout << "实际结果: " << goodCounter.getCounter() << std::endl;
    if (goodCounter.getCounter() == 50000)
        std::cout << "✅ 完全正确！Mutex 保护了数据" << std::endl;
    else
        std::cout << "❌ 出错了（不应该发生）" << std::endl;


    // --------------------------------------
    // 测试3：保护控制台输出
    // --------------------------------------
    std::cout << "\n【测试3】使用 Mutex 保护控制台输出" << std::endl;
    std::cout << "启动 3 个工作线程..." << std::endl;
    Thread::sleep(500);

    Mutex outputMutex;
    Worker worker1("工人A", outputMutex);
    Worker worker2("工人B", outputMutex);
    Worker worker3("工人C", outputMutex);

    Thread w1, w2, w3;
    w1.start(worker1);
    w2.start(worker2);
    w3.start(worker3);

    w1.join();
    w2.join();
    w3.join();

    std::cout << "\n==================================" << std::endl;
    std::cout << "   所有测试完成！" << std::endl;
    std::cout << "==================================" << std::endl;

    return 0;
}
