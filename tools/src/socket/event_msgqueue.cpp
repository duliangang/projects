#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../thread/base.h"

#include "event_msgqueue.h"

struct circqueue {
   unsigned int head;
   unsigned int tail;
   unsigned int count;
   unsigned int max_entries;
   unsigned int array_elements;
   void **entries;
};

#define DEFAULT_UNBOUNDED_QUEUE_SIZE 1024

struct event_msgqueue {
   int push_fd;
   int pop_fd;
   int unlock_between_callbacks;

   struct event queue_ev;

   Thread_Mutex lock;
   void (*callback)(void *, void *);
   void *cbarg;
   struct circqueue *queue;
};

static unsigned int nextpow2(unsigned int num) {
    --num;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    return ++num;
}

#define circqueue_get_length(q) ((q)->count)
#define circqueue_is_empty(q) (!circqueue_get_length(q))
#define circqueue_is_full(q) ((q)->count == (q)->array_elements)

static struct circqueue *circqueue_new(unsigned int size) {
   struct circqueue *cq;

   if (!(cq = (circqueue*)calloc(1, sizeof(struct circqueue))))
      return(NULL);

   cq->max_entries = size;
   if (!size || !(cq->array_elements = nextpow2(size)))
      cq->array_elements = DEFAULT_UNBOUNDED_QUEUE_SIZE;
   cq->entries = (void**)malloc(sizeof(void *) * cq->array_elements);
   if (!cq->entries) {
      free(cq);
      return(NULL);
   }

   return(cq);
}

static void circqueue_destroy(struct circqueue *cq) {
   free(cq->entries);
   free(cq);
}

static int circqueue_grow(struct circqueue *cq) {
   void **newents;
   unsigned int newsize = cq->array_elements << 1;
   unsigned int headchunklen = 0, tailchunklen = 0;
   
   if (!(newents = (void**)malloc(sizeof(void*) * newsize)))
      return(-1);

   if (cq->head < cq->tail)
      headchunklen = cq->tail - cq->head;
   else {
      headchunklen = cq->array_elements - cq->head;
      tailchunklen = cq->tail;
   }

   memcpy(newents, &cq->entries[cq->head], sizeof(void *) * headchunklen);
   if (tailchunklen)
      memcpy(&newents[headchunklen], cq->entries, sizeof(void *) * tailchunklen);

   cq->head = 0;
   cq->tail = headchunklen + tailchunklen;
   cq->array_elements = newsize;

   free(cq->entries);
   cq->entries = newents;

   return(0);
}

static int circqueue_push_tail(struct circqueue *cq, void *elem) {
   if (cq->max_entries) {
      if (cq->count == cq->max_entries)
         return(-1);
   } else if (circqueue_is_full(cq) && circqueue_grow(cq) != 0) 
      return(-1);

   cq->count++;
   cq->entries[cq->tail++] = elem;
   cq->tail &= cq->array_elements - 1;

   return(0);
}

static void *circqueue_pop_head(struct circqueue *cq) {
   void *data;

   if (!cq->count)
      return(NULL);

   cq->count--;
   data = cq->entries[cq->head++];
   cq->head &= cq->array_elements - 1;

   return(data);
}

static void msgqueue_pop(int fd, short flags, void *arg) {
   struct event_msgqueue *msgq = (struct event_msgqueue*)arg;
   char buf[64];

   recv(fd, buf, sizeof(buf),0);

   msgq->lock.acquire();
   while(!circqueue_is_empty(msgq->queue)) {
      void *qdata;

      qdata = circqueue_pop_head(msgq->queue);

      if (msgq->unlock_between_callbacks)
		  msgq->lock.release();
        

      msgq->callback(qdata, msgq->cbarg);

      if (msgq->unlock_between_callbacks)
        msgq->lock.acquire();
   }
   msgq->lock.release();
}

struct event_msgqueue *msgqueue_new(struct event_base *base, unsigned int max_size, void (*callback)(void *, void *), void *cbarg) {
   struct event_msgqueue *msgq;
   struct circqueue *cq;
   int fds[2];

   if (!(cq = circqueue_new(max_size)))
      return(NULL);

   if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) {
      circqueue_destroy(cq);
      return(NULL);
   }

   if (!(msgq =(event_msgqueue*) malloc(sizeof(struct event_msgqueue)))) {
      circqueue_destroy(cq);
      evutil_closesocket(fds[0]);
      evutil_closesocket(fds[1]);
      return(NULL);
   }

   msgq->push_fd = fds[0];
   msgq->pop_fd = fds[1];
   msgq->queue = cq;
   msgq->callback = callback;
   msgq->cbarg = cbarg;
   //sp_thread_mutex_init(&msgq->lock, NULL);
   event_set(&msgq->queue_ev, msgq->pop_fd, EV_READ | EV_PERSIST, msgqueue_pop, msgq);
   event_base_set(base, &msgq->queue_ev);
   event_add(&msgq->queue_ev, NULL);

   msgq->unlock_between_callbacks = 1;
   
   return(msgq);
}

void msgqueue_destroy(struct event_msgqueue *msgq)
{
   for( ; msgqueue_length(msgq) > 0; ) {
#if WIN32 
      Sleep( 1000);
#else 
	   sleep(1);
#endif
   }

   event_del(&msgq->queue_ev);
   circqueue_destroy(msgq->queue);
   evutil_closesocket(msgq->push_fd);
   evutil_closesocket(msgq->pop_fd);
   free(msgq);
}

int msgqueue_push(struct event_msgqueue *msgq, void *msg) {
   const char buf[1] = { 0 };
   int r = 0;
   msgq->lock.acquire();
   if ((r = circqueue_push_tail(msgq->queue, msg)) == 0) {
      if (circqueue_get_length(msgq->queue) == 1)
         send(msgq->push_fd, buf, 1,0);
   }
     msgq->lock.release();
   return(r);
}

unsigned int msgqueue_length(struct event_msgqueue *msgq) {
   unsigned int len;

	 msgq->lock.acquire();
   len = circqueue_get_length(msgq->queue);
    msgq->lock.release();
   return(len);
}