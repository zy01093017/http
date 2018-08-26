#!/usr/bin/python
#coding:utf-8

import urllib
from bs4 import BeautifulSoup
import re
import os
import time

class HtmlParser(object):
    def _get_new_urls(self,page_url,soup):#good
        new_urls=set()
        #爬取糗百的url，按照下面格式
        #<a href="/imgrank/page/2/"</a>
        links=soup.find_all('a',href=re.compile('imgrank/page/\d'))
        for link in links:
            new_url=link['href']
            new_url='https://www.qiushibaike.com'+new_url
            new_urls.add(new_url)
        return new_urls

    def _get_new_img(self,html):
        #<img src="//pic.qiushibaike.com/system/pictures/12078/120788923/medium/app120788923.jpeg" alt="糗事#120788923" class="illustration" width="100%" height="auto">
        imgre = re.compile('//pic.qiushibaike.com\S*.jpeg')
        # 匹配出所有的图片链接，返回结果为url列表
        imglist = imgre.findall(html)

        road='/www/wwwroot/ftpuser/http/source/img/fun_images/'
        if not os.path.exists(road):
            os.makedirs(road)

        ct = time.time()
        local_time = time.localtime(ct)
        now_time = time.strftime("%Y-%m-%d%H:%M:%S", local_time)

        x=0
        for imgurl in imglist:
            imgurl='http:'+imgurl
            print imgurl
            #将每张图片保存到指定文件夹
            urllib.urlretrieve(imgurl,road+now_time+'-%s.jpg'%x)
            x+=1

    def _get_new_price(self,html):
        #{"names":"页岩灰 官方标配 ","pvs":"1627207:789488376;5919063:6536025","skuId":"3473513664832"},
        #{"priceCent":119800,"price":"1198.00","cspuId":1000032169140842,"stock":99,"skuId":"3473513664832"}
        #goods_imgre = re.compile(r'"names":'+u'".{1,9}"')
        prices_imgre = re.compile(r'"price":"\d+.\d+"')
        # 匹配出所有的图片链接，返回结果为url列表
        #goods= goods_imgre.findall(html)
        #for good in goods:
            #good=good[9:]
            #good=good[:-1]
            #print good
        prices= prices_imgre.findall(html)
        for price in prices:
            price=price[9:]
            return price[:-1]

    def parse_image(self,page_url,html_cont):#
        if page_url is None or html_cont is None:
            return
        soup = BeautifulSoup(html_cont,'html.parser',from_encoding='utf-8')
        new_urls=self._get_new_urls(page_url,soup)#继续获取当前页面的其他url
        self._get_new_img(html_cont)#将该页面的图片下载到本地images文件夹中
        return new_urls
    def parse_price(self,page_url,html_cont):#
        if page_url is None or html_cont is None:
            return
        new_price=self._get_new_price(html_cont)
        return new_price



