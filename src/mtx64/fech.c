/*
              fech.c     Disk chopping echelon form
              ======     R. A. Parker   7.6.2017
     Does not yet chop on disk
 */

#include <stdint.h>
#include "mezz.h"

uint64_t fech(const char *m1, int s1, const char *b2, int s2,
         const char *b3, int s3, const char *m4, int s4,
         const char *m5, int s5, const char *m6, int s6)
{
    return mech(m1,s1,b2,s2,b3,s3,m4,s4,m5,s5,m6,s6);
}

/* end of fech.c  */
