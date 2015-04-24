#ifndef _DL_LIBRARYIMPL_H
#define _DL_LIBRARYIMPL_H

#include <string>
#include <dlfcn.h>
using std::string;
namespace wyf {

class LibraryImpl
{
    public:
        LibraryImpl()
        : _refs(1)
        , _handle(0) { }

        LibraryImpl(const std::string& path)
        : _refs(1)
        , _handle(0) {
            this->open(path);
        }

        ~LibraryImpl() {
            if(_handle)
                ::dlclose(_handle);
        }

        unsigned refs() const
        {
            return _refs;
        }

        unsigned ref() {
            return ++_refs;
        }

        unsigned unref() {
            return --_refs;
        }

        void open(const std::string& path);

        void close();

        void* resolve(const char* symbol) const;

        bool failed() { return _handle == 0; }

        static std::string suffix() {
            return ".so";
        }

        static std::string prefix() {
            return "lib";
        }

    private:
        unsigned _refs;
        void* _handle;
};

} // namespace wyf

#endif
