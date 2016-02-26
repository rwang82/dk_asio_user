#ifndef __LAUDIO_PACK_4_ACP_H__
#define __LAUDIO_PACK_4_ACP_H__
#include "laudio_pack_defs.h"

//

#define LAUDIO_PACKAGE_HEADER_INIT_4_ACP( ptr_header, cmd, len_payload ) LAUDIO_PACKAGE_HEADER_INIT( ptr_header, LAUDIO_SORT_ACP, cmd, len_payload )

//
///////////////////////////////////////////////////////
// ACP -> PD
// payload: None.
#define LAUDIO_ACP_CMD_FIND_PD                          (1)
///////////////////////////////////////////////////////
// PD -> ACP
// payload:
//   39: PDID as "{5053A32F-867B-4324-B460-923AE7F96CF0}"
//   1: bind status, 1 for bind, 0 for unbind.
//   39: ASID as "{5053A32F-867B-4324-B460-923AE7F96CF0}"
//   1: number of channels. (1-200)
//   1: sort as LAUDIO_SORT_ASIO
#define PAYLOAD_LEN_4_CMD_FIND_PD_ACK (39+1+39+1+1)
#define LAUDIO_ACP_CMD_FIND_PD_ACK                      (2)

/////////////////////////////////////////////////////////
//// ACP -> PD
//// payload:
////   39: ASID as "{5053A32F-867B-4324-B460-923AE7F96CF0}"
////   1: sort as LAUDIO_SORT_ASIO
////   39: PDID as "{5053A32F-867B-4324-B460-923AE7F96CF0}" for check.
//#define LAUDIO_ACP_CMD_BIND_PD2AS_REQ                   (3)
/////////////////////////////////////////////////////////
//// PD -> ACP
//// payload:
////   1: result of bind. (1 for success, other for failed.)
////   40: describe failed reason.
//#define LAUDIO_ACP_CMD_BIND_PD2AS_ACK                   (4)

///////////////////////////////////////////////////////
// AS -> ACP
// payload:
//   1: sort as LAUDIO_SORT_ASIO
//   4: PID. (process id, maybe some sort no pid.)
//   256: process name.
#define PAYLOAD_LEN_4_CMD_REGIST_AS2ACP_REQ (1 + 4 + 256)
#define LAUDIO_ACP_CMD_REGIST_AS2ACP_REQ                (5)
///////////////////////////////////////////////////////
// ACP -> AS
// payload:
//   1: result (1 for success, other for failed.)
//   39: ASID as "{5053A32F-867B-4324-B460-923AE7F96CF0}"
#define PAYLOAD_LEN_4_CMD_REGIST_AS2ACP_ACK (1 + 39)
#define LAUDIO_ACP_CMD_REGIST_AS2ACP_ACK                (6)
  
///////////////////////////////////////////////////////
// ACP -> AS
// payload:
//   17: ip address of play deivce (connect to)
//   2£º port of play device listenning.
#define PAYLOAD_LEN_4_CMD_CONNECT_PD_4_RENDER (17 + 2)
#define LAUDIO_ACP_CMD_CONNECT_PD_4_RENDER              (7)

///////////////////////////////////////////////////////
// AS -> ACP
// payload:
//   17: ip address of play device (connect to)
//   1: result of connect( 1 for success, other for failed.)
#define PAYLOAD_LEN_4_CMD_CONNECT_PD_4_RENDER_ACK (17 + 1)
#define LAUDIO_ACP_CMD_CONNECT_PD_4_RENDER_ACK          (8)

///////////////////////////////////////////////////////
// AS -> ACP
// payload:
//   17£º ip address of play device
#define PAYLOAD_LEN_4_CMD_AS_DISCONNECT_WITH_PD_4_RENDER (17)
#define LAUDIO_ACP_CMD_AS_DISCONNECT_WITH_PD_4_RENDER   (9)

///////////////////////////////////////////////////////
// ACP -> AS
// payload:
//   17: ip address of play deivce (connect to)
//   2£º port of play device listenning.
#define PAYLOAD_LEN_4_CMD_CONNECT_PD_4_CAPTURE (17 + 2)
#define LAUDIO_ACP_CMD_CONNECT_PD_4_CAPTURE             (10)

///////////////////////////////////////////////////////
// AS -> ACP
// payload:
//   17: ip address of play device (connect to)
//   1: result of connect( 1 for success, other for failed.)
#define PAYLOAD_LEN_4_CMD_CONNECT_PD_4_CAPTURE_ACK (17 + 1)
#define LAUDIO_ACP_CMD_CONNECT_PD_4_CAPTURE_ACK         (11)

///////////////////////////////////////////////////////
// AS -> ACP
// payload:
//   17£º ip address of play device
#define PAYLOAD_LEN_4_CMD_AS_DISCONNECT_WITH_PD_4_CAPTURE (17)
#define LAUDIO_ACP_CMD_AS_DISCONNECT_WITH_PD_4_CAPTURE  (12)








#endif //__LAUDIO_PACK_4_ACP_H__

