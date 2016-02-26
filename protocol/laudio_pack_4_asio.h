//////////////////////////////////////////////////////
// #pragma pack(push,1)
// struct laudio_header {
// 	  unsigned char m_start_flag;
// 	  unsigned char m_sort;  // LAUDIO_SORT_ASIO
// 	  unsigned char m_cmd;
// 	  unsigned int m_len;
//    unsigned char m_end_flag;
//    unsigned char m_crc8;
// };
// #pragma pack(pop)
// 
// when m_sort specify LAUDIO_SORT_ASIO
// follow commands avail used at m_cmd.
//////////////////////////////////////////////////////

#ifndef __LAUDIO_PACK_4_ASIO_H__
#define __LAUDIO_PACK_4_ASIO_H__

//
#define LAUDIO_PACKAGE_HEADER_INIT_4_ASIO( ptr_header, cmd, len_payload ) LAUDIO_PACKAGE_HEADER_INIT( ptr_header, LAUDIO_SORT_ASIO, cmd, len_payload )

/////////////////////////////////////////////////////////
// from AS -> PD 
// payload:
//   1: specify the channel of PCM data. 
//   1: specify the count of channels.
//   2: specify the size of PCM data(pcm_size).
//   pcm_size: specify the buffer of PCM data.
#define LAUDIO_ASIO_CMD_PCMDATA_BUF0              (0)
/////////////////////////////////////////////////////////
// from AS -> PD 
// payload:
//   1: specify the channel of PCM data. 
//   1: specify the count of channels.
//   2: specify the size of PCM data(pcm_size).
//   pcm_size: specify the buffer of PCM data.
#define LAUDIO_ASIO_CMD_PCMDATA_BUF1              (1)
/////////////////////////////////////////////////////////
// from PD -> AS
// payload:
//   39: sepcify the PDID of play device. as "{5053A32F-867B-4324-B460-923AE7F96CF0}"
#define PAYLOAD_LEN_4_CMD_WANTDATA_BUF0           (39)
#define LAUDIO_ASIO_CMD_WANTDATA_BUF0             (201)
/////////////////////////////////////////////////////////
// from PD -> AS
// payload:
//   39: sepcify the PDID of play device. as "{5053A32F-867B-4324-B460-923AE7F96CF0}"
#define PAYLOAD_LEN_4_CMD_WANTDATA_BUF1           (39)
#define LAUDIO_ASIO_CMD_WANTDATA_BUF1             (202)
/////////////////////////////////////////////////////////
// from AS -> PD
// payload:
#define LAUDIO_ASIO_CMD_NEW_INSTANCE              (203)
/////////////////////////////////////////////////////////
















#endif //__LAUDIO_PACK_4_ASIO_H__