# c++ 创建线程
## 基本功能:
### 1. 主进程主动创建和结束线程;
### 2. 线程发生错误时不影响主进程(try catch(...));
### 3. 主进程误新增线程时, 线程数量不增加(利用互斥锁);
### 4. 线程在延时参数arg(单位:秒)内工作, 超时退出;
    

#
初始程序代码来源见下:    
版权声明：本文为CSDN博主「Jagen在路上」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/jiajun2001/article/details/12624923