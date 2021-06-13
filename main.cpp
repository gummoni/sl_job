#include <setjmp.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "Job.h"
#include <memory>

typedef struct job1 {
	job job;
	//--property
	int count;
} job1;

static void job1_ready(job1* self) {
	printf("start : j1\n");
}

static void job1_start(job1* self) {
	job_push((job**)&self);

	while (self->count++ <= 10) {
		printf("yield1: %d\n", self->count);
		job_yield(self);
	}

	job_pop((job**)&self);
}

job* job1_init(job1* self) {
	job_init((job*)self);
	self->count = 3;
	self->job.start = (job_delegate)job1_start;
	return &self->job;
}


typedef struct job2 {
	job job;
	//--property
	int count;
	//--register
	job1 j1;
} job2;

void job2_ready(job2* self) {
	job_push((job**)&self);
	printf("start : j2\n");
	job_pop((job**)&self);
}

void job2_nest(job2* self) {
	job_push((job**)&self);

	job1_init(&self->j1);
	self->j1.count = -10;
	job_invoke(current_scheduler, &self->j1.job);
	job_wait(self, &self->j1);

	job_pop((job**)&self);
}

void job2_start(job2* self) {
	job_push((job**)&self);

	while (self->count++ <= 10) {
		printf("yield2: %d\n", self->count);
		job_yield(self);
	}

	job2_nest(self);
	self = (job2*)job_current();

	printf("yield2: %d\n", self->count);

	job_pop((job**)&self);
}

job* job2_init(job2* self) {
	job_init((job*)self);
	self->count = 0;
	self->job.start = (job_delegate)job2_start;
	return &self->job;
}


/// <summary>
/// ƒƒCƒ“ˆ—
/// </summary>
/// <param name=""></param>
/// <returns></returns>
int main(void) {
	job_scheduler sc;
	job1 jj1;
	job2 jj2;

	sc_init(&sc);

	job1_init(&jj1);
	job2_init(&jj2);


	job_invoke(&sc, &jj1.job);
	job_invoke(&sc, &jj2.job);

	//Às
	sc_start(&sc);

	return 0;
}
