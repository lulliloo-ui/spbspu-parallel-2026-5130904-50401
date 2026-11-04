#include "file_descriptor.hpp"

#include <unistd.h>

namespace
{
  const int invalid_fd = -1;
}

madieva::FileDescriptor::FileDescriptor(int fd):
  fd_(fd)
{}

void madieva::FileDescriptor::close()
{
  if (fd_ != invalid_fd)
  {
    ::close(fd_);
    fd_ = invalid_fd;
  }
}

madieva::FileDescriptor::FileDescriptor(FileDescriptor && other) noexcept:
  fd_(other.fd_)
{
  other.fd_ = invalid_fd;
}

madieva::FileDescriptor & madieva::FileDescriptor::operator=(FileDescriptor && other) noexcept
{
  if (this != &other)
  {
    close();
    fd_ = other.fd_;
    other.fd_ = invalid_fd;
  }
  return *this;
}

madieva::FileDescriptor::~FileDescriptor()
{
  if (fd_ != invalid_fd)
  {
    close();
  }
}

int madieva::FileDescriptor::get() const
{
  return fd_;
}
