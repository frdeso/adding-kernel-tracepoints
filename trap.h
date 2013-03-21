#undef TRACE_SYSTEM
#define TRACE_SYSTEM trap

#if !defined(_TRACE_TRAP_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_TRAP_H

#include <linux/tracepoint.h>

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

#endif /* _TRACE_TRAP_H */
/* This part must be outside protection */
#include <trace/define_trace.h>
