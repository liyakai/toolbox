#include "exceptions.h"
#include "utility.h"
#include "libxorp/c_format.hh"

XorpException::XorpException(const char* init_what,
                             const char* file,
                             size_t line)
    : _what(init_what), _file(file), _line(line)
{
}

XorpException::~XorpException()
{
}

const std::string
XorpException::where() const
{
    return c_format("line %u of %s", XORP_UINT_CAST(_line), _file);
}

const std::string
XorpException::why() const
{
    return "Not specified";
}

std::string
XorpException::str() const
{
    return what() + " from " + where() + ": " + why();
}

XorpReasonedException::XorpReasonedException(const char* init_what,
        const char* file,
        size_t line,
        const std::string& init_why)
    : XorpException(init_what, file, line), _why(init_why)
{
}

const std::string
XorpReasonedException::why() const
{
    return ( _why.size() != 0 ) ? _why : std::string("Not specified");
}


InvalidPacket::InvalidPacket(const char* file,
                             size_t line,
                             const std::string& init_why)
    : XorpReasonedException("XorpInvalidPacket", file, line, init_why)
{
}