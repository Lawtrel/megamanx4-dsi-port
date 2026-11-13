#include "x4_content.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const struct {const char* name;size_t size;int container;} index_table[]={
#include "x4_content_index.inc"
};
const char* x4_content_name(unsigned id){return id<138?index_table[id].name:NULL;}
size_t x4_content_expected_size(unsigned id){return id<138?index_table[id].size:0;}
void x4_content_close(X4Content* c){x4_archive_free(&c->archive);memset(c,0,sizeof *c);}
int x4_content_open(X4Content* c,const char* root,unsigned id){
    if(!c||!root||id>=138)return X4_ARC_FORMAT;
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
