#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <signal.h>
#include <hiredis/hiredis.h>
#include <sys/time.h>
#include <string>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>

#include "redis.h" 

using namespace std;

#define IP  "172.27.0.14"
#define PORT 2020
#define MAX  1024

int64_t getCurrentTime()      //直接调用这个函数就行了，返回值最好是int64_t，long long应该也可以
{    
  struct timeval tv;    
  gettimeofday(&tv,NULL);    //该函数在sys/time.h头文件中
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;    
}

void echo_html(int sock, char* status_code, const char* response)
{
  //printf("code:%s\n",code);
  char erro[MAX/16];
  char path[MAX];

  sprintf(erro,"/%s",status_code);

  //printf("erro:%s\n",erro);
  strcpy(erro+4,".html");
  printf("path:%s\n",erro);
  sprintf(path,"source%s",erro);
  printf("errpath:%s\n",path);

  
  cout<<path<<endl;
  int fd = open(path, O_RDONLY);//响应的html文件
  printf("htmlfd:%d \n",fd);

  char header_msg[MAX];//响应行
  strcpy(header_msg,response);

  printf("header_msg:%s\n",header_msg);

  if(send(sock,header_msg, strlen(header_msg), 0) < 0){//发
    perror("send");
  }
  struct stat st;
  cout<<path<<endl;
  if(stat(path,&st)< 0){
    perror("stat");
  }
  int Content_length = st.st_size;//响应报头
  char len[MAX];
  sprintf(len, "CONTENT-LENGTH:%d",Content_length);
  //printf("len:%s\n",len);

  if(send(sock,len, strlen(len), 0) < 0){//发送响应报头
    perror("send");
  }


  const char* msg ="\r\n";
  if(send(sock,msg, strlen(msg), 0)< 0){//空行
    perror("send");
  }
  //printf("eeeeeeeeeee\n");

  if(sendfile(sock,fd, NULL, st.st_size) < 0){//正文
    perror("sendfile");
  }
  //printf("ok\n");
  close(fd);
}


void echo_erro(int sock,int status_code)
{
  switch(status_code){
    case 400:
      {
        char* erro = "400";
        const char* head = "HTTP/1.0 400 Bad Request!";
        echo_html(sock, erro, head);

      }
      break;
    case 403:
      {
        char* erro = "403";
        const char* head = "HTTP/1.0 403 Forbidden!";
        echo_html(sock, erro, head);

      }
      break;
    case 404:
      {
        char* erro = "404";
        const char* head = "HTTP/1.0 404 Not Found!";
        echo_html(sock, erro, head);

      }
      break;
    case 500:
      {
        char* erro = "500";
        const char* head = "HTTP/1.0 400 Internal Server Error!";
        echo_html(sock, erro, head);

      }
      break;
    case 503:
      {
        char* erro = "503";
        const char* head = "HTTP/1.0 503 Server Unavailable!";
        echo_html(sock, erro, head);
      }
      break;
    default:
      break;
  }
}

int Start()
{
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0){
    perror("socket");
    exit(1);
  }

  int opt = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  struct sockaddr_in local;
  local.sin_family = AF_INET;
  local.sin_addr.s_addr = inet_addr(IP);
  local.sin_port = htons(PORT);

  int bin = bind(sock, (struct sockaddr*)&local, sizeof(local));
  if(bin < 0){
    perror("bind");
    exit(2);
  }

  if(listen(sock, 5) < 0){
    perror("listen");
    exit(3);
  }

  return sock;
}

int GetLine(int sock, char line[], int size)
{
  int i = 0;
  char c = '\0';
  while(c != '\n' && i < size)
  {
    ssize_t s = recv(sock, &c, 1, 0);
    if(s < 0){
      perror("recv");
      return 400;
    }
    else if(s>0){
      if(c == '\r'){//判断下一个字符是否是'\n'，如果是'\n'读上来,修改，不是，说明读了一行
        recv(sock, &c, 1, MSG_PEEK);
        if(c == '\n'){//表示这是'\r\n'
          recv(sock,&c,1,0);//那么直接将'\n'读取上来,此时c直接被改为'\n'
        }
        else{//将'\r'直接置为'\n'
          c = '\n';
        }
      }
      line[i] = c;
      ++i;
    }
    else 
    {
      perror("client closed...!");
      return -1;
    }
  }
  line[i] = '\0';
  return i; 

}

void clean_header(int sock)
{
  //清理头部,一直读完
  char buf[MAX];
  do{
    GetLine(sock,buf,MAX);
  }while(strcmp(buf,"\n") != 0);
}

int find_point(char* str)
{
  char* cur=str;
  while(*str!='.')
  {
    ++str;
  }
  return str-cur;
}

string get_time()//获取当前日期
{
  time_t timep;
  time (&timep);
  char tmp[64];
  strftime(tmp, sizeof(tmp), "%Y-%m-%d",localtime(&timep) );
  return tmp;

}

string read_file_to_string(const char* filename)//读取文件字节流到string中
{
  ifstream ifile(filename);
  //将文件读入到ostringstream对象buf中
  ostringstream buf;
  char ch;
  while(buf&&ifile.get(ch))
    buf.put(ch); 
  //返回与流对象buf关联的字符串
  return buf.str();
}

int write_string_to_file(const char* file, const string& str )//将string包含的字节流写成文件
{
  ofstream OsWrite(file);
  OsWrite<<str;
  OsWrite.close();
  return 0;
}

int string_replace(string& str,const char* path)
{
  string mode="img src=\"xxxxx\" alt=\"false\"";
  string sub="img src=\"xxxxx\" alt=\"true\"";
  size_t pos=-1;
  if((pos=str.find(mode))!=string::npos)
  {
    size_t sub_pos=sub.find("xxxxx");
    sub.replace(sub_pos,5,path);
    str.replace(pos, mode.size(), sub);    //用s1替换s中从pos开始（包括0）的n个字符的子串
    return 0;
  }
  return -1;
}

int change_html_to_send(string& str,string& path)
{
  struct dirent *direntp;
  DIR *dirp = opendir(path.c_str());

  struct stat s_buf;
  string image_path="img/fun_images/";
  if (dirp != NULL) {
    while ((direntp = readdir(dirp)) != NULL){
      stat(direntp->d_name,&s_buf);
      if((strcmp(direntp->d_name,"..")!=0) && (strcmp(direntp->d_name,".")!=0))
      {
        if(string_replace(str,(image_path+direntp->d_name).c_str())!=0)
        {
          break;
        }
      }
    }
  }
  closedir(dirp);
  return 200;
}

int send_html(int sock,char* path)
{
  string image_path="source/img/fun_images/";//动态加载的图片所在的路径
  Redis *r = new Redis();  
  if(!r->connect("127.0.0.1", 6179))  
  {  
    printf("connect error!\n");  
    return 404;  
  } 
  string today=get_time();
  today+=".html";
  string html_data=r->get(today);
  string road="/www/wwwroot/ftpuser/http/source/";
  road+=today;
  if(html_data.empty())//redis没有缓存
  {
    html_data=read_file_to_string(path);
    change_html_to_send(html_data,image_path);
    write_string_to_file((road).c_str(),html_data);//将string包含的字节流写成文件
    r->set(today,html_data);
  }
  int fd=open((road).c_str(),O_RDONLY);
  struct stat file;
  stat((road).c_str(),&file);

  if(sendfile(sock,fd,NULL,file.st_size) < 0)
  {
    perror("send...!");
  }
  delete r;
  return 200;
}

static int echo_www(int sock,char* path, int size)
{
  printf("path:%s\n",path);
  int fd = open(path,O_RDONLY);
  char header_msg[MAX];//响应行
  const char* response = "HTTP/1.0 200 OK!\r\n";
  strcpy(header_msg,response);

  if(send(sock,header_msg, strlen(header_msg)+1, 0) < 0){
    perror("send");
  }

  int Content_length = size;//响应报头
  char len[MAX];
  char tail[5] = {0};
  char type[MAX] = {0};
  int index = find_point(path);
  strcpy(tail,path+index+1);
  if(strcmp(tail,"jpg") == 0){
    char str[30]={0};
    strcpy(str,"image/jpeg\r\n");
    sprintf(type, "Content-Type:%s",str);
    printf("type:%s\n",type);
    if(send(sock,type, strlen(type)+1, 0) < 0){//发送响应报头
      perror("send");
    }
  }else if(strcmp(tail,"css") == 0){
    char str[30]={0};
    strcpy(str,"text/css\r\n");
    sprintf(type, "CONTENT-TYPE:%s",str);
    if(send(sock,type, strlen(type)+1, 0) < 0){//发送响应报头
      perror("send");
    }

  }
  else if(strcmp(tail,"js") == 0){
    char str[30]={0};
    strcpy(str,"application/x-javascript\r\n");
    sprintf(type, "CONTENT-TYPE:%s",str);
    if(send(sock,type, strlen(type)+1, 0) < 0){//发送响应报头
      perror("send");
    }

  }else if(strcmp(tail,"gif") == 0){
    char str[30]={0};
    strcpy(str,"image/gif\r\n");
    sprintf(type, "CONTENT-TYPE:%s",str);
    if(send(sock,type, strlen(type)+1, 0) < 0){//发送响应报头
      perror("send");
    }

  }else if(strcmp(tail,"png") == 0){
    char str[30]={0};
    strcpy(str,"image/png\r\n");
    sprintf(type, "CONTENT-TYPE:%s",str);
    if(send(sock,type, strlen(type)+1, 0) < 0){//发送响应报头
      perror("send");
    }

  }else if(strcmp(tail,"html") == 0){
    char str[30]={0};
    strcpy(str,"text/html\r\n");
    sprintf(type, "CONTENT-TYPE:%s",str);
    if(send(sock,type, strlen(type)+1, 0) < 0){//发送响应报头
      perror("send");
    }

  }else if(strcasecmp(tail,"mp4") == 0){
    char str[30]={0};
    strcpy(str,"video/mpeg4\r\n");
    sprintf(type, "CONTENT-TYPE:%s",str);
    if(send(sock,type, strlen(type), 0) < 0){//发送响应报头
      perror("send");
    }

  }
  else if(strcmp(tail,"ico") == 0){
    char str[30]={0};
    strcpy(str,"image/x-icon\r\n");
    sprintf(type, "CONTENT-TYPE:%s",str);
    printf("type:%s\n",type);
    if(send(sock,type, strlen(type), 0) < 0){//发送响应报头
      perror("send");
    }

  }
  printf("length:%d\n",Content_length);
  sprintf(len, "CONTENT-LENGTH:%d\r\n",Content_length);
  
  printf("contnet-len:%s\n",len);
  if(send(sock,len, strlen(len), 0) < 0){//发送响应报头
    perror("send");
  }


  const char* msg ="\r\n";
  if(send(sock,msg, strlen(msg), 0)< 0){//空行
    perror("send");
  }
  
  if(strcmp(path,"source/home.html")==0)
  {
    return send_html(sock,path);//发送正文
  }
  return sendfile(sock,fd,NULL,size);
}


int echo_cgi(int sock,char method[],char path[],char* query_string)
{
  char line[MAX];
  int  Content_length = -1;
  char cont_len_env[MAX];
  char method_env[MAX/16];
  char query_string_env[MAX];

  if(strcasecmp(method, "GET") == 0){//是GET方法,去掉头部
    clean_header(sock);
  }
  else{//POST方法,丢弃头部，提取Content-length

    do{
      GetLine(sock,line, MAX);
      if(strncasecmp(line,"CONTENT-LENGTH",16) == 0){
        Content_length = atoi(line+16);
      }

    }while(strcmp(line,"\n")); 
    if(Content_length == -1)
    {
      //出错 
      return 404; 
    }
    // printf("length:%d \n",Content_length);
  }


  const char* msg = "HTTP/1.0 200 OK";
  if(send(sock,msg,strlen(msg),0) < 0){
    perror("send");
  }
  const char* Content = "Content-Type:test/html";
  if(send(sock,Content,strlen(Content),0) < 0){
    perror("send");
  }
  const char* empty = "\r\n";
  if(send(sock,empty,strlen(empty),0) < 0){
    perror("send");
  }
  //1.开始发送正文
  //2.POST方法，父进程将正文部分数据给子进程
  //3.子进程进行进程替换,将执行结果放回管道,给父进程
  int input[2];//子进程
  int output[2];

  pipe(input);
  pipe(output);

  pid_t id = fork();
  if(id < 0){
    perror("fork");
    return 500;
  }
  else if(id == 0){//子进程
    //1.判断是GET方法还是POST方法
    close(input[1]);//关闭子进程写
    close(output[0]);//关闭父进程度


    //进行进程替换,但是将方法和参数都传给子进程,需要将其导入环境变量
    //又环境变量不会被进程替换,因此需要将其导出

    sprintf(method_env,"METHOD=%s",method);
    putenv(method_env);
    printf("method:%s\n",method); 
    if(strcasecmp(method,"GET") == 0){

      sprintf(query_string_env,"QUERY_STRING=%s",query_string);
      printf("%s\n",query_string_env);
      if(putenv(query_string_env)){
        perror("putenv");
      }
    }
    else{
      sprintf(cont_len_env,"CONTENT_LENGTH=%d",Content_length);
      putenv(cont_len_env);
      printf("%s\n",cont_len_env);
    }
    dup2(input[0],0);//从标准输入里读,父进程读到数据写入了子进程的管道,此时子进程从标准输入里读取数据
    dup2(output[1],1);//往父进程管道里写,父进程写到标准输出
    execl(path,path,NULL);//进程替换,执行可执行程序
    perror("execl");
    exit(1);
  }
  else{//父进程
    close(input[0]);//关闭子进程的读
    close(output[1]);//关闭父进程的读

    signal(SIGPIPE,SIG_IGN);
    char c = 'a';
    if(strcasecmp(method, "POST") == 0){
      int i = 0;
      for(; i < Content_length; ++i){//将读取到的数据写道子进程的管道
        recv(sock, &c, 1, 0);
        write(input[1],&c,1);

      }
    }

    while(read(output[0], &c, 1) > 0){//从父进程的管道当中读取数据写会浏览器
      send(sock, &c, 1, 0);
    }

    close(input[1]);
    close(output[0]);

    waitpid(id, NULL, 0);

  }
  return 200;

}

void update_price_html(const char* file)//更新price.html页面的内容
{
    string str=read_file_to_string(file); 
    Redis *r = new Redis();  
    if(!r->connect("127.0.0.1", 6179))  
    {  
      printf("connect error!\n");  
    }
    string time=get_time();
    string price = r->get(time);
    if(price=="finish")
    {
      return;
    }
    r->set(time,"finish");
    string mode="future: true }]";
    string sub="{ label: \'";
    sub+=time;
    sub+="\', value: ";
    sub+=price;
    sub+=", future: true }]";
    size_t pos=-1;
    if((pos=str.find(mode))!=string::npos)
    {
      str.replace(pos+mode.size()-1,1, ",");    //用s1替换s中从pos开始（包括0）的n个字符的子串
      str.replace(pos+mode.size(),0, sub);    //用s1替换s中从pos开始（包括0）的n个字符的子串
    }

    write_string_to_file(file,str);
    delete r;
}

void hander(int sock, int epoll_fd)
//void* hander(int* arg)
{
  //int sock = (int)arg;
  char line[MAX];//提取一行
  char method[MAX/16];//方法
  char url[MAX];//url资源
  char path[MAX];//路径
  char* query_string;//参数
  int  status_code = 200;//状态码
  int  cgi = 0;//cgi参数
  //printf("sock:%d\n",sock);
  int start=getCurrentTime();
  int ret = GetLine(sock, line, sizeof(line));
  int end1=getCurrentTime();
  printf("Getline() : %ld\n",end1-start);
  // printf("aaaaaaaaaaaa%d\n",ret);
  if(ret  <= 0){
    status_code = 400;
    if(status_code != 200){
      echo_erro(sock,status_code);
    }
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock, NULL);
    close(sock);
    return;
    //return NULL;
    //goto end;
  }//读取一行数据
  // printf("line: %s\n", line);

  //进行分析，提取方法，只处理GET和POST
  size_t  i = 0;
  size_t  j = 0;

  //GET / HTTP/1.0
  while((i < sizeof(line)-1) && j < sizeof(method)-1 && !isspace(line[j])){//以空格为分割,读取方法
    method[j] = line[i];
    j++,i++;
  }
  method[j] = '\0';
  //printf("method:%s ",method);

  j = 0;
  i+=1; 
  //读取url
  while((i < sizeof(url)-1) && !isspace(line[i])){
    url[j] = line[i];
    ++j,++i;
  }
  url[j] = '\0';
  //printf("url:%s ",url);


  //表示读取到方法
  if(strcasecmp(method, "GET") == 0){
    //此时是GET方法，判断有没有参数，如果有参数，是cgi模式
    //url里面有？表示有参数，提取参数和路径
    query_string = url;//获取参数
    while(*query_string != '\0'){

      if(*query_string == '?'){//表示有参数
        *query_string = '\0';
        cgi = 1;
        ++query_string;
        break;
      }
      ++query_string;
    }
    //printf("path:%s ,url: %s,query_string: %s \n",path,url, query_string);
  }
  else if(strcasecmp(method, "POST") == 0){
    cgi = 1;//是POST方法,是cgi处理
  }else{//出错
    status_code = 400;
    clean_header(sock);
    echo_erro(sock,status_code);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock, NULL);
    close(sock);
    return;
  }


  sprintf(path,"source%s",url);

  //printf("path:%s\n",path);
  if(path[strlen(path)-1] == '/'){

    Redis *r = new Redis();  
    if(!r->connect("127.0.0.1", 6179))  
    {  
      printf("connect error!\n");  
    }
    string today=get_time();
    string html_data=r->get(today);
    if(html_data.empty())//redis没有缓存
    {
      strcat(path,"home.html");
    }
    else 
    {
      strcat(path,(today+".html").c_str());
    }
    delete r;
  }
  printf("path:%s \n",path);
  struct stat buf;
  if(stat(path,&buf) < 0){
    cout <<"1111111111"<<endl;
    status_code = 404;
    clean_header(sock);
    echo_erro(sock,status_code);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock, NULL);
    close(sock);
    return;
  }
  else{
    if(strcmp(url,"/price.html")==0)
    {
      update_price_html(path);
    }
    if(S_ISDIR(buf.st_mode))
    {
      strcat(path,"index.html");
    }
    else if(S_ISREG(buf.st_mode)){
      if((buf.st_mode & S_IXGRP) || (buf.st_mode & S_IXUSR) || (buf.st_mode & S_IXOTH))
      {
        cgi = 1;
      }
    }
    else{

    }

  }

  int end2=getCurrentTime();
  printf("判断请求的资源类型 : %ld\n",end2-start);
  printf("cgi:%d\n",cgi);
  if(cgi == 0){//开始响应,此时表示是GET方法,且目录绝对存在

    clean_header(sock);
    int start=getCurrentTime();
    status_code = echo_www(sock,path,buf.st_size);
    // printf("%d\n",status_code);
    int end=getCurrentTime();
    printf("echo_www() : %ld\n",end-start);

  }else{//cgi处理
    // printf("cgi:%d",cgi);
    // if(*query_string == '\0' && strcasecmp(method,"get") == 0)
    // {
    //   status_code = 400;
    //   clean_header(sock);
    //   goto end;
    // }
    int start=getCurrentTime();
    status_code = echo_cgi(sock,method,path,query_string);     
    int end=getCurrentTime();
    printf("echo_cgi() : %ld\n",end-start);
  }

end:
  if(status_code != 200){
    echo_erro(sock,status_code);
  }
  epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock, NULL);
  close(sock);
}

void setNonBlock(int fd)
{
  int fl = fcntl(fd, F_GETFL);
  fcntl(fd, F_SETFL, fl | O_NONBLOCK);
}

//void my_accept(int epoll_fd, int fd)
//{
//  struct epoll_event ev;
//  //while(1){
//    struct sockaddr_in client;
//    socklen_t len = sizeof(client);
//
//    int sock = accept(fd, (struct sockaddr*)&client, &len);
//    if(sock < 0){
//    // continue;
//    }
//    printf("get new client..........\n");
//
//    setNonBlock(sock);//获取到新连接，设置成非阻塞式
//    ev.data.fd = sock;
//    ev.events = EPOLLIN | EPOLLET;
//    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,sock,&ev);
////  }
//  printf("sssssssss\n");
//}
void handlerReadyEvents(int epoll_fd, struct epoll_event  revents[],int num,int listen_sock)
{
  int i = 0;
  //struct epoll_event ev;
  for(; i < num; ++i){
    int fd = revents[i].data.fd;
    uint32_t events = revents[i].events;
    if(listen_sock == fd && (events & EPOLLIN)){//表示监听到了新的链接
      //1.将事件添加到红黑树中，此时还没有就绪事件
      //my_accept(epoll_fd, fd);
      struct epoll_event ev;
      int sock = accept(fd,NULL,NULL);
      if(sock < 0){
        perror("accept");
      }
      printf("get new client...\n");
      //printf("sock:%d\n",sock);
      ev.data.fd = sock;
      ev.events = EPOLLIN;
      if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &ev)==-1)
      {
        perror("epoll_ctl...!");
        return;
      }
    }
    else if(events & EPOLLIN){//关心的读事件就绪,这里表示已经有数据可以读了
      //读取一行,来分析数据,是get还是post,分析是cgi还是返回页面
      hander(fd,epoll_fd); 
    }
  }
}
int main()
{
  int64_t start=getCurrentTime();
  int listen_fd = Start();
  int64_t end=getCurrentTime();
  printf("Start() : %ld\n",end-start);


  //for(; ; ){
  //  int sockfd = accept(listen_fd, NULL, NULL);
  //  if(sockfd < 0){
  //    perror("accept");
  //    continue;
  //  }

  //  printf("get a new accept...\n");
  //  int64_t start=getCurrentTime();
  //  pthread_t tid; 
  //  pthread_create(&tid, NULL,hander, (void*)sockfd); 
  //  pthread_detach(tid); 
  //  end=getCurrentTime();
  //  printf("Accept()->pthread_detach() : %ld\n",end-start);

  start=getCurrentTime();
  int epoll_fd = epoll_create(65535);
  if(epoll_fd < 0){
    perror("epoll_create");
    return 1;
  }
  end=getCurrentTime();
  printf("epoll_create() : %ld\n",end-start);

  struct epoll_event events;
  events.data.fd = listen_fd;
  events.events = EPOLLIN;

  start=getCurrentTime();
  int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &events);
  if(ret < 0){
    perror("epoll_ctl");
    return 2;
  }
  end=getCurrentTime();
  printf("epoll_ctl() : %ld\n",end-start);

  struct epoll_event revents[2048];
  for(; ; ){
    int timeout = -1;
    int num = epoll_wait(epoll_fd,revents,sizeof(revents)/sizeof(revents[0]),timeout);
    switch(num){
      case -1:
        perror("epoll_wait");
        break;
      case 0:
        printf("timeout.......\n");
        break;
      default:
        {
          start=getCurrentTime();
          handlerReadyEvents(epoll_fd, revents,num,listen_fd);
          end=getCurrentTime();
          printf("handlerReadyEvents() : %ld\n",end-start);
        }
    }
  }
  //  }
  close(listen_fd);
  close(epoll_fd);
} 
