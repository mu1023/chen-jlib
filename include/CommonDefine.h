#ifndef _COMMON_DEFINE_H_
#define _COMMON_DEFINE_H_

#include<stdio.h>

#if (defined _WIN32) || (defined _WIN64) || (defined WIN32)||(defined WIN64)
#define WINDOWS_FLAG 1
#elif (defined __linux__) ||(defined __unix__) || #defined(linux)||(defined unix)
#define UNIX_FLAG 1
#endif
	///类型定义
	typedef unsigned char			UInt8;
	typedef char					Int8;
	typedef unsigned short			UInt16;
	typedef short					Int16;
	typedef int						Int32;
	typedef unsigned int			UInt32;

#ifdef WINDOWS_FLAG
	typedef __int64					Int64;
	typedef unsigned __int64		UInt64;
#else
	typedef long long				Int64;
	typedef unsigned long long		UInt64;
#endif


	//杂项
#ifdef WINDOWS_FLAG

	const char FOLDER_SEP = '\\';

#define Localtime(x,y) (localtime_s(x,y))

#define fwriteUnlock(buffer,size,count,stream)  _fwrite_nolock(buffer,size,count,stream);

#else 

	const char FOLDER_SEP = '/';

#define Localtime(x,y) (localtime_r(y,x))

#define fwriteUnlock(buffer,size,count,stream)  fwrite_unlocked(buffer,size,count,stream);

#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),(mode)))==nullptr
#endif
#endif // !_COMMON_DEFINE_H_
