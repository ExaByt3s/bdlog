策略：

1. 日志查看器开始监控时，会检查bdxlog.ini中的管道输出设备是否打开。
   >> 如果没有打开，则日志查看器自动将其打开。这种情况下，当日志查看器停止监控时，会再自动的把管道输出设备关闭。

2. 进程ID和线程ID是不稳定的过滤条件。在保存过滤规则的时候，这两类过滤规则不会被保存。



实现：

1. 日志提供者 ILogProvider 和 日志监听者 ILogListener
   日志提供者把
   
   

TODO

1. 存储界面布局(cfg.ui.placement)
2. 配置模块使用复杂
