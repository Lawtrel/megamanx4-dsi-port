#include "x4_archive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint32_t read32(const uint8_t* p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
int x4_archive_directory(X4ArchiveEntry entries[128],unsigned* count,const uint8_t data[2048],size_t size){
    if(!data||size<2048||size%2048)return X4_ARC_FORMAT;
    unsigned total=read32(data);
    if(total>128||read32(data+4)!=size)return X4_ARC_FORMAT;
    size_t offset=2048;
    for(unsigned i=0;i<total;++i){
        size_t n=read32(data+12+i*8);
        if(n>size-offset)return X4_ARC_FORMAT;
        entries[i]=(X4ArchiveEntry){read32(data+8+i*8),offset,n};
        size_t padded=(n+2047)&~(size_t)2047;
        if(padded>size-offset)return X4_ARC_FORMAT;
        offset+=padded;
    }
    if(offset!=size)return X4_ARC_FORMAT;
    *count=total;return X4_ARC_OK;
}
int x4_archive_parse(X4Archive* a,uint8_t* data,size_t size){
    memset(a,0,sizeof *a);
    X4ArchiveEntry entries[128];unsigned count;
    int result=x4_archive_directory(entries,&count,data,size);
    if(result)return result;
    for(unsigned i=0;i<count;++i)
        a->members[i]=(X4ArchiveMember){entries[i].type,data+entries[i].offset,entries[i].size};
    a->count=count;a->size=size;return X4_ARC_OK;
}
int x4_resource_load(uint8_t** data,size_t* size,const char* path) {
    *data=NULL;*size=0;FILE* f=fopen(path,"rb");if(!f)return X4_ARC_IO;
    if(fseek(f,0,SEEK_END)){fclose(f);return X4_ARC_IO;}
    long n=ftell(f);rewind(f);
    if(n<=0||n>16*1024*1024){fclose(f);return X4_ARC_FORMAT;}
    uint8_t* p=malloc(n);if(!p){fclose(f);return X4_ARC_NOMEM;}
    size_t got=fread(p,1,n,f);fclose(f);
    if(got!=(size_t)n){free(p);return X4_ARC_IO;}
    *data=p;*size=n;return X4_ARC_OK;
}
int x4_archive_load(X4Archive* a,const char* path) {
    memset(a,0,sizeof *a);uint8_t* p;size_t n;
    int status=x4_resource_load(&p,&n,path);if(status)return status;
    status=x4_archive_parse(a,p,n);
    if(status){free(p);memset(a,0,sizeof *a);return status;}
    a->storage=p;return X4_ARC_OK;
}
const X4ArchiveMember* x4_archive_find(const X4Archive* a,uint32_t type,unsigned occurrence) {
    for(unsigned i=0;i<a->count;++i)if(a->members[i].type==type) {
        if(!occurrence)return &a->members[i];
        --occurrence;
    }
    return NULL;
}
void x4_archive_free(X4Archive* a){free(a->storage);memset(a,0,sizeof *a);}
