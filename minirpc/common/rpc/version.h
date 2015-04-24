#ifndef _H_COMMON_RPC_VERSION_H_
#define _H_COMMON_RPC_VERSION_H_
#define RPC_SERVER_VERSION "1.0.0"
#define RPC_SERVER_NAME    "RpcServer"
#define FULL_VERSION  RPC_SERVER_NAME " " RPC_SERVER_VERSION
const char FullVersion[] = FULL_VERSION;
static char CompileInfo[]=  __TIME__ " " __DATE__;
#ifdef EN_HELP
static char HelpMsg[]="\
==================================================================================================\n\
  Usage: " RPC_SERVER_NAME " [options][-c "RPC_SERVER_NAME".ini]\n\
  Options:\n\
  -p port     listen port.\n\
  -p port     Basic server name displayed at agent node.\n\
  -c config   configuration file "NAME".ini.\n\
  -d daemon   Running daemon.\n\
  -l loglevel log level comments:\n\
              1. ERROR: error.\n\
              2. WARN:  warning.\n\
              3. NOTICE:  notice.\n\
              4. INFO:  important information.\n\
              5. VARS:  variable info.\n\
              6. DEBUG: debug info, used by programmer.\n\
              7. ALL:   all info, lowest level.\n\
              default: 2\n\
  -v          Version informations.\n\
  -h          This help message\n\
  e.g.:  " RPC_SERVER_NAME " -p 9090 -c "RPC_SERVER_NAME".ini -l 2 -d\n\
==================================================================================================\r\n";
#else
static char HelpMsg[]="\
==================================================================================================\n\
  使用方法: " RPC_SERVER_NAME " [参数选项][-c 配置文件]\n\
  参数选项描述:\n\
  -p 监听端口 程序启动时绑定的监听端口。\n\
  -s 服务名称 程序启动时提供的服务名称。\n\
  -c 配置文件。\n\
  -d 后台运行  守护进程方式运行。\n\
  -l 日志级别  日志级别见如下描述：\n\
               1. 错误日志；\n\
               2. 警告日志；\n\
               3. 通知日志；\n\
               4. 重要的提示性日志；\n\
               5. 打印关键变量的值；\n\
               6. 开发人员的调试日志；\n\
               7. 打开所有的日志。\n\
  -v           输出版本信息。\n\
  -h           输出运行帮助信息。\n\
  启动实例：   " RPC_SERVER_NAME "  -p 9090 -c "RPC_SERVER_NAME".ini -l 2 -d\n\
==================================================================================================\r\n";
#endif // EN_HELP
#endif // _H_COMMON_RPC_VERSION_H_