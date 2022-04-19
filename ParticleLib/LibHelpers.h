/// LibHelpers.h
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// Defines these classes: PInternalState_t

#ifndef LibHelpers_h
#define LibHelpers_h

#include "Particle/pError.h"

#define LIB_ASSERT(x, msg)                          \
    {                                               \
        if (!(x)) { throw PErrInternalError(msg); } \
    }

#endif
