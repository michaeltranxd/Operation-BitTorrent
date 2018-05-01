#ifndef VAR_H
#define VAR_H

#define MAXTASKSCOUNT 10

extern pthread_mutex_t task_lock;
extern pthread_cond_t tasks_cond[MAXTASKSCOUNT];
extern pthread_cond_t add_task_cond;
extern char **tasks_name;
extern int *tasks_count;

#endif
