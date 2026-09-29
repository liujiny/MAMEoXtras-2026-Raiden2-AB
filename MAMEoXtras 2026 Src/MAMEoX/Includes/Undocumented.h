// RXDK owns the NT kernel ABI. Do not redeclare its internal structures.
#ifndef __XBOX_INTERNAL_H__
#define __XBOX_INTERNAL_H__
#include <xboxkrnl/xboxkrnl.h>
#include <winsockx.h>

// Thanks and credit go to Team Evox
typedef struct   
{   
	DWORD	Data_00;            // Check Block Start   
	DWORD	Data_04;   
	DWORD	Data_08;   
	DWORD	Data_0c;   
	DWORD	Data_10;            // Check Block End   
  
	DWORD	V1_IP;              // 0x14   
	DWORD	V1_Subnetmask;      // 0x18   
	DWORD	V1_Defaultgateway;  // 0x1c   
	DWORD	V1_DNS1;            // 0x20   
	DWORD	V1_DNS2;            // 0x24   

	DWORD	Data_28;            // Check Block Start   
	DWORD	Data_2c;   
	DWORD	Data_30;   
	DWORD	Data_34;   
	DWORD	Data_38;            // Check Block End   

	DWORD	V2_Tag;             // V2 Tag "XBV2"   
 
	DWORD	Flag;				// 0x40   
	DWORD	Data_44;   

	DWORD	V2_IP;              // 0x48   
	DWORD	V2_Subnetmask;      // 0x4c   
	DWORD	V2_Defaultgateway;  // 0x50   
	DWORD	V2_DNS1;            // 0x54   
	DWORD	V2_DNS2;            // 0x58   

	DWORD   Data_xx[0x200-0x5c];

} TXNetConfigParams,*PTXNetConfigParams;

// These two legacy XNet entry points are provided by RXDK libxnet.lib.
#ifdef __cplusplus
extern "C" {
#endif
INT WINAPI XNetLoadConfigParams(LPBYTE);
INT WINAPI XNetSaveConfigParams(LPBYTE);
#ifdef __cplusplus
}
#endif
#endif // __XBOX_INTERNAL_H__
