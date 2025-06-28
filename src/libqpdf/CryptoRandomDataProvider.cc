#include <qpdf/CryptoRandomDataProvider.hh>

#include <qpdf/QPDFCryptoProvider.hh>

void
CryptoRandomDataProvider::provideRandomData(unsigned char* data, size_t len)
{
    auto crypto = QPDFCryptoProvider::getImpl();
    crypto->provideRandomData(data, len);
}

RandomDataProvider*
CryptoRandomDataProvider::getInstance()
{
    static CryptoRandomDataProvider instance;
    return &instance;
}
