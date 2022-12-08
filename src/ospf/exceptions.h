#pragma once


/**
 * @short A base class for XORP exceptions.
 */
class XorpException
{
public:
    /**
     * Constructor for a given type for exception, file name,
     * and file line number.
     *
     * @param init_what the type of exception.
     * @param file the file name where the exception was thrown.
     * @param line the line in @ref file where the exception was thrown.
     */
    XorpException(const char* init_what, const char* file, size_t line);

    /**
     * Destructor
     */
    virtual ~XorpException();

    /**
     * Get the type of this exception.
     *
     * @return the std::string with the type of this exception.
     */
    const std::string& what() const
    {
        return _what;
    }

    /**
     * Get the location for throwing an exception.
     *
     * @return the std::std::string with the location (file name and file line number)
     * for throwing an exception.
     */
    const std::string where() const;

    /**
     * Get the reason for throwing an exception.
     *
     * @return the std::string with the reason for throwing an exception.
     */
    virtual const std::string why() const;

    /**
     * Convert this exception from binary form to presentation format.
     *
     * @return C++ std::string with the human-readable ASCII representation
     * of the exception.
     */
    std::string str() const;

protected:
    std::string  _what;      // The type of exception
    const char* _file;      // The file name where exception occured
    size_t  _line;      // The line number where exception occured
};


/**
 * @short A base class for XORP exceptions that keeps the reason for exception.
 */
class XorpReasonedException : public XorpException
{
public:
    /**
     * Constructor for a given type for exception, file name,
     * file line number, and a reason.
     *
     * @param init_what the type of exception.
     * @param file the file name where the exception was thrown.
     * @param line the line in @ref file where the exception was thrown.
     * @param init_why the reason for the exception that was thrown.
     */
    XorpReasonedException(const char* init_what, const char* file,
                          size_t line, const std::string& init_why);

    /**
     * Get the reason for throwing an exception.
     *
     * @return the std::string with the reason for throwing an exception.
     */
    const std::string why() const;

protected:
    std::string _why;        // The reason for the exception
};

/**
 * @short A standard XORP exception that is thrown if the packet is invalid.
 */
class InvalidPacket : public XorpReasonedException
{
public:
    InvalidPacket(const char* file, size_t line, const std::string& init_why = "");
};

/**
 * @short A standard XORP exception that is thrown if an IP netmask length is
 * invalid.
 */
class InvalidNetmaskLength : public XorpException
{
public:
    InvalidNetmaskLength(const char* file, size_t line, int netmask_length);
    const std::string why() const;

protected:
    int _netmask_length;
};

