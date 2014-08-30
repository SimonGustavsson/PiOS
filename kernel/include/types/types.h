#ifndef TYPES_H
#define TYPES_H
#ifndef __cplusplus
	#ifndef bool
		typedef enum { false, true } bool;
	#endif
        typedef struct {
            int width;
            int height;
        } size;
#endif
#endif