adding-kernel-tracepoints
=========================

Once upon a time there was a Linux Kernel tracing suite called LTTng that was creating its own patch to add tracepoints in the kernel.



##Repository of my analysis on tracepoints currently in the Linux kernel.


How static tracepoints are add to the Linux Kernel?
I first started by looking inside the kernel to know how tracepoint were added.

###How to add a tracepoint in the kernel
To add a tracepoint, one has to use that macro [TRACE_EVENT](http://lxr.linux.no/linux+v3.7.4/include/linux/tracepoint.h#L388).
The use of this macro is shown in the next code snippet. In this example, we can see that the event declared is the sched_switch which is a very commun event.
~~~sh

/*
 * Tracepoint for task switches, performed by the scheduler:
 */
TRACE_EVENT(sched_switch,
	
#1	
	TP_PROTO(struct task_struct *prev,
		 struct task_struct *next),

#2
	TP_ARGS(prev, next),

#3
	TP_STRUCT__entry(
		__array(	char,	prev_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	prev_pid			)
		__field(	int,	prev_prio			)
		__field(	long,	prev_state			)
		__array(	char,	next_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	next_pid			)
		__field(	int,	next_prio			)
	),

#4	
	TP_fast_assign(
		memcpy(__entry->next_comm, next->comm, TASK_COMM_LEN);
		__entry->prev_pid	= prev->pid;
		__entry->prev_prio	= prev->prio;
		__entry->prev_state	= __trace_sched_switch_state(prev);
		memcpy(__entry->prev_comm, prev->comm, TASK_COMM_LEN);
		__entry->next_pid	= next->pid;
		__entry->next_prio	= next->prio;
	),

#5
	TP_printk(
	"prev_comm=%s prev_pid=%d prev_prio=%d prev_state=%s%s ==> next_comm=%s next_pid=%d next_prio=%d",
		__entry->prev_comm, __entry->prev_pid, __entry->prev_prio,
		__entry->prev_state & (TASK_STATE_MAX-1) ?
		  __print_flags(__entry->prev_state & (TASK_STATE_MAX-1), "|",
				{ 1, "S"} , { 2, "D" }, { 4, "T" }, { 8, "t" },
				{ 16, "Z" }, { 32, "X" }, { 64, "x" },
				{ 128, "W" }) : "R",
		__entry->prev_state & TASK_STATE_MAX ? "+" : "",
		__entry->next_comm, __entry->next_pid, __entry->next_prio)
);
~~~
Here i will explain the five macro calls that are the arguments of this tracepoint declaration.

1.	[TP_PROTO](http://lxr.linux.no/linux+v3.7.5/include/linux/tracepoint.h#L101)
2.	[TP_ARGS](http://lxr.linux.no/linux+v3.7.5/include/linux/tracepoint.h#L102)
3.	[TP_STRUCT__entry](http://lxr.linux.no/linux+v3.7.5/include/trace/ftrace.h#L57)
4.	[TP_fast_assign](http://lxr.linux.no/linux+v3.7.5/include/trace/ftrace.h#L509)
5.	[TP_printk](http://lxr.linux.no/linux+v3.7.5/include/trace/ftrace.h#L579)


The first part of the analysis was to find what were the tracepoint already in page in the Linux Kenel.


###Information about page fault tracepoint

Discussions on the Linux Kernel Mailing List

[1] https://lkml.org/lkml/2010/11/10/89

[2] https://lkml.org/lkml/2011/4/29/488
