#include "log.h"

Log::Log()
    : lineCount_(0), isAsync_(false), writeThread_(nullptr), deque_(nullptr),
      toDay_(0), fp_(nullptr) {}

// 关闭thread关闭文件指针
Log::~Log() {
  if (writeThread_ && writeThread_->joinable()) {
    while (!deque_->empty())
      deque_->flush();
    deque_->Close();
    writeThread_->join();
  }

  if (fp_) {
    std::lock_guard<std::mutex> lg(mtx_);
    flush();
    fclose(fp_);
  }
}

LOG_LEVEL Log::getLevel() {
  std::lock_guard<std::mutex> lg(mtx_);
  return level_;
}

void Log::setLevel(LOG_LEVEL level) {
  std::lock_guard<std::mutex> lg(mtx_);
  level_ = level;
}

void Log::init(LOG_LEVEL level = DEBUG, const char *path, const char *suffix,
               int maxQueueSize) {
  isOpen_ = true;
  level_ = level;
  if (maxQueueSize > 0) {
    isAsync_ = true;

    if (!deque_) {
      deque_ = std::make_unique<SafeDeque<std::string>>();
      writeThread_ = std::make_unique<std::thread>(flushLogThread);
    } else
      isAsync_ = false;
    lineCount_ = 0;

    time_t timer = time(nullptr);
    struct tm *sysTime = localtime(&timer);
    struct tm t = *sysTime;
    path_ = path;
    suffix_ = suffix;
    char fileName[LOG_NAME_LEN];
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", path_,
             t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
    toDay_ = t.tm_mday;

    {
      std::lock_guard<std::mutex> lg(mtx_);
      buff_.retrieveAll();

      if (fp_) {
        flush();
        fclose(fp_);
      }

      fp_ = fopen(fileName, "a");
      if (fp_ == nullptr) {
        mkdir(path_, 0777);
        fp_ = fopen(fileName, "a");
      }
      assert(fp_ != nullptr);
    }
  }
}

void Log::write(LOG_LEVEL level, const char *format, ...) {
  struct timeval now {
    0, 0
  };
  gettimeofday(&now, nullptr);
  time_t tSec = now.tv_sec;
  struct tm *sysTime = localtime(&tSec);
  struct tm t = *sysTime;
  va_list vaList;

  if (toDay_ != t.tm_mday || (lineCount_ && (lineCount_ % MAX_LINES == 0))) {

    char newFile[LOG_NAME_LEN];
    char tail[36] = {0};
    snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1,
             t.tm_mday);
    if (toDay_ != t.tm_mday) {
      snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path_, tail, suffix_);
      toDay_ = t.tm_mday;
      lineCount_ = 0;
    } else {
      snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail,
               (lineCount_ / MAX_LINES), suffix_);
    }
    std::unique_lock<std::mutex> ul(mtx_);
    flush();
    fclose(fp_);
    fp_ = fopen(newFile, "a");
    assert(fp_ != nullptr);
  }

  {
    std::lock_guard<std::mutex> lg(mtx_);
    ++lineCount_;
    int n =
        snprintf(buff_.beginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                 t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min,
                 t.tm_sec, now.tv_usec);
    buff_.hasWriteten(n);
    appendLogLevelTitle_(level);

    va_start(vaList, format);
    int m = vsnprintf(buff_.beginWrite(), buff_.writable(), format, vaList);
    va_end(vaList);

    buff_.hasWriteten(m);
    buff_.append("\n\0", 2);

    if (isAsync_ && deque_ && !deque_->full()) {
      deque_->push_back(buff_.retrieveAllToStr());
    } else {
      fputs(buff_.peek(), fp_);
    }
    buff_.retrieveAll();
  }
}

void Log::appendLogLevelTitle_(LOG_LEVEL level) {
  switch (level) {
  case DEBUG:
    buff_.append("[DEBUG]: ", 9);
    break;
  case INFO:
    buff_.append("[INFO]: ", 9);
    break;
  case WARN:
    buff_.append("[WARN]: ", 9);
    break;
  case ERROR:
    buff_.append("[ERROR]: ", 9);
    break;
  case FATAL:
    buff_.append("[FATAL]: ", 9);
    break;
  }
}

// 不懂
void Log::flush() {
  if (isAsync_) {
    deque_->flush();
  }
  fflush(fp_);
}

void Log::AsyncWrite_() {
  std::string str = "";
  while (deque_->pop(str)) {
    std::lock_guard<std::mutex> lg(mtx_);
    fputs(str.c_str(), fp_);
  }
}

Log *Log::instance() {
  static Log instance;
  return &instance;
}

void Log::flushLogThread() { Log::instance()->AsyncWrite_(); }
