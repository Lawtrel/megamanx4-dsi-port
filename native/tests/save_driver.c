#include "common.h"
#include "x4_save.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
int main(int argc,char** argv){
 assert(argc==2);assert(mmx4_pc_card_info(0)==1);assert(!x4_save_set_root(argv[1]));
 unsigned char data[8192],readback[8192];for(unsigned i=0;i<sizeof data;++i)data[i]=i*37;
 assert(!mmx4_pc_card_format(0));assert(mmx4_pc_card_read(0,readback,sizeof readback)==1);
 assert(!mmx4_pc_card_write(0,data,sizeof data));assert(!mmx4_pc_card_read(0,readback,sizeof readback));assert(!memcmp(data,readback,sizeof data));
 unsigned char patch[16];memset(patch,0xaa,sizeof patch);assert(!mmx4_pc_card_write(0,patch,sizeof patch));
 memcpy(data,patch,sizeof patch);assert(!mmx4_pc_card_read(0,readback,sizeof readback));assert(!memcmp(data,readback,sizeof data));
 struct MemcardFileList list;mmx4_pc_card_list(0,&list,"");assert(list.count==1&&list.total_size==8192);
 /* Tear the newer journal's payload: recover the preceding complete save. */
 char path[1024];snprintf(path,sizeof path,"%s/mmx4_save0_a.dat",argv[1]);FILE* f=fopen(path,"wb");assert(f);fwrite("torn",1,4,f);fclose(f);
 assert(!mmx4_pc_card_read(0,readback,sizeof readback));for(unsigned i=0;i<8192;++i)assert(readback[i]==(unsigned char)(i*37));
 assert(mmx4_pc_card_write(2,data,16)==2&&mmx4_pc_card_read(0,readback,8193)==2);
 assert(!mmx4_pc_card_write(1,data,sizeof data));assert(!mmx4_pc_card_format(0));
 assert(mmx4_pc_card_read(0,readback,16)==1);assert(!mmx4_pc_card_read(1,readback,16));
 puts("PASS native save roundtrip, partial write, slot isolation, journal CRC recovery and format tombstone");
}
