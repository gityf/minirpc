#include "common/dl/libraryimpl.h"
#include "common/dl/library.h"
#include <string>
#include <memory>

namespace wyf {

Library::Library()
: _impl(0) {
    _impl = new LibraryImpl();
}

Library::Library(const std::string& path)
: _impl(0) {
    std::auto_ptr<LibraryImpl> impl(new LibraryImpl());
    _impl = impl.get();
    open(path);
    impl.release();
}

Library::Library(const Library& other) {
    _path = other._path;
    _impl = other._impl;
    _impl->ref();
}

Library& Library::operator=(const Library& other) {
    if(_impl == other._impl) return *this;

    _path = other._path;

    other._impl->ref();

    if(!_impl->unref())
        delete _impl;

    _impl = other._impl;

    return *this;
}

Library::~Library() {
    if (!_impl->unref())
        delete _impl;
}

void Library::detach() {
    if (_impl->refs() == 1)
        return;

    _path.clear();

    LibraryImpl* x = _impl;
    _impl = new LibraryImpl();

    if(!x->unref()) delete x;
}

Library& Library::open(const std::string& libname) {
    this->detach();

    // libmylib.so
    try {
        _impl->open(libname);
        _path = libname;
        return *this;
    } catch(...) { }

    std::string path = libname;
    path += suffix();
    // libmylib --> libmylib.so
    try {
        _impl->open(path);
        _path = path;
        return *this;
    } catch(...) { }

    std::string::size_type idx = path.rfind("/");
    if(idx == std::string::npos) {
        idx = 0;
    } else if(++idx == path.length()) {
        throw path;
    }

    // mylib.so --> libmylib.so
    path.insert(idx, prefix());
    _impl->open(path);
    _path = path;

    return *this;
}

void Library::close() {
    this->detach();

    _impl->close();
}

void* Library::resolve(const char* symbol) const
{
  return _impl->resolve(symbol);
}

Symbol Library::getSymbol(const char* symbol) const
{
    void* sym = this->resolve(symbol);
    if (sym == 0) {
        throw string(symbol);
    }

    return Symbol(*this, sym);
}

Library::operator const void*() const
{
    return _impl->failed() ? 0 : this;
}

bool Library::operator!() const
{
    return _impl->failed();
}

const std::string& Library::path() const
{
    return _path;
}

std::string Library::suffix() {
    return LibraryImpl::suffix();
}

std::string Library::prefix() {
    return LibraryImpl::prefix();
}

} // namespace wyf
