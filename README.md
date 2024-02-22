- [EChat概述](#echat--)
  * [应用技术](#----)
  * [代码结构](#----)
  * [重点待改进点](#----)
- [功能模块（图片形式展示）](#------------)
  * [1. 登录注册模块](#1-------)
  * [2.消息收发模块](#2------)
  * [3.联系人管理模块](#3-------)
  * [4.文件管理模块](#4------)
  * [5.用户配置模块](#5------)
  * [6.用户反馈模块](#6------)
- [部分截图](#----)
  * [系统登录界面效果图](#---------)
  * [好友消息收发界面效果图](#-----------)
  * [好友申请列表效果图](#---------)
  * [个人和好友信息展示效果图](#------------)
  * [发送图片效果图](#-------)
  * [创建群聊效果图](#-------)
  * [用户配置界面图](#-------)
 
## EChat概述
​	Qt即时通讯软件EChat。23年大四下毕业设计，本项目主要提供稳定且支持多平台的通讯软件，以提升用户体验。
  至于代码方面也存在很多问题的，很多其实需要重写和优化的。
 
### 应用技术
  Linux 、 C/C++ 、 QT 、 Socket 、 epoll 、 CMake 、 QSS 、 JSON 、 HTML 、 MySQL 、 SQLite 。

### 代码结构
  服务器代码：epoll+多线程实现高并发，数据存储使用MySQL
  客户端代码：qt编写，数据存储使用SQLite
  
### 重点待改进点
  主要列举几个其他的可以自行进行完善和补充哈
  
  1、使用线程池去管理线程的创建和销毁，提高线程处理效率和减少CPU负载；
  
  2、使用内存管理，这样对于请求报文(json)这种大小相对固定的数据不必每次读取申请内存；2、大文件收发的实现，这里程序中并没有解决此问题，如果文件过大可能会导致服务器崩溃（几十M以上），恰好我这里采用了systemd去管理服务端程序，一定长度上缓解了此问题。
  
## 功能模块（图片形式展示）

### 1. 登录注册模块

![image](https://github.com/swansfought/EChat/blob/main/%E9%83%A8%E5%88%86%E6%88%AA%E5%9B%BE/md/%E7%99%BB%E5%BD%95%E6%B3%A8%E5%86%8C%E6%A8%A1%E5%9D%97%E5%A4%84%E7%90%86%E6%B5%81%E7%A8%8B%E5%9B%BE.png)


### 2.消息收发模块

![image](https://github.com/swansfought/EChat/blob/main/%E9%83%A8%E5%88%86%E6%88%AA%E5%9B%BE/md/%E6%B6%88%E6%81%AF%E6%94%B6%E5%8F%91%E5%8A%9F%E8%83%BD%E6%A8%A1%E5%9D%97%E5%9B%BE.png)


### 3.联系人管理模块

![image](https://github.com/swansfought/EChat/blob/main/%E9%83%A8%E5%88%86%E6%88%AA%E5%9B%BE/md/%E8%81%94%E7%B3%BB%E4%BA%BA%E7%AE%A1%E7%90%86%E6%A8%A1%E5%9D%97%E5%9B%BE.png)


### 4.文件管理模块

只列出已实现的

![image](https://github.com/swansfought/EChat/blob/main/%E9%83%A8%E5%88%86%E6%88%AA%E5%9B%BE/md/%E6%96%87%E4%BB%B6%E7%AE%A1%E7%90%86%E6%A8%A1%E5%9D%97%E5%9B%BE.png)


### 5.用户配置模块

只列出已实现的

![image](https://github.com/swansfought/EChat/blob/main/%E9%83%A8%E5%88%86%E6%88%AA%E5%9B%BE/md/%E7%94%A8%E6%88%B7%E9%85%8D%E7%BD%AE%E5%8A%9F%E8%83%BD%E6%A8%A1%E5%9D%97%E5%9B%BE(%E5%9B%BE%E4%B8%AD%E4%B8%BA%E5%B7%B2%E5%AE%9E%E7%8E%B0%E7%9A%84).png)


### 6.用户反馈模块


## 部分截图
### 系统登录界面效果图

![image](https://github.com/swansfought/EChat/blob/main/%E9%83%A8%E5%88%86%E6%88%AA%E5%9B%BE/%E7%B3%BB%E7%BB%9F%E7%99%BB%E5%BD%95%E7%95%8C%E9%9D%A2%E6%95%88%E6%9E%9C%E5%9B%BE.png)


### 好友消息收发界面效果图

![image](https://github.com/swansfought/EChat/blob/main/%E9%83%A8%E5%88%86%E6%88%AA%E5%9B%BE/%E5%A5%BD%E5%8F%8B%E6%B6%88%E6%81%AF%E6%94%B6%E5%8F%91%E7%95%8C%E9%9D%A2%E6%95%88%E6%9E%9C%E5%9B%BE.png)


### 好友申请列表效果图

![image](https://github.com/swansfought/EChat/blob/main/%E9%83%A8%E5%88%86%E6%88%AA%E5%9B%BE/%E5%A5%BD%E5%8F%8B%E7%94%B3%E8%AF%B7%E5%88%97%E8%A1%A8%E6%95%88%E6%9E%9C%E5%9B%BE.png)


### 个人和好友信息展示效果图

![image](https://github.com/swansfought/EChat/blob/main/%E9%83%A8%E5%88%86%E6%88%AA%E5%9B%BE/%E4%B8%AA%E4%BA%BA%E5%92%8C%E5%A5%BD%E5%8F%8B%E4%BF%A1%E6%81%AF%E5%B1%95%E7%A4%BA%E6%95%88%E6%9E%9C%E5%9B%BE.png)


### 发送图片效果图

![image](https://github.com/swansfought/EChat/blob/main/%E9%83%A8%E5%88%86%E6%88%AA%E5%9B%BE/%E5%8F%91%E9%80%81%E5%9B%BE%E7%89%87%E6%95%88%E6%9E%9C%E5%9B%BE.png)


### 创建群聊效果图

![image](https://github.com/swansfought/EChat/blob/main/%E9%83%A8%E5%88%86%E6%88%AA%E5%9B%BE/%E5%88%9B%E5%BB%BA%E7%BE%A4%E8%81%8A%E6%95%88%E6%9E%9C%E5%9B%BE.png)


### 用户配置界面图

![image](https://github.com/swansfought/EChat/blob/main/%E9%83%A8%E5%88%86%E6%88%AA%E5%9B%BE/%E7%94%A8%E6%88%B7%E9%85%8D%E7%BD%AE%E7%95%8C%E9%9D%A2%E5%9B%BE.png)


>>>>>>> 135ed8d (2023年5月中旬)
