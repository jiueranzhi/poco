//
// threadpool_demo_simple.cpp
//
// 线程池完整教程 - 简化版（可立即运行）
// 编译: g++ -std=c++17 threadpool_demo_simple.cpp -o threadpool_demo -lpthread
// 运行: ./threadpool_demo
//

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>
#include <atomic>

// 简单的线程池实现（模拟 POCO ThreadPool）
class SimpleThreadPool
{
public:
    SimpleThreadPool(size_t numThreads) : stop(false)
    {
        // 创建指定数量的工作线程
        for (size_t i = 0; i < numThreads; ++i)
        {
            workers.emplace_back([this, i] {
                std::cout << "工作线程 " << i << " 启动" << std::endl;

                while (true)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(queueMutex);

                        // 等待任务或停止信号
                        condition.wait(lock, [this] {
                            return stop || !tasks.empty();
                        });

                        if (stop && tasks.empty())
                            return;

                        task = std::move(tasks.front());
                        tasks.pop();
                    }

                    task();  // 执行任务
                }
            });
        }
    }

    ~SimpleThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();

        for (std::thread& worker : workers)
            worker.join();
    }

    // 添加任务到线程池
    void enqueue(std::function<void()> task)
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.push(std::move(task));
        }
        condition.notify_one();
    }

    size_t getTaskCount()
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        return tasks.size();
    }

private:
    std::vector<std::thread> workers;         // 工作线程
    std::queue<std::function<void()>> tasks;  // 任务队列
    std::mutex queueMutex;                    // 互斥锁
    std::condition_variable condition;         // 条件变量
    bool stop;                                // 停止标志
};


// ========================================
// 示例1: 最简单的使用
// ========================================
void example1_basic()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例1: 线程池基础使用" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // 创建一个有 3 个线程的线程池
    SimpleThreadPool pool(3);

    std::cout << "创建了 3 个工作线程的线程池\n" << std::endl;

    // 提交 5 个任务
    for (int i = 0; i < 5; ++i)
    {
        pool.enqueue([i] {
            std::cout << "任务 " << i << " 开始执行（线程ID: "
                      << std::this_thread::get_id() << "）" << std::endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            std::cout << "任务 " << i << " 完成" << std::endl;
        });
    }

    std::cout << "\n已提交 5 个任务到线程池" << std::endl;
    std::cout << "等待所有任务完成...\n" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "\n💡 注意观察:" << std::endl;
    std::cout << "   - 3 个线程同时处理 5 个任务" << std::endl;
    std::cout << "   - 前 3 个任务并行执行" << std::endl;
    std::cout << "   - 后 2 个任务等待空闲线程\n" << std::endl;
}


// ========================================
// 示例2: 对比性能 - 不用线程池 vs 使用线程池
// ========================================
void example2_performance()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例2: 性能对比" << std::endl;
    std::cout << "========================================\n" << std::endl;

    const int TASK_COUNT = 20;

    // 方法1: 不使用线程池（每个任务创建一个线程）
    std::cout << "【方法1】不使用线程池 - 每个任务创建一个新线程" << std::endl;

    auto start1 = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < TASK_COUNT; ++i)
    {
        threads.emplace_back([i] {
            // 模拟工作
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }

    for (auto& t : threads)
        t.join();

    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);

    std::cout << "完成时间: " << duration1.count() << " 毫秒" << std::endl;
    std::cout << "创建了 " << TASK_COUNT << " 个线程\n" << std::endl;


    // 方法2: 使用线程池
    std::cout << "【方法2】使用线程池 - 4 个线程复用" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();

    SimpleThreadPool pool(4);
    std::atomic<int> completed(0);

    for (int i = 0; i < TASK_COUNT; ++i)
    {
        pool.enqueue([&completed] {
            // 模拟工作
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            completed++;
        });
    }

    // 等待所有任务完成
    while (completed < TASK_COUNT)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);

    std::cout << "完成时间: " << duration2.count() << " 毫秒" << std::endl;
    std::cout << "只使用了 4 个线程（复用）\n" << std::endl;

    std::cout << "💡 结论:" << std::endl;
    std::cout << "   线程池避免了频繁创建/销毁线程的开销" << std::endl;
    std::cout << "   线程数量可控，避免资源耗尽\n" << std::endl;
}


// ========================================
// 示例3: 实际应用 - 批量处理数据
// ========================================
void example3_batch_processing()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例3: 批量处理数据" << std::endl;
    std::cout << "========================================\n" << std::endl;

    SimpleThreadPool pool(4);
    std::mutex outputMutex;
    std::atomic<int> totalProcessed(0);

    std::cout << "模拟处理 100 个数据项...\n" << std::endl;

    // 提交 100 个任务
    for (int i = 0; i < 100; ++i)
    {
        pool.enqueue([i, &outputMutex, &totalProcessed] {
            // 模拟数据处理（例如：图像处理、文件转换等）
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            totalProcessed++;

            // 每处理 10 个输出一次
            if (totalProcessed % 10 == 0)
            {
                std::lock_guard<std::mutex> lock(outputMutex);
                std::cout << "已处理: " << totalProcessed << "/100" << std::endl;
            }
        });
    }

    // 等待完成
    while (totalProcessed < 100)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "\n✅ 所有数据处理完成！" << std::endl;
    std::cout << "\n💡 适用场景:" << std::endl;
    std::cout << "   - 批量图片处理" << std::endl;
    std::cout << "   - 文件格式转换" << std::endl;
    std::cout << "   - 数据导入/导出" << std::endl;
    std::cout << "   - 并行计算\n" << std::endl;
}


// ========================================
// 示例4: 带返回值的任务
// ========================================
void example4_with_results()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例4: 计算并收集结果" << std::endl;
    std::cout << "========================================\n" << std::endl;

    SimpleThreadPool pool(4);
    std::vector<int> results(10, 0);
    std::mutex resultMutex;
    std::atomic<int> completed(0);

    std::cout << "使用线程池计算 1-10 的平方...\n" << std::endl;

    // 提交计算任务
    for (int i = 0; i < 10; ++i)
    {
        pool.enqueue([i, &results, &resultMutex, &completed] {
            // 计算平方
            int square = (i + 1) * (i + 1);

            // 保存结果
            {
                std::lock_guard<std::mutex> lock(resultMutex);
                results[i] = square;
            }

            completed++;
        });
    }

    // 等待完成
    while (completed < 10)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // 显示结果
    std::cout << "计算结果:" << std::endl;
    for (int i = 0; i < 10; ++i)
    {
        std::cout << "  " << (i + 1) << "² = " << results[i] << std::endl;
    }

    std::cout << "\n💡 线程池也可以用于需要返回结果的任务\n" << std::endl;
}


int main()
{
    std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║      线程池（ThreadPool）教程          ║" << std::endl;
    std::cout << "║    (展示 POCO ThreadPool 原理)         ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝" << std::endl;

    example1_basic();            // 基础使用
    example2_performance();      // 性能对比
    example3_batch_processing(); // 批量处理
    example4_with_results();     // 带结果

    std::cout << "\n========================================" << std::endl;
    std::cout << "🎓 核心概念总结" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "1. 线程池 = 预先创建的线程集合" << std::endl;
    std::cout << "2. 任务队列 = 待处理的工作项" << std::endl;
    std::cout << "3. 线程复用 = 避免频繁创建/销毁线程" << std::endl;
    std::cout << "4. 资源控制 = 限制并发线程数量" << std::endl;
    std::cout << "\n使用场景:" << std::endl;
    std::cout << "  ✅ 批量数据处理" << std::endl;
    std::cout << "  ✅ 并发网络请求" << std::endl;
    std::cout << "  ✅ 文件/图片处理" << std::endl;
    std::cout << "  ✅ 任何需要并发执行的任务" << std::endl;
    std::cout << "\nPOCO ThreadPool 用法类似，但功能更强大！\n" << std::endl;

    return 0;
}
