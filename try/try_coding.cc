#include <iostream>

using namespace std;

void print_pointer(char* dst, int size) {
    for (int i = 0; i < size; ++i) {
        for (int j = 7; j >= 0; --j) {
            cout << ((dst[i] >> j) & 1);
        }
        cout << " ";
    }
    cout << endl;
}

char* EncodeVarint32(char* dst, uint32_t v) {
    // operate on characters as unsigneds
    uint8_t* ptr = reinterpret_cast<uint8_t*>(dst);
    static const int B = 128;
    if (v < (1 << 7)) {
        *(ptr++) = v;
    } else if (v < (1 << 14)) {
        *(ptr++) = v | B;
        *(ptr++) = v >> 7;
    } else if (v < (1 << 21)) {
        *(ptr++) = v | B;
        *(ptr++) = (v >> 7) | B;
        *(ptr++) = v >> 14;
    } else if (v < (1 << 28)) {
        *(ptr++) = v | B;
        *(ptr++) = (v >> 7) | B;
        *(ptr++) = (v >> 14) | B;
        *(ptr++) = v >> 21;
    } else {
        *(ptr++) = v | B;
        *(ptr++) = (v >> 7) | B;
        *(ptr++) = (v >> 14) | B;
        *(ptr++) = (v >> 21) | B;
        *(ptr++) = v >> 28;
    }
    return reinterpret_cast<char*>(ptr);
}


int main() {
    char buf[5] = {0};
    auto res = EncodeVarint32(buf, 127);
    print_pointer(buf, 5);
}