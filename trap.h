#undef TRACE_SYSTEM
#define TRACE_SYSTEM trap

#if !defined(_TRACE_TRAP_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_TRAP_H

#include <linux/tracepoint.h>

DECLARE_EVENT_CLASS(trap,
    TP_PROTO(int trap_nb),
		    
    TP_ARGS(id),
			
    TP_STRUCT__entry(
            __field(    int,    trap_nb )
    ),
    TP_fast_assign(
       __entry->trap_nb = trap_nb;
    ),
				
    TP_printk("number=%d", __entry->trap_nb)
);

DEFINE_EVENT(trap, trap_entry,
		
	TP_PROTO(int trap_nb),
		    
	TP_ARGS(trap_nb)
);
DEFINE_EVENT(trap, trap_exit,
	
	TP_PROTO(int trap_nb),
		    
	TP_ARGS(trap_nb)
);

#endif /* _TRACE_TRAP_H */
/* This part must be outside protection */
#include <trace/define_trace.h>
