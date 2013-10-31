#include "semaphore.h"
int sema_init(sema_t* s,const char* name,unsigned int value,unsigned int max,SECURITY_ATTRIBUTES* sa)
{
#ifdef _POSIX_SEM
	if(name){
		s->name_ = strdup(name);
		if(0==s->name_) 
			return -1;
		s->proc_sem_ = sem_open(name,O_CREAT,DEFAULT_FILE_PERMS,value);
		if(SEM_FAILED==s->proc_sem_) {
			free(s->name_);
			return -1;
		}
	}else{
		if(-1==sem_init(&s->thr_sem_,0,value))
			return -1;
		s->name_ = 0;
	}
	return 0;
#elif defined(_SYSV_SEM)
	if(name){
		s->name_ = strdup(name);
		if(0==s->name_)
			return -1;
		if(-1==__sysv_sem_open(&s->id_,name,value)){
			free(s->name_);
			return -1;
		}
		return 0;
	}else{
		if(-1==__sysv_init(&s->id_,value))
			return -1;
		s->name_ = 0;
	}
	return 0;
#elif defined(_PTHREAD_SEM)
	if(name){
		s->name_ = strdup(name);
		if(0==s->name_)
			return -1;
		s->proc_sem_ = __pthread_sem_open(name,value);
		if(0==s->proc_sem_){
			free(s->name_);
			return -1;
		}
	}else{
		if(-1==__pthread_init(&s->thr_sem_,value))
			return -1;
		s->name_ = 0;
	}
	return 0;
#else
	return (*s = CreateSemaphoreA(sa,value,max,name)) ? 0 : -1;
#endif
}

int sema_wait(sema_t* s)
{
#ifdef _POSIX_SEM
	sem_t* sem = s->name_ ? s->proc_sem_ : &s->thr_sem_;
	return sem_wait(sem);
#elif defined(_SYSV_SEM)
	struct sembuf op;
	int ret;
	op.sem_num = 0;
	op.sem_op = -1;
	op.sem_flg = 0;
	return semop(s->id_, &op, 1);
#elif defined(_PTHREAD_SEM)
	sem_t* sem = s->name_ ? s->proc_sem_ : &s->thr_sem_;

	int ret = pthread_mutex_lock(&sem->lock_);
	if(ret) {
		errno = ret; return -1;
	}
	while(0==sem->value_)
		pthread_cond_wait(&sem->cond_,&sem->lock_);
	--sem->value_;
	pthread_mutex_unlock(&sem->lock_);

	return 0;
#else
	switch (WaitForSingleObject(*s, INFINITE))
	{
	case WAIT_OBJECT_0:  return 0;
	case WAIT_ABANDONED: return 1;
	default: return -1;
	}
#endif
}

int sema_trywait(sema_t* s)
{
#ifdef _POSIX_SEM
	sem_t* sem = s->name_ ? s->proc_sem_ : &s->thr_sem_;
	return sem_trywait(sem);
#elif defined(_SYSV_SEM)
	struct sembuf op;
	op.sem_num = 0;
	op.sem_op = -1;
	op.sem_flg = IPC_NOWAIT;
	return semop(s->id_, &op, 1);
#elif defined(_PTHREAD_SEM)
	sem_t* sem = s->name_ ? s->proc_sem_ : &s->thr_sem_;

	int ret = pthread_mutex_lock(&sem->lock_);
	if(ret) {
		errno = ret; return -1;
	}
	if(0==sem->value_){
		ret = -1; errno = EAGAIN;
	}else {
		ret = 0; --sem->value_;
	}
	pthread_mutex_unlock(&sem->lock_);

	return ret;
#else
	switch (WaitForSingleObject (*s, 0))
	{
	case WAIT_OBJECT_0:  return 0;
	case WAIT_ABANDONED: return 1;
	case WAIT_TIMEOUT:   return 2;
	default: return -1;
	}
#endif
}

int sema_post(sema_t* s)
{
#ifdef _POSIX_SEM
	sem_t* sem = s->name_ ? s->proc_sem_ : &s->thr_sem_;
	return sem_post(sem);
#elif defined(_SYSV_SEM)
	struct sembuf op;
	op.sem_num = 0;
	op.sem_op = 1;
	op.sem_flg = 0;
	return semop(s->id_, &op, 1);
#elif defined(_PTHREAD_SEM)
	sem_t* sem = s->name_ ? s->proc_sem_ : &s->thr_sem_;

	pthread_mutex_lock(&sem->lock_);
	if(0==sem->value_)
		pthread_cond_signal(&sem->cond_);
	++sem->value_;
	pthread_mutex_unlock(&sem->lock_);

	return 0;
#else
	return ReleaseSemaphore(*s,1,0) ? 0 : -1;
#endif
}

int sema_getvalue(sema_t* s,int* val)
{
#ifdef _POSIX_SEM
	sem_t* sem = s->name_ ? s->proc_sem_ : &s->thr_sem_;
	return sem_getvalue(sem,val);
#elif defined(_SYSV_SEM)
	int tmp = semctl(s->id_,0,GETVAL);
	if(tmp < 0) return -1;
	*val = tmp;
	return 0;
#elif defined(_PTHREAD_SEM)
	sem_t* sem = s->name_ ? s->proc_sem_ : &s->thr_sem_;
	int ret = pthread_mutex_lock(&sem->lock_);
	if(ret){
		errno = ret; return -1;
	}
	*val = sem->value_;
	pthread_mutex_unlock(&sem->lock_);
#else
	return -1;
#endif
}

int sema_destroy(sema_t* s)
{
#ifdef _POSIX_SEM
	if(s->name_){    
		sem_unlink(s->name_);
		free(s->name_);
		if(-1==sem_close(s->proc_sem_))
			return -1;
	}else{
		if(-1==sem_destroy(&s->thr_sem_))
			return -1;
	}
	return 0;
#elif defined(_SYSV_SEM)
	return semctl(s->id_,0,IPC_RMID);
#elif defined(_PTHREAD_SEM)
	if(s->name_) {
		sem_t* sem = s->proc_sem_;

		unlink(s->name_);
		free(s->name_);
		pthread_mutex_destroy(&sem->lock_);
		pthread_cond_destroy(&sem->cond_);

		return munmap(sem,sizeof(sem_t));
	}else {
		pthread_mutex_destroy(&s->thr_sem_.lock_);
		pthread_cond_destroy(&s->thr_sem_.cond_);
	}
	return 0;
#else
	return CloseHandle(*s) ? 0 : -1;
#endif
}