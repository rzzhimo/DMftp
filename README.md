# dameng database on centos of docker and ftp implemented with C

## the origin code
[reference of the code](https://www.cnblogs.com/wanghao-boke/p/11930651.html)  
Thanks to the code author
## build a image of dmsql on docker
docker 镜像路径：[dmlinux](https://hub.docker.com/repository/docker/rzzhimo/dmlinux)  
odbc 接口：[dm interface](https://eco.dameng.com/docs/zh-cn/app-dev/c_c++_odbc.html#安装-DM-数据库)
## implement a ftp with C
## ftp usage
Usage： Server端：./ftserver port  
Client端：./ftclient host_ip:host_port  
ls <dir_path> #服务器目录下的内容  
upload [-f] <local_file> <remote_file> #上传文件， -f参数表示强制覆盖  
download <remote_file> <local_file> #下载文件  
delete <remote_file> #删除服务器上的文件  
history [-n <number>]#操作历史查看 -n记录行数  
exit #退出  
