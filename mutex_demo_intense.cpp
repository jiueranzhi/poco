//
// mutex_demo_intense.cpp - 更容易看到错误的版本
//
// 编译: g++ -std=c++17 mutex_demo_intense.cpp -o mutex_demo_intense -lpthread
//

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

// 没有保护的计数器
int bad_counter = 0;

// 有保护的计数器
int good_counter = 0;
std::mutex mtx;

void bad_increase()
{
    for (int i = 0; i < 100000; ++i)  // 增加到 10 万次
    {
        bad_counter++;  // ❌ 没有保护
    }
}

void good_increase()
{
    for (int i = 0; i < 100000; ++i)
    {
        std::lock_guard<std::mutex> lock(mtx);  // ✅ 有保护
        good_counter++;
    }
}

int main()
{
    std::cout << "\n========== Mutex 演示程序 ==========\n" << std::endl;

    // 测试1：没有 Mutex
    std::cout << "【测试1】没有 Mutex 保护" << std::endl;
    std::cout << "启动 10 个线程，每个增加 100000 次..." << std::endl;

    bad_counter = 0;
    std::vector<std::thread> threads1;
    for (int i = 0; i < 10; ++i)
    {
        threads1.emplace_back(bad_increase);
    }
    for (auto& t : threads1)
    {
        t.join();
    }

    std::cout << "期望结果: 1000000" << std::endl;
    std::cout << "实际结果: " << bad_counter << std::endl;

    if (bad_counter != 1000000)
    {
        std::cout << "❌ 数据错了！丢失了 " << (1000000 - bad_counter) << " 次增加！" << std::endl;
        std::cout << "   这就是为什么需要 Mutex！\n" << std::endl;
    }
    else
    {
        std::cout << "✅ 这次没错（但这只是运气好）\n" << std::endl;
    }


    // 测试2：使用 Mutex
    std::cout << "【测试2】使用 Mutex 保护" << std::endl;
    std::cout << "启动 10 个线程，每个增加 100000 次..." << std::endl;

    good_counter = 0;
    std::vector<std::thread> threads2;
    for (int i = 0; i < 10; ++i)
    {
        threads2.emplace_back(good_increase);
    }
    for (auto& t : threads2)
    {
        t.join();
    }

    std::cout << "期望结果: 1000000" << std::endl;
    std::cout << "实际结果: " << good_counter << std::endl;

    if (good_counter == 1000000)
    {
        std::cout << "✅ 完全正确！Mutex 保证了数据安全！" << std::endl;
    }
    else
    {
        std::cout << "❌ 出错了（这不应该发生）" << std::endl;
    }

    std::cout << "\n===================================\n" << std::endl;

    return 0;
}
