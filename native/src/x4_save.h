#ifndef X4_SAVE_H
#define X4_SAVE_H
/* Native save directory must already exist. No memory-card image or BIOS.
 * Two journal files per slot keep the preceding valid save after a torn write. */
int x4_save_set_root(const char* directory);
struct MemcardFileList;
int mmx4_pc_card_info(int port);
int mmx4_pc_card_load(int port);
int mmx4_pc_card_format(int port);
void mmx4_pc_card_list(int port,struct MemcardFileList* list,const char* pattern);
int mmx4_pc_card_read(int port,void* buffer,int size);
int mmx4_pc_card_write(int port,const void* buffer,int size);
#endif
