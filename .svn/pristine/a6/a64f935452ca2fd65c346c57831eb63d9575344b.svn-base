#ifndef _APP_FACE_H_
#define _APP_FACE_H_

#ifdef __cplusplus
 extern "C" {
#endif
/* 内部头文件 */
#include "..\Server\Face.h"
/* 标准头文件 */
#include "stdint.h"
/*---------------------------------------验证人脸--------------------------------*/
typedef enum
{
	FACE_VERIFY_IDLE,        //IDLE
	FACE_VERIFY_SUCCESS,     //成功
	FACE_VERIFY_NOFACE,      //未检测到人脸播报及下电
	FACE_VERIFY_FAR,         //检测持续过远播报
	FACE_VERIFY_NEAR, 		 //检测持续过近播报
	FACE_VERIFY_OCCLUSION,   //检测持续遮挡播报
	FACE_VERIFY_MODULE_FAIL, //模组验证失败
	FACE_VERIFY_TIME_FAIL,   //锁具时效失败
	FACE_VERIFY_EE_FAIL,     //锁具ID失败
	FACE_VERIFY_ADMIN_FAIL,  //验证管理员失败
	FACE_VERIFY_MODULE_ERR   //模组故障
}FACE_VERIFY_RESULT_E; //识别结果返回枚举

typedef enum
{
	FACE_VERIFY_CHECK_DEMO,
	FACE_VERIFY_CHECK,
	FACE_VERIFY_POWERDOWN
}FACE_VERIFY_PRO_E; //识别过程
/*---------------------------------------注册人脸--------------------------------*/
typedef enum
{
	FACE_ADD_FRONT,
	FACE_WAIT_ADD_FRONT,
	FACE_ADD_UP,
	FACE_WAIT_ADD_UP,
	FACE_ADD_DOWN,
	FACE_WAIT_ADD_DOWN,
	FACE_ADD_RIGHT,
	FACE_WAIT_ADD_RIGHT,
	FACE_ADD_LEFT,
	FACE_WAIT_ADD_LEFT,
	FACE_ADD_SUCCESSFUL,
	FACE_ADD_ERROR,
	FACE_ADD_POWERDOWN,
	FACE_WAIT_POWERDOWN,
	FACE_ADD_OVER
}FACE_ADD_USER_E; //注册过程

//-----------------------------EE存储结构---------------------------------
struct Face_tm{
	uint16_t year;
	uint8_t  mon;
	uint8_t  day;
	uint8_t  hour;
	uint8_t  min;
	uint8_t  sec;
};

typedef struct {
	uint8_t   FaceVaild;	//有效
	uint8_t   FaceLim;  	// 主人,还是客人
	uint16_t  FacePageId;   //存储ID
	uint16_t  FaceId;   	//人脸序号
	struct {
		uint8_t  fig;	
		RTCType start; //起始时间
		RTCType stop;  //结束时间
		uint8_t  wday;		//周周时效 BIT1 -BIT 7表示周一到周日
	}tm_vaild;			//时效
	uint16_t checksum;	   	
}FACE_ATTRIBUTE;		

extern uint8_t FaceTimer;









extern FACE_ATTRIBUTE FaceAttribute; //人脸属性 EE存储结构

uint8_t FaceEnrollPro(uint8_t FaceLim);//注册流程
uint8_t  FaceGetVeifyState(uint8_t AdminEn,uint16_t *Pageid, uint8_t *unlockStatus);//识别流程
void FaceEepromEmpty (void); //清空
uint8_t FaceDeleteAppUser(uint8_t PageId); //删除指定用户
void App_Face_SleepInit(void);
void FaceDeleteGuestUser(void); //删除普通用户
extern void APP_FACE_Tim1s (void);
extern void FaceWrite(FACE_ATTRIBUTE FACE_MEG, uint16_t PageId);
extern void FaceRead(FACE_ATTRIBUTE FACE_MEG, uint16_t PageId);
#ifdef __cplusplus
}
#endif

#endif
//.end of the file.
