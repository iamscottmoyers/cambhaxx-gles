#ifndef UTIL_H
#define UTIL_H

/** Removes unused variable warnings. */
#define UNUSED(_var) ((void)(_var))

/** Returns the number of elements in an array. */
#define NELEMS(_arr) (sizeof((_arr))/sizeof((_arr)[0]))

#endif
