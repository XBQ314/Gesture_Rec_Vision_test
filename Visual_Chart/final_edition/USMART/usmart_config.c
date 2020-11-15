#include "usmart.h"
#include "usmart_str.h"
////////////////////////////�û�������///////////////////////////////////////////////
//������Ҫ�������õ��ĺ�����������ͷ�ļ�(�û��Լ����) 
#include "delay.h"	 	
#include "sys.h"
#include "lcd.h"
#include "sdram.h"
#include "ltdc.h"
#include "nand.h"  
#include "nandtester.h"  
#include "ftl.h" 
								 
extern void led_set(u8 sta);
extern void test_fun(void(*ledset)(u8),u8 sta);										  
//�������б��ʼ��(�û��Լ����)
//�û�ֱ������������Ҫִ�еĺ�����������Ҵ�
struct _m_usmart_nametab usmart_nametab[]=
{
#if USMART_USE_WRFUNS==1 	//���ʹ���˶�д����
	(void*)read_addr,"u32 read_addr(u32 addr)",
	(void*)write_addr,"void write_addr(u32 addr,u32 val)",	 
#endif	
	(void*)delay_ms,"void delay_ms(u16 nms)",
 	(void*)delay_us,"void delay_us(u32 nus)",	 

 	(void*)NAND_ModeSet,"u8 NAND_ModeSet(u8 mode)",    
	(void*)NAND_EraseBlock,"u8 NAND_EraseBlock(u32 BlockNum)",    
	(void*)NAND_EraseChip,"void NAND_EraseChip(void)",    
	(void*)NAND_CopyPageWithoutWrite,"u8 NAND_CopyPageWithoutWrite(u32 Source_PageNum,u32 Dest_PageNum)",    
    		
	(void*)test_copypageandwrite,"u8 test_copypageandwrite(u32 spnum,u32 dpnum,u16 colnum,u16 writebytes)",    
	(void*)test_readpage,"u8 test_readpage(u32 pagenum,u16 colnum,u16 readbytes)",    
	(void*)test_writepage,"u8 test_writepage(u32 pagenum,u16 colnum,u16 writebytes)",
	(void*)test_readspare,"u8 test_readspare(u32 pagenum,u16 colnum,u16 readbytes)",
	(void*)test_readallblockinfo,"void test_readallblockinfo(u32 sblock)",
	(void*)test_ftlwritesectors,"u8 test_ftlwritesectors(u32 secx,u16 secsize,u16 seccnt)",
	(void*)test_ftlreadsectors,"u8 test_ftlreadsectors(u32 secx,u16 secsize,u16 seccnt)",
		
	(void*)FTL_Init,"u8 FTL_Init(void)",
	(void*)FTL_CheckBadBlock,"u8 FTL_CheckBadBlock(u32 blocknum)",
	(void*)FTL_UsedBlockMark,"u8 FTL_UsedBlockMark(u32 blocknum)",
 	(void*)FTL_FindUnusedBlock,"u32 FTL_FindUnusedBlock(u32 sblock,u8 flag)",
	(void*)FTL_FindSamePlaneUnusedBlock,"u32 FTL_FindSamePlaneUnusedBlock(u32 blocknum)",
	(void*)FTL_LBNToPBN,"u16 FTL_LBNToPBN(u32 blocknum)",
	(void*)FTL_CreateLUT,"u8 FTL_CreateLUT(void)",
	(void*)FTL_Format,"u8 FTL_Format(void)",    
					
};						  
///////////////////////////////////END///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//�������ƹ�������ʼ��
//�õ������ܿغ���������
//�õ�����������
struct _m_usmart_dev usmart_dev=
{
	usmart_nametab,
	usmart_init,
	usmart_cmd_rec,
	usmart_exe,
	usmart_scan,
	sizeof(usmart_nametab)/sizeof(struct _m_usmart_nametab),//��������
	0,	  	//��������
	0,	 	//����ID
	1,		//������ʾ����,0,10����;1,16����
	0,		//��������.bitx:,0,����;1,�ַ���	    
	0,	  	//ÿ�������ĳ����ݴ��,��ҪMAX_PARM��0��ʼ��
	0,		//�����Ĳ���,��ҪPARM_LEN��0��ʼ��
};   



















