#include <qpdf/Pl_Buffer.hh>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

class Pl_Buffer::Members
{
  public:
    Members() = default;
    Members(Members const&) = delete;

    bool ready{true};
    std::string data;
};

Pl_Buffer::Pl_Buffer(char const* identifier, Pipeline* next) :
    Pipeline(identifier, next),
    m(std::make_unique<Members>())
{
}

// Must be explicit and not inline -- see QPDF_DLL_CLASS in README-maintainer
Pl_Buffer::~Pl_Buffer() = default;

void
Pl_Buffer::write(unsigned char const* buf, size_t len)
{
    if (!len) {
        return;
    }
    m->data.append(reinterpret_cast<char const*>(buf), len);
    m->ready = false;

    if (next()) {
        next()->write(buf, len);
    }
}

void
Pl_Buffer::finish()
{
    m->ready = true;
    if (next()) {
        next()->finish();
    }
}

Buffer*
Pl_Buffer::getBuffer()
{
    if (!m->ready) {
        throw std::logic_error("Pl_Buffer::getBuffer() called when not ready");
    }
    auto* b = new Buffer(std::move(m->data));
    m->data.clear();
    return b;
}

std::string
Pl_Buffer::getString()
{
    if (!m->ready) {
        throw std::logic_error("Pl_Buffer::getString() called when not ready");
    }
    auto s = std::move(m->data);
    m->data.clear();
    return s;
}

std::shared_ptr<Buffer>
Pl_Buffer::getBufferSharedPointer()
{
    return std::shared_ptr<Buffer>(getBuffer());
}

void
Pl_Buffer::getMallocBuffer(unsigned char** buf, size_t* len)
{
    if (!m->ready) {
        throw std::logic_error("Pl_Buffer::getMallocBuffer() called when not ready");
    }
    auto size = m->data.size();
    *len = size;
    if (size > 0) {
        *buf = reinterpret_cast<unsigned char*>(malloc(size));
        memcpy(*buf, m->data.data(), size);
    } else {
        *buf = nullptr;
    }
    m->data.clear();
}
