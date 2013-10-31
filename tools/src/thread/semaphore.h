#ifndef _SAMAPHORE_H_
#define _SAMAPHORE_H_
#include "../shared/Define.h"
#ifdef _POSIX
#define _POSIX_SEM
#elif defined _LINUX
#define _PTHREAD_SEM
#elif defined WIN32
#define  _WIN32_SEM
#elif defined _SYSV
#define _SYSV_SEM
#endif

#ifdef _POSIX_SEM
#include <semaphore.h>
typedef struct 
{
	union 
	{
		sem_t* proc_sem_;
		sem_t thr_sem_;
	};
	char* name_;
} sema_t;

typedef void SECURITY_ATTRIBUTES;

#elif defined(_SYSV_SEM)
typedef struct 
{
	int  id_;
	char* name_;
}sema_t;

typedef void SECURITY_ATTRIBUTES;

#elif defined(_PTHREAD_SEM)
#include <pthread.h>
typedef struct 
{
	pthread_cond_t cond_;
	pthread_mutex_t lock_;
	int value_;
}sem_t;

typedef struct
{
	union {
		sem_t* proc_sem_;
		sem_t thr_sem_;
	};
	char* name_;
}sema_t;

typedef void SECURITY_ATTRIBUTES;

#elif defined(_WIN32_SEM)
#include <windows.h>
typedef HANDLE sema_t;

#else
#error Currently only support posix,system v,pthread and win32 semaphore.
#endif

int sema_init(sema_t* s,const char* name,unsigned int value,unsigned int max,SECURITY_ATTRIBUTES* sa);

int sema_wait(sema_t* s);

int sema_trywait(sema_t* s);

int sema_post(sema_t* s);

int sema_getvalue(sema_t*s,int* val);

int sema_destroy(sema_t* s);

#endif