//
// logger_demo_simple.cpp
//
// 简化版日志演示（模拟 POCO Logger 的工作原理）
// 编译: g++ -std=c++17 logger_demo_simple.cpp -o logger_demo_log
// 运行: ./logger_demo_log
//

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

// 模拟 POCO 的日志级别
enum LogLevel
{
    TRACE = 0,
    DEBUG = 1,
    INFORMATION = 2,
    NOTICE = 3,
    WARNING = 4,
    ERROR = 5,
    CRITICAL = 6,
    FATAL = 7
};

// 简单的 Logger 类（模拟 POCO Logger）
class SimpleLogger
{
public:
    SimpleLogger(const std::string& name, LogLevel level = INFORMATION, bool toFile = false)
        : m_name(name), m_level(level), m_toFile(toFile)
    {
        if (m_toFile)
        {
            m_file.open(name + ".log", std::ios::app);
        }
    }

    ~SimpleLogger()
    {
        if (m_file.is_open())
        {
            m_file.close();
        }
    }

    void trace(const std::string& msg) { log(TRACE, msg); }
    void debug(const std::string& msg) { log(DEBUG, msg); }
    void information(const std::string& msg) { log(INFORMATION, msg); }
    void notice(const std::string& msg) { log(NOTICE, msg); }
    void warning(const std::string& msg) { log(WARNING, msg); }
    void error(const std::string& msg) { log(ERROR, msg); }
    void critical(const std::string& msg) { log(CRITICAL, msg); }
    void fatal(const std::string& msg) { log(FATAL, msg); }

    void setLevel(LogLevel level) { m_level = level; }

private:
    std::string m_name;
    LogLevel m_level;
    bool m_toFile;
    std::ofstream m_file;

    void log(LogLevel level, const std::string& msg)
    {
        // 过滤低于设定级别的日志
        if (level < m_level)
            return;

        // 格式化日志消息
        std::string formattedMsg = formatMessage(level, msg);

        // 输出到控制台
        std::cout << formattedMsg << std::endl;

        // 输出到文件
        if (m_toFile && m_file.is_open())
        {
            m_file << formattedMsg << std::endl;
            m_file.flush();
        }
    }

    std::string formatMessage(LogLevel level, const std::string& msg)
    {
        std::stringstream ss;

        // 时间戳
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

        // 日志级别
        ss << " [" << levelToString(level) << "] ";

        // 日志来源
        ss << m_name << ": ";

        // 消息
        ss << msg;

        return ss.str();
    }

    std::string levelToString(LogLevel level)
    {
        switch (level)
        {
            case TRACE: return "TRACE";
            case DEBUG: return "DEBUG";
            case INFORMATION: return "INFO ";
            case NOTICE: return "NOTICE";
            case WARNING: return "WARN ";
            case ERROR: return "ERROR";
            case CRITICAL: return "CRIT ";
            case FATAL: return "FATAL";
            default: return "UNKNOWN";
        }
    }
};


void example1_basic()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例1: 基础日志使用" << std::endl;
    std::cout << "========================================\n" << std::endl;

    SimpleLogger logger("App", INFORMATION);

    logger.fatal("这是致命错误 (FATAL)");           // 会显示
    logger.critical("这是严重错误 (CRITICAL)");     // 会显示
    logger.error("这是普通错误 (ERROR)");           // 会显示
    logger.warning("这是警告 (WARNING)");          // 会显示
    logger.notice("这是通知 (NOTICE)");            // 会显示
    logger.information("这是信息 (INFORMATION)");   // 会显示
    logger.debug("这条不会显示 (DEBUG)");           // 不会显示
    logger.trace("这条也不会显示 (TRACE)");         // 不会显示

    std::cout << "\n💡 说明: 当前级别是 INFORMATION，只显示 >= INFORMATION 的日志\n" << std::endl;
}


void example2_log_levels()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例2: 日志级别对比" << std::endl;
    std::cout << "========================================\n" << std::endl;

    SimpleLogger errorLogger("ErrorOnly", ERROR);
    SimpleLogger debugLogger("DebugMode", DEBUG);

    std::cout << "--- ErrorOnly Logger (只显示 ERROR 及以上) ---" << std::endl;
    errorLogger.error("错误信息 - 会显示");
    errorLogger.warning("警告信息 - 不会显示（级别太低）");
    errorLogger.information("普通信息 - 不会显示");

    std::cout << "\n--- DebugMode Logger (显示 DEBUG 及以上) ---" << std::endl;
    debugLogger.error("错误信息");
    debugLogger.warning("警告信息");
    debugLogger.information("普通信息");
    debugLogger.debug("调试信息");
    debugLogger.trace("跟踪信息 - 不会显示");

    std::cout << "\n💡 日志级别从高到低:" << std::endl;
    std::cout << "   FATAL(7) > CRITICAL(6) > ERROR(5) > WARNING(4)" << std::endl;
    std::cout << "   > NOTICE(3) > INFORMATION(2) > DEBUG(1) > TRACE(0)\n" << std::endl;
}


void example3_file_logging()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例3: 写入日志文件" << std::endl;
    std::cout << "========================================\n" << std::endl;

    SimpleLogger logger("MyApp", DEBUG, true);  // 第三个参数 = 输出到文件

    logger.information("应用程序启动");
    logger.debug("加载配置文件...");
    logger.information("配置加载完成");
    logger.warning("内存使用率 85%");
    logger.error("数据库连接失败");
    logger.information("尝试重新连接...");

    std::cout << "\n✅ 日志已同时输出到:" << std::endl;
    std::cout << "   1. 控制台（上面显示的）" << std::endl;
    std::cout << "   2. MyApp.log 文件" << std::endl;
    std::cout << "\n   可以运行: cat MyApp.log 查看文件内容\n" << std::endl;
}


void example4_real_world()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例4: 真实场景 - Web 服务器" << std::endl;
    std::cout << "========================================\n" << std::endl;

    SimpleLogger serverLogger("WebServer", INFORMATION, true);

    // 模拟服务器运行
    serverLogger.information("服务器启动，监听端口 8080");

    // 模拟请求
    for (int i = 1; i <= 5; ++i)
    {
        if (i == 3)
        {
            serverLogger.error("请求 #" + std::to_string(i) + " 失败: 404 Not Found");
        }
        else if (i == 4)
        {
            serverLogger.warning("请求 #" + std::to_string(i) + " 响应缓慢: 2.5s");
        }
        else
        {
            serverLogger.information("请求 #" + std::to_string(i) + " 处理成功");
        }
    }

    serverLogger.information("服务器关闭");

    std::cout << "\n💡 这是典型的应用场景:" << std::endl;
    std::cout << "   - 正常操作用 information()" << std::endl;
    std::cout << "   - 潜在问题用 warning()" << std::endl;
    std::cout << "   - 错误情况用 error()" << std::endl;
    std::cout << "   - 调试信息用 debug()\n" << std::endl;
}


void example5_different_components()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例5: 不同组件使用不同 Logger" << std::endl;
    std::cout << "========================================\n" << std::endl;

    SimpleLogger dbLogger("Database", INFORMATION);
    SimpleLogger authLogger("Auth", WARNING);
    SimpleLogger cacheLogger("Cache", DEBUG);

    std::cout << "--- 数据库组件 ---" << std::endl;
    dbLogger.information("连接到数据库");
    dbLogger.debug("执行查询: SELECT * FROM users");  // 不会显示

    std::cout << "\n--- 认证组件 ---" << std::endl;
    authLogger.information("用户登录尝试");            // 不会显示（低于WARNING）
    authLogger.warning("登录失败3次");

    std::cout << "\n--- 缓存组件 ---" << std::endl;
    cacheLogger.debug("从缓存读取: user_123");
    cacheLogger.information("缓存命中率: 95%");

    std::cout << "\n💡 建议:" << std::endl;
    std::cout << "   - 生产环境: 使用 INFORMATION 或 WARNING" << std::endl;
    std::cout << "   - 开发环境: 使用 DEBUG" << std::endl;
    std::cout << "   - 排查问题: 使用 TRACE\n" << std::endl;
}


int main()
{
    std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║     日志系统使用教程                   ║" << std::endl;
    std::cout << "║  (展示 POCO Logger 的工作原理)         ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝" << std::endl;

    example1_basic();                   // 基础使用
    example2_log_levels();              // 日志级别
    example3_file_logging();            // 文件输出
    example4_real_world();              // 真实场景
    example5_different_components();    // 多组件

    std::cout << "\n========================================" << std::endl;
    std::cout << "🎓 核心概念总结" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "1. Logger: 日志记录器，每个模块可以有自己的 Logger" << std::endl;
    std::cout << "2. Level: 日志级别，控制哪些日志会被输出" << std::endl;
    std::cout << "3. Channel: 输出目标（控制台、文件、网络等）" << std::endl;
    std::cout << "4. Formatter: 格式化器，控制日志的显示格式" << std::endl;
    std::cout << "\n使用步骤:" << std::endl;
    std::cout << "  1) 创建 Logger" << std::endl;
    std::cout << "  2) 设置日志级别" << std::endl;
    std::cout << "  3) 调用 logger.information() 等方法记录日志" << std::endl;
    std::cout << "\nPOCO Logger 的用法类似，但功能更强大！\n" << std::endl;

    return 0;
}
