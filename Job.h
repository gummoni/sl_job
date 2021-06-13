#pragma once
#include <setjmp.h>

/// <summary>
/// ジョブ状態
/// </summary>
enum class job_state {
	INIT = 0,								//起動時
	START,									//開始
	BUSY,									//動作中
	CANCELED,								//ユーザによる中断
	SUCCESS,								//正常終了
	FAILED,									//システムによる中断
};

typedef void (*job_delegate)(void*);


/// <summary>
/// ジョブ
/// </summary>
typedef struct job {
	unsigned memory[16];
	int idx;
	jmp_buf buf;
	int error_code;
	job_state state;
	struct job* next;

	void (*start)(void*);
} job;

/// <summary>
/// ジョブスケジューラ
/// </summary>
typedef struct job_scheduler {
	job* top;
	job* last;
} job_scheduler;

extern jmp_buf SuspendJmp;
extern job_scheduler* current_scheduler;

//初期化
#define job_init(__job) {						\
	(__job)->idx = 0;							\
	(__job)->error_code = 0;					\
	(__job)->state = job_state::START;			\
	(__job)->next = NULL;						\
}

/// <summary>
/// 処理退避
/// </summary>
/// <param name="self"></param>
#define job_yield(__self) {										\
	job* __tmp;													\
	if (NULL == (__tmp = (job*)setjmp(__self->job.buf))) {		\
		__tmp = (job*)__self;									\
		longjmp(SuspendJmp, (int)job_state::BUSY);				\
	}															\
	else {														\
		unsigned* __tmp2 = (unsigned*)&self;					\
		*__tmp2 = (unsigned)__tmp;								\
	}															\
}

#define job_wait(parent, child) { while ((child)->job.state <= job_state::BUSY) { job_yield(parent); } }

#define job_cancel(__job) { __job->state = job_state::CANCELED; }
#define job_current() (current_scheduler->top)
//#define job_failed(__job, error_code) { ((job*)__job)->state = job_state::FAILED; ((job*)__job)->error_code = error_code; longjmp(SuspendJmp, (int)job_state::FAILED); }

extern void job_push(job** self);
extern void job_pop(job** self);
extern void sc_init(job_scheduler* self);
extern void sc_start(job_scheduler* self);
extern job* job_invoke(job_scheduler* self, job* job);
