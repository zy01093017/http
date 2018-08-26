#include "sql_api.h"
#include <string.h>
#include <sys/time.h>

RedisSql::RedisSql()
  :sql(NULL)
   ,reply(NULL)
{}

RedisSql::~RedisSql()
{
  if(sql != NULL){
    //   redisFree(sql);
    // sql = NULL;
     }
}
int64_t getCurrentTime()      //直接调��这个函数就行了，返回值最好是int64_t，long long应该也可以
{    
  struct timeval tv;    
  gettimeofday(&tv,NULL);    //该函数在sys/time.h头文件中
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;    
}

bool RedisSql::sql_connect()
{
  int start=getCurrentTime();
  sql = redisConnect("127.0.0.1",6179);//指定端口和ip嗲知进行连接数据库
  if(sql==NULL&&sql->err){
    perror("redisConnect");
    redisFree(sql);
    int end=getCurrentTime();
    printf("connect : %ld\n",end-start);
    return false;
  }
  //  printf("connect success\n");
  //reply = (redisReply*)redisCommand(sql,"AUTH 123");//设置了密码进行密码验证
  //if(reply == NULL){
  //perror("redisCommand");
  //redisFree(sql);
  //return false;
  //}
  //cout<<reply->type<<endl;
}

void RedisSql::sql_elements(string& data)
{
  int start=getCurrentTime();
  reply = (redisReply*)redisCommand(sql,"SMEMBERS %s",data.c_str());

  if(reply == NULL){
    perror("redisCommand");
    redisFree(sql);
  }
  int end=getCurrentTime();
  printf("reply : %ld\n",end-start);
  //固定格式
  int start1=getCurrentTime();
  //cout<<"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"<<endl;

  //cout<<"<html xmlns=\"http://www.w3.org/1999/xhtml\">"<<endl;

  //cout<<"  <head>"<<endl;

  //cout<<"      <meta charset=\"UTF-8\" http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />"<<endl;

  //cout<<"    <title>内涵段子</title>"<<endl;

  //cout<<"    <style type=\"text/css\">"<<endl;

  //cout<<".head404{ width:580px; height:234px; margin:50px auto 0 auto; background:url(https://www.daixiaorui.com/Public/images/head404.png) no-repeat;    }"<<endl;

  //cout<<".txtbg404{ width:499px; height:169px; margin:10px auto 0 auto; background:url(https://www.daixiaorui.com/Public/images/txtbg404.png) no-repeat;   }"<<endl;

  //cout<<".txtbg404 .txtbox{ width:390px; position:relative; top:30px; left:60px;color:#eee; font-size:13px;   }"<<endl;

  //cout<<".txtbg404 .txtbox p {margin:5px 0; line-height:18px;}"<<endl;

  //cout<<".txtbg404 .txtbox .paddingbox { padding-top:15px;   }"<<endl;

  //cout<<".txtbg404 .txtbox p a { color:#eee; text-decoration:none;   }"<<endl;

  //cout<<".txtbg404 .txtbox p a:hover { color:#FC9D1D; text-decoration:underline;   }"<<endl;

  //cout<<"    </style>"<<endl;

  //cout<<"  </head>"<<endl;

  //cout<<"  <body bgcolor=\"#494949\">"<<endl;
  //固定格式

  if(reply->type==2){
    int i=0;
    for(i=0;i < (reply->elements);++i)
    {
      redisReply* childReply=reply->element[i];
      if(childReply->type==1){
        //    cout<<"<img src=\"data:image/jpg;base64,"<<childReply->str<<"\"/>"<<endl;
      }

    }
  }

  //cout<<"  </body>"<<endl;

  //cout<<"</html>"<<endl;
  //cout<<"</html>"<<endl;
  int end1=getCurrentTime();
  printf("html : %ld\n",end1-start1);
}
