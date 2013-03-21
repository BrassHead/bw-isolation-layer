/* bx11.h -- Forward declarations for X11 types

Copyright 1997-2013 Brian Bray

*/


//
// This file has minimal declarations of common X11 types so that
// pointers and references to these structures can be placed in client
// accessible header files.
//
// These declarations constitute a non-opaque access to X11, so they
// are subject to change with the X11 version and environment.  Adjust
// as needed to match your X11/*.h files.
//

typedef struct _XDisplay Display;
typedef unsigned long Window;
typedef unsigned long Font;
typedef struct _XGC *GC;


