#include "ftl.h"
#include "string.h"
#include "malloc.h"
#include "nand.h"
#include "usart.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//NAND FLASH FTL���㷨����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/15
//�汾��V1.3
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//********************************************************************************
//����˵��
//V1.1 20160124
//�޸�FTL_CopyAndWriteToBlock��FTL_WriteSectors����,��߷�0XFFʱ��д���ٶ�.  
//V1.2 20160520
//1,�޸�FTL_ReadSectors,����ECC�����ж�,��⻵�鴦��,�����Ӷ������,����ٶ�
//2,����FTL_BlockCompare��FTL_SearchBadBlock����,������Ѱ����
//3,�޸�FTL_Format�����ⷽʽ,����FTL_USE_BAD_BLOCK_SEARCH��
//V1.3 20160530
//�޸ĵ�1bit ECC�������ʱ����ȡ2�Σ���ȷ��1bit �����Է�������޸�����
////////////////////////////////////////////////////////////////////////////////// 	

//ÿ����,��һ��page��spare��,ǰ�ĸ��ֽڵĺ���:
//��һ���ֽ�,��ʾ�ÿ��Ƿ��ǻ���:0XFF,������;����ֵ,����.
//�ڶ����ֽ�,��ʾ�ÿ��Ƿ��ù�:0XFF,û��д������;0XCC,д��������.
//�����͵��ĸ��ֽ�,��ʾ�ÿ��������߼�����. 

//ÿ��page,spare��16�ֽ��Ժ���ֽں���:
//��ʮ���ֽڿ�ʼ,����ÿ4���ֽ����ڴ洢һ������(��С:NAND_ECC_SECTOR_SIZE)��ECCֵ,����ECCУ��


//FTL���ʼ��
//����ֵ:0,����
//    ����,ʧ��
u8 FTL_Init(void)
{
    u8 temp;
    if(NAND_Init())return 1;									//��ʼ��NAND FLASH
	if(nand_dev.lut)myfree(SRAMIN,nand_dev.lut);
	nand_dev.lut=mymalloc(SRAMIN,(nand_dev.block_totalnum)*2); 	//��LUT�������ڴ�
	memset(nand_dev.lut,0,nand_dev.block_totalnum*2);			//ȫ������
    if(!nand_dev.lut)return 1;				//�ڴ�����ʧ�� 
    temp=FTL_CreateLUT(1);
    if(temp) 
    {   
        printf("format nand flash...\r\n");
        temp=FTL_Format();     //��ʽ��NAND
        if(temp)
        {
            printf("format failed!\r\n");
            return 2;
        }
    }else 	//����LUT��ɹ�
	{
		printf("total block num:%d\r\n",nand_dev.block_totalnum);
		printf("good block num:%d\r\n",nand_dev.good_blocknum);
		printf("valid block num:%d\r\n",nand_dev.valid_blocknum);
    }
	return 0;
} 

//���ĳһ����Ϊ����
//blocknum:����,��Χ:0~(block_totalnum-1)
void FTL_BadBlockMark(u32 blocknum)
{
    u32 temp=0XAAAAAAAA;//������mark,����ֵ��OK,ֻҪ����0XFF.����дǰ4���ֽ�,����FTL_FindUnusedBlock������黵��.(����鱸����,������ٶ�)
    NAND_WriteSpare(blocknum*nand_dev.block_pagenum,0,(u8*)&temp,4);	//�ڵ�һ��page��spare��,��һ���ֽ���������(ǰ4���ֽڶ�д)
    NAND_WriteSpare(blocknum*nand_dev.block_pagenum+1,0,(u8*)&temp,4);	//�ڵڶ���page��spare��,��һ���ֽ���������(������,ǰ4���ֽڶ�д)
} 
//���ĳһ���Ƿ��ǻ���
//blocknum:����,��Χ:0~(block_totalnum-1)
//����ֵ:0,�ÿ�
//	  ����,����
u8 FTL_CheckBadBlock(u32 blocknum)
{
    u8 flag=0; 
    NAND_ReadSpare(blocknum*nand_dev.block_pagenum,0,&flag,1);//��ȡ�����־
    if(flag==0XFF)//�ÿ�?,��ȡ������������
    {
        NAND_ReadSpare(blocknum*nand_dev.block_pagenum+1,0,&flag,1);//��ȡ�����������־
        if(flag==0XFF)return 0;	//�ÿ�
        else return 1;  		//����
    }   
	return 2; 
}
//���ĳһ�����Ѿ�ʹ��
//blocknum:����,��Χ:0~(block_totalnum-1)
//����ֵ:0,�ɹ�
//    ����,ʧ��
u8 FTL_UsedBlockMark(u32 blocknum)
{
    u8 Usedflag=0XCC;
    u8 temp=0;
    temp=NAND_WriteSpare(blocknum*nand_dev.block_pagenum,1,(u8*)&Usedflag,1);//д����Ѿ���ʹ�ñ�־
    return temp;
}   
//�Ӹ����Ŀ鿪ʼ�ҵ���ǰ�ҵ�һ��δ��ʹ�õĿ�(ָ������/ż��)
//sblock:��ʼ��,��Χ:0~(block_totalnum-1)
//flag:0,ż����;1,������.
//����ֵ:0XFFFFFFFF,ʧ��
//           ����ֵ,δʹ�ÿ��
u32 FTL_FindUnusedBlock(u32 sblock,u8 flag)
{
    u32 temp=0;
    u32 blocknum=0; 
	for(blocknum=sblock+1;blocknum>0;blocknum--)
    {
        if(((blocknum-1)%2)==flag)//��ż�ϸ�,�ż��
		{
		    NAND_ReadSpare((blocknum-1)*nand_dev.block_pagenum,0,(u8*)&temp,4);//�����Ƿ�ʹ�ñ��
 			if(temp==0XFFFFFFFF)return(blocknum-1);//�ҵ�һ���տ�,���ؿ���
		}
    }
    return 0XFFFFFFFF;	//δ�ҵ������
    
} 
//�������������ͬһ��plane�ڵ�δʹ�õĿ�
//sblock��������,��Χ:0~(block_totalnum-1)
//����ֵ:0XFFFFFFFF,ʧ��
//           ����ֵ,δʹ�ÿ��
u32 FTL_FindSamePlaneUnusedBlock(u32 sblock)
{
	static u32 curblock=0XFFFFFFFF;
	u32 unusedblock=0;  
	if(curblock>(nand_dev.block_totalnum-1))curblock=nand_dev.block_totalnum-1;//������Χ��,ǿ�ƴ����һ���鿪ʼ
  	unusedblock=FTL_FindUnusedBlock(curblock,sblock%2);					//�ӵ�ǰ��,��ʼ,��ǰ���ҿ���� 
 	if(unusedblock==0XFFFFFFFF&&curblock<(nand_dev.block_totalnum-1))	//δ�ҵ�,�Ҳ��Ǵ���ĩβ��ʼ�ҵ�
	{
		curblock=nand_dev.block_totalnum-1;								//ǿ�ƴ����һ���鿪ʼ
		unusedblock=FTL_FindUnusedBlock(curblock,sblock%2);				//����ĩβ��ʼ,������һ��  
	}
	if(unusedblock==0XFFFFFFFF)return 0XFFFFFFFF;						//�Ҳ�������block 
	curblock=unusedblock;												//��ǰ��ŵ���δʹ�ÿ���.�´���Ӵ˴���ʼ����
 	return unusedblock;													//�����ҵ��Ŀ���block
}    
//��һ��������ݿ�������һ��,���ҿ���д������ 
//Source_PageNo:Ҫд�����ݵ�ҳ��ַ,��Χ:0~(block_pagenum*block_totalnum-1)
//ColNum:Ҫд����п�ʼ��ַ(Ҳ����ҳ�ڵ�ַ),��Χ:0~(page_totalsize-1)
//pBuffer:Ҫд������� 
//NumByteToWrite:Ҫд����ֽ�������ֵ���ܳ�������ʣ��������С
//����ֵ:0,�ɹ�
//    ����,ʧ��
u8 FTL_CopyAndWriteToBlock(u32 Source_PageNum,u16 ColNum,u8 *pBuffer,u32 NumByteToWrite)
{
    u16 i=0,temp=0,wrlen;
    u32 source_block=0,pageoffset=0;
    u32 unusedblock=0; 
    source_block=Source_PageNum/nand_dev.block_pagenum;	//���ҳ���ڵĿ��
    pageoffset=Source_PageNum%nand_dev.block_pagenum;	//���ҳ�����ڿ��ڵ�ƫ�� 
retry:      
    unusedblock=FTL_FindSamePlaneUnusedBlock(source_block);//������Դ����һ��plane��δʹ�ÿ�
    if(unusedblock>nand_dev.block_totalnum)return 1;	//���ҵ��Ŀ����Ŵ��ڿ��������Ļ��϶��ǳ�����
    for(i=0;i<nand_dev.block_pagenum;i++)				//��һ��������ݸ��Ƶ��ҵ���δʹ�ÿ���
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                       
        if(i>=pageoffset&&NumByteToWrite)				//����Ҫд�뵽��ǰҳ
        { 
			if(NumByteToWrite>(nand_dev.page_mainsize-ColNum))//Ҫд�������,�����˵�ǰҳ��ʣ������
			{
				wrlen=nand_dev.page_mainsize-ColNum;	//д�볤�ȵ��ڵ�ǰҳʣ�����ݳ���
			}else wrlen=NumByteToWrite;					//д��ȫ������ 
            temp=NAND_CopyPageWithWrite(source_block*nand_dev.block_pagenum+i,unusedblock*nand_dev.block_pagenum+i,ColNum,pBuffer,wrlen);
			ColNum=0;						//�е�ַ����
			pBuffer+=wrlen;					//д��ַƫ��
			NumByteToWrite-=wrlen;			//д�����ݼ���			
 		}else								//������д��,ֱ�ӿ�������
		{
			temp=NAND_CopyPageWithoutWrite(source_block*nand_dev.block_pagenum+i,unusedblock*nand_dev.block_pagenum+i);
		}
		if(temp)							//����ֵ����,�����鴦��
		{ 
 			FTL_BadBlockMark(unusedblock);	//���Ϊ����
			FTL_CreateLUT(1);				//�ؽ�LUT��
			goto retry;
		}
    } 
    if(i==nand_dev.block_pagenum) 			//�������
    {
        FTL_UsedBlockMark(unusedblock);		//��ǿ��Ѿ�ʹ��	
        NAND_EraseBlock(source_block);		//����Դ��
		//printf("\r\ncopy block %d to block %d\r\n",source_block,unusedblock);//��ӡ������Ϣ
		for(i=0;i<nand_dev.block_totalnum;i++)	//����LUT����unusedblock�滻source_block
		{
			if(nand_dev.lut[i]==source_block)
			{
				nand_dev.lut[i]=unusedblock;
				break;
			}
		}  
    }
    return 0;                               //�ɹ�
}   
//�߼����ת��Ϊ������
//LBNNum:�߼�����
//����ֵ:�������
u16 FTL_LBNToPBN(u32 LBNNum)
{
    u16 PBNNo=0;
    //���߼���Ŵ�����Ч������ʱ�򷵻�0XFFFF
    if(LBNNum>nand_dev.valid_blocknum)return 0XFFFF;
    PBNNo=nand_dev.lut[LBNNum];
    return PBNNo;
}
//д����(֧�ֶ�����д)��FATFS�ļ�ϵͳʹ��
//pBuffer:Ҫд�������
//SectorNo:��ʼ������
//SectorSize:������С(���ܴ���NAND_ECC_SECTOR_SIZE����Ĵ�С,��������!!)
//SectorCount:Ҫд�����������
//����ֵ:0,�ɹ�
//	  ����,ʧ��
u8 FTL_WriteSectors(u8 *pBuffer,u32 SectorNo,u16 SectorSize,u32 SectorCount)
{
    u8 flag=0;
	u16 temp;
    u32 i=0;
	u16 wsecs;		//дҳ��С
	u32 wlen;		//д�볤��
    u32 LBNNo;      //�߼����
    u32 PBNNo;      //������
    u32 PhyPageNo;  //����ҳ��
    u32 PageOffset; //ҳ��ƫ�Ƶ�ַ
    u32 BlockOffset;//����ƫ�Ƶ�ַ
	u32 markdpbn=0XFFFFFFFF;		//����˵��������  
	for(i=0;i<SectorCount;i++)
    {
        LBNNo=(SectorNo+i)/(nand_dev.block_pagenum*(nand_dev.page_mainsize/SectorSize));//�����߼������ź�������С������߼����
        PBNNo=FTL_LBNToPBN(LBNNo);					//���߼���ת��Ϊ�����
        if(PBNNo>=nand_dev.block_totalnum)return 1;	//�����Ŵ���NAND FLASH���ܿ���,��ʧ��. 
        BlockOffset=((SectorNo+i)%(nand_dev.block_pagenum*(nand_dev.page_mainsize/SectorSize)))*SectorSize;//�������ƫ��
        PhyPageNo=PBNNo*nand_dev.block_pagenum+BlockOffset/nand_dev.page_mainsize;	//���������ҳ��
        PageOffset=BlockOffset%nand_dev.page_mainsize;								//�����ҳ��ƫ�Ƶ�ַ 
 		temp=nand_dev.page_mainsize-PageOffset;	//page��ʣ���ֽ���
		temp/=SectorSize;						//��������д���sector�� 
		wsecs=SectorCount-i;					//��ʣ���ٸ�sectorҪд
		if(wsecs>=temp)wsecs=temp;				//���ڿ�����д���sector��,��д��temp������  
		wlen=wsecs*SectorSize;					//ÿ��дwsecs��sector  
		//����д���С�������ж��Ƿ�ȫΪ0XFF
		flag=NAND_ReadPageComp(PhyPageNo,PageOffset,0XFFFFFFFF,wlen/4,&temp);		//��һ��wlen/4��С������,����0XFFFFFFFF�Ա�
		if(flag)return 2;						//��д���󣬻��� 
		if(temp==(wlen/4)) flag=NAND_WritePage(PhyPageNo,PageOffset,pBuffer,wlen);	//ȫΪ0XFF,����ֱ��д����
		else flag=1;							//��ȫ��0XFF,����������
		if(flag==0&&(markdpbn!=PBNNo))			//ȫ��0XFF,��д��ɹ�,�ұ���˵�������뵱ǰ����鲻ͬ
		{
			flag=FTL_UsedBlockMark(PBNNo);		//��Ǵ˿��Ѿ�ʹ��  
			markdpbn=PBNNo;						//������,��ǿ�=��ǰ��,��ֹ�ظ����
		}
		if(flag)//��ȫΪ0XFF/���ʧ�ܣ�������д����һ����   
        {
			temp=((u32)nand_dev.block_pagenum*nand_dev.page_mainsize-BlockOffset)/SectorSize;//��������block��ʣ�¶��ٸ�SECTOR����д��
 			wsecs=SectorCount-i;				//��ʣ���ٸ�sectorҪд
			if(wsecs>=temp)wsecs=temp;			//���ڿ�����д���sector��,��д��temp������ 
			wlen=wsecs*SectorSize;				//ÿ��дwsecs��sector   
            flag=FTL_CopyAndWriteToBlock(PhyPageNo,PageOffset,pBuffer,wlen);//����������һ��block,��д������
            if(flag)return 3;//ʧ�� 
        } 
		i+=wsecs-1;
		pBuffer+=wlen;//���ݻ�����ָ��ƫ��
    }
    return 0;   
} 
//������(֧�ֶ�������)��FATFS�ļ�ϵͳʹ��
//pBuffer:���ݻ�����
//SectorNo:��ʼ������
//SectorSize:������С
//SectorCount:Ҫд�����������
//����ֵ:0,�ɹ�
//	  ����,ʧ��
u8 FTL_ReadSectors(u8 *pBuffer,u32 SectorNo,u16 SectorSize,u32 SectorCount)
{
    u8 flag=0;
	u16 rsecs;		//���ζ�ȡҳ�� 
    u32 i=0;
    u32 LBNNo;      //�߼����
    u32 PBNNo;      //������
    u32 PhyPageNo;  //����ҳ��
    u32 PageOffset; //ҳ��ƫ�Ƶ�ַ
    u32 BlockOffset;//����ƫ�Ƶ�ַ 
    for(i=0;i<SectorCount;i++)
    {
        LBNNo=(SectorNo+i)/(nand_dev.block_pagenum*(nand_dev.page_mainsize/SectorSize));//�����߼������ź�������С������߼����
        PBNNo=FTL_LBNToPBN(LBNNo);					//���߼���ת��Ϊ�����
        if(PBNNo>=nand_dev.block_totalnum)return 1;	//�����Ŵ���NAND FLASH���ܿ���,��ʧ��.  
        BlockOffset=((SectorNo+i)%(nand_dev.block_pagenum*(nand_dev.page_mainsize/SectorSize)))*SectorSize;//�������ƫ��
        PhyPageNo=PBNNo*nand_dev.block_pagenum+BlockOffset/nand_dev.page_mainsize;	//���������ҳ��
        PageOffset=BlockOffset%nand_dev.page_mainsize;                     			//�����ҳ��ƫ�Ƶ�ַ 
		rsecs=(nand_dev.page_mainsize-PageOffset)/SectorSize;						//����һ�������Զ�ȡ����ҳ
		if(rsecs>(SectorCount-i))rsecs=SectorCount-i;								//��಻�ܳ���SectorCount-i
		flag=NAND_ReadPage(PhyPageNo,PageOffset,pBuffer,rsecs*SectorSize);			//��ȡ����
		if(flag==NSTA_ECC1BITERR)													//����1bit ecc����,����Ϊ����
		{	
			flag=NAND_ReadPage(PhyPageNo,PageOffset,pBuffer,rsecs*SectorSize);		//�ض�����,�ٴ�ȷ��
			if(flag==NSTA_ECC1BITERR)
			{
 				FTL_CopyAndWriteToBlock(PhyPageNo,PageOffset,pBuffer,rsecs*SectorSize);	//�������� 
				flag=FTL_BlockCompare(PhyPageNo/nand_dev.block_pagenum,0XFFFFFFFF);		//ȫ1���,ȷ���Ƿ�Ϊ����
				if(flag==0)
				{
					flag=FTL_BlockCompare(PhyPageNo/nand_dev.block_pagenum,0X00);		//ȫ0���,ȷ���Ƿ�Ϊ����
					NAND_EraseBlock(PhyPageNo/nand_dev.block_pagenum);					//�����ɺ�,���������
				}
				if(flag)																//ȫ0/ȫ1������,�϶��ǻ�����.
				{
					FTL_BadBlockMark(PhyPageNo/nand_dev.block_pagenum);					//���Ϊ����
					FTL_CreateLUT(1);													//�ؽ�LUT�� 
				}
				flag=0;
			}
		}
		if(flag==NSTA_ECC2BITERR)flag=0;	//2bit ecc����,������(�����ǳ���д�����ݵ��µ�)
		if(flag)return 2;					//ʧ��
		pBuffer+=SectorSize*rsecs;			//���ݻ�����ָ��ƫ�� 
		i+=rsecs-1;
    }
    return 0; 
}
//���´���LUT��
//mode:0,������һ��������
//     1,���������Ƕ�Ҫ���(������ҲҪ���)
//����ֵ:0,�ɹ�
//    ����,ʧ��
u8 FTL_CreateLUT(u8 mode)
{
    u32 i;
 	u8 buf[4];
    u32 LBNnum=0;								//�߼���� 
    for(i=0;i<nand_dev.block_totalnum;i++)		//��λLUT����ʼ��Ϊ��Чֵ��Ҳ����0XFFFF
    {
        nand_dev.lut[i]=0XFFFF;
    } 
	nand_dev.good_blocknum=0;
    for(i=0;i<nand_dev.block_totalnum;i++)
    {
		NAND_ReadSpare(i*nand_dev.block_pagenum,0,buf,4);	//��ȡ4���ֽ�
		if(buf[0]==0XFF&&mode)NAND_ReadSpare(i*nand_dev.block_pagenum+1,0,buf,1);//�ÿ�,����Ҫ���2�λ�����
		if(buf[0]==0XFF)//�Ǻÿ� 				 
        { 
			LBNnum=((u16)buf[3]<<8)+buf[2];		//�õ��߼�����
            if(LBNnum<nand_dev.block_totalnum)	//�߼���ſ϶�С���ܵĿ�����
            {
                nand_dev.lut[LBNnum]=i;			//����LUT��дLBNnum��Ӧ���������
            }
			nand_dev.good_blocknum++;
		}else printf("bad block index:%d\r\n",i);
    } 
    //LUT��������Ժ�����Ч�����
    for(i=0;i<nand_dev.block_totalnum;i++)
    {
        if(nand_dev.lut[i]>=nand_dev.block_totalnum)
        {
            nand_dev.valid_blocknum=i;
            break;
        }
    }
    if(nand_dev.valid_blocknum<100)return 2;	//��Ч����С��100,������.��Ҫ���¸�ʽ�� 
    return 0;	//LUT�������
} 
//FTL����Block��ĳ�����ݶԱ�
//blockx:block���
//cmpval:Ҫ��֮�Աȵ�ֵ
//����ֵ:0,���ɹ�,ȫ�����
//       1,���ʧ��,�в���ȵ����
u8 FTL_BlockCompare(u32 blockx,u32 cmpval)
{
	u8 res;
	u16 i,j,k; 
	for(i=0;i<3;i++)//����3�λ���
	{
		for(j=0;j<nand_dev.block_pagenum;j++)
		{
			NAND_ReadPageComp(blockx*nand_dev.block_pagenum,0,cmpval,nand_dev.page_mainsize/4,&k);//���һ��page,����0XFFFFFFFF�Ա�
			if(k!=(nand_dev.page_mainsize/4))break;
		}
		if(j==nand_dev.block_pagenum)return 0;		//���ϸ�,ֱ���˳�
		res=NAND_EraseBlock(blockx);
		if(res)printf("error erase block:%d\r\n",i);
		else
		{ 
			if(cmpval!=0XFFFFFFFF)//�����ж�ȫ1,����Ҫ��д����
			{
				for(k=0;k<nand_dev.block_pagenum;k++)
				{
					NAND_WritePageConst(blockx*nand_dev.block_pagenum+k,0,0,nand_dev.page_mainsize/4);//дPAGE 
				}
			}
		}
	}
	printf("bad block checked:%d\r\n",blockx);
	return 1;
}
//FTL��ʼ��ʱ����Ѱ���л���,ʹ��:��-д-�� ��ʽ
//512M��NAND ,��ҪԼ3����ʱ��,����ɼ��
//����RGB��,����Ƶ����дNAND,��������Ļ����
//����ֵ���ÿ������
u32 FTL_SearchBadBlock(void)
{
	u8 *blktbl;
	u8 res;
	u32 i,j; 
	u32 goodblock=0;
	blktbl=mymalloc(SRAMIN,nand_dev.block_totalnum);//����block������ڴ�,��Ӧ��:0,�ÿ�;1,����;
	NAND_EraseChip(); 						//ȫƬ����
    for(i=0;i<nand_dev.block_totalnum;i++)	//��һ�׶μ��,���ȫ1
    {
 		res=FTL_BlockCompare(i,0XFFFFFFFF);	//ȫ1��� 
		if(res)blktbl[i]=1;					//���� 
		else
		{ 
			blktbl[i]=0;					//�ÿ� 
			for(j=0;j<nand_dev.block_pagenum;j++)//дblockΪȫ0,Ϊ����ļ��׼��
			{
				NAND_WritePageConst(i*nand_dev.block_pagenum+j,0,0,nand_dev.page_mainsize/4);
			} 
		}
    }	
    for(i=0;i<nand_dev.block_totalnum;i++)	//�ڶ��׶μ��,���ȫ0
    { 
 		if(blktbl[i]==0)					//�ڵ�һ�׶�,û�б���ǻ����,�ſ����Ǻÿ�
		{
			res=FTL_BlockCompare(i,0);		//ȫ0��� 
			if(res)blktbl[i]=1;				//��ǻ���
			else goodblock++; 
		}
    }
	NAND_EraseChip();  	//ȫƬ����
    for(i=0;i<nand_dev.block_totalnum;i++)	//�����׶μ��,��ǻ���
    { 
		if(blktbl[i])FTL_BadBlockMark(i);	//�ǻ���
	}
	return goodblock;	//���غÿ������
}

//��ʽ��NAND �ؽ�LUT��
//����ֵ:0,�ɹ�
//    ����,ʧ��
u8 FTL_Format(void)
{
    u8 temp;
    u32 i,n;
    u32 goodblock=0;
	nand_dev.good_blocknum=0;
#if FTL_USE_BAD_BLOCK_SEARCH==1				//ʹ�ò�-д-���ķ�ʽ,��⻵��
	nand_dev.good_blocknum=FTL_SearchBadBlock();//��Ѱ����.��ʱ�ܾ�
#else										//ֱ��ʹ��NAND FLASH�ĳ��������־(������,Ĭ���Ǻÿ�)
    for(i=0;i<nand_dev.block_totalnum;i++)	
    {
		temp=FTL_CheckBadBlock(i);			//���һ�����Ƿ�Ϊ����
        if(temp==0)							//�ÿ�
        { 
			temp=NAND_EraseBlock(i);
			if(temp)						//����ʧ��,��Ϊ����
			{
				printf("Bad block:%d\r\n",i);
				FTL_BadBlockMark(i);		//����ǻ���
			}else nand_dev.good_blocknum++;	//�ÿ�������һ 
		}
	} 
#endif
    printf("good_blocknum:%d\r\n",nand_dev.good_blocknum); 
    if(nand_dev.good_blocknum<100) return 1;	//����ÿ����������100����NAND Flash����   
    goodblock=(nand_dev.good_blocknum*93)/100;	//%93�ĺÿ����ڴ洢����  
    n=0;										
    for(i=0;i<nand_dev.block_totalnum;i++)		//�ںÿ��б�����߼�����Ϣ
    {
        temp=FTL_CheckBadBlock(i);  			//���һ�����Ƿ�Ϊ����
        if(temp==0)                  			//�ÿ�
        { 
            NAND_WriteSpare(i*nand_dev.block_pagenum,2,(u8*)&n,2);//д���߼�����
            n++;								//�߼����ż�1
            if(n==goodblock) break;				//ȫ���������
        }
    } 
    if(FTL_CreateLUT(1))return 2;      			//�ؽ�LUT��ʧ�� 
    return 0;
}










