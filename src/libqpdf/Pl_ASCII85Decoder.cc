#include <qpdf/Pl_ASCII85Decoder.hh>

#include <qpdf/QTC.hh>
#include <cstring>
#include <stdexcept>

Pl_ASCII85Decoder::Pl_ASCII85Decoder(char const* identifier, Pipeline* next) :
    Pipeline(identifier, next)
{
    if (!next) {
        throw std::logic_error("Attempt to create Pl_ASCII85Decoder with nullptr as next");
    }
}

void
Pl_ASCII85Decoder::write(unsigned char const* buf, size_t len)
{
    if (eod > 1) {
        return;
    }
    for (size_t i = 0; i < len; ++i) {
        switch (buf[i]) {
        case ' ':
        case '\f':
        case '\v':
        case '\t':
        case '\r':
        case '\n':
            QTC::TC("libtests", "Pl_ASCII85Decoder ignore space");
            // ignore whitespace
            continue;
        }
        if (eod > 1) {
            break;
        } else if (eod == 1) {
            if (buf[i] == '>') {
                flush();
                eod = 2;
            } else {
                throw std::runtime_error("broken end-of-data sequence in base 85 data");
            }
        } else {
            switch (buf[i]) {
            case '~':
                eod = 1;
                break;

            case 'z':
                if (pos != 0) {
                    throw std::runtime_error("unexpected z during base 85 decode");
                } else {
                    QTC::TC("libtests", "Pl_ASCII85Decoder read z");
                    unsigned char zeroes[4];
                    memset(zeroes, '\0', 4);
                    next()->write(zeroes, 4);
                }
                break;

            default:
                if ((buf[i] < 33) || (buf[i] > 117)) {
                    error = true;
                    throw std::runtime_error("character out of range during base 85 decode");
                } else {
                    this->inbuf[this->pos++] = buf[i];
                    if (pos == 5) {
                        flush();
                    }
                }
                break;
            }
        }
    }
}

void
Pl_ASCII85Decoder::flush()
{
    if (this->pos == 0) {
        QTC::TC("libtests", "Pl_ASCII85Decoder no-op flush");
        return;
    }
    unsigned long lval = 0;
    for (int i = 0; i < 5; ++i) {
        lval *= 85;
        lval += (this->inbuf[i] - 33U);
    }

    unsigned char outbuf[4];
    memset(outbuf, 0, 4);
    for (int i = 3; i >= 0; --i) {
        outbuf[i] = lval & 0xff;
        lval >>= 8;
    }

    QTC::TC("libtests", "Pl_ASCII85Decoder partial flush", (this->pos == 5) ? 0 : 1);
    // Reset before calling getNext()->write in case that throws an exception.
    auto t = this->pos - 1;
    this->pos = 0;
    memset(this->inbuf, 117, 5);

    next()->write(outbuf, t);
}

void
Pl_ASCII85Decoder::finish()
{
    if (error) {
        return;
    }
    flush();
    next()->finish();
}
