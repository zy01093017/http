#!/usr/bin/python
#coding:utf-8

import urllib2
import requests

class HtmlDownloader(object):
    def login_download(self,name,password,url):
        session = requests.session()
        login_data = {
            'userName': name,
            'passWord': password,
            'enter': 'true'
        }
        session.post(url, data=login_data)
        #good_url='https://detail.tmall.com/item.htm?spm=a220m.1000858.1000725.1.62cd7b4dUewMuH&id=558701823537&skuId=3473513664832&areaId=610100&standard=1&user_id=2121207515&cat_id=2&is_b=1&rn=b518fda6c5d20afabc47f54f7ff7ff3c'
        good_url='https://detail.tmall.com/item.htm?spm=a220m.1000858.1000725.8.62cd7b4dUewMuH&id=528567826384&skuId=3235149736520&areaId=610100&standard=1&user_id=2121207515&cat_id=2&is_b=1&rn=b518fda6c5d20afabc47f54f7ff7ff3c'
        res = session.get(good_url)
        return (res.text)

    def download(self,url):
        if url is None:
            return None
        #伪装成浏览器去爬取网页html源码，反爬机制1
        headers={'Accept': '*/*',
                 'Accept-Language': 'en-US,en;q=0.8',
                 'Cache-Control': 'max-age=0',
                 'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.116 Safari/537.36',
                 'Connection': 'keep-alive',
                 'Referer': 'http://www.baidu.com/'
                 }
        req = urllib2.Request(url,None,headers)
        response=urllib2.urlopen(req)#打开url网页
        if response.getcode()!=200:#判断是否打开成功
            return None
        return response.read()
