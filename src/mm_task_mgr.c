
/*
 * machinarium.
 *
 * cooperative multitasking engine.
*/

#include <machinarium.h>
#include <machinarium_private.h>

enum {
	MM_TASK,
	MM_TASK_EXIT
};

static void
mm_taskmgr_main(void *arg)
{
	mm_queuerd_t *reader;
	reader = mm_queuerdpool_pop(&mm_self->queuerd_pool);
	if (reader == NULL)
		return;
	for (;;)
	{
		mm_msg_t *msg;
		msg = mm_queue_get(&machinarium.task_mgr.queue, reader, INT_MAX);
		assert(msg != NULL);
		if (msg->type == MM_TASK_EXIT) {
			mm_msg_unref(&machinarium.msg_pool, msg);
			break;
		}
		assert(msg->type == MM_TASK);
		assert(mm_buf_used(&msg->data) == sizeof(mm_task_t));

		mm_task_t *task;
		task = (mm_task_t*)msg->data.start;
		task->function(task->arg);

		mm_queue_put(&task->on_complete, msg);
	}
	mm_queuerdpool_push(&mm_self->queuerd_pool, reader);
	(void)arg;
}

void mm_taskmgr_init(mm_taskmgr_t *mgr)
{
	mgr->workers_count = 0;
	mgr->workers = NULL;
	mm_queue_init(&mgr->queue);
}

int mm_taskmgr_start(mm_taskmgr_t *mgr, int workers_count)
{
	mgr->workers_count = workers_count;
	mgr->workers = malloc(sizeof(int) * workers_count);
	if (mgr->workers == NULL)
		return -1;
	int i = 0;
	for (; i < workers_count; i++) {
		char name[32];
		snprintf(name, sizeof(name), "mm_worker: %d", i);
		mgr->workers[i] = machine_create(name, mm_taskmgr_main, NULL);
	}
	return 0;
}

void mm_taskmgr_stop(mm_taskmgr_t *mgr)
{
	int i;
	for (i = 0; i < mgr->workers_count; i++) {
		machine_msg_t msg;
		msg = machine_msg_create(MM_TASK_EXIT, 0);
		mm_queue_put(&mgr->queue, msg);
	}
	for (i = 0; i < mgr->workers_count; i++) {
		machine_wait(mgr->workers[i]);
	}
	mm_queue_free(&mgr->queue);
	free(mgr->workers);
}

int mm_taskmgr_new(mm_taskmgr_t *mgr,
                   mm_task_function_t function, void *arg,
                   int time_ms)
{
	mm_queuerd_t *reader;
	reader = mm_queuerdpool_pop(&mm_self->queuerd_pool);
	if (reader == NULL)
		return -1;

	mm_msg_t *msg;
	msg = machine_msg_create(MM_TASK, sizeof(mm_task_t));
	if (msg == NULL) {
		mm_queuerdpool_push(&mm_self->queuerd_pool, reader);
		return -1;
	}

	mm_task_t *task;
	task = (mm_task_t*)msg->data.start;
	task->function = function;
	task->arg = arg;
	mm_queue_init(&task->on_complete);

	/* schedule task */
	mm_queue_put(&mgr->queue, msg);

	/* wait for completion */
	time_ms = INT_MAX;

	mm_msg_t *result;
	result = mm_queue_get(&task->on_complete, reader, time_ms);
	if (result == NULL) {
		/* todo: */
		abort();
	}

	mm_queuerdpool_push(&mm_self->queuerd_pool, reader);
	mm_msg_unref(&machinarium.msg_pool, result);
	return 0;
}
