/* A simple event-driven programming library. Originally I wrote this code
 * for the Jim's event-loop (Jim is a Tcl interpreter) but later translated
 * it in form of a library for easy reuse.
 *
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __AE_H__
#define __AE_H__

#include <time.h>

#define AE_OK 0
#define AE_ERR -1

#define AE_NONE 0       /* No events registered. */
#define AE_READABLE 1   /* Fire when descriptor is readable. */
#define AE_WRITABLE 2   /* Fire when descriptor is writable. */
#define AE_BARRIER 4    /* With WRITABLE, never fire the event if the
                           READABLE event already fired in the same event
                           loop iteration. Useful when you want to persist
                           things to disk before sending replies, and want
                           to do that in a group fashion. */

#define AE_FILE_EVENTS 1
#define AE_TIME_EVENTS 2
#define AE_ALL_EVENTS (AE_FILE_EVENTS|AE_TIME_EVENTS)
#define AE_DONT_WAIT 4

#define AE_NOMORE -1
#define AE_DELETED_EVENT_ID -1

/* Macros */
#define AE_NOTUSED(V) ((void) V)

struct aeEventLoop;

/* Types and data structures */
typedef void aeFileProc(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);
typedef int aeTimeProc(struct aeEventLoop *eventLoop, long long id, void *clientData);
typedef void aeEventFinalizerProc(struct aeEventLoop *eventLoop, void *clientData);
typedef void aeBeforeSleepProc(struct aeEventLoop *eventLoop);

/* File event structure
 * BARRIER事件类型：通常情况下 ae是先处理可读事件再处理可写事件，但如果类型为BARRIER则相反，先处理可写事件再处理可读事件。
 * 比如aof flush 需要在回复客户端响应之前将 aof 刷盘 */
typedef struct aeFileEvent {
    int mask; /* 监听的文件事件类型。one of AE_(READABLE|WRITABLE|BARRIER) */
    aeFileProc *rfileProc;  /* AE_READABLE读事件处理函数 */
    aeFileProc *wfileProc;  /* AE_WRITABLE写事件处理函数 */
    void *clientData;       /* 附加数据 */
} aeFileEvent;

/* Time event structure */
typedef struct aeTimeEvent {
    long long id; /* 时间事件id，从1开始自增。time event identifier. */
    long when_sec; /* 时间事件下一次触发的时间。seconds */
    long when_ms; /* milliseconds */
    aeTimeProc *timeProc;  /* 时间事件处理函数 */
    aeEventFinalizerProc *finalizerProc;  /* 时间事件的析构函数 */
    void *clientData;  /* 客户端传入的附加数据 */
    struct aeTimeEvent *next;   /* 下一个时间事件 */
} aeTimeEvent;

/* A fired event
 * 就绪的文件事件 */
typedef struct aeFiredEvent {
    int fd;     /* 文件描述符 */
    int mask;   /* 产生的事件类型掩码 */
} aeFiredEvent;

/* State of an event based program */
typedef struct aeEventLoop {
    int maxfd;   /* 当前已注册最大文件描述符。highest file descriptor currently registered */
    int setsize; /* 当前已跟踪的最大描述符数量。max number of file descriptors tracked */
    long long timeEventNextId  /* 用于生产时间事件，next_id，下一个时间事件id */;
    time_t lastTime;     /* 最后一次执行时间事件的时间，用于检测时钟偏移。Used to detect system clock skew */
    aeFileEvent *events; /* 已注册的文件事件。Registered events */
    aeFiredEvent *fired; /* 已就绪的文件事件。Fired events */
    aeTimeEvent *timeEventHead;  /* 时间事件链表头节点。*/
    int stop;  /* 事件处理器开关，是否停止。*/
    void *apidata; /* 多路复用库的私有数据，例如select里面是:rfds,wfds。This is used for polling API specific data */
    aeBeforeSleepProc *beforesleep;  /* 每次事件循环开始之前的钩子函数。*/
} aeEventLoop;

/* Prototypes */
aeEventLoop *aeCreateEventLoop(int setsize);
void aeDeleteEventLoop(aeEventLoop *eventLoop);
void aeStop(aeEventLoop *eventLoop);
int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask,
        aeFileProc *proc, void *clientData);
void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask);
int aeGetFileEvents(aeEventLoop *eventLoop, int fd);
long long aeCreateTimeEvent(aeEventLoop *eventLoop, long long milliseconds,
        aeTimeProc *proc, void *clientData,
        aeEventFinalizerProc *finalizerProc);
int aeDeleteTimeEvent(aeEventLoop *eventLoop, long long id);
int aeProcessEvents(aeEventLoop *eventLoop, int flags);
int aeWait(int fd, int mask, long long milliseconds);
void aeMain(aeEventLoop *eventLoop);
char *aeGetApiName(void);
void aeSetBeforeSleepProc(aeEventLoop *eventLoop, aeBeforeSleepProc *beforesleep);
int aeGetSetSize(aeEventLoop *eventLoop);
int aeResizeSetSize(aeEventLoop *eventLoop, int setsize);

#endif
