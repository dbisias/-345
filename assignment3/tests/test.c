#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <stdlib.h>

#define set_task_params(arg0, arg1) syscall(341, arg0, arg1)
#define get_task_params(arg0) syscall(342, arg0)

int main() {
	int res = set_task_params(5,4000);

	
	if(res != 0) {
		printf("Problem with set_task_params\n");
	}
	
	struct params {
		int deadline;
		int estimated_runtime;
	};
	
	struct params *p = (struct params*)malloc(sizeof(struct params));


	res = get_task_params(p);
	if( res == 0) {
		printf("deadline: %d sec, estimated_runtime: %d ms\n", p->deadline, p->estimated_runtime);
	}
	else {
		printf("Problem with get_task_params\n");
	}

	return 0;	
}



