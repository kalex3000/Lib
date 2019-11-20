/**
  ******************************************************************************
  * @file    gps.h
  * @author  w
  * @version V1.0
  * @date    05/18/2015
  * @brief   drivers for gps.
  *****************************************************************************
**/ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPS_H
#define __GPS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "drvpublic.h"

/* Exported Macros -----------------------------------------------------------*/		   						    
//GPS NMEA-0183协议重要参数结构体定义 
//卫星信息
#define MODE_GPS         0     //MODE=0:GPS
#define MODE_GPSASBAS    1     //MODE=1:GPS（+SBAS）  这种模式是增强型gps
#define MODE_Beidou      2     //MODE=2:Beidou
#define MODE_GPSABeidou  3     //MODE=3:GPS&Beidou

#define rate_5hz         200
#define rate_10hz        100

/* Exported Typedef ----------------------------------------------------------*/
__packed typedef struct  
{										    
 	u8 num;		//卫星编号
	u8 eledeg;	//卫星仰角
	u16 azideg;	//卫星方位角
	u8 sn;		//信噪比		   
}nmea_slmsg;  

//UTC时间信息
__packed typedef struct  
{										    
 	u16 year;	//年份
	u8 month;	//月份
	u8 date;	//日期
	u8 hour; 	//小时
	u8 min; 	//分钟
	u8 sec; 	//秒钟
}nmea_utc_time;   	   

//NMEA 0183 协议解析后数据存放结构体
__packed typedef struct  
{										    
 	u8 svnum;					//可见卫星数
	nmea_slmsg slmsg[12];		//最多12颗卫星
	nmea_utc_time utc;			//UTC时间
//	u32 latitude;				//纬度 分扩大100000倍,实际要除以100000
	u8 nshemi;					//北纬/南纬,N:北纬;S:南纬				  
//	u32 longitude;			    //经度 分扩大100000倍,实际要除以100000
	u8 ewhemi;					//东经/西经,E:东经;W:西经
	u8 Cewhemi;					//磁偏角东经/西经,E:东经;W:西经
	u8 gpssta;					//GPS状态:0,未定位;1,非差分定位;2,差分定位;6,正在估算.				  
 	u8 posslnum;				//用于定位的卫星数,0~12.
 	u8 possl[12];				//用于定位的卫星编号
	u8 fixmode;					//定位类型:1,没有定位;2,2D定位;3,3D定位
	u16 pdop;					//位置精度因子 0~500,对应实际值0~50.0
	u16 hdop;					//水平精度因子 0~500,对应实际值0~50.0
	u16 vdop;					//垂直精度因子 0~500,对应实际值0~50.0 

	double longitude;//纬度
	double latitude;//经度
	double altitude;			//海拔高度 
	double speed;				//地面速率,放大了1000倍,实际除以10.单位:0.001公里/小时
    double  cipianjiao;         //磁偏角	
}nmea_msg; 
	
//UBLOX NEO-8M 配置(清除,保存,加载等)结构体
__packed typedef struct  
{										    
 	u16 header;					//cfg header,固定为0X62B5(小端模式)
	u16 id;						//CFG CFG ID:0X0906 (小端模式)
	u16 dlength;				//数据长度 12/13
	u32 clearmask;				//子区域清除掩码(1有效)
	u32 savemask;				//子区域保存掩码
	u32 loadmask;				//子区域加载掩码
	u8  devicemask; 		  	//目标器件选择掩码	b0:BK RAM;b1:FLASH;b2,EEPROM;b4,SPI FLASH
	u8  cka;		 			//校验CK_A 							 	 
	u8  ckb;			 		//校验CK_B							 	 
}_ublox_cfg_cfg; 

//UBLOX NEO-8M 卫星选择结构体
__packed typedef struct  
{										    
 	u16 header;					//cfg header,固定为0X62B5(小端模式)
	u16 id;						//CFG CFG ID:0X0906 (小端模式)
	u16 dlength;				//数据长度
	u8 msgver;				
	u8 numTrkChHw;				
	u8 numTrkChUse;				
	u8  numConfigBlocks; 
	
	u8  GgnssId;	     //gps配置信息
    u8 GresTrkCh;
    u8 GmaxTrkCh;
    u8 Greserved1;
    u8 Gflag4;	         //低位，使能位
	u8 Gflag3;	
	u8 Gflag2;	
	u8 Gflag1;	
	
    u8  SgnssId;	     //SBAS配置信息
    u8 SresTrkCh;
    u8 SmaxTrkCh;
    u8 Sreserved1;
    u8 Sflag4;	         //低位，使能位
	u8 Sflag3;	
	u8 Sflag2;	
	u8 Sflag1;
	
	u8  BgnssId;         //北斗配置信息	
    u8 BresTrkCh;
    u8 BmaxTrkCh;
    u8 Breserved1;
    u8 Bflag4;	         //低位，使能位
	u8 Bflag3;	
	u8 Bflag2;	
	u8 Bflag1;	
	
	u8  QgnssId;         //Qzss配置信息	
    u8 QresTrkCh;
    u8 QmaxTrkCh;
    u8 Qreserved1;
    u8 Qflag4;	         //低位，使能位
	u8 Qflag3;	
	u8 Qflag2;	
	u8 Qflag1;	
	
	u8  LgnssId;	     //Glonass配置信息
    u8 LresTrkCh;
    u8 LmaxTrkCh;
    u8 Lreserved1;
    u8 Lflag4;	         //低位，使能位
	u8 Lflag3;	
	u8 Lflag2;	
	u8 Lflag1;			
	u8  cka;		 	 //校验CK_A 		
	u8  ckb;			 //校验CK_B							 	 
}_ublox_cfg_wx; 

//UBLOX NEO-8M 消息设置结构体
__packed typedef struct  
{										    
 	u16 header;					//cfg header,固定为0X62B5(小端模式)
	u16 id;						//CFG MSG ID:0X0106 (小端模式)
	u16 dlength;				//数据长度 8
	u8  msgclass;				//消息类型(F0 代表NMEA消息格式)
	u8  msgid;					//消息 ID 
								//00,GPGGA;01,GPGLL;02,GPGSA;
								//03,GPGSV;04,GPRMC;05,GPVTG;
								//06,GPGRS;07,GPGST;08,GPZDA;
								//09,GPGBS;0A,GPDTM;0D,GPGNS;

    u8  iicset;					//IIC消输出设置    0,关闭;1,使能.
	u8  uart1set;				//UART1输出设置	   0,关闭;1,使能.
	u8  uart2set;				//UART2输出设置	   0,关闭;1,使能.
	u8  usbset;					//USB输出设置	   0,关闭;1,使能.
	u8  spiset;					//SPI输出设置	   0,关闭;1,使能.
	u8  ncset;					//未知输出设置	   默认为1即可.
 	u8  cka;			 		//校验CK_A 							 	 
	u8  ckb;			    	//校验CK_B							 	 
}_ublox_cfg_msg; 

//UBLOX NEO-8M UART端口设置结构体
__packed typedef struct  
{										    
 	u16 header;					//cfg header,固定为0X62B5(小端模式)
	u16 id;						//CFG PRT ID:0X0006 (小端模式)
	u16 dlength;				//数据长度 20
	u8  portid;					//端口号,0=IIC;1=UART1;2=UART2;3=USB;4=SPI;
	u8  reserved;				//保留,设置为0
	u16 txready;				//TX Ready引脚设置,默认为0
	u32 mode;					//串口工作模式设置,奇偶校验,停止位,字节长度等的设置.
 	u32 baudrate;				//波特率设置
 	u16 inprotomask;		 	//输入协议激活屏蔽位  默认设置为0X07 0X00即可.
 	u16 outprotomask;		 	//输出协议激活屏蔽位  默认设置为0X07 0X00即可.
 	u16 reserved4; 				//保留,设置为0
 	u16 reserved5; 				//保留,设置为0 
 	u8  cka;			 		//校验CK_A 							 	 
	u8  ckb;			    	//校验CK_B							 	 
}_ublox_cfg_prt; 

//UBLOX NEO-8M 时钟脉冲配置结构体
__packed typedef struct  
{										    
 	u16 header;					//cfg header,固定为0X62B5(小端模式)
	u16 id;						//CFG TP ID:0X0706 (小端模式)
	u16 dlength;				//数据长度
	u32 interval;				//时钟脉冲间隔,单位为us
	u32 length;				 	//脉冲宽度,单位为us
	signed char status;			//时钟脉冲配置:1,高电平有效;0,关闭;-1,低电平有效.			  
	u8 timeref;			   		//参考时间:0,UTC时间;1,GPS时间;2,当地时间.
	u8 flags;					//时间脉冲设置标志
	u8 reserved;				//保留			  
 	signed short antdelay;	 	//天线延时
 	signed short rfdelay;		//RF延时
	signed int userdelay; 	 	//用户延时	
	u8 cka;						//校验CK_A 							 	 
	u8 ckb;						//校验CK_B							 	 
}_ublox_cfg_tp; 

//UBLOX NEO-8M 刷新速率配置结构体
__packed typedef struct  
{										    
 	u16 header;					//cfg header,固定为0X62B5(小端模式)
	u16 id;						//CFG RATE ID:0X0806 (小端模式)
	u16 dlength;				//数据长度
	u16 measrate;				//测量时间间隔，单位为ms，最少不能小于200ms（5Hz）
	u16 navrate;				//导航速率（周期），固定为1
	u16 timeref;				//参考时间：0=UTC Time；1=GPS Time；
 	u8  cka;					//校验CK_A 							 	 
	u8  ckb;					//校验CK_B							 	 
}_ublox_cfg_rate; 

typedef struct  taggpsdata
{
	double longitude;           //纬度
	double latitude;            //经度
	double altitude;			//海拔高度
	double speed;				//地面速率,放大了1000倍,实际除以10.单位:0.001公里/小时
    double  cipianjiao;         //磁偏角
	u8 posslnum;                //卫星颗数
}GPSDATA_ST, *ptGPSDATA_ST;

/* Exported Variables --------------------------------------------------------*/
extern GPSDATA_ST stgpsdata;             // 实时获取的gps数据

extern u8 ack_data;

/* Exported Functions --------------------------------------------------------*/
int NMEA_Str2num(u8 *buf,u8*dx);
void GPS_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GPGGA_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GPGSA_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GPGSA_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GPRMC_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GPVTG_Analysis(nmea_msg *gpsx,u8 *buf);
void satellites_choose(u8 MODE);
void MSG_choose(void);
u8 posslnum_check(void);
void Rate_chose(u8 rate);
u8 Ublox_Cfg_Cfg_Save(void);
u8 Ublox_Cfg_wx(u8 g,u8 s,u8 b);
u8 Ublox_Cfg_Msg(u8 msgid,u8 uart1set);
u8 Ublox_Cfg_Prt(u32 baudrate);
u8 Ublox_Cfg_Tp(u32 interval,u32 length,signed char status);
u8 Ublox_Cfg_Rate(u16 measrate,u8 reftime);

void gps_datacheck(void);
void gps_init(void);
void change(u8 *buf,u16 m);

#ifdef __cplusplus
}
#endif

#endif /* __GPS_H */
/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/