#ifndef FILE_DESCRIPTOR_HPP
#define FILE_DESCRIPTOR_HPP

namespace madieva
{
  class FileDescriptor
  {
  public:
    FileDescriptor(int fd);
    FileDescriptor(const FileDescriptor &) = delete;
    FileDescriptor &operator=(const FileDescriptor &) = delete;
    FileDescriptor(FileDescriptor &&) noexcept;
    FileDescriptor &operator=(FileDescriptor &&) noexcept;
    ~FileDescriptor();

    void close();
    int get() const;

  private:
    int fd_ = -1;
  };
}

#endif
