/// PError.h
///
/// Copyright 1997-2007, 2022 by David K. McAllister
///
/// This file defines the error classes that are thrown.

#ifndef _perror_h
#define _perror_h

#include <string>

namespace PAPI {
/// Base type of all exceptions thrown by the Particle System API.
struct PError_t {
    std::string ErrMsg;
    PError_t(const std::string Er) : ErrMsg(Er) {}
};

struct PErrInNewActionList : PError_t /// An operation was illegally attempted while in the midst of compiling an action list
{
    PErrInNewActionList(const std::string Er) : PError_t(Er) {}
};
struct PErrNotImplemented : PError_t /// API encountered an unimplemented code path like Bounce for a new
{
    PErrNotImplemented(const std::string Er) : PError_t(Er) {}
};
struct PErrInternalError : PError_t /// Internal API error (a PASSERT failed)
{
    PErrInternalError(const std::string Er) : PError_t(Er) {}
};
struct PErrParticleGroup : PError_t /// A particle group call had an error
{
    PErrParticleGroup(const std::string Er) : PError_t(Er) {}
};
struct PErrActionList : PError_t /// An action list call had an error
{
    PErrActionList(const std::string Er) : PError_t(Er) {}
};
struct PErrInvalidValue : PError_t /// An invalid value was passed to an API call
{
    PErrInvalidValue(const std::string Er) : PError_t(Er) {}
};
}; // namespace PAPI

#define PASSERT(x, msg)                    \
    {                                      \
        if (!(x)) { throw PError_t(msg); } \
    }

#endif
