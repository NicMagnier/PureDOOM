
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "playdate.h"

void PD_LogFile( char* str )
{
	SDFile* fHandle = playdate->file->open( "log.txt", kFileAppend );

	int len = 0;
	while (*str++) ++len;

	playdate->file->write( fHandle, str, len );
	playdate->file->close( fHandle );
}

void* PD_malloc(size_t size)
{
	return playdate->system->realloc(NULL, size);
}

void* PD_calloc(size_t num, size_t size)
{
	void* result = playdate->system->realloc(NULL, num*size);

	if (result==NULL )
		return NULL;

	return memset(result, 0, num*size);
}

void* PD_realloc(void *ptr, size_t size)
{
	return playdate->system->realloc(ptr, size);
}


void  PD_free(void *ptr)
{
	playdate->system->realloc(ptr, 0);
}

void* PD_open(const char* filename, const char* mode)
{
	FileOptions fo = kFileRead;
	switch ( mode[0] )
	{
	case 'r':
		fo = kFileRead;
		break;
	case 'w':
		fo = kFileWrite;
		break;
	case 'a':
		fo = kFileAppend;
		break;
	}

	return (void*)playdate->file->open( filename, fo);
}

void PD_close(void* handle)
{	
	playdate->file->close( (SDFile*)handle );
}

int PD_read(void* handle, void *buf, int count)
{
	return playdate->file->read( (SDFile*)handle, buf, (unsigned int)count );
}

int PD_write(void* handle, const void *buf, int count)
{
	return playdate->file->write( (SDFile*)handle, buf, (unsigned int)count );
}

int PD_seek(void* handle, int offset, int origin)
{
	return playdate->file->seek( (SDFile*)handle, offset, origin );
}

int PD_flush(void* handle)
{
	return playdate->file->flush( (SDFile*)handle );
}

int PD_tell(void* handle)
{
	return playdate->file->tell( (SDFile*)handle );
}

int PD_eof(void* handle)
{
	int fpos = playdate->file->tell( (SDFile*)handle );
	char read_buffer;
	int result = playdate->file->read( (SDFile*)handle, (void*)&read_buffer, 1 );
	playdate->file->seek( (SDFile*)handle, fpos, SEEK_SET );

	return !result;
}