#! /bin/sh  
  
proc_name="udpserver"                               # 进程名  
file_name="~/workspace/udpserver/Debug/restart.log"             # 日志文件  
pid=0  
  
proc_num()                                             # 计算进程数  
{  
    num=`ps -ef | grep $proc_name | grep -v grep | wc -l`  
    return $num  
}  
  
proc_id()                                               # 进程号  
{  
    pid=`ps -ef | grep $proc_name | grep -v grep | awk '{print $2}'`  
}  
  
proc_num  
number=$?  
if [ $number -eq 0 ]                                    # 判断进程是否存在  
then   
    cd   ~/workspace/udpserver/Debug
        ./udpserver   &                                       # 重启进程的命令  
    proc_id                                               # 获取新进程号  
    echo `udpserver  restart ` >> $file_name   
    echo ${pid}, `date` >>  $file_name                    # 将新进程号和重启时间记录  
  
fi  
