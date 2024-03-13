#include "sqlconnpool.h"

SqlConnPool::SqlConnPool() : busyCount_(0), lazyCount_(0) {}

SqlConnPool::~SqlConnPool() { closePool(); }

void SqlConnPool::init(const char *host, int port, const char *user,
                       const char *pwd, const char *dbName, int connSize = 10) {
  assert(connSize > 0);
  for (int i = 0; i < connSize; ++i) {
    MYSQL *sql = mysql_init(nullptr);
    if (!sql) {
      LOG_ERROR("Mysql init error!");
      assert(sql);
    }
    sql = mysql_real_connect(sql, host, user, pwd, dbName, port, nullptr, 0);

    if (!sql)
      LOG_ERROR("Mysql connect failed!");

    connQue_.push(sql);
  }
  MAX_CONN_ = connSize;
  sem_init(&semId_, 0, MAX_CONN_);
}

SqlConnPool *SqlConnPool::instance() {
  static SqlConnPool connPool;
  return &connPool;
}

void SqlConnPool::freeConn(MYSQL *sql) {
  assert(sql);
  std::lock_guard<std::mutex> lg(mtx_);
  connQue_.push(sql);
  sem_post(&semId_);
}

void SqlConnPool::closePool() {
  std::lock_guard<std::mutex> lg(mtx_);
  while (!connQue_.empty()) {
    auto item = connQue_.front();
    connQue_.pop();
    mysql_close(item);
  }
  mysql_library_end();
}

int SqlConnPool::getFreeConnCount() {
  std::lock_guard<std::mutex> lg(mtx_);
  return connQue_.size();
}

MYSQL *SqlConnPool::getConn() {
  MYSQL *sql = nullptr;
  if (connQue_.empty()) {
    LOG_WARN("SqlConnPool busy!");
    return nullptr;
  }
  sem_wait(&semId_);
  {
    std::lock_guard<std::mutex> lg(mtx_);
    sql = connQue_.front();
    connQue_.pop();
  }
  return sql;
}
