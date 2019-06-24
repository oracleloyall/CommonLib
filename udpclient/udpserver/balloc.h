/*
 * balloc.h
 *
 *  Created on: 2015-12-31
 *      Author:
 */

#ifndef BALLOC_H_
#define BALLOC_H_
#ifdef __cplusplus
extern "C" {
#endif



/*
 *	Allocation flags
 */
#define VALUE_ALLOCATE		0x1

#define value_numeric(t)	(t >= byteint && t <= big)
#define value_str(t) 		(t >= string && t <= bytes)
#define value_ok(t) 		(t > undefined && t <= symbol)

#define VALUE_VALID			{ {0}, integer, 1 }
#define VALUE_INVALID		{ {0}, undefined, 0 }


/*
 *	Ring queue buffer structure
 */
typedef struct {
	unsigned char	*buf;				/* Holding buffer for data */
	unsigned char	*servp;				/* Pointer to start of data */
	unsigned char	*endp;				/* Pointer to end of data */
	unsigned char	*endbuf;			/* Pointer to end of buffer */
	int				buflen;				/* Length of ring queue */
	int				maxsize;			/* Maximum size */
	int				increment;			/* Growth increment */
} ringq_t;

/*
 *	Block allocation (balloc) definitions
 */
#ifdef	B_STATS
#ifndef B_L
#define B_L				T(__FILE__), __LINE__
#define B_ARGS_DEC		char_t *file, int line
#define B_ARGS			file, line
#endif /* B_L */
#endif /* B_STATS */

/*
 *	Block classes are: 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192,
 *					   16384, 32768, 65536
 */
typedef struct {
	union {
		void	*next;							/* Pointer to next in q */
		int		size;							/* Actual requested size */
	} u;
	int			flags;							/* Per block allocation flags */
} bType;

#define B_SHIFT			4					/* Convert size to class */
#define B_ROUND			((1 << (B_SHIFT)) - 1)
#define B_MAX_CLASS		13					/* Maximum class number + 1 */
#define B_MALLOCED		0x80000000			/* Block was malloced */
#define B_DEFAULT_MEM	(64 * 1024)			/* Default memory allocation */
#define B_MAX_FILES		(512)				/* Maximum number of files */
#define B_FILL_CHAR		(0x77)				/* Fill byte for buffers */
#define B_FILL_WORD		(0x77777777)		/* Fill word for buffers */
#define B_MAX_BLOCKS	(64 * 1024)			/* Maximum allocated blocks */

/*
 *	Flags. The integrity value is used as an arbitrary value to fill the flags.
 */
#define B_INTEGRITY			0x8124000		/* Integrity value */
#define B_INTEGRITY_MASK	0xFFFF000		/* Integrity mask */
#define B_USE_MALLOC		0x1				/* Okay to use malloc if required */
#define B_USER_BUF			0x2				/* User supplied buffer for mem */

int bopen(void *buf, int bufsize, int flags);
void bclose();
void *balloc( int size);
void bfree( void *mp);
void bfreeSafe( void *mp);
char *bstrdup( char *s);
void *brealloc(void *mp, int newsize);


#ifdef __cplusplus
}
#endif
#endif /* BALLOC_H_ */
