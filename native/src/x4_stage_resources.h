#ifndef X4_STAGE_RESOURCES_H
#define X4_STAGE_RESOURCES_H
#include "x4_archive.h"
/* Loads no files and takes no ownership. Success means resource bindings only,
 * not that stage scripts, rendering, enemies or audio have been activated. */
int x4_stage_install_resources(const X4Archive* player,const X4Archive* stage,
                               const X4Archive* palette,unsigned stage_id,unsigned substage);
#endif
