#ifndef RC4_NATIVE_HH
#define RC4_NATIVE_HH

#include <cstring>

class RC4_native
{
  public:
    // key_len of -1 means treat key_data as a null-terminated string
    RC4_native(unsigned char const* key_data, int key_len = -1);

    // out_data = 0 means to encrypt/decrypt in place
    void process(unsigned char const* in_data, size_t len, unsigned char* out_data = 0);

  private:
    class RC4Key
    {
      public:
        unsigned char state[256];
        unsigned char x;
        unsigned char y;
    };

    RC4Key key;
};

#endif // RC4_NATIVE_HH
