#ifndef _DSM_H
#define _DSM_H

#include "mrporting.h"

#define DSM_MAX_FILE_LEN 256
#define DSM_MEM_SIZE (5 * 1024 * 1024)  //DSM内存大小
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// 需要平台实现的函数
typedef struct {
    void (*panic)(char *msg);
    void (*log)(char *msg);
    void (*exit)(void);
    void (*srand)(uint32 seed);
    int32 (*rand)(void);
    int32 (*mem_get)(char **mem_base, uint32 *mem_len);
    int32 (*mem_free)(char *mem, uint32 mem_len);
    int32 (*timerStart)(uint16 t);
    int32 (*timerStop)(void);
    int64 (*get_time_ms)(void);
    int32 (*getDatetime)(mr_datetime *datetime);
    int32 (*sleep)(uint32 ms);
    int32 (*open)(const char *filename, uint32 mode);
    int32 (*close)(int32 f);
    int32 (*read)(int32 f, void *p, uint32 l);
    int32 (*write)(int32 f, void *p, uint32 l);
    int32 (*seek)(int32 f, int32 pos, int method);
    int32 (*info)(const char *filename);
    int32 (*remove)(const char *filename);
    int32 (*rename)(const char *oldname, const char *newname);
    int32 (*mkDir)(const char *path);
    int32 (*rmDir)(const char *path);
    int32 (*opendir)(const char *name);
    char *(*readdir)(int32 f);
    int32 (*closedir)(int32 f);
    int32 (*getLen)(const char *filename);
    void (*drawBitmap)(uint16 *bmp, int16 x, int16 y, uint16 w, uint16 h);

} DSM_REQUIRE_FUNCS;

// 平台可以调用的函数
typedef struct {
    int32 (*mr_start_dsm)(const char *entry);
    int32 (*mr_pauseApp)(void);
    int32 (*mr_resumeApp)(void);
    int32 (*mr_timer)(void);
    int32 (*mr_event)(int16 type, int32 param1, int32 param2);
} DSM_EXPORT_FUNCS;

DSM_EXPORT_FUNCS *dsm_init(DSM_REQUIRE_FUNCS *inFuncs);

#endif
