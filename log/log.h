#ifndef LOG_H_
#define LOG_H_

#include "../buffer/buffer.h"
#include "safequeu.hpp"
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <mutex>
#include <string>
#include <sys/stat.h>
#include <sys/time.h>
#include <thread>

/*
DEBUG 调试级别：用于输出程序的调试信息，通常仅在开发和测试环境中启用。
INFO 信息级别：用于输出程序的运行信息，例如程序的启动、停止、重启等事件。
WARNING
警告级别：用于输出程序的一些非致命性错误和异常情况，例如函数返回值不符合预期等。
ERROR
错误级别：用于输出程序的一些致命性错误和异常情况，例如程序崩溃、无法打开文件等。
FATAL
致命级别：用于输出程序的一些致命性错误和异常情况，例如内存不足、文件系统挂载失败等，通常会导致程序终止运行。
*/

enum LOG_LEVEL { DEBUG = 0, INFO, WARN, ERROR, FATAL };

class Log {
  static const int LOG_PATH_LEN = 256;
  static const int LOG_NAME_LEN = 256;
  static const int MAX_LINES = 50000;

  const char *path_;
  const char *suffix_;

  int MAX_LINES_;

  int lineCount_;
  int toDay_;

  bool isOpen_;

  Buffer buff_;
  LOG_LEVEL level_;
  bool isAsync_;

  FILE *fp_;
  std::unique_ptr<SafeDeque<std::string>> deque_;
  std::unique_ptr<std::thread> writeThread_;
  std::mutex mtx_;

  Log();

  void appendLogLevelTitle_(LOG_LEVEL level);
  virtual ~Log();
  void AsyncWrite_();

public:
  void init(LOG_LEVEL levle, const char *path = "./log",
            const char *suffix = ".log", int maxQueueCapacity = 1024);

  static Log *instance();
  static void flushLogThread();

  void write(LOG_LEVEL level, const char *format, ...);
  void flush();

  LOG_LEVEL getLevel();
  void setLevel(LOG_LEVEL level);
  bool isOpen() { return isOpen_; }
};

#define LOG_BASE(level, format, ...)                                           \
  do {                                                                         \
    Log *log = Log::instance();                                                \
    if (log->isOpen() && log->getLevel() <= level) {                           \
      log->write(level, format, ##__VA_ARGS__);                                \
      log->flush();                                                            \
    }                                                                          \
  } while (0);

#define LOG_DEBUG(format, ...)                                                 \
  do {                                                                         \
    LOG_BASE(DEBUG, format, ##__VA_ARGS__)                                     \
  } while (0);

#define LOG_INFO(format, ...)                                                  \
  do {                                                                         \
    LOG_BASE(INFO, format, ##__VA_ARGS__)                                      \
  } while (0);

#define LOG_WARN(format, ...)                                                  \
  do {                                                                         \
    LOG_BASE(WARN, format, ##__VA_ARGS__)                                      \
  } while (0);

#define LOG_ERROR(format, ...)                                                 \
  do {                                                                         \
    LOG_BASE(ERROR, format, ##__VA_ARGS__)                                     \
  } while (0);

#define LOG_FATAL(format, ...)                                                 \
  do {                                                                         \
    LOG_BASE(FATAL, format, ##__VA_ARGS__)                                     \
  } while (0);

#endif
