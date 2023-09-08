/*
 * Copyright (c) 2009-2012, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 * Copyright (c) 2009-2012, Salvatore Sanfilippo <antirez at gmail dot com>
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

#ifndef __INTSET_H
#define __INTSET_H
#include <stdint.h>

/**
 * intset即整数集合。用一个连续数组来存储整数元素。
 * 元素存储在contents中，从小到大排序，并且无重复。
 *
 * 有三种encoding
 *  #define INTSET_ENC_INT16 (sizeof(int16_t))
 *  #define INTSET_ENC_INT32 (sizeof(int32_t))
 *  #define INTSET_ENC_INT64 (sizeof(int64_t))
 * 分别对应int16, int32, int64，代表元素类型。
 *
 * 默认encoding为int16，当插入元素超过该encoding的整数范围时，会进行encoding升级，重新创建contents。时间复杂度O(n)
 * 由于contents有序，所以可以用二分查找定位元素。
 *
 */
typedef struct intset {
    // 数据编码，表示 intset 中每个整数是用几个字节来编码存储的
    // 默认为INTSET_ENC_INT16，即contents[]元素类型为int16
    uint32_t encoding;
    // intset 中的元素个数，即contents数组长度
    uint32_t length;
    /**
     * 柔性数组，存储真正的整数数据项，按从小到大排序，并且不重复
     * 虽然 contents 被声明为 int8_t 类型，但实际上它并不会保存任何 int8_t 元素，元素类型取决于 encoding 属性的值.
     */
    int8_t contents[];
} intset;

intset *intsetNew(void);
intset *intsetAdd(intset *is, int64_t value, uint8_t *success);
intset *intsetRemove(intset *is, int64_t value, int *success);
uint8_t intsetFind(intset *is, int64_t value);
int64_t intsetRandom(intset *is);
uint8_t intsetGet(intset *is, uint32_t pos, int64_t *value);
uint32_t intsetLen(intset *is);
size_t intsetBlobLen(intset *is);

#ifdef REDIS_TEST
int intsetTest(int argc, char *argv[]);
#endif

#endif // __INTSET_H
