#!/usr/bin/python
#coding:utf-8

import url_manager
import html_downloader
import html_parser
import redis_connecter
import requests

class SpiderMain(object):#爬虫类
    def __init__(self):#初始化函数
        self.urls=url_manager.UrlManager()#url管理器
        self.downloader=html_downloader.HtmlDownloader()#网页下载器
        self.parser=html_parser.HtmlParser()#网页解析器
        self.connecter=redis_connecter.Connecter()#网页输出器

    def craw_image(self,root_url):#爬虫调度器（按步骤调用各个函数）
        count=1
        self.urls.add_new_url(root_url)#添加主网页的url

        while self.urls.has_new_url():#遍历所有的url
            try:#抛出异常
                new_url=self.urls.get_new_url()#从网页管理器中获取一个未爬取的url
                print "craw %d : %s" %(count,new_url)#打印该url及其是第几个
                html_cont=self.downloader.download(new_url)#下载该url网页中的html代码
                new_urls=self.parser.parse_image(new_url,html_cont)#解析html代码
                self.urls.add_new_urls(new_urls)#将解析后的所有url添加到网页管理器中
                if count==3:
                    break
                count=count+1
            except:
                print "craw failed"

    def craw_price(self,root_url):#爬虫调度器（按步骤调用各个函数）
        try:#抛出异常
            html_cont=self.downloader.login_download('默许菲徐','8613wy.',root_url) #下载该url网页中的html代码
            new_price=self.parser.parse_price(root_url,html_cont) #解析html代码
        except:
            print "craw failed"
        conn=self.connecter.connect_to_redis()
        self.connecter.price_to_redis(conn,new_price)

if __name__=="__main__":
    root_url1='https://www.qiushibaike.com/imgrank'
    #root_url='https://detail.tmall.com/item.htm?id=558701823537&ali_refid=a3_430583_1006:1109480139:N:%E8%80%B3%E6%9C%BA%20%E9%AA%A8%E4%BC%A0%E5%AF%BC:8700dfde1d41c7c8e74eff41275b6584&ali_trackid=1_8700dfde1d41c7c8e74eff41275b6584&spm=a230r.1.14.1&sku_properties=5919063:6536025'
    root_url2='https://login.tmall.com/'

    obj_spider=SpiderMain()
    obj_spider.craw_image(root_url1)
    obj_spider.craw_price(root_url2)

