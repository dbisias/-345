#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/current.h>
#include <asm-generic/errno-base.h>
#include <asm/uaccess.h>
#include <linux/task_params.h>

asmlinkage long sys_get_task_params(struct task_params * params) {
    struct task_struct *task;

    printk("Dimitris Bisias, 4273, get_task_params\n");

    if(params == NULL)
	return EINVAL;

    task = get_current();
    if(access_ok(VERIFY_WRITE, params, sizeof(struct task_params)) == 0) {
	return EINVAL;
    }

    copy_to_user((void*)&params->deadline,(void*)&task->deadline ,sizeof(int));
    copy_to_user((void*)&params->estimated_runtime, (void*) &task->estimated_runtime, sizeof(int));

    return 0;
}
