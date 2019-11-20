/**
  ******************************************************************************
  * @file    gps.c
  * @author  w
  * @version V1.0
  * @Library
  * @date    05/18/2015
  * @brief   drivers for gps
  *****************************************************************************
**/

/* Includes ------------------------------------------------------------------*/
#include "gps.h"

#include "stdio.h"	 
#include "stdarg.h"	 
#include "string.h"	 
#include "math.h"
#include "led.h"
#include "usart.h"

/* Global Variables ----------------------------------------------------------*/
GPSDATA_ST stgpsdata;             // Real-time gps data

u8 Choose_mode;
nmea_msg gpsx;        //GPS信息
extern u8 num;
u16 jing;

/* Private Macros ------------------------------------------------------------*/

/* Private Variables ---------------------------------------------------------*/

/* Private Functions ---------------------------------------------------------*/

/* Public Functions ----------------------------------------------------------*/

//Get the location of the cx comma from buf
//Return value: 0~0xFE, representing the offset of the location of the comma.
//       0xFF,Represents the absence of the cx comma		
u8 const ASCII[10]={48,49,50,51,52,53,54,55,56,57};
void change(u8 *buf,u16 m)
{
	u8 wei[5],i;
	wei[0]=m/10000;
	wei[1]=m%10000/1000;
	wei[2]=m%1000/100;
	wei[3]=m%100/10;
	wei[4]=m%10;
	
	for(i=0;i<=4;i++)
	{
		buf[i+1]=ASCII[wei[i]];
//		printf("%d\r\n",buf[i+1]);
	}
	buf[0]=5;
}

u8 NMEA_Comma_Pos(u8 *buf,u8 cx)
{	 		    
	u8 *p=buf;
	while(cx)
	{		 
		if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//Encounter '*' or illegal characters, there is no cx comma
		if(*buf==',')cx--;
		buf++;
	}
	return buf-p;	 
}

//m^n function
//Return value: m^n power.
u32 NMEA_Pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}

//str converts to a number, ending with ',' or '*'
//buf:Digital storage area
//dx:The number of decimal places is returned to the calling function
//Return value: converted value
int NMEA_Str2num(u8 *buf,u8*dx)
{
	u8 *p=buf;
	u32 ires=0,fres=0;
	u8 ilen=0,flen=0,i;
	u8 mask=0;
	int res;
 
	while(1) //得到整数和小数的长度
	{
		if(*p=='-'){mask|=0X02;p++;}//Is negative
		if(*p==','||(*p=='*'))break;//Encountered the end
		if(*p=='.'){mask|=0X01;p++;}//Encountered a decimal point
		else if(*p>'9'||(*p<'0'))	//Have illegal characters
		{	
			ilen=0;
			flen=0;
			break;
		}	
		if(mask&0X01)flen++;
		else ilen++;
		p++;
	}
	if(mask&0X02)buf++;	//Remove the negative sign
	for(i=0;i<ilen;i++)	//Get the integer part of the data
	{  
		ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
    //if(flen>5)flen=5;	//Take up to 5 decimal places
	*dx=flen;	 		//Decimal point
	for(i=0;i<flen;i++)	//Get fractional data
	{  
		fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
	} 
	res=ires*NMEA_Pow(10,flen)+fres;
	if(mask&0X02)res=-res;		   
	return res;
}

//Analyze GPGSV information
//gpsx:nmea Information structure
//buf:Received GPS data buffer first address
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p,*p1,dx;
	u8 len,i,j,slx=0;
	u8 posx;   	 

	p=buf;
	p1=(u8*)strstr((const char *)p,"$GPGSV");
	len=p1[7]-'0';								//Get the number of GPGSV
	posx=NMEA_Comma_Pos(p1,3); 					//Get the total number of visible satellites
	if(posx!=0XFF)
        gpsx->svnum=NMEA_Str2num(p1+posx,&dx);

	for(i=0;i<len;i++)
	{	 
		p1=(u8*)strstr((const char *)p,"$GPGSV");  
		for(j=0;j<4;j++)
		{	  
			posx=NMEA_Comma_Pos(p1,4+j*4);
            
			if(posx!=0XFF)
                gpsx->slmsg[slx].num=NMEA_Str2num(p1+posx,&dx);	//Get the satellite number
			else
                break; 
			
            posx=NMEA_Comma_Pos(p1,5+j*4);
			
            if(posx!=0XFF)
                gpsx->slmsg[slx].eledeg=NMEA_Str2num(p1+posx,&dx);//Get the satellite elevation angle
			else
                break;

			posx=NMEA_Comma_Pos(p1,6+j*4);

			if(posx!=0XFF)
                gpsx->slmsg[slx].azideg=NMEA_Str2num(p1+posx,&dx);//Get satellite azimuth
			else
                break; 

			posx=NMEA_Comma_Pos(p1,7+j*4);

			if(posx!=0XFF)
                gpsx->slmsg[slx].sn=NMEA_Str2num(p1+posx,&dx);	//Get satellite signal to noise ratio
			else
                break;

			slx++;	   
		}   
 		p=p1+1;//Switch to the next GPGSV message
	}   
}

//Analyze GPGGA information
//gpsx:nmea Information structure
//buf:Received GPS data buffer first address
void NMEA_GPGGA_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p1,dx;			 
	u8 posx;

    if(Choose_mode==0|Choose_mode==1)
        p1=(u8*)strstr((const char *)buf,"$GPGGA");

	if(Choose_mode==2)	
        p1=(u8*)strstr((const char *)buf,"$GBGGA");
    
    if(Choose_mode==3)	
        p1=(u8*)strstr((const char *)buf,"$GNGGA");
	
    posx=NMEA_Comma_Pos(p1,6);								//Get GPS status
	
    if(posx!=0XFF)
        gpsx->gpssta=NMEA_Str2num(p1+posx,&dx);	
    
    posx=NMEA_Comma_Pos(p1,7);								//Get the number of satellites used for positioning
	
    if(posx!=0XFF)
        gpsx->posslnum=NMEA_Str2num(p1+posx,&dx); 
    
    posx=NMEA_Comma_Pos(p1,9);								//Get altitude
	
    if(posx!=0XFF)
        gpsx->altitude=NMEA_Str2num(p1+posx,&dx)/10;	
}

//Analyze GPGSA information
//gpsx:nmea Information structure
//buf:Received GPS data buffer first address
void NMEA_GPGSA_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p1,dx;			 
	u8 posx; 
	u8 i;   
	
    p1=(u8*)strstr((const char *)buf,"$GNGSA");
	
    posx=NMEA_Comma_Pos(p1,2);								//Get targeting type
	
    if(posx!=0XFF)
        gpsx->fixmode=NMEA_Str2num(p1+posx,&dx);	
	
    for(i=0;i<12;i++)										//Get the positioning satellite number
	{
		posx=NMEA_Comma_Pos(p1,3+i);					 
		if(posx!=0XFF)gpsx->possl[i]=NMEA_Str2num(p1+posx,&dx);
		else break; 
	}				  
	
    posx=NMEA_Comma_Pos(p1,15);								//Get PDOP positional accuracy factor
	
    if(posx!=0XFF)
        gpsx->pdop=NMEA_Str2num(p1+posx,&dx);  
	
    posx=NMEA_Comma_Pos(p1,16);								//Get HDOP positional accuracy factor
	
    if(posx!=0XFF)
        gpsx->hdop=NMEA_Str2num(p1+posx,&dx);  
	
    posx=NMEA_Comma_Pos(p1,17);								//Get VDOP positional accuracy factor
	
    if(posx!=0XFF)
        gpsx->vdop=NMEA_Str2num(p1+posx,&dx);  
}

//Analyze GPRMC information
//gpsx:nmea Information structure
//buf:Received GPS data buffer first address
void NMEA_GPRMC_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p1,dx;			 
	u8 posx;     
	u32 temp;	
	float rs;


    if(Choose_mode==0|Choose_mode==1)	
        p1=(u8*)strstr((const char *)buf,"$GPRMC");
	
    if(Choose_mode==2)	
        p1=(u8*)strstr((const char *)buf,"$GBRMC");
    
    if(Choose_mode==3)	
        p1=(u8*)strstr((const char *)buf,"$GNRMC");  
        //p1=(u8*)strstr((const char *)buf,"GNRMC");//"$GPRMC",There are often cases where it is separated from GPRMC, so only GPRMC is judged..
	
    posx=NMEA_Comma_Pos(p1,3);								//Get latitude
    
    if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		   //ex:30348257 	 
		gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//Get °  ex:31
		rs=temp%NMEA_Pow(10,dx+2);				//get'	  ex:5560926
		gpsx->latitude=(gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60)/100000.0;//Convert to  .  ° 
	}	
	posx=NMEA_Comma_Pos(p1,4);								//South latitude or north latitude 
	
    if(posx!=0XFF)
        gpsx->nshemi=*(p1+posx);
	
	
 	posx=NMEA_Comma_Pos(p1,5);								//Obtain longitude
	
    if(posx!=0XFF)
	{												  
		temp=NMEA_Str2num(p1+posx,&dx);	

		gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//得到°
		
		rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
		gpsx->longitude=(gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60)/100000.0;//转换为  .  ° 
	}
	
    posx=NMEA_Comma_Pos(p1,6);								//东经还是西经
	
    if(posx!=0XFF)
    gpsx->ewhemi=*(p1+posx);		 
  

	
	posx=NMEA_Comma_Pos(p1,10);								//得到磁偏角
	
    if(posx!=0XFF)
	{
		gpsx->cipianjiao=NMEA_Str2num(p1+posx,&dx);		 	 
	}	
	
    posx=NMEA_Comma_Pos(p1,11);
	
    if(posx!=0XFF)
        gpsx->Cewhemi=*(p1+posx);	
    //printf("磁偏角:%f %1c\r\n",gpsx->cipianjiao,gpsx->Cewhemi);	//得到磁偏角字符串	
	
	posx=NMEA_Comma_Pos(p1,7);	           	//得到速率
	
    if(posx!=0XFF)
	temp=NMEA_Str2num(p1+posx,&dx);
	gpsx->speed=temp/1000.0;
    //printf("速度:%f knot\r\n",gpsx->speed);
}

//分析GPVTG信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPVTG_Analysis(nmea_msg *gpsx,u8 *buf)
{
	u8 *p1,dx;
	u8 posx;
	p1=(u8*)strstr((const char *)buf,"$GNVTG");					 
	posx=NMEA_Comma_Pos(p1,7);								//得到地面速率
	if(posx!=0XFF)
	{
		gpsx->speed=NMEA_Str2num(p1+posx,&dx);
		if(dx<3)gpsx->speed*=NMEA_Pow(10,3-dx);	 	 		//确保扩大1000倍
	}
}

//提取NMEA-0183信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void GPS_Analysis(nmea_msg *gpsx,u8 *buf)
{
    //NMEA_GPGSV_Analysis(gpsx,buf);	//GPGSV解析
	NMEA_GPGGA_Analysis(gpsx,buf);	    //GPGGA解析 	
    //NMEA_GPGSA_Analysis(gpsx,buf);	//GPGSA解析
	NMEA_GPRMC_Analysis(gpsx,buf);	    //GPRMC解析
    //NMEA_GPVTG_Analysis(gpsx,buf);	//GPVTG解析
}

//GPS校验和计算
//buf:数据缓存区首地址
//len:数据长度
//cka,ckb:两个校验结果.
void Ublox_CheckSum(u8 *buf,u16 len,u8* cka,u8*ckb)
{
	u16 i;
	*cka=0;*ckb=0;
	for(i=0;i<len;i++)
	{
		*cka=*cka+buf[i];
		*ckb=*ckb+*cka;
	}
}

/////////////////////////////////////////UBLOX 配置代码/////////////////////////////////////
//检查CFG配置执行情况
//返回值:0,ACK成功
//       1,接收超时错误
//       2,没有找到同步字符
//       3,接收到NACK应答
u8 Ublox_Cfg_Ack_Check(void)
{
	u16 len=0,i;
	u8 rval=0;
	while((USART1_RX_STA&0X8000)==0 && len<100)//等待接收到应答   
	{
		len++;
		delay_ms(5);
	}

    //超时错误.
	if(len<250)
	{
		len=USART1_RX_STA&0X7FFF;	//此次接收到的数据长度 
		for(i=0;i<len;i++)if(USART1_RX_BUF[i]==0XB5)break;//查找同步字符 0XB5
		if(i==len)rval=2;						//没有找到同步字符
		else if(USART1_RX_BUF[i+3]==0X00)rval=3;//接收到NACK应答
		else rval=0;	   						//接收到ACK应答
	}else rval=1;								//接收超时错误
    USART1_RX_STA=0;							//清除接收
	return rval;  
}

//配置保存
//将当前配置保存在外部EEPROM里面
//返回值:0,执行成功;1,执行失败.
u8 Ublox_Cfg_Cfg_Save(void)
{
	u8 i;
	_ublox_cfg_cfg *cfg_cfg=(_ublox_cfg_cfg *)USART1_TX_BUF;
	cfg_cfg->header=0X62B5;		//cfg header
	cfg_cfg->id=0X0906;			//cfg cfg id
	cfg_cfg->dlength=13;		//数据区长度为13个字节.		 
	cfg_cfg->clearmask=0;		//清除掩码为0
	cfg_cfg->savemask=0XFFFF; 	//保存掩码为0XFFFF
	cfg_cfg->loadmask=0; 		//加载掩码为0 
	cfg_cfg->devicemask=4; 		//保存在EEPROM里面		 
	Ublox_CheckSum((u8*)(&cfg_cfg->id),sizeof(_ublox_cfg_cfg)-4,&cfg_cfg->cka,&cfg_cfg->ckb);
	while(DMA1_Channel4->CNDTR!=0);	//等待通道7传输完成   
	UART_DMA_Enable(DMA1_Channel4,sizeof(_ublox_cfg_cfg));	//通过dma发送出去
	for(i=0;i<6;i++)if(Ublox_Cfg_Ack_Check()==0)break;		//EEPROM写入需要比较久时间,所以连续判断多次
	return i==6?1:0;
}

//配置NMEA输出信息格式
//msgid:要操作的NMEA消息条目,具体见下面的参数表
//      00,GPGGA;01,GPGLL;02,GPGSA;
//		03,GPGSV;04,GPRMC;05,GPVTG;
//		06,GPGRS;07,GPGST;08,GPZDA;
//		09,GPGBS;0A,GPDTM;0D,GPGNS;
//uart1set:0,输出关闭;1,输出开启.	  
//返回值:0,执行成功;其他,执行失败.
u8 Ublox_Cfg_Msg(u8 msgid,u8 uart1set)
{
	_ublox_cfg_msg *cfg_msg=(_ublox_cfg_msg *)USART1_TX_BUF;
	cfg_msg->header=0X62B5;		//cfg header
	cfg_msg->id=0X0106;			//cfg msg id
	cfg_msg->dlength=8;			//数据区长度为8个字节.	
	cfg_msg->msgclass=0XF0;  	//NMEA消息
	cfg_msg->msgid=msgid; 		//要操作的NMEA消息条目
	cfg_msg->iicset=1; 			//默认开启
	cfg_msg->uart1set=uart1set; //开关设置
	cfg_msg->uart2set=1; 	 	//默认开启
	cfg_msg->usbset=1; 			//默认开启
	cfg_msg->spiset=1; 			//默认开启
	cfg_msg->ncset=1; 			//默认开启	  
	Ublox_CheckSum((u8*)(&cfg_msg->id),sizeof(_ublox_cfg_msg)-4,&cfg_msg->cka,&cfg_msg->ckb);
	while(DMA1_Channel4->CNDTR!=0);	//等待通道7传输完成   
	UART_DMA_Enable(DMA1_Channel4,sizeof(_ublox_cfg_msg));	//通过dma发送出去
	return Ublox_Cfg_Ack_Check();
}

//配置NMEA输出信息波特率
//baudrate:波特率,4800/9600/19200/38400/57600/115200/230400	  
//返回值:0,执行成功;其他,执行失败(这里不会返回0了)
u8 Ublox_Cfg_Prt(u32 baudrate)
{
	_ublox_cfg_prt *cfg_prt=(_ublox_cfg_prt *)USART1_TX_BUF;
	cfg_prt->header=0X62B5;		//cfg header
	cfg_prt->id=0X0006;			//cfg prt id
	cfg_prt->dlength=20;		//数据区长度为20个字节.	
	cfg_prt->portid=1;			//操作串口1
	cfg_prt->reserved=0;	 	//保留字节,设置为0
	cfg_prt->txready=0;	 		//TX Ready设置为0
	cfg_prt->mode=0X08D0; 		//8位,1个停止位,无校验位
	cfg_prt->baudrate=baudrate; //波特率设置
	cfg_prt->inprotomask=0X0007;//0+1+2
	cfg_prt->outprotomask=0X0003;//0+1
 	cfg_prt->reserved4=0; 		//保留字节,设置为0
 	cfg_prt->reserved5=0; 		//保留字节,设置为0 
	DOG=~DOG;
	Ublox_CheckSum((u8*)(&cfg_prt->id),sizeof(_ublox_cfg_prt)-4,&cfg_prt->cka,&cfg_prt->ckb);
	DOG=~DOG;
	while(DMA1_Channel4->CNDTR!=0);	//等待通道7传输完成   
	UART_DMA_Enable(DMA1_Channel4,sizeof(_ublox_cfg_prt));	//通过dma发送出去
	delay_ms(200);				//等待发送完成 
	uart_init( baudrate);	//重新初始化串口2   
	DOG=~DOG;
	return Ublox_Cfg_Ack_Check();//这里不会反回0,因为UBLOX发回来的应答在串口重新初始化的时候已经被丢弃了.
}

//配置UBLOX NEO-8M的时钟脉冲输出
//interval:脉冲间隔(us)
//length:脉冲宽度(us)
//status:脉冲配置:1,高电平有效;0,关闭;-1,低电平有效.
//返回值:0,发送成功;其他,发送失败.
u8 Ublox_Cfg_Tp(u32 interval,u32 length,signed char status)
{
	_ublox_cfg_tp *cfg_tp=(_ublox_cfg_tp *)USART1_TX_BUF;
	cfg_tp->header=0X62B5;		//cfg header
	cfg_tp->id=0X0706;			//cfg tp id
	cfg_tp->dlength=20;			//数据区长度为20个字节.
	cfg_tp->interval=interval;	//脉冲间隔,us
	cfg_tp->length=length;		//脉冲宽度,us
	cfg_tp->status=status;	   	//时钟脉冲配置
	cfg_tp->timeref=0;			//参考UTC 时间
	cfg_tp->flags=0;			//flags为0
	cfg_tp->reserved=0;		 	//保留位为0
	cfg_tp->antdelay=820;    	//天线延时为820ns
	cfg_tp->rfdelay=0;    		//RF延时为0ns
	cfg_tp->userdelay=0;    	//用户延时为0ns
	Ublox_CheckSum((u8*)(&cfg_tp->id),sizeof(_ublox_cfg_tp)-4,&cfg_tp->cka,&cfg_tp->ckb);
	while(DMA1_Channel4->CNDTR!=0);	//等待通道7传输完成   
	UART_DMA_Enable(DMA1_Channel4,sizeof(_ublox_cfg_tp));	//通过dma发送出去
	return Ublox_Cfg_Ack_Check();
}

//配置UBLOX NEO-8M的更新速率	    
//measrate:测量时间间隔，单位为ms，最少不能小于200ms（5Hz）
//reftime:参考时间，0=UTC Time；1=GPS Time（一般设置为1）
//返回值:0,发送成功;其他,发送失败.
u8 Ublox_Cfg_Rate(u16 measrate,u8 reftime)
{
	_ublox_cfg_rate *cfg_rate=(_ublox_cfg_rate *)USART1_TX_BUF;
    //if(measrate<200)return 1;	//小于200ms，直接退出
 	cfg_rate->header=0X62B5;	//cfg header
	cfg_rate->id=0X0806;	 	//cfg rate id
	cfg_rate->dlength=6;	 	//数据区长度为6个字节.
	cfg_rate->navrate=1;		//导航速率（周期），固定为1
	cfg_rate->measrate=measrate;//脉冲间隔,us

	cfg_rate->timeref=reftime; 	//参考时间为GPS时间
	Ublox_CheckSum((u8*)(&cfg_rate->id),sizeof(_ublox_cfg_rate)-4,&cfg_rate->cka,&cfg_rate->ckb);

	while(DMA1_Channel4->CNDTR!=0);	//等待通道7传输完成   
	UART_DMA_Enable(DMA1_Channel4,sizeof(_ublox_cfg_rate));//通过dma发送出去
	return Ublox_Cfg_Ack_Check();
}

u8 Ublox_Cfg_wx(u8 g,u8 s,u8 b)
{
	_ublox_cfg_wx *cfg_wx=(_ublox_cfg_wx *)USART1_TX_BUF;

 	cfg_wx->header=0X62B5;	//cfg header
	cfg_wx->id=0X3E06;	 	//cfg rate id
	cfg_wx->dlength=44; 	
	cfg_wx->msgver=0;
	cfg_wx->numTrkChHw=0;
	cfg_wx->numTrkChUse=32;
	cfg_wx->numConfigBlocks=5;	
	
	cfg_wx->GgnssId=0;           //gps配置信息
	cfg_wx->GresTrkCh=8;
    cfg_wx->GmaxTrkCh=16;
	cfg_wx->Greserved1=0;
	cfg_wx->Gflag1=1;
	cfg_wx->Gflag2=1;
	cfg_wx->Gflag3=0;
	cfg_wx->Gflag4=g;
	
	cfg_wx->SgnssId=1;           //SBAS配置信息
	cfg_wx->SresTrkCh=1;
    cfg_wx->SmaxTrkCh=3;
	cfg_wx->Sreserved1=0;	
	cfg_wx->Sflag1=1;
	cfg_wx->Sflag2=1;
	cfg_wx->Sflag3=0;
	cfg_wx->Sflag4=s;
	
	cfg_wx->BgnssId=3;            //北斗配置信息
	cfg_wx->BresTrkCh=8;
    cfg_wx->BmaxTrkCh=16;
	cfg_wx->Breserved1=0;
	cfg_wx->Bflag1=1;
	cfg_wx->Bflag2=1;
	cfg_wx->Bflag3=0;
	cfg_wx->Bflag4=b;
	
	cfg_wx->QgnssId=5;             //Qzss配置信息
	cfg_wx->QresTrkCh=0;
    cfg_wx->QmaxTrkCh=3;
	cfg_wx->Qreserved1=0;
	cfg_wx->Qflag1=1;
	cfg_wx->Qflag2=1;
	cfg_wx->Qflag3=0;
	cfg_wx->Qflag4=0;
	
	cfg_wx->LgnssId=6;              //Glonass配置信息
	cfg_wx->LresTrkCh=8;
    cfg_wx->LmaxTrkCh=14;
	cfg_wx->Lreserved1=0;
	cfg_wx->Lflag1=1;
	cfg_wx->Lflag2=1;
	cfg_wx->Lflag3=0;
	cfg_wx->Lflag4=0;
	Ublox_CheckSum((u8*)(&cfg_wx->id),sizeof(_ublox_cfg_wx)-4,&cfg_wx->cka,&cfg_wx->ckb);
	
	while(DMA1_Channel4->CNDTR!=0);	//等待通道7传输完成   
	UART_DMA_Enable(DMA1_Channel4,sizeof(_ublox_cfg_wx));//通过dma发送出去
	return Ublox_Cfg_Ack_Check();
}

//定位卫星选择
//MODE_GPS:        GPS
//MODE_GPSASBAS:   GPS（+SBAS）  这种模式是增强型gps
//MODE_Beidou :    Beidou
//MODE_GPSABeidou: GPS(+SBAS)&Beidou
void satellites_choose(u8 MODE)
{
	u8 g,s,b;

	switch(MODE)
	{
		case MODE_GPS:
            {
                g=1;
                s=0;
                b=0;
            }
            break;

		case MODE_GPSASBAS:
            {
                g=1;
                s=1;
                b=0;
            }
            break;

		case MODE_Beidou:
            {
                g=0;
                s=0;
                b=1;
            }
            break;

		case MODE_GPSABeidou:
            {
                g=1;
                s=1;
                b=1;
            }
            break;

        default:
            break;
	}

    while( Ublox_Cfg_wx(g,s,b));

}

//需要参数的NMEA数据：
// 00,GxGGA;01,GxGLL;02,GxGSA;
// 03,GxGSV;04,GxRMC;05,GxVTG;
// 06,GxGRS;07,GxGST;08,GxZDA;
// 09,GxGBS;0A,GxDTM;0D,GxGNS;
void MSG_choose()
{  
    Ublox_Cfg_Msg(0,1);        //打开GGA
    Ublox_Cfg_Msg(1,0);        
    Ublox_Cfg_Msg(2,0);
    Ublox_Cfg_Msg(3,0);
    Ublox_Cfg_Msg(4,1);        //打开RMC
    Ublox_Cfg_Msg(5,0);
    Ublox_Cfg_Msg(6,0);
    Ublox_Cfg_Msg(7,0);
    Ublox_Cfg_Msg(8,0);
    Ublox_Cfg_Msg(9,0);
    Ublox_Cfg_Msg(0xa,0);
    Ublox_Cfg_Msg(0xb,0);
    Ublox_Cfg_Msg(0xc,0);
    Ublox_Cfg_Msg(0xd,0);
	
}

//rate_5hz： 并行5hz
//rate_10hz：单独10hz
void Rate_chose(u8 rate)
{

    while(Ublox_Cfg_Rate(rate,1));
    
}

/* gps data check and parse */
void gps_datacheck(void)
{
    u16 rxlen;
    u8 send[6];

    if(USART1_RX_STA&0X8000)		//接收到一次数据了
    {
        rxlen=USART1_RX_STA&0X7FFF;	//得到数据长度			
        GPS_Analysis(&gpsx,(u8*)USART1_RX_BUF);//分析字符串


        
        rxlen=rxlen;       // 屏蔽警告
		
        stgpsdata.posslnum=gpsx.posslnum;
        stgpsdata.longitude = gpsx.longitude;
        stgpsdata.latitude = gpsx.latitude;
        stgpsdata.altitude = gpsx.altitude;
        stgpsdata.speed = gpsx.speed;
        stgpsdata.cipianjiao = gpsx.cipianjiao;
		
		
//        change(send,gpsx.posslnum);
//				
//        NRF24L01_TX_Mode();            //24L01切换进入发送模式
//        NRF24L01_TxPacket(send);       //启动NRF24L01发送一次数据
        
        USART1_RX_STA=0;		   	//启动下一次接收
    }

    return;
}

/**
  * @brief  检测gps卫星颗数.
  * @param  None
  * @retval None
  */
u8 posslnum_check(void)
{
	gps_datacheck();
	if(gpsx.posslnum>=5)
		return 0;
	else
		return 1;
}
/**
  * @brief  gps init.
  * @param  None
  * @retval None
  */
void gps_init(void)
{

    Choose_mode=MODE_GPSABeidou ;
    Rate_chose(rate_5hz);               /* Gps module rate selection */

    satellites_choose(Choose_mode);     /* Gps module single and dual frequency mode selection */
		
    Ublox_Cfg_Prt(9600);				/*Gps module baud rate setting */

    MSG_choose();                       /* NMEA information setting required by gps module */


    return;
}