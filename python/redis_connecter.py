#!/usr/bin/python
#coding:utf-8

import os
import redis
import base64
import datetime

class Connecter(object):
    def connect_to_redis(self):
        conn=redis.StrictRedis(host='localhost',port='6179',db='0')#链接redis数据库
        return conn

    def image_to_redis(self,conn):#将本地图片文件全部导入redis数据库
        rootdir = '../images'
        list = os.listdir(rootdir) #列出文件夹下所有的目录与文件
        for i in range(0,len(list)):
            path = os.path.join(rootdir,list[i])
            if os.path.isfile(path):
                with open(path,"rb") as f:# 打开路径对应图片
                    # b64encode是编码，b64decode是解码
                    base64_data = base64.b64encode(f.read())#读取图片转换的二进制文件，并给赋值
                    # base64.b64decode(base64data)
                    conn.sadd("images", base64_data)
    def price_to_redis(self,conn,price):#将本地图片文件全部导入redis数据库
        date=datetime.datetime.now().strftime('%Y-%m-%d')
        conn.set(date, price)
