//
// logger_demo.cpp
//
// POCO Logger 日志系统完整教程
// 编译: 需要先编译 POCO 库
//
// 如果 POCO 未编译，可以看 logger_demo_simple.cpp（简化版）
//

#include "Poco/Logger.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FormattingChannel.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/FileChannel.h"
#include "Poco/SplitterChannel.h"
#include "Poco/Message.h"
#include "Poco/AutoPtr.h"
#include <iostream>

using namespace Poco;

void example1_basic()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例1: 基础日志使用" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // 1. 创建一个输出到控制台的 Channel
    AutoPtr<ConsoleChannel> pChannel(new ConsoleChannel);

    // 2. 创建一个 Logger
    Logger& logger = Logger::create("BasicLogger", pChannel, Message::PRIO_INFORMATION);

    // 3. 记录不同级别的日志
    logger.fatal("这是致命错误 (FATAL)");           // 最严重
    logger.critical("这是严重错误 (CRITICAL)");
    logger.error("这是普通错误 (ERROR)");
    logger.warning("这是警告 (WARNING)");
    logger.notice("这是通知 (NOTICE)");
    logger.information("这是信息 (INFORMATION)");   // 当前级别
    logger.debug("这条不会显示 (DEBUG)");            // 低于当前级别
    logger.trace("这条也不会显示 (TRACE)");          // 最低级别

    std::cout << "\n💡 说明: 只显示 >= INFORMATION 级别的日志\n" << std::endl;
}


void example2_log_levels()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例2: 日志级别详解" << std::endl;
    std::cout << "========================================\n" << std::endl;

    AutoPtr<ConsoleChannel> pChannel(new ConsoleChannel);

    // 创建不同级别的 Logger
    Logger& errorLogger = Logger::create("ErrorLogger", pChannel, Message::PRIO_ERROR);
    Logger& debugLogger = Logger::create("DebugLogger", pChannel, Message::PRIO_DEBUG);

    std::cout << "--- ErrorLogger (只显示 ERROR 及以上) ---" << std::endl;
    errorLogger.error("错误信息 - 会显示");
    errorLogger.warning("警告信息 - 不会显示（级别太低）");
    errorLogger.information("普通信息 - 不会显示");

    std::cout << "\n--- DebugLogger (显示 DEBUG 及以上) ---" << std::endl;
    debugLogger.error("错误信息");
    debugLogger.warning("警告信息");
    debugLogger.information("普通信息");
    debugLogger.debug("调试信息");
    debugLogger.trace("跟踪信息 - 不会显示（级别太低）");

    std::cout << "\n💡 日志级别从高到低:" << std::endl;
    std::cout << "   FATAL > CRITICAL > ERROR > WARNING > NOTICE > INFORMATION > DEBUG > TRACE\n" << std::endl;
}


void example3_formatting()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例3: 日志格式化" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // 创建一个格式化器（自定义日志格式）
    AutoPtr<PatternFormatter> pFormatter(new PatternFormatter);
    pFormatter->setProperty("pattern", "%Y-%m-%d %H:%M:%S [%p] %t");
    //                                  日期      时间     级别  消息

    // 将格式化器和控制台 Channel 组合
    AutoPtr<FormattingChannel> pFChannel(new FormattingChannel(pFormatter, new ConsoleChannel));

    Logger& logger = Logger::create("FormattedLogger", pFChannel, Message::PRIO_DEBUG);

    logger.information("这是格式化的日志");
    logger.warning("带有时间戳的警告");
    logger.error("带有时间戳的错误");

    std::cout << "\n💡 常用格式化符号:" << std::endl;
    std::cout << "   %Y-%m-%d = 日期 (2024-01-07)" << std::endl;
    std::cout << "   %H:%M:%S = 时间 (14:30:25)" << std::endl;
    std::cout << "   %p = 优先级 (Information)" << std::endl;
    std::cout << "   %t = 消息文本" << std::endl;
    std::cout << "   %s = 日志来源名称" << std::endl;
    std::cout << "   %N = 节点/主机名" << std::endl;
    std::cout << "   %P = 进程ID\n" << std::endl;
}


void example4_file_logging()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例4: 输出到文件" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // 创建文件 Channel
    AutoPtr<FileChannel> pFileChannel(new FileChannel);
    pFileChannel->setProperty("path", "app.log");           // 文件路径
    pFileChannel->setProperty("rotation", "10 K");          // 每10KB轮转
    pFileChannel->setProperty("archive", "timestamp");      // 归档方式
    pFileChannel->setProperty("times", "local");            // 使用本地时间

    // 格式化
    AutoPtr<PatternFormatter> pFormatter(new PatternFormatter);
    pFormatter->setProperty("pattern", "%Y-%m-%d %H:%M:%S.%i [%p] %t");

    AutoPtr<FormattingChannel> pFChannel(new FormattingChannel(pFormatter, pFileChannel));

    Logger& logger = Logger::create("FileLogger", pFChannel, Message::PRIO_TRACE);

    // 写入日志
    for (int i = 0; i < 5; ++i)
    {
        logger.information("日志消息 #" + std::to_string(i));
        logger.debug("调试信息 #" + std::to_string(i));
    }

    std::cout << "✅ 日志已写入到 app.log 文件" << std::endl;
    std::cout << "   可以用 cat app.log 查看\n" << std::endl;
}


void example5_multiple_outputs()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例5: 同时输出到多个地方" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // 创建 SplitterChannel（分流器）
    AutoPtr<SplitterChannel> pSplitter(new SplitterChannel);

    // 添加控制台输出
    AutoPtr<ConsoleChannel> pConsole(new ConsoleChannel);
    pSplitter->addChannel(pConsole);

    // 添加文件输出
    AutoPtr<FileChannel> pFile(new FileChannel);
    pFile->setProperty("path", "multi.log");
    pSplitter->addChannel(pFile);

    // 格式化
    AutoPtr<PatternFormatter> pFormatter(new PatternFormatter("[%p] %t"));
    AutoPtr<FormattingChannel> pFChannel(new FormattingChannel(pFormatter, pSplitter));

    Logger& logger = Logger::create("MultiLogger", pFChannel, Message::PRIO_INFORMATION);

    logger.information("这条消息同时输出到控制台和文件");
    logger.warning("警告信息也会同时输出");
    logger.error("错误信息同样会同时输出");

    std::cout << "\n✅ 日志同时输出到:" << std::endl;
    std::cout << "   1. 控制台（屏幕上）" << std::endl;
    std::cout << "   2. multi.log 文件\n" << std::endl;
}


void example6_macros()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例6: 使用日志宏（推荐方式）" << std::endl;
    std::cout << "========================================\n" << std::endl;

    AutoPtr<PatternFormatter> pFormatter(new PatternFormatter("[%p] %t (文件:%U 行:%u)"));
    AutoPtr<FormattingChannel> pFChannel(new FormattingChannel(pFormatter, new ConsoleChannel));

    Logger& logger = Logger::create("MacroLogger", pFChannel, Message::PRIO_DEBUG);

    // 使用宏记录日志（会自动添加文件名和行号）
    poco_information(logger, "使用宏记录信息");
    poco_warning(logger, "使用宏记录警告");
    poco_error(logger, "使用宏记录错误");

    // 带参数的宏（类似 printf）
    int userId = 12345;
    std::string userName = "张三";
    poco_information_f2(logger, "用户登录: ID=%d, 名称=%s", userId, userName.c_str());

    // 带条件的宏（只有条件为真才记录）
    int errorCode = 404;
    poco_error_f1(logger, "发生错误: 错误码=%d", errorCode);

    std::cout << "\n💡 常用宏:" << std::endl;
    std::cout << "   poco_information(logger, msg) - 信息日志" << std::endl;
    std::cout << "   poco_warning(logger, msg) - 警告日志" << std::endl;
    std::cout << "   poco_error(logger, msg) - 错误日志" << std::endl;
    std::cout << "   poco_debug(logger, msg) - 调试日志" << std::endl;
    std::cout << "   poco_information_f1(logger, fmt, arg1) - 带1个参数" << std::endl;
    std::cout << "   poco_information_f2(logger, fmt, arg1, arg2) - 带2个参数\n" << std::endl;
}


void example7_get_logger()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例7: 获取已存在的 Logger" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // 在其他地方可以通过名称获取已创建的 Logger
    Logger& logger1 = Logger::get("BasicLogger");
    logger1.information("从任何地方都可以获取已创建的 Logger");

    Logger& logger2 = Logger::get("FileLogger");
    logger2.information("使用 Logger::get(name) 获取");

    std::cout << "\n💡 使用方式:" << std::endl;
    std::cout << "   1. 在程序启动时: Logger::create(\"MyApp\", channel, level)" << std::endl;
    std::cout << "   2. 在其他地方使用: Logger::get(\"MyApp\")\n" << std::endl;
}


int main()
{
    std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║   POCO Logger 日志系统完整教程         ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝" << std::endl;

    example1_basic();           // 基础使用
    example2_log_levels();      // 日志级别
    example3_formatting();      // 格式化
    example4_file_logging();    // 文件输出
    example5_multiple_outputs();// 多路输出
    example6_macros();          // 使用宏
    example7_get_logger();      // 获取 Logger

    std::cout << "\n========================================" << std::endl;
    std::cout << "🎓 总结" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "1. 创建 Logger: Logger::create(name, channel, level)" << std::endl;
    std::cout << "2. 记录日志: logger.information(\"消息\")" << std::endl;
    std::cout << "3. 格式化: 使用 PatternFormatter" << std::endl;
    std::cout << "4. 输出目标: ConsoleChannel, FileChannel" << std::endl;
    std::cout << "5. 推荐使用: poco_information() 等宏\n" << std::endl;

    return 0;
}
