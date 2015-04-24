#ifndef _LOCAL_DEF_H_
#define _LOCAL_DEF_H_
/*
* return code defination.
*/
#define RET_OK     0
#define RET_ERROR -1
#define RET_BUSY   2

#define MAX_SIZE_1K    1024
#define MAX_SIZE_2K    2048
#define MAX_SIZE_64K   65546
#define MAX_SIZE_1M    1048576

/*
* udp protocol infos.
*/
#define PKG_HEADFLAG_LEN  4
#define PKG_CMDLEN_LEN    4
#define PKG_CHKSUM_LEN    8
#define PKG_MAX_LEN       1024
#define SERVICE_ITEM_LEN  250
#define PKG_HEADER_LEN    8

// define for agent service.address pairs.
#define PKG_UDP_HEAD_FLAG "'FY'"
#define PKG_TCP_HEAD_FLAG "'YF'"
#define PKG_CMD_REGISTER  "REGI"
#define PKG_CMD_CANCEL    "CANC"
#define PKG_CMD_REQUEST   "REQU"
#define PKG_CMD_HEARTBEAT "BEAT"
#define PKG_CMD_ACK       " ACK"
#define PKG_GETALLSERVICE "GETALLSERVICE"
#define PKG_GETSERVICE    "GETSERVICE"
#define SERVICE_NOT_FOUND "4 {Service not find.}"

#endif // _LOCAL_DEF_H_
