#ifndef PLAYDATE_REF_H
#define PLAYDATE_REF_H

#include "pd_api.h"

extern PlaydateAPI* playdate;

#define PD_log playdate->system->logToConsole

void PD_LogFile( char* str);

void* PD_malloc(size_t size);
void* PD_calloc(size_t num, size_t size);
void* PD_realloc(void *ptr, size_t size);
void  PD_free(void *ptr);


void* PD_open(const char* filename, const char* mode);
void PD_close(void* handle);
int PD_read(void* handle, void *buf, int count);
int PD_write(void* handle, const void *buf, int count);
int PD_seek(void* handle, int offset, int origin);
int PD_flush(void* handle);
int PD_tell(void* handle);
int PD_eof(void* handle);


#endif