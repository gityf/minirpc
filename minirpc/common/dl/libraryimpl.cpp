#include "common/dl/libraryimpl.h"

namespace wyf {

void LibraryImpl::open(const std::string& path) {
    if(_handle) return;

    /* RTLD_NOW: since lazy loading is not supported by every target platform
        RTLD_GLOBAL: make the external symbols in the loaded library available for subsequent libraries.
                    see also http://gcc.gnu.org/faq.html#dso
    */
    int flags = RTLD_NOW | RTLD_GLOBAL;

    _handle = ::dlopen(path.c_str(), flags);
    if(!_handle) {
        throw string(dlerror());
    }
}


void LibraryImpl::close() {
    if(_handle) ::dlclose(_handle);
}

void* LibraryImpl::resolve(const char* symbol) const
{
    if(_handle) {
        return ::dlsym(_handle, symbol);
    }

    return 0;
}

} // namespace wyf
