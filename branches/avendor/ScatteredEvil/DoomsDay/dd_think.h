#ifndef __DOOMSDAY_THINKER_H__
#define __DOOMSDAY_THINKER_H__

// think_t is a function pointer to a routine to handle an actor
typedef void (*think_t) (struct thinker_s *th);

typedef struct thinker_s
{
	struct		thinker_s	*prev, *next;
	think_t		function;
} thinker_t;


#endif