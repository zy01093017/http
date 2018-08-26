#!/usr/bin/python
#coding:utf-8

class UrlManager(object):#url管理器（类）
    def __init__(self):#初始化 good
        self.new_urls=set()#未爬取的url
        self.old_urls=set()#爬取过的url

    def add_new_url(self,url):#添加一个新的url good
        if url is None:
            return
        if url not in self.new_urls and url not in self.old_urls:#这是一个新的url
            self.new_urls.add(url)

    def add_new_urls(self,urls):#添加批量的url
        if urls is None or len(urls)==0:
            return
        for url in urls:
            self.add_new_url(url)

    def has_new_url(self):#判断是否有新的待爬取的url good
        return len(self.new_urls)!=0

    def get_new_url(self):#获取一个新的待爬取url good
        new_url=self.new_urls.pop()
        self.old_urls.add(new_url)
        return new_url

