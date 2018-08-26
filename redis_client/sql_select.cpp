#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "sql_api.h"


#define SIZE 1024


int main()
{
  string key;
  RedisSql s;
  s.sql_connect();
  string set_name="images";
  s.sql_elements(set_name);
  return 0;

}
