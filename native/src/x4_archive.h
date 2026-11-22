#ifndef X4_ARCHIVE_H
#define X4_ARCHIVE_H
#include <stdint.h>
#include <stddef.h>
/* Original ARC data containers, independent of CD hardware and PS1 memory. */
typedef struct {uint32_t type;const uint8_t* data;size_t size;} X4ArchiveMember;
typedef struct {uint8_t* storage;size_t size;unsigned count;X4ArchiveMember members[128];} X4Archive;
enum {X4_ARC_OK=0,X4_ARC_IO=-1,X4_ARC_NOMEM=-2,X4_ARC_FORMAT=-3};
typedef struct {uint32_t type;size_t offset,size;} X4ArchiveEntry;
int x4_archive_directory(X4ArchiveEntry entries[128],unsigned* count,const uint8_t header[2048],size_t size);
int x4_archive_parse(X4Archive* archive,uint8_t* data,size_t size);
/* Some retail .ARC filenames are raw font/sequence banks, not containers.
 * Caller owns the returned buffer and must free it. */
int x4_resource_load(uint8_t** data,size_t* size,const char* path);
int x4_archive_load(X4Archive* archive,const char* path);
const X4ArchiveMember* x4_archive_find(const X4Archive* archive,uint32_t type,unsigned occurrence);
void x4_archive_free(X4Archive* archive);
#endif
