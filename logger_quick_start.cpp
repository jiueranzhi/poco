//
// logger_quick_start.cpp - 最简单的日志使用示例
//
// 编译: g++ -std=c++17 logger_quick_start.cpp -o quick_log
// 运行: ./quick_log
//

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>

// 超级简单的日志类
class EasyLogger
{
public:
    EasyLogger(const std::string& filename = "")
    {
        if (!filename.empty())
        {
            logFile.open(filename, std::ios::app);
        }
    }

    // 信息日志
    void info(const std::string& msg)
    {
        write("INFO", msg);
    }

    // 警告日志
    void warn(const std::string& msg)
    {
        write("WARN", msg);
    }

    // 错误日志
    void error(const std::string& msg)
    {
        write("ERROR", msg);
    }

private:
    std::ofstream logFile;

    void write(const std::string& level, const std::string& msg)
    {
        // 获取当前时间
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);

        std::stringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
           << " [" << level << "] " << msg;

        std::string line = ss.str();

        // 输出到屏幕
        std::cout << line << std::endl;

        // 输出到文件
        if (logFile.is_open())
        {
            logFile << line << std::endl;
            logFile.flush();
        }
    }
};


int main()
{
    std::cout << "\n=== 最简单的日志使用示例 ===\n" << std::endl;

    // 1. 创建日志对象（输出到 app.log 文件）
    EasyLogger log("app.log");

    // 2. 记录日志 - 就这么简单！
    log.info("程序启动");
    log.info("正在加载配置...");
    log.warn("配置文件缺少某些选项，使用默认值");
    log.info("配置加载完成");

    // 模拟一些操作
    log.info("连接数据库...");
    log.error("数据库连接失败：超时");
    log.info("尝试重连...");
    log.info("数据库连接成功");

    log.info("程序运行中...");
    log.info("处理了 100 个请求");
    log.warn("内存使用率 75%");

    log.info("程序退出");

    std::cout << "\n✅ 日志已保存到 app.log 文件" << std::endl;
    std::cout << "   运行 cat app.log 查看\n" << std::endl;

    // ============================================
    // POCO Logger 的用法也差不多：
    // ============================================
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "POCO Logger 的用法（概念相同）：" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

    std::cout << "// 1. 创建 Logger" << std::endl;
    std::cout << "Logger& logger = Logger::create(\"MyApp\", channel, level);" << std::endl;
    std::cout << std::endl;

    std::cout << "// 2. 记录日志" << std::endl;
    std::cout << "logger.information(\"程序启动\");" << std::endl;
    std::cout << "logger.warning(\"内存使用率高\");" << std::endl;
    std::cout << "logger.error(\"连接失败\");" << std::endl;
    std::cout << std::endl;

    std::cout << "// 3. 推荐使用宏（自动添加文件名和行号）" << std::endl;
    std::cout << "poco_information(logger, \"程序启动\");" << std::endl;
    std::cout << "poco_warning(logger, \"内存使用率高\");" << std::endl;
    std::cout << "poco_error(logger, \"连接失败\");" << std::endl;
    std::cout << std::endl;

    return 0;
}
