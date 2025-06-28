#include <qpdf/Pl_Function.hh>

#include <stdexcept>

class Pl_Function::Members
{
  public:
    Members(writer_t fn) :
        fn(fn)
    {
    }
    Members(Members const&) = delete;
    ~Members() = default;

    writer_t fn;
};

Pl_Function::Pl_Function(char const* identifier, Pipeline* next, writer_t fn) :
    Pipeline(identifier, next),
    m(std::make_unique<Members>(fn))
{
}

Pl_Function::Pl_Function(char const* identifier, Pipeline* next, writer_c_t fn, void* udata) :
    Pipeline(identifier, next),
    m(std::make_unique<Members>(nullptr))
{
    m->fn = [identifier, fn, udata](unsigned char const* data, size_t len) {
        int code = fn(data, len, udata);
        if (code != 0) {
            throw std::runtime_error(
                std::string(identifier) + " function returned code " + std::to_string(code));
        }
    };
}

Pl_Function::Pl_Function(char const* identifier, Pipeline* next, writer_c_char_t fn, void* udata) :
    Pipeline(identifier, next),
    m(new Members(nullptr))
{
    m->fn = [identifier, fn, udata](unsigned char const* data, size_t len) {
        int code = fn(reinterpret_cast<char const*>(data), len, udata);
        if (code != 0) {
            throw std::runtime_error(
                std::string(identifier) + " function returned code " + std::to_string(code));
        }
    };
}

Pl_Function::~Pl_Function() // NOLINT (modernize-use-equals-default)
{
    // Must be explicit and not inline -- see QPDF_DLL_CLASS in README-maintainer
}

void
Pl_Function::write(unsigned char const* buf, size_t len)
{
    m->fn(buf, len);
    if (next()) {
        next()->write(buf, len);
    }
}

void
Pl_Function::finish()
{
    if (next()) {
        next()->finish();
    }
}
