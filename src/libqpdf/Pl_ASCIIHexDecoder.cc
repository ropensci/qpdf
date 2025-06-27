#include <qpdf/Pl_ASCIIHexDecoder.hh>

#include <qpdf/QTC.hh>
#include <cctype>
#include <stdexcept>

using namespace std::literals;

Pl_ASCIIHexDecoder::Pl_ASCIIHexDecoder(char const* identifier, Pipeline* next) :
    Pipeline(identifier, next)
{
    if (!next) {
        throw std::logic_error("Attempt to create Pl_ASCIIHexDecoder with nullptr as next");
    }
}

void
Pl_ASCIIHexDecoder::write(unsigned char const* buf, size_t len)
{
    if (this->eod) {
        return;
    }
    for (size_t i = 0; i < len; ++i) {
        char ch = static_cast<char>(toupper(buf[i]));
        switch (ch) {
        case ' ':
        case '\f':
        case '\v':
        case '\t':
        case '\r':
        case '\n':
            QTC::TC("libtests", "Pl_ASCIIHexDecoder ignore space");
            // ignore whitespace
            break;

        case '>':
            this->eod = true;
            flush();
            break;

        default:
            if (((ch >= '0') && (ch <= '9')) || ((ch >= 'A') && (ch <= 'F'))) {
                this->inbuf[this->pos++] = ch;
                if (this->pos == 2) {
                    flush();
                }
            } else {
                char t[2];
                t[0] = ch;
                t[1] = 0;
                throw std::runtime_error("character out of range during base Hex decode: "s + t);
            }
            break;
        }
        if (this->eod) {
            break;
        }
    }
}

void
Pl_ASCIIHexDecoder::flush()
{
    if (this->pos == 0) {
        QTC::TC("libtests", "Pl_ASCIIHexDecoder no-op flush");
        return;
    }
    int b[2];
    for (int i = 0; i < 2; ++i) {
        if (this->inbuf[i] >= 'A') {
            b[i] = this->inbuf[i] - 'A' + 10;
        } else {
            b[i] = this->inbuf[i] - '0';
        }
    }
    auto ch = static_cast<unsigned char>((b[0] << 4) + b[1]);

    QTC::TC("libtests", "Pl_ASCIIHexDecoder partial flush", (this->pos == 2) ? 0 : 1);
    // Reset before calling getNext()->write in case that throws an exception.
    this->pos = 0;
    this->inbuf[0] = '0';
    this->inbuf[1] = '0';
    this->inbuf[2] = '\0';

    next()->write(&ch, 1);
}

void
Pl_ASCIIHexDecoder::finish()
{
    flush();
    next()->finish();
}
