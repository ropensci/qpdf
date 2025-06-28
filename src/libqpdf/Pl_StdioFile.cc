#include <qpdf/qpdf-config.h> // include first for large file support

#include <qpdf/Pl_StdioFile.hh>

#include <qpdf/QUtil.hh>
#include <cerrno>
#include <stdexcept>

class Pl_StdioFile::Members
{
  public:
    Members(FILE* f) :
        file(f)
    {
    }
    Members(Members const&) = delete;
    ~Members() = default;

    FILE* file;
};

Pl_StdioFile::Pl_StdioFile(char const* identifier, FILE* f) :
    Pipeline(identifier, nullptr),
    m(std::make_unique<Members>(f))
{
}

// Must be explicit and not inline -- see QPDF_DLL_CLASS in README-maintainer
Pl_StdioFile::~Pl_StdioFile() = default;

void
Pl_StdioFile::write(unsigned char const* buf, size_t len)
{
    size_t so_far = 0;
    while (len > 0) {
        so_far = fwrite(buf, 1, len, m->file);
        if (so_far == 0) {
            QUtil::throw_system_error(this->identifier + ": Pl_StdioFile::write");
        } else {
            buf += so_far;
            len -= so_far;
        }
    }
}

void
Pl_StdioFile::finish()
{
    if ((fflush(m->file) == -1) && (errno == EBADF)) {
        throw std::logic_error(this->identifier + ": Pl_StdioFile::finish: stream already closed");
    }
}
