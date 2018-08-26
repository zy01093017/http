#include <iostream>
#include <string>
#include <hiredis/hiredis.h>


using namespace std;

class RedisSql
{
  public:
    RedisSql();
    ~RedisSql();

    bool sql_connect();
    void sql_elements(string& data);
  private:
    redisContext* sql;//数据结构
    redisReply* reply;

};

