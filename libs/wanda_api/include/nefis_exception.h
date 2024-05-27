#ifndef _NEFIS_EXCEPTION_
#define _NEFIS_EXCEPTION_

#include <nefis_file.h>
#include <stdexcept>

/**
 * \brief
 * nefis_exception gets the NEFIS error message using static get_last_error()
 * function. Note that multiple nefis functions are called in rapid succession on
 * multiple files, it is possible that the error message returned by Neferr()
 * doesn't match the error that was caused by the function, but was the result
 * of another nefis call.  NEFIS is NOT thread safe.
 */
class nefis_exception : public std::runtime_error
{
  public:
    explicit nefis_exception(const std::string &_Message)
        : runtime_error(_Message), origin_(nullptr), error_string_(_Message)
    {
    }

    explicit nefis_exception(const char *_Message) : runtime_error(_Message), origin_(nullptr), error_string_(_Message)
    {
    }

    explicit nefis_exception(const nefis_file *file)
        : runtime_error(file->get_filename() + ", Error:" + nefis_file::get_last_error()), origin_(file),
          error_string_(file->get_filename() + ", Error:" + nefis_file::get_last_error())
    {
    }

    char const *what() const throw() override
    {
        return error_string_.data();
    }

  private:
    const nefis_file *origin_;
    const std::string error_string_;
};

#endif
