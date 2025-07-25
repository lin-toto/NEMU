/***************************************************************************************
* Copyright (c) 2014-2021 Zihao Yu, Nanjing University
* Copyright (c) 2020-2022 Institute of Computing Technology, Chinese Academy of Sciences
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __UTILS_H__
#define __UTILS_H__

#include <common.h>

// ----------- state -----------

enum { NEMU_RUNNING, NEMU_STOP, NEMU_END, NEMU_ABORT, NEMU_QUIT };

typedef struct {
  int state;
  vaddr_t halt_pc;
  uint32_t halt_ret;
} NEMUState;

extern NEMUState nemu_state;

enum {
  dflag_none = 0,
  dflag_mem,
  dflag_translate,
  dflag_trace_bb,
  dflag_trace_inst,
  dflag_trace_inst_dasm,
  dflag_exit,
  dflag_simpoint,
};


// ----------- timer -----------

void init_time();
uint64_t get_time();

// ----------- log -----------

// control when the log is printed, unit: number of instructions
#define LOG_START (0)
#define LOG_END   (1024 * 1024 * 50)

#define SMALL_LOG_ROW_NUM (50 * 1024 * 1024) // row number, 50M instructions
#define SMALL_LOG_ROW_BYTES 512

#ifdef __cplusplus
extern "C" {
#endif

extern bool enable_fast_log;
extern bool enable_small_log;
extern FILE *log_fp;
extern char *log_filebuf;
extern uint64_t record_row_number;
extern void log_buffer_flush(void);

#ifdef __cplusplus
}
#endif

/* #define log_write(...) MUXDEF(CONFIG_DEBUG, \
  do { \
    extern FILE* log_fp; \
    extern void log_flush(); \
    if (log_fp != NULL) { \
      log_flush(); \
      fprintf(log_fp, __VA_ARGS__); \
      fflush(log_fp); \
    }else{ \
      printf(__VA_ARGS__); \
    } \
  } while (0), \
  do { \
    printf(__VA_ARGS__); \
  }while (0)\
 )*/
#define log_write(...) \
  do { \
    if (log_fp != NULL) { \
      if (enable_fast_log || enable_small_log) { \
        snprintf(log_filebuf + record_row_number * SMALL_LOG_ROW_BYTES, SMALL_LOG_ROW_BYTES, __VA_ARGS__);\
        log_buffer_flush(); \
      } else { \
        fprintf(log_fp, __VA_ARGS__); \
        fflush(log_fp); \
      } \
    }else{ \
      printf(__VA_ARGS__); \
    } \
  } while (0)

#define _Log(...) \
  do { \
    log_write(__VA_ARGS__); \
  } while (0)

// ----------- expr -----------

word_t expr(char *e, bool *success);

// ----------- iqueue -----------
void iqueue_commit(vaddr_t pc, uint8_t *instr_buf, uint8_t ilen);
void iqueue_dump();

// ----------- br_log -----------
enum {
  BR_BRANCH = 0,
  BR_JUMP = 1,
};
void br_log_commit(vaddr_t pc, vaddr_t target, int taken, int type);
void br_log_dump();
void * br_log_query();
uint64_t br_log_get_count();
void br_log_set_count(uint64_t val);

#ifdef __cplusplus
extern "C" {
#endif
bool is_gz_file(const char *filename);
bool is_zstd_file(const char *filename);
#ifdef __cplusplus
}
#endif

#endif
