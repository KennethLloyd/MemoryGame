/*
  Name: RamDisk Driver for dex-os
  Copyright: 
  Author: Joseph Emmanuel DL Dayo
  Date: 02/04/04 19:54
  Description: This module provides support for a ramdisk device.
  
  RamDisk driver for dex-os
  Copyright (C) 2004  Joseph Emmanuel DL Dayo

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 
*/


#define _DEX_DEVICE_

#include "dexsdk.h"
#include "dll.h"
#include <dex\dextypes.h>
#include <dex\dex32_devmgr.h>



#define BLOCK_SIZE 512

typedef struct __attribute__((packed)) fat_bootsector			/* Warning: this struct must be packed */
{
   BYTE  jump[3];               /* 16-bit JMP to boot code, or 8-bit JMP + NOP */
   BYTE  oem_id[8];             /* e.g. 'MSWIN4.0' */
   WORD  bytes_per_sector;	/* usu. =512 */
   BYTE  sectors_per_cluster;
   WORD  num_boot_sectors;	/* usu. =1 */
   BYTE  num_fats;              /* usu. =2 */
   WORD num_root_dir_ents;
   WORD total_sectors;		/* 16-bit; 0 if num sectors > 65535 */
   BYTE  media_ID_byte;         /* usu. =0F0h */
   WORD sectors_per_fat;
   WORD sectors_per_track;
   WORD heads;
   DWORD hidden_sectors;	/* =LBA partition start */
   DWORD total_sectors_large;	/* 32-bit; 0 if num sectors < 65536 */
   BYTE  boot_code[474];
   BYTE  magic[2];              /* 55h, 0AAh */
} BPB;            

devmgr_stdlib stdlib;
devmgr_block_desc myinterface;

typedef struct _block {
char data[BLOCK_SIZE];
} block;

int ramdisk_handle;
DWORD RAMDISK_SIZE = 4000;
block *ramdiskmem;


/*Tools used by device drivers, this pointers to functions will
      be filled in by the dex device driver stub*/
      
void (*kfree)(void *ptr) = 0;
void *(*kmalloc)(size_t size) = 0;
int  (*kprintf)(const char *fmt, ...) = 0;
void *(*krealloc)(void *ptr, size_t size) = 0;

int  (*devmgr_register)(devmgr_generic *) = 0;
char *(*devmgr_identify)(int type,char *buf) = 0;
int  (*devmgr_finddevice)(const char *name) = 0;
int  (*devmgr_copyinterface)(const char *name,devmgr_generic *interface) = 0;
devmgr_generic (*devmgr_getdevice)(int deviceid) = 0;
void (*devmgr_disableints)() = 0;
void (*devmgr_enableints)() = 0;
int  (*extension_override)(devmgr_generic *ext, devmgr_generic **prev) = 0;



int read_block(int block,char *blockbuff,DWORD numblocks);
int write_block(int block,char *blockbuff,DWORD numblocks);

void writecluster(int cluster,int value,BYTE *fat)
 {
  WORD *temp;
  int x,index;
  index=cluster;
  x=(index*3)/2; //index=cluster*3/2
  temp=(WORD*)(fat+x);

  if (cluster & 1) //cluster is odd
          {
          value = value << 4;
          *temp= (*temp&0x000F) | value;
          }
          else
          {
          value =value & 0x0FFF;
          *temp=(*temp&0xF000) | value;
          };
 };


/*format ramdisk to conform to the FAT12 specification*/
int init_device()
{
    BPB diskBPB;
    int i,i2;
    char *FAT;
    char temp[255];
    for (i=0;i<RAMDISK_SIZE;i++)
       
    memset(ramdiskmem[i].data,0,BLOCK_SIZE);
    
    
    memset(&diskBPB,0,sizeof(diskBPB));
    diskBPB.jump[0] = 0xEB; diskBPB.jump[1] = 0x00;diskBPB.jump[2] = 0x90 ;
    memcpy(diskBPB.oem_id,"MSWIN4.1",8);
    diskBPB.bytes_per_sector =512;
    diskBPB.sectors_per_cluster = 1;
    diskBPB.num_boot_sectors=1;
    diskBPB.num_fats=2;
    diskBPB.num_root_dir_ents=224;
    diskBPB.total_sectors=0xBF0;
    diskBPB.media_ID_byte=0xF0;
    diskBPB.sectors_per_fat   = 0x9;
    diskBPB.sectors_per_track = 0x12;
    diskBPB.heads= 2;
    diskBPB.magic[0]=0x55;
    diskBPB.magic[1]=0xaa;
    
    write_block(0,(char*)&diskBPB,1);
    
    kprintf("allocating FAT..\n");
    FAT = (char*)kmalloc(512*(diskBPB.sectors_per_fat+1));
    sprintf(temp,"%X",FAT);
    kprintf("buffer located at %s\n", temp);

    memset(FAT,0,512*diskBPB.sectors_per_fat);
    
    kprintf("writing to FAT..");
    for (i=0;i<13;i++)
     {
          writecluster(i,i+1,FAT);
     };
     
    writecluster(13,0x0FF8,FAT);
    
    kprintf("formatting..\n");
    //write FAT to disk
    write_block(diskBPB.num_boot_sectors,(void*)FAT,diskBPB.sectors_per_fat);
    write_block(diskBPB.num_boot_sectors+diskBPB.sectors_per_fat,
                                         (void*)FAT,diskBPB.sectors_per_fat);
    kprintf("done.\n");
    kfree(FAT);

    
    return 1;
};

int getcache(char *buf,DWORD block,DWORD numblocks)
{
    read_block(block,buf,numblocks);
    return 1;
};

int putcache(char *buf,DWORD block,DWORD numblocks)
{
    write_block(block,buf,numblocks);
    return 1;
};

int read_block(int block,char *blockbuff,DWORD numblocks)
{
int i,ofs=0,res;

        
for (i=0;i<numblocks;i++)
{
    if (block<RAMDISK_SIZE)
        memcpy(blockbuff + ofs,&ramdiskmem[ block + i ].data , BLOCK_SIZE);
    else
    return -1;
    
    ofs+=BLOCK_SIZE;
};    
return 1;
};

int write_block(int block,char *blockbuff,DWORD numblocks)
{
int i,res;
DWORD ofs;
ofs = 0;
for (i=0;i<numblocks;i++)
{
    res = 0;
    if (block + i <RAMDISK_SIZE)
       {
         memcpy(&ramdiskmem[ block + i ].data, blockbuff + ofs, BLOCK_SIZE);
       } 
      else
    return -1;
    
    ofs+=BLOCK_SIZE;
};
return 1;
};

int total_blocks()
{
    return RAMDISK_SIZE;
};

int get_block_size()
{
    return BLOCK_SIZE;
};



void device_init( devmgr_interface *devmgr,int c,char *args[])
{
    char temp[256];
    char name[256];
    char *p;
    int i;
    
    strcpy(name,"ramdisk");
    
    for (i=0; i < c ;i++)
    {
        if (strcmp(args[i],"-name")==0)
          {
                if (i + 1 < c)    
                {
                strcpy(name,args[i + 1]);
                i++;
                };
          }
        else  
        if (strcmp(args[i],"-blocks")==0)
          {
              if (i + 1 < c)    
               {
                 RAMDISK_SIZE = atoi( args[i+1] );
                 i++;
               };
          };
    };
    
    
    
   
    kprintf("Dayosoft Ramdisk Driver 1.02\n");
    ramdiskmem = kmalloc(RAMDISK_SIZE*512);
    
    kprintf("RamDisk memory alloated at 0x%x\n", ramdiskmem);
    //allocate 2MB of data for the ramdisk
    init_device();
    memset(&myinterface,0,sizeof(devmgr_block_desc));
    myinterface.hdr.size=sizeof(devmgr_block_desc);
    kprintf("Ramdisk installed as %s\n",name);
    strcpy(myinterface.hdr.name,name);
    strcpy(myinterface.hdr.description,"Virtual Block Device");
    myinterface.hdr.type = DEVMGR_BLOCK;
    myinterface.read_block = read_block;
    myinterface.write_block =write_block;
    myinterface.total_blocks= total_blocks;
    myinterface.get_block_size= get_block_size;
    myinterface.getcache = getcache;
    myinterface.putcache = putcache;
    ramdisk_handle = devmgr_register((devmgr_generic*)&myinterface);
    if (ramdisk_handle != -1)
    {
        kprintf("Initialization successful!\n");
        kprintf("Driver was assigned handle %d\n",ramdisk_handle);
    }
        else
    {
        kprintf("RamDisk was unable to register itself to the\n");
        kprintf("device manager. A device with the same name\n");
        kprintf("might be present, use the -name parameter to\n");
        kprintf("assign your own name.\n");
    };
};

DLLIMPORT int dex32_libmain(devmgr_interface *devmgr,char *parameter)
{
   char *s;
   int c=0;
   char *p[100];
   char params[500];
   
   devmgr_stdlib stdlib;
   devmgr_block_desc myinterface;
   
   strcpy(params,parameter);
   s=strtok(params," ");
   do {
    p[c]=s;
    c++;
    s=strtok(0," ");
   } while (s!=0);
   
   
   stdlib.hdr.size=sizeof(devmgr_stdlib);
   devmgr->devmgr_copyinterface("stdlib",(devmgr_generic*) &stdlib);
   
   kfree = stdlib.free;
   kmalloc = stdlib.malloc;
   kprintf = stdlib.printf;
   krealloc = stdlib.realloc;
   
   devmgr_register = devmgr->devmgr_register;
   devmgr_copyinterface = devmgr->devmgr_copyinterface;
   
   device_init(devmgr,c,p);
};



int main2(int argc, char *argv[])
{
    printf("Ramdisk v 1.00 - use loadmod to install\n");
};
