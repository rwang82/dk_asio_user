#ifndef __DK_BIT_HELPER_H__
#define __DK_BIT_HELPER_H__

#define DKBIT_HITTEST(val, bit) ( ( val & bit ) == bit )
#define DKBIT_ADD(val, bit) do { val |= bit; } while(0)
#define DKBIT_CLEAR(val, bit) do { val &= ~bit; } while( 0 )





#endif //__DK_BIT_HELPER_H__