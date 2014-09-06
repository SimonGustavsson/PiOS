#ifndef TYPES_H
#define TYPES_H
#ifndef __cplusplus
	#ifndef bool
		typedef enum { false, true } bool;
	#endif
#endif
#endif

#ifndef SIZE_STRUCT_DEF
#define SIZE_STRUCT_DEF
	typedef struct {
	    int width;
	    int height;
	} size;
#endif