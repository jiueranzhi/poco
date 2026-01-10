//
// threadpool_poco_demo.cpp
//
// POCO ThreadPool 完整示例（需要编译 POCO 库）
// 编译: 需要先编译 POCO Foundation 库
//

#include "Poco/ThreadPool.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "Poco/Mutex.h"
#include <iostream>
#include <string>

using namespace Poco;


// ========================================
// 示例1: 简单任务
// ========================================
class SimpleTask : public Runnable
{
public:
    SimpleTask(int id) : _id(id) {}

    void run() override
    {
        std::cout << "任务 " << _id << " 开始执行 (线程: "
                  << Thread::currentTid() << ")" << std::endl;

        // 模拟工作
        Thread::sleep(500);

        std::cout << "任务 " << _id << " 完成" << std::endl;
    }

private:
    int _id;
};


// ========================================
// 示例2: 共享数据的任务
// ========================================
class Counter
{
public:
    Counter() : _count(0) {}

    void increment()
    {
        Mutex::ScopedLock lock(_mutex);
        _count++;
    }

    int getCount() const
    {
        return _count;
    }

private:
    int _count;
    mutable Mutex _mutex;
};

class CounterTask : public Runnable
{
public:
    CounterTask(Counter& counter, int iterations)
        : _counter(counter), _iterations(iterations) {}

    void run() override
    {
        for (int i = 0; i < _iterations; ++i)
        {
            _counter.increment();
        }
    }

private:
    Counter& _counter;
    int _iterations;
};


// ========================================
// 示例3: 数据处理任务
// ========================================
class DataProcessor : public Runnable
{
public:
    DataProcessor(const std::string& name, int start, int end)
        : _name(name), _start(start), _end(end) {}

    void run() override
    {
        std::cout << "[" << _name << "] 处理数据 " << _start
                  << " 到 " << _end << std::endl;

        int sum = 0;
        for (int i = _start; i <= _end; ++i)
        {
            sum += i;
            Thread::sleep(10);  // 模拟计算
        }

        std::cout << "[" << _name << "] 完成，结果: " << sum << std::endl;
    }

private:
    std::string _name;
    int _start;
    int _end;
};


void example1_basic()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例1: 基础使用" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // 创建线程池：最少2个线程，最多8个线程
    ThreadPool pool(2, 8);

    std::cout << "线程池容量: " << pool.capacity() << std::endl;
    std::cout << "可用线程: " << pool.available() << std::endl;
    std::cout << std::endl;

    // 提交任务
    for (int i = 0; i < 5; ++i)
    {
        SimpleTask task(i);
        pool.start(task);
    }

    std::cout << "已提交 5 个任务" << std::endl;
    std::cout << "使用中的线程: " << pool.used() << std::endl;
    std::cout << std::endl;

    // 等待所有任务完成
    pool.joinAll();

    std::cout << "\n所有任务已完成" << std::endl;
}


void example2_shared_data()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例2: 共享数据（线程安全）" << std::endl;
    std::cout << "========================================\n" << std::endl;

    Counter counter;
    ThreadPool pool(4, 4);

    std::cout << "启动 10 个任务，每个增加计数器 1000 次" << std::endl;

    // 10 个任务，每个增加 1000 次
    for (int i = 0; i < 10; ++i)
    {
        CounterTask task(counter, 1000);
        pool.start(task);
    }

    pool.joinAll();

    std::cout << "期望结果: 10000" << std::endl;
    std::cout << "实际结果: " << counter.getCount() << std::endl;
    std::cout << "✅ Mutex 保证了线程安全！" << std::endl;
}


void example3_data_processing()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例3: 并行数据处理" << std::endl;
    std::cout << "========================================\n" << std::endl;

    ThreadPool pool(4, 4);

    std::cout << "将 1-100 的求和任务分成 4 份并行处理\n" << std::endl;

    // 分成 4 个任务
    DataProcessor task1("任务1", 1, 25);
    DataProcessor task2("任务2", 26, 50);
    DataProcessor task3("任务3", 51, 75);
    DataProcessor task4("任务4", 76, 100);

    pool.start(task1);
    pool.start(task2);
    pool.start(task3);
    pool.start(task4);

    pool.joinAll();

    std::cout << "\n✅ 并行处理完成" << std::endl;
    std::cout << "   (1+2+...+100 = 5050，分 4 段计算)" << std::endl;
}


void example4_default_pool()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例4: 使用默认线程池" << std::endl;
    std::cout << "========================================\n" << std::endl;

    std::cout << "POCO 提供了一个默认的全局线程池" << std::endl;
    std::cout << "不需要自己创建，直接使用即可\n" << std::endl;

    // 使用默认线程池
    for (int i = 0; i < 3; ++i)
    {
        SimpleTask task(i);
        ThreadPool::defaultPool().start(task);
    }

    std::cout << "默认线程池容量: "
              << ThreadPool::defaultPool().capacity() << std::endl;

    ThreadPool::defaultPool().joinAll();

    std::cout << "\n💡 推荐使用默认线程池，简单方便！" << std::endl;
}


int main()
{
    std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║     POCO ThreadPool 完整教程           ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝" << std::endl;

    example1_basic();           // 基础使用
    example2_shared_data();     // 共享数据
    example3_data_processing(); // 数据处理
    example4_default_pool();    // 默认线程池

    std::cout << "\n========================================" << std::endl;
    std::cout << "🎓 总结" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "1. 创建任务类：继承 Runnable，实现 run()" << std::endl;
    std::cout << "2. 创建线程池：ThreadPool pool(min, max)" << std::endl;
    std::cout << "3. 提交任务：pool.start(task)" << std::endl;
    std::cout << "4. 等待完成：pool.joinAll()" << std::endl;
    std::cout << "5. 推荐使用：ThreadPool::defaultPool()\n" << std::endl;

    return 0;
}
