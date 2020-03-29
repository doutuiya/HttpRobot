# HttpRobot

### 背景
* 只适配 2.8.112版本wx
* 基于https://github.com/codeByDog/pcWechat.git 改造，增加了http接口,大家可以自由发挥，做成web版的robot

### 使用方法
* 点击Debug/robot.exe，启动mfc的同时会启动httpserver,访问 http://localhost:7999/ 可测试相关功能
* 本项目仅限学习研究使用，请务用于违法项目，后果自负！！！

### 问题解决
* JSON序列化使用jsoncpp组件，需要使用vcpkg包管理器安装，网速如果慢可能需要翻墙
* Release 编译报错：重定义或不同的链接，解决方案：项目属性-C/C+->预处理器->预处理器定义->增加如下定义WIN32_LEAN_AND_MEAN
