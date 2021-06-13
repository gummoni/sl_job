#include "Job.h"
#include <setjmp.h>
#include <string.h>

jmp_buf SuspendJmp;
job_scheduler* current_scheduler;

static job* __ret;
static int __size;

void job_push(job** self) {
	unsigned* addr = (unsigned*)self;
	(*self)->memory[(*self)->idx++] = *(--addr);	//EIP
	(*self)->memory[(*self)->idx++] = *(--addr);	//EBP
	(*self)->memory[(*self)->idx++] = (unsigned)addr;
}
void job_pop(job** self) {
	unsigned* addr = (unsigned*)(*self)->memory[--(*self)->idx];
	*(addr++) = (*self)->memory[--(*self)->idx];	//EBP
	*(addr++) = (*self)->memory[--(*self)->idx];	//EIP
}


/// <summary>
/// �X�P�W���[��������
/// </summary>
/// <param name="self"></param>
void sc_init(job_scheduler* self) {
	self->top = NULL;
	self->last = NULL;
}

/// <summary>
/// �����ڏ�
/// </summary>
/// <param name="self"></param>
/// <param name="job"></param>
/// <returns></returns>
job* job_invoke(job_scheduler* self, job* job) {
	if (NULL == self->top) {
		self->top = job;
		self->last = job;
	}
	else {
		self->last->next = job;
		self->last = job;
	}
	return job;
}

/// <summary>
/// �X�P�W���[���J�n
/// </summary>
/// <param name="self"></param>
void sc_start(job_scheduler* self) {
	current_scheduler = self;

	//�t�B�[�h�o�b�N
	switch (current_scheduler->top->state = (job_state)setjmp(SuspendJmp)) {
	case job_state::INIT:
		longjmp(SuspendJmp, (int)current_scheduler->top->state);
		break;

	case job_state::START:
		current_scheduler->top->state = job_state::BUSY;

		if (NULL != current_scheduler->top->start)
			current_scheduler->top->start(current_scheduler->top);

		current_scheduler->top->state = job_state::SUCCESS;

		break;

	case job_state::BUSY:
		job_invoke(current_scheduler, current_scheduler->top);
		break;

	default:
		break;		//�p�X
	}
	
	//���̃L���[��
	job* tmp = current_scheduler->top->next;
	if (tmp == NULL)
		return;
	current_scheduler->top->next = NULL;
	current_scheduler->top = tmp;

	//���s
	if (job_state::BUSY == current_scheduler->top->state)
		longjmp(current_scheduler->top->buf, (int)current_scheduler->top);	//�ĊJ
	else
		longjmp(SuspendJmp, (int)current_scheduler->top->state);			//�t�B�[�h�o�b�N
}

