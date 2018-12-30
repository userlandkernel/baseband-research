#ifndef _TEST_HEADER_H
#define _TEST_HEADER_H_

/***************************************************************************
 * Structures for test purposes
 **************************************************************************/
typedef struct {
  union {
	  int  arg1;
	  int  arg2;
  } argu;
  int  arg3;
} test_struct;

#endif /* _TEST_HEADER_H */
