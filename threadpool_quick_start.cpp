//
// threadpool_quick_start.cpp - 线程池最简单入门
//
// 编译: g++ -std=c++17 threadpool_quick_start.cpp -o quick_threadpool -lpthread
// 运行: ./quick_threadpool
//

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>

// 超级简单的线程池
class EasyThreadPool
{
public:
    EasyThreadPool(int numThreads)
    {
        for (int i = 0; i < numThreads; ++i)
        {
            workers.emplace_back([this] {
                while (true)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mtx);
                        cv.wait(lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    ~EasyThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        for (auto& w : workers) w.join();
    }

    void submit(std::function<void()> task)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            tasks.push(std::move(task));
        }
        cv.notify_one();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop = false;
};


int main()
{
    std::cout << "\n=== 线程池最简单示例 ===\n" << std::endl;

    // 1. 创建线程池（4 个工作线程）
    EasyThreadPool pool(4);
    std::cout << "✅ 创建了 4 个工作线程的线程池\n" << std::endl;

    // 2. 提交任务 - 就这么简单！
    std::cout << "提交 10 个任务..." << std::endl;

    for (int i = 0; i < 10; ++i)
    {
        pool.submit([i] {
            std::cout << "任务 " << i << " 正在执行 (线程: "
                      << std::this_thread::get_id() << ")" << std::endl;

            // 模拟工作
            std::this_thread::sleep_for(std::chrono::milliseconds(300));

            std::cout << "任务 " << i << " 完成" << std::endl;
        });
    }

    std::cout << "\n等待所有任务完成..." << std::endl;

    // 等待一段时间让任务完成
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "\n✅ 所有任务已完成！\n" << std::endl;

    // ============================================
    // POCO ThreadPool 的用法
    // ============================================
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "POCO ThreadPool 的用法（概念相同）：" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "// 1. 创建线程池" << std::endl;
    std::cout << "ThreadPool pool(4);  // 最少4个线程，最多16个\n" << std::endl;

    std::cout << "// 2. 创建任务类（实现 Runnable 接口）" << std::endl;
    std::cout << "class MyTask : public Runnable {" << std::endl;
    std::cout << "    void run() { /* 任务代码 */ }" << std::endl;
    std::cout << "};\n" << std::endl;

    std::cout << "// 3. 提交任务" << std::endl;
    std::cout << "MyTask task;" << std::endl;
    std::cout << "pool.start(task);  // 在线程池中执行\n" << std::endl;

    std::cout << "// 4. 使用默认线程池（更简单）" << std::endl;
    std::cout << "ThreadPool::defaultPool().start(task);\n" << std::endl;

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "💡 线程池的好处：" << std::endl;
    std::cout << "  1. 避免频繁创建/销毁线程（提高性能）" << std::endl;
    std::cout << "  2. 控制并发数量（防止资源耗尽）" << std::endl;
    std::cout << "  3. 统一管理线程（方便监控）" << std::endl;
    std::cout << "  4. 简化代码（不用手动管理线程）\n" << std::endl;

    return 0;
}
