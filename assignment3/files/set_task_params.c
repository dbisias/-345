#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/current.h>
#include <asm-generic/errno-base.h>
#include <linux/task_params.h>

asmlinkage long sys_set_task_params(int deadline, int estimated_runtime) {
    struct task_struct *task;

    printk("Dimitris Bisias, 4273, set_task_params\n");
    task = get_current();

    if(deadline < 0 || estimated_runtime < 0 || estimated_runtime > deadline*1000) { //since est_runtime is in milliseconds, multiply deadline x1000 to check values
	return EINVAL;
    }

    task->deadline = deadline;
    task->estimated_runtime = estimated_runtime;

    return 0;
}
