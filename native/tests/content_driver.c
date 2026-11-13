#include "x4_content.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc,char** argv){
    assert(argc==2);X4Content c={0};unsigned containers=0,raw=0;size_t bytes=0;
    for(unsigned repeat=0;repeat<3;++repeat)for(unsigned id=0;id<138;++id){
        assert(!x4_content_open(&c,argv[1],id));assert(c.id==id);
        assert(c.archive.size==x4_content_expected_size(id));
        if(!repeat){bytes+=c.archive.size;if(c.is_container)++containers;else ++raw;}
        unsigned char* copy=malloc(c.archive.size);assert(copy);
        assert(x4_content_copy(&c,copy,c.archive.size-1)==X4_ARC_FORMAT);
        assert(!x4_content_copy(&c,copy,c.archive.size));
        assert(!memcmp(copy,c.archive.storage,c.archive.size));free(copy);
        /* Neither an invalid ID nor a failed read may invalidate live banks. */
        void* before=c.archive.storage;unsigned previous=c.id;
        assert(x4_content_open(&c,argv[1],138)==X4_ARC_FORMAT);
        assert(x4_content_open(&c,"/missing-native-content",id)==X4_ARC_IO);
        assert(c.archive.storage==before&&c.id==previous);
    }
    x4_content_close(&c);x4_content_close(&c);assert(!c.archive.storage);
    assert(!x4_content_name(138)&&!x4_content_expected_size(138));
    printf("PASS 414 native resource replacements; %u containers, %u raw banks, %zu original bytes; copy bounds and rollback\n",containers,raw,bytes);
}
