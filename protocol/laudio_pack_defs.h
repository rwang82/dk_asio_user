//////////////////////////////////////////////////////
//
//   |-----------------|---------------------------------------|
//   |  laudio_header  |             payload                   |
//   |-----------------|---------------------------------------|
//   
//   |<------------------ m_len ------------------------------>|
//   
//////////////////////////////////////////////////////
#ifndef __LAUDIO_PACK_DEFS_H__
#define __LAUDIO_PACK_DEFS_H__

#define LAUDIO_SORT_ACP (0x00)
#define LAUDIO_SORT_ASIO (0x01)

#pragma pack(push,1)
struct laudio_header {
	unsigned char m_start_flag; // LAUDIO_START_FLAG
	unsigned char m_sort;       // specify the sort of output. as LAUDIO_SORT_ASIO for asio.
	unsigned char m_cmd;        // specify the cmd of output protocol. more detail see laudio_pack_4_xxx (as laudio_pack_4_asio)
	unsigned int m_len;         // specify the whole package size.
	unsigned char m_end_flag;   // LAUDIO_END_FLAG
	unsigned char m_crc8;       // header content check code. use LAUDIO_CALC_CRC8 to calc the code.
};
#pragma pack(pop)

#define LAUDIO_START_FLAG (0xAA)
#define LAUDIO_END_FLAG (0xDD)
#define LAUDIO_PACKHEADER_SIZE (sizeof(laudio_header))
#define LAUDIO_PAYLOAD_SIZE_MAX (65536)
#define LAUDIO_PACKAGE_LEN_MAX (LAUDIO_PAYLOAD_SIZE_MAX+LAUDIO_PACKHEADER_SIZE)
#define LAUDIO_CALC_CRC8( ptr_header ) ( ((laudio_header*)(ptr_header))->m_sort\
	+ ((laudio_header*)(ptr_header))->m_cmd\
	+ ((laudio_header*)(ptr_header))->m_len&0xFF\
	+ (((laudio_header*)(ptr_header))->m_len>>8)&0xFF\
	+ (((laudio_header*)(ptr_header))->m_len>>16)&0xFF\
	+ (((laudio_header*)(ptr_header))->m_len>>24)&0xFF)
#define LAUDIO_PACKAGE_HEADER_INIT( ptr_header, sort, cmd, len_payload ) \
	do { \
        ((laudio_header*)(ptr_header))->m_start_flag = LAUDIO_START_FLAG;\
		((laudio_header*)(ptr_header))->m_sort = sort;\
		((laudio_header*)(ptr_header))->m_cmd = cmd;\
		((laudio_header*)(ptr_header))->m_len = len_payload + LAUDIO_PACKHEADER_SIZE;\
		((laudio_header*)(ptr_header))->m_end_flag = LAUDIO_END_FLAG;\
		((laudio_header*)(ptr_header))->m_crc8 = LAUDIO_CALC_CRC8(ptr_header);\
	} while ( 0 )
#define IS_VALID_LAUDIO_PACKAGE ( ptr_header ) \
	( ((laudio_header*)(ptr_header))->m_crc8 == LAUDIO_CALC_CRC8(ptr_header) \
	&& ((laudio_header*)(ptr_header))->m_len <= LAUDIO_PACKAGE_LEN_MAX )





#endif //__LAUDIO_PACK_DEFS_H__