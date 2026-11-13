#ifndef X4_CONTENT_H
#define X4_CONTENT_H
#include "x4_archive.h"
/* Owned native resource, including the eight original non-container banks.
 * Zero-initialize before opening. Failed replacement preserves the old data.
 * Do not retain game pointers across a successful replacement or close. */
typedef struct {
    X4Archive archive;
    unsigned id;
    int is_container;
} X4Content;
const char* x4_content_name(unsigned id);
size_t x4_content_expected_size(unsigned id);
int x4_content_open(X4Content* content,const char* root,unsigned id);
int x4_content_copy(const X4Content* content,void* destination,size_t capacity);
void x4_content_close(X4Content* content);
#endif
