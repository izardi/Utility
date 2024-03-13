#ifndef SQLCONNRAII_H_
#define SQLCONNRAII_H_

#include "sqlconnpool.h"

class SqlConnRAII {
  MYSQL *sql_;
  SqlConnPool *connpool_;

public:
  SqlConnRAII(MYSQL **sql, SqlConnPool *connpool) : connpool_(connpool) {
    assert(connpool);
    sql_ = connpool->getConn();
  }
  ~SqlConnRAII() {
    if (sql_) {
      connpool_->freeConn(sql_);
    }
  }
};

#endif
