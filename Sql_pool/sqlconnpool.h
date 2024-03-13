#ifndef SQLCONN_POOL_H_
#define SQLCONN_POOL_H_

#include "../log/log.h"
#include <cassert>
#include <mutex>
#include <mysql/mysql.h>
#include <queue>
#include <semaphore.h>
#include <string>
#include <thread>

class SqlConnPool {
  SqlConnPool();
  ~SqlConnPool();

  int MAX_CONN_;
  int busyCount_;
  int lazyCount_;

  std::queue<MYSQL *> connQue_;
  std::mutex mtx_;
  sem_t semId_;

public:
  static SqlConnPool *instance();
  MYSQL *getConn();
  void freeConn(MYSQL *conn);
  int getFreeConnCount();

  void init(const char *host, int port, const char *user, const char *pwd,
            const char *dbName, int connSize);

  void closePool();
};

#endif
