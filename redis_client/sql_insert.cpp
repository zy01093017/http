#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "sql_api.h"


#define SIZE 1024

static void insert_data(char* data)
{
  int i = 0;
  //key=key&100=20 
  cout<<data<<endl;
  string str; while(data[i] != '\0'){ if(data[i] == '='){//第一个参数 
    ++i;
    while(data[i] != '&'){
      str += data[i];
      ++i;
      if(data[i] == '\0'){
        break;
      }
    }
    str += ' ';
  }
  ++i;
  }
  cout<<str.c_str()<<endl;
  RedisSql s;
  s.sql_connect();
  //s.sql_inseert(str);
}

int main()
{
  char method[SIZE];
  int  len;
  char query_string[SIZE];

  if(getenv("METHOD")){
    printf("%p\n",method);
    strcpy(method, getenv("METHOD"));
    printf("method: %s\n",method);
  }
  else{
    cout<<"METHOD is not exist"<<endl;
    return 1;
  }

  if(strcasecmp(method,"get") == 0){
    if(getenv("QUERY_STRING")){
      printf("%p\n",query_string);
      strcpy(query_string,getenv("QUERY_STRING"));
      printf("query_string: %s\n",query_string);
    }
    else{
      cout<<"QUERY_STRING is not enxit"<<endl;
      return 2;
    }
  }
  else{
    int i = 0;
    len = atoi(getenv("CONTENT_LENGTH"));
    char ch = '\0';
    for(; i < len; ++i){
      read(0,&ch,1);
      query_string[i] = ch;
    }
    query_string[i] = 0;
  }
  insert_data(query_string);
  return 0;
}
