#include "x4_content.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum {RESOURCE_COUNT=138, NAME_BYTES=16, RECORD_BYTES=NAME_BYTES+5};
static struct {char name[NAME_BYTES];size_t size;int container;} index_table[RESOURCE_COUNT];
static int index_loaded;

int x4_content_load_index(const char* root){
    index_loaded=0;
    if(!root)return X4_ARC_FORMAT;
    char path[2048];
    int length=snprintf(path,sizeof path,"%s/resources.idx",root);
    if(length<0||(size_t)length>=sizeof path)return X4_ARC_FORMAT;
    FILE* file=fopen(path,"rb");
    if(!file)return X4_ARC_IO;
    int result=X4_ARC_FORMAT;
    for(unsigned i=0;i<RESOURCE_COUNT;++i){
        uint8_t record[RECORD_BYTES];
        if(fread(record,1,sizeof record,file)!=sizeof record)goto done;
        size_t name_length=0;
        while(name_length<NAME_BYTES&&record[name_length]){
            unsigned c=record[name_length++];
            if(!((c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_'||c=='.'))goto done;
        }
        if(!name_length||name_length==NAME_BYTES||record[0]=='.')goto done;
        unsigned offset=NAME_BYTES;
        uint32_t size=(uint32_t)record[offset]|(uint32_t)record[offset+1]<<8|
                      (uint32_t)record[offset+2]<<16|(uint32_t)record[offset+3]<<24;
        unsigned container=record[offset+4];
        if(!size||size>16*1024*1024||container>1)goto done;
        memcpy(index_table[i].name,record,name_length+1);
        index_table[i].size=size;
        index_table[i].container=container;
    }
    if(fgetc(file)==EOF&&!ferror(file)){index_loaded=1;result=X4_ARC_OK;}
done:
    fclose(file);
    return result;
}

const char* x4_content_name(unsigned id){return index_loaded&&id<RESOURCE_COUNT?index_table[id].name:NULL;}
size_t x4_content_expected_size(unsigned id){return index_loaded&&id<RESOURCE_COUNT?index_table[id].size:0;}
int x4_content_is_container(unsigned id){return index_loaded&&id<RESOURCE_COUNT&&index_table[id].container;}
void x4_content_close(X4Content* c){x4_archive_free(&c->archive);memset(c,0,sizeof *c);}
int x4_content_open(X4Content* c,const char* root,unsigned id){
    if(!c||!root||!index_loaded||id>=RESOURCE_COUNT)return X4_ARC_FORMAT;
    char path[2048];int n=snprintf(path,sizeof path,"%s/%s",root,index_table[id].name);
    if(n<0||(size_t)n>=sizeof path)return X4_ARC_FORMAT;
    uint8_t* data;size_t size;int status=x4_resource_load(&data,&size,path);
    if(status)return status;
    if(size!=index_table[id].size){free(data);return X4_ARC_FORMAT;}
    X4Content next={0};next.id=id;next.is_container=index_table[id].container;
    if(next.is_container){
        status=x4_archive_parse(&next.archive,data,size);
        if(status){free(data);return status;}
    }else next.archive.size=size;
    next.archive.storage=data;
    x4_content_close(c);*c=next;return 0;
}
int x4_content_copy(const X4Content* c,void* dst,size_t capacity){
    if(!c||!c->archive.storage||!dst||capacity<c->archive.size)return X4_ARC_FORMAT;
    memcpy(dst,c->archive.storage,c->archive.size);return 0;
}
