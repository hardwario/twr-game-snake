#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <bcl.h>
#include <twr.h>

typedef struct
{
	int x;
	int y;

} Point_t;

typedef enum
{
	WELCOME,
	GAME,
	END

} state_t;

enum
{
	EASY = 0,
	MEDIUM = 1,
	HARD = 2
};

#endif // _APPLICATION_H
