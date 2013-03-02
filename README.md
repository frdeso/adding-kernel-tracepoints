adding-kernel-tracepoints
=========================


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
##Traps
###What is a system trap?
A trap is type of interruption. It is sometime thought as a synchronous interrupt. Those
are caused after executing an instruction that might bring the system in an illegale state.


Here are the different traps that are present in the x86 architecture. I suppose that the amd64(or x86_64) 
architecture use those as well.

~~~sh
/*arch/x86/include/asm/traps.h*/
/* Interrupts/Exceptions */
 enum {
        X86_TRAP_DE = 0,        /*  0, Divide-by-zero */
        X86_TRAP_DB,            /*  1, Debug */
        X86_TRAP_NMI,           /*  2, Non-maskable Interrupt */
        X86_TRAP_BP,            /*  3, Breakpoint */
        X86_TRAP_OF,            /*  4, Overflow */
        X86_TRAP_BR,            /*  5, Bound Range Exceeded */
        X86_TRAP_UD,            /*  6, Invalid Opcode */
        X86_TRAP_NM,            /*  7, Device Not Available */
        X86_TRAP_DF,            /*  8, Double Fault */
        X86_TRAP_OLD_MF,        /*  9, Coprocessor Segment Overrun */
        X86_TRAP_TS,            /* 10, Invalid TSS */
        X86_TRAP_NP,            /* 11, Segment Not Present */
        X86_TRAP_SS,            /* 12, Stack Segment Fault */
        X86_TRAP_GP,            /* 13, General Protection Fault */
        X86_TRAP_PF,            /* 14, Page Fault */
        X86_TRAP_SPURIOUS,      /* 15, Spurious Interrupt */
        X86_TRAP_MF,            /* 16, x87 Floating-Point Exception */
        X86_TRAP_AC,            /* 17, Alignment Check */
        X86_TRAP_MC,            /* 18, Machine Check */
        X86_TRAP_XF,            /* 19, SIMD Floating-Point Exception */
        X86_TRAP_IRET = 32,     /* 32, IRET Exception */
 }
~~~

###Implementation
####Trap tracepoint

~~~sh
#1
DECLARE_EVENT_CLASS(trap,

	TP_PROTO(int id),

	TP_ARGS(id),

	TP_STRUCT__entry(
		__field(	int,	id	)
	),

	TP_fast_assign(
		__entry->id = id;
	),

	TP_printk("number=%d", __entry->id)
);
#2
DEFINE_EVENT(trap, trap_entry,

	TP_PROTO(int id),

	TP_ARGS(id)
);
#3
DEFINE_EVENT(trap, trap_exit,

	TP_PROTO(int id),

	TP_ARGS(id)
);
~~~


This is the implementation of 2 trap tracepoints. Once those are declare, it is possible the call the tracepoints by 
calling the trace_trap_entry and trace_trap_exit functions.

1.	This declare a class of tracepoint. The macro __TP_PROTO__ decribe the prototype of the fonction. We can
	that the resulting function will have 1 argument which is a integer named id. __TP_ARGS__ define the 
	function signature.(Difference between TP_PROTO?). __TP_STRUCT_entry__ could be seen as a standard C struct.
	__TP_fast_assign__ save the value that needs to be save for that particular set of tracepoint. Here we save 
	the ID of the trap. TP_printk describe how the string printed in the kernel log should be formated.
2.	This declare the __trace_trap_entry__ tracepoint.
3.	This declare the __trace_trap_exit__ tracepoint.



##Pagefault
### What really are pagefault?

A page fault  is a trap to the software raised by the hardware when a program accesses a page that is mapped in the virtual address space, but not loaded in physical memory. [Wikipedia](http://en.wikipedia.org/wiki/Page_fault)

It's important to note that a pagefault is not a error or a problem. It is a necessary downside of having virtual memory.

1.	__Minor pagefault__: Minor pagefault happen when the page is in main memory but the Memory Management
	Unit haven marked it has loaded. This may happen when another process had load that page at a previous 
	moment. So the system has to initialize that page. This work is typically has a negligable impact on the 
	latency of the system.
2.	__Major pagefault__: Major page fault occur when a process need virtual memory address that is not translate to a
	physical memory addresse. The page is still on the hard drive and has to be load in the main memory. This has
	may have a significant impact of the latency of the system.


###Pagefault tracepoint
After discussions with my mentor in this project, we have reached the conclusion that i should focus on adding
the pagefault tracepoint in the kernel. This will be done in those three steps : 

1.	Create a patch using adding the tracepoint. I will study the already submitted patches on the LKML (see [1] and [2]).
	and adapt them considering the comments from the kernel developers.
2.	Submitting the patch to the LTTng team for comments, code review and testing.
3.	Submitting the patch on the LKML emphasizing the fact that this patch has been tested and has prove itself useful.


Discussions on the Linux Kernel Mailing List

[1] https://lkml.org/lkml/2010/11/10/89

[2] https://lkml.org/lkml/2011/4/29/488

####Notes from those discussions

Some of the comments were about the use cases of this tracepoint. Maintainers were not sure of the when this tracepoint would be useful.

###Questions? 

1. How to handle recursive page faulting. Can the tracer page fault during the recording of a page fault?

Jiri Olsa wrote ([sources](https://lkml.org/lkml/2010/11/10/556)):
> The user stack trace can fault when examining the trace. Which
> would call the do_page_fault handler, which would trace again,
> which would do the user stack trace, which would fault and call
> do_page_fault again ...

2.

