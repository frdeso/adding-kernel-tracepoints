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


The first part of the analysis was to find what were the tracepoint already in place in the Linux Kenel.
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
####Trap tracepoint declaration
Here is a copy of the [file](https://github.com/frdeso/adding-kernel-tracepoints/blob/master/trap.h) 
that i used while compiling my kernel.
~~~sh
/* include/kernel/trace/events/traps.h */
#1
TRACE_EVENT(trap_entry,

	TP_PROTO(struct pt_regs *regs, long trap),

	TP_ARGS(regs, trap),

	TP_STRUCT__entry(
		__field(	long,		trap	)
		__field(	unsigned long,	ip	)
	),

	TP_fast_assign(
		__entry->trap	= trap;
		__entry->ip	= regs ? instruction_pointer(regs) : 0UL;
	),

	TP_printk("number=%ld ip=%lu", __entry->trap, __entry->ip)
);

#2
TRACE_EVENT(trap_exit,

	TP_PROTO(long trap),

	TP_ARGS(trap),

	TP_STRUCT__entry(
		__field(	long,	trap	)
	),

	TP_fast_assign(
		__entry->trap	= trap;
	),

	TP_printk("number=%ld", __entry->trap)
);


~~~


This is the implementation of 2 trap tracepoints. Once those are declare, it is possible the call the tracepoints by 
calling the trace_trap_entry and trace_trap_exit functions.

The macro __TP_PROTO__ decribe the prototype of the fonction. We can
that the resulting function will have 1 argument which is a integer named id. __TP_ARGS__ define the 
function signature.(Difference between TP_PROTO?). __TP_STRUCT_entry__ could be seen as a standard C struct.
__TP_fast_assign__ save the value that needs to be save for that particular set of tracepoint. Here we save 
the ID of the trap. TP_printk describe how the string printed in the kernel log should be formated.

1.	This declare the __trace_trap_entry__ tracepoint.
2.	This declare the __trace_trap_exit__ tracepoint.

####Trap tracepoint localization

Now that those are declared i just need to add this header file in a file to use the <code> trace_trap_entry() </code> function.
This is what i had done in the file arch/x86/kernel/traps.c.

~~~sh
/* arch/x86/kernel/traps.c */
/* ........ */
   34 #include <linux/mm.h>
   35 #include <linux/smp.h>
   36 #include <linux/io.h>
   37 
   38 #define CREATE_TRACE_POINTS
   39 #include <trace/events/trap.h>
   40 
   41 #ifdef CONFIG_EISA
   42 #include <linux/ioport.h>

/* ........ */
~~~

The hard part is to find a location where the tracepoint will have less effect on the performance of the system.
I have placed it in the <code>do_trap()</code> function which, wierdly enough is not call by all traps. I chose to place it 
therein order to test the other part of my assignement which is to implement the lttng-probe for this tracepoint.  You can also
that i used a <code>printk()</code> for debugging. I will have to find the right function to place the tracepoint
to record as much traps as possible.

~~~sh
/* arch/x86/kernel/traps.c */
 /* ........ */

   144 static void __kprobes
   145 do_trap(int trapnr, int signr, char *str, struct pt_regs *regs,
   146         long error_code, siginfo_t *info)
   147 {
   148         struct task_struct *tsk = current;
   149 
   150         printk("<0>Trap occurs : %d\n",trapnr);
   151         trace_trap_entry(trapnr);
   152 
   153         if (!do_trap_no_signal(tsk, trapnr, str, regs, error_code))
   154                 return;
 
 /* ........ */

~~~

#####Traps

The following lines expose where the tracepoint are placed in the de source code for each trap. I made 
this list to make sure i dont miss one of the trap.

1. __/*0, Divide-by-zero */__

	In DO_ERROR_INFO macro. Called [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L209)

2. __/*  1, Debug */__

	Handle [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L393)

3. __/*  2, Non-maskable Interrupt */__

4. __/*  3, Breakpoint */__

	Handle [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L307)

5. __/*  4, Overflow */__

	In DO_ERROR macro. Called [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L211)

6. __/*  5, Bound Range Exceeded */__

	In DO_ERROR macro. Called [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L212)

7. __/*  6, Invalid Opcode */__

	In DO_ERROR_INFO macro. Called [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L213)

8. __/*  7, Device Not Available */__

	Function declared [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L640)

9. __/* 8, Double Fault */__

	Handle [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L239) 
The kernel never returns from a double fault, there is only a trace_trap_entry for this trap.

10. __/*  9, Coprocessor Segment Overrun */__

	In DO_ERROR macro.Called [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L215)

11. __/* 10, Invalid TSS */__

	In DO_ERROR macro. Called [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L217)

12. __/* 11, Segment Not Present */__

	In DO_ERROR macro. Called [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L218)

13. __/* 12, Stack Segment Fault */__

	x86_32:	In DO_ERROR macro. Called [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L220)
	
	x86_64: Called [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L227)
14. __/* 13, General Protection Fault */__

	Handle [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L260)
The kernel never returns from a double fault, there is only a trace_trap_entry for this trap.

15. __/* 14, Page Fault */__

	arch/x86/mm/fault.c:do_page_fault()

16. __/* 15, Spurious Interrupt */__

	Function declared [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L778)
 
17. __/* 16, x87 Floating-Point Exception */__

	Handle [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L562)

18. __/* 17, Alignment Check */__

	In DO_ERROR_INFO macro. Called [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L222)

19. __/* 18, Machine Check */__

20. __/* 19, SIMD Floating-Point Exception */__

	Handle [here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L577)

21. __/* 32, IRET Exception */__

	X86_32:	Handle
[here](http://lxr.linux.no/linux+v3.8.5/arch/x86/kernel/traps.c#L665)


#### LTTng probe

A LTTng probe is a kernel module that is install in order to trace a system. This section will discuss
of the modifications that i had to make in [lttng-modules](http://git.lttng.org/?p=lttng-modules.git;a=summary) 
in order to trace my first tracepoint.

##### What is a kernel module?
Here is the definition of a kernel module on linux.die.net
> Modules are pieces of code that can be loaded and unloaded 
into the kernel upon demand. They extend the functionality of the kernel without the need to
reboot the system. For example, one type of module is the device driver, which allows the
kernel to access hardware connected to the system. Without modules, we would have to build 
monolithic kernels and add new functionality directly into the kernel image. Besides having
larger kernels, this has the disadvantage of requiring us to rebuild and reboot the kernel 
every time we want new functionality.[1]

[1] http://linux.die.net/lkmpg/x40.html

##### Files
In order to have a working lttng-probe someone needs to add 3 files and modify 1. 
Here is the [commit](https://github.com/frdeso/lttng-modules/commit/39058e586083e49ec00841da89491f40b2d0fc95) 
that group those modifications.

##### probes/lttng-probe-trap.c
[Implementation](https://github.com/frdeso/lttng-modules/blob/e31629e986742946cfdeafdd5a68888a824ba798/probes/lttng-probe-trap.c)
~~~sh
/* probes/lttng-probe-trap.c */

/* ... */

#include <linux/module.h>

/* ... */

#include <trace/events/trap.h>
#include "../wrapper/tracepoint.h"

/* ... */

#define LTTNG_PACKAGE_BUILD
#define CREATE_TRACE_POINTS
#define TRACE_INCLUDE_PATH ../instrumentation/events/lttng-module

#include "../instrumentation/events/lttng-module/trap.h"
~~~
##### instrumentation/events/lttng-module/trap.h
[Implementation](https://github.com/frdeso/lttng-modules/blob/master/instrumentation/events/lttng-module/trap.h)


##### instrumentation/events/mainline/trap.h
[Implementation](https://github.com/frdeso/lttng-modules/blob/master/instrumentation/events/mainline/trap.h)

##### probes/Makefile

I had to add the line #18 in order to build the module.

~~~sh
/* ... */
     17 obj-m += lttng-probe-power.o
     18 +obj-m += lttng-probe-trap.o
     19 
     20 obj-m += lttng-probe-statedump.o
/* ... */
~~~

###Test
This section discuss about the way that i am testing the trap tracepoint.

This is short program will be use to test the different traps that can be triggered manualy. 

~~~sh
/*
 * File : onDemandTrap.c 
 * Author : Francis Deslauriers fdeslaur@gmail.com
 */
 1 #include <stdio.h>
 2 int main()
 3 {
 4         printf("X86_TRAP_BP, INT3\n");
 5         asm("INT3");
 6         return 0;
 7 }
~~~

I am not sure of how i will test the traps that cannot be triggered.

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

