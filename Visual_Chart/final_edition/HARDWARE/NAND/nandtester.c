#include "nandtester.h"
#include "nand.h"
#include "ftl.h"
#include "string.h"
#include "usart.h"
#include "malloc.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//NAND FLASH USMART���Դ���	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	  


//��NANDĳһҳд��ָ����С������
//pagenum:Ҫд���ҳ��ַ
//colnum:Ҫд��Ŀ�ʼ�е�ַ(ҳ�ڵ�ַ)
//writebytes:Ҫд������ݴ�С��MT29F16G���Ϊ4320��MT29F4G���Ϊ2112
u8 test_writepage(u32 pagenum,u16 colnum,u16 writebytes)
{
	u8 *pbuf;
	u8 sta=0;
    u16 i=0;
	pbuf=mymalloc(SRAMIN,5000);  
    for(i=0;i<writebytes;i++)//׼��Ҫд�������,�������,��0��ʼ����
    { 
        pbuf[i]=i;	
    }
	sta=NAND_WritePage(pagenum,colnum,pbuf,writebytes);	//��nandд������	
	myfree(SRAMIN,pbuf);	//�ͷ��ڴ�
	return sta;
}

//��ȡNANDĳһҳָ����С������
//pagenum:Ҫ��ȡ��ҳ��ַ
//colnum:Ҫ��ȡ�Ŀ�ʼ�е�ַ(ҳ�ڵ�ַ)
//readbytes:Ҫ��ȡ�����ݴ�С��MT29F16G���Ϊ4320��MT29F4G���Ϊ2112
u8 test_readpage(u32 pagenum,u16 colnum,u16 readbytes)
{
	u8 *pbuf;
	u8 sta=0;
    u16 i=0;
	pbuf=mymalloc(SRAMIN,5000);  
	sta=NAND_ReadPage(pagenum,colnum,pbuf,readbytes);	//��ȡ����
	if(sta==0||sta==NSTA_ECC1BITERR||sta==NSTA_ECC2BITERR)//��ȡ�ɹ�
	{ 
		printf("read page data is:\r\n");
		for(i=0;i<readbytes;i++)	 
		{ 
			printf("%x ",pbuf[i]);  //���ڴ�ӡ��ȡ��������
		}
		printf("\r\nend\r\n");
	}
	myfree(SRAMIN,pbuf);	//�ͷ��ڴ�
	return sta;
}

//��һҳ���ݿ���������һҳ,��д��һ��������.
//ע��:Դҳ��Ŀ��ҳҪ��ͬһ��Plane�ڣ�(ͬΪ����/ͬΪż��)
//spnum:Դҳ��ַ
//epnum:Ŀ��ҳ��ַ
//colnum:Ҫд��Ŀ�ʼ�е�ַ(ҳ�ڵ�ַ)
//writebytes:Ҫд������ݴ�С�����ܳ���ҳ��С
u8 test_copypageandwrite(u32 spnum,u32 dpnum,u16 colnum,u16 writebytes)
{
	u8 *pbuf;
	u8 sta=0;
    u16 i=0;
	pbuf=mymalloc(SRAMIN,5000);  
    for(i=0;i<writebytes;i++)//׼��Ҫд�������,�������,��0X80��ʼ����
    { 
        pbuf[i]=i+0X80;	
    }
	sta=NAND_CopyPageWithWrite(spnum,dpnum,colnum,pbuf,writebytes);	//��nandд������	
	myfree(SRAMIN,pbuf);	//�ͷ��ڴ�
	return sta;
}
 
//��ȡNANDĳһҳSpare��ָ����С������
//pagenum:Ҫ��ȡ��ҳ��ַ
//colnum:Ҫ��ȡ��spare����ʼ��ַ
//readbytes:Ҫ��ȡ�����ݴ�С��MT29F16G���Ϊ64��MT29F4G���Ϊ224
u8 test_readspare(u32 pagenum,u16 colnum,u16 readbytes)
{
	u8 *pbuf;
	u8 sta=0;
    u16 i=0;
	pbuf=mymalloc(SRAMIN,512);  
	sta=NAND_ReadSpare(pagenum,colnum,pbuf,readbytes);	//��ȡ����
	if(sta==0)//��ȡ�ɹ�
	{ 
		printf("read spare data is:\r\n");
		for(i=0;i<readbytes;i++)	 
		{ 
			printf("%x ",pbuf[i]);  //���ڴ�ӡ��ȡ��������
		}
		printf("\r\nend\r\n");
	}
	myfree(SRAMIN,pbuf);	//�ͷ��ڴ�
	return sta;
}

//��ָ��λ�ÿ�ʼ,��ȡ����NAND,ÿ��BLOCK�ĵ�һ��page��ǰ5���ֽ�
//sblock:ָ����ʼ��block���
void test_readallblockinfo(u32 sblock)
{
    u8 j=0;
    u32 i=0;
	u8 sta;
    u8 buffer[5];
    for(i=sblock;i<nand_dev.block_totalnum;i++)
    {
        printf("block %d info:",i);
        sta=NAND_ReadSpare(i*nand_dev.block_pagenum,0,buffer,5);//��ȡÿ��block,��һ��page��ǰ5���ֽ�
        if(sta)printf("failed\r\n");
		for(j=0;j<5;j++)
        {
            printf("%x ",buffer[j]);
        }
        printf("\r\n");
    }	
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
//FTL����Դ���

//��ĳ��������ʼ,д��seccnt������������
//secx:��ʼ���������
//secsize:������С
//seccnt:Ҫд�����������
u8 test_ftlwritesectors(u32 secx,u16 secsize,u16 seccnt)
{
	u8 *pbuf;
	u8 sta=0;
    u32 i=0;
	pbuf=mymalloc(SRAMIN,secsize*seccnt);  
    for(i=0;i<secsize*seccnt;i++)	//׼��Ҫд�������,�������,��0��ʼ����
    { 
        pbuf[i]=i;	
    }
	sta=FTL_WriteSectors(pbuf,secx,secsize,seccnt);	//��nandд������	
	myfree(SRAMIN,pbuf);	//�ͷ��ڴ�
	return sta;
}


//��ĳ��������ʼ,����seccnt������������
//secx:��ʼ���������
//secsize:������С
//seccnt:Ҫ��ȡ����������
u8 test_ftlreadsectors(u32 secx,u16 secsize,u16 seccnt)
{
	u8 *pbuf;
	u8 sta=0;
    u32 i=0;
	pbuf=mymalloc(SRAMIN,secsize*seccnt);   
	sta=FTL_ReadSectors(pbuf,secx,secsize,seccnt);	//��ȡ����
	if(sta==0)
	{
		printf("read sec %d data is:\r\n",secx); 
		for(i=0;i<secsize*seccnt;i++)	//׼��Ҫд�������,�������,��0��ʼ����
		{ 
			printf("%x ",pbuf[i]);  //���ڴ�ӡ��ȡ��������
		}
		printf("\r\nend\r\n");
	}
	myfree(SRAMIN,pbuf);	//�ͷ��ڴ�
	return sta;
}





























