#include <iostream>
#include <fstream>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <immintrin.h>

using namespace std;
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned __int64 uint64;

void stream_read_plain_loop(const char* path)
{
    ifstream ifs(path);
    char c = 0;
    size_t count = 0;
    while (ifs.get(c)) {
        if (c == '\n')
            ++count;
    }
    cout << count << " " << path << endl;
}

#define BUF_SZ (4096)
void api_read_plain_loop(const char* path)
{
    HANDLE f = CreateFileA(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    char buf[BUF_SZ];
    int count = 0;
    while (true) {
        DWORD was_read = 0;
        ReadFile(f, buf, BUF_SZ, &was_read, NULL);
        for (int i = 0; i < BUF_SZ; ++i)
            if (buf[i] == '\n')
                ++count;
        if (was_read != BUF_SZ)
            break;
    }

    CloseHandle(f);
    cout << count << " " << path << endl;
}


void map_and(const char* path, size_t(*run_buf)(const char*, size_t))
{
    HANDLE f = CreateFileA(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    size_t sz = 0;
    GetFileSizeEx(f, (LARGE_INTEGER*)&sz);
    HANDLE m = CreateFileMapping(f, NULL, PAGE_READONLY, 0, 0, NULL);
    char* buf = (char*)MapViewOfFile(m, FILE_MAP_READ, 0, 0, sz);

    size_t count = run_buf(buf, sz);

    UnmapViewOfFile(buf);
    CloseHandle(m);
    CloseHandle(f);

    cout << count << " " << path << endl;
}

size_t loop_byte(const char* buf, size_t sz) {
    size_t count = 0;
    for (int i = 0; i < sz; ++i) {
        if (buf[i] == '\n')
            ++count;
    }
    return count;
}

// buggy - doesn't count all
size_t loop_int_cond(const char* buf, size_t sz) {
    size_t count = 0;
    uint* ibuf = (uint*)buf;
    sz /= 4;
    for(int i = 0; i < sz; ++i) {
        uint v = ibuf[i];
        if ( ((v & 0xff) == 0x0a) || 
             ((v & 0xff00) == 0x0a00) || 
             ((v & 0xff0000) == 0x0a0000) || 
             ((v & 0xff000000) == 0x0a000000) )
            ++count;
    }

    return count;
}

size_t loop_8_sum(const char* buf, size_t sz) {
    size_t count = 0;
    uint64* ibuf = (uint64*)buf;
    sz /= 8;
    for (int i = 0; i < sz; ++i) {
        uint64 v = ibuf[i];
        count += (v & (0xffLL << 0)) == 0x0aLL << 0;
        count += (v & (0xffLL << 8)) == 0x0aLL << 8;
        count += (v & (0xffLL << 16)) == 0x0aLL << 16;
        count += (v & (0xffLL << 24)) == 0x0aLL << 24;
        count += (v & (0xffLL << 32)) == 0x0aLL << 32;
        count += (v & (0xffLL << 40)) == 0x0aLL << 40;
        count += (v & (0xffLL << 48)) == 0x0aLL << 48;
        count += (v & (0xfflL << 56)) == 0x0aLL << 56;
    }

    return count;
}


size_t only_read_8(const char* buf, size_t sz) {
    size_t count = 0;
    uint64* ibuf = (uint64*)buf;
    sz /= 8;
    for (int i = 0; i < sz; ++i) {
        uint64 v = ibuf[i];
        count ^= v;
    }

    return count;
}



size_t simd_xor(const char* buf, size_t sz) {

    __m256i* ibuf = (__m256i*)buf;
    sz /= 32;
    __m256i x = _mm256_setzero_si256();
    for (int i = 0; i < sz; ++i) {
        __m256i v = _mm256_load_si256(ibuf + i);
        x = _mm256_xor_si256(x, v);
    }
    int *ptr = (int*)&x;
    return ptr[0];
   // _mm256_cmpeq_epi8
}

size_t simd_count(const char* buf, size_t sz) {

    __m256i* ibuf = (__m256i*)buf;
    sz = sz / 32 / 256;
    __m256i x = _mm256_setzero_si256();
    __m256i eols = _mm256_set1_epi8(0x0a);
    __m256i ones = _mm256_set1_epi8(1);
    __m256i v, r, ar;

    size_t tsum = 0;

    for (int i = 0; i < sz; ++i) {
        __m256i sum = _mm256_setzero_si256();
        for (int j = 0; j < 256; ++j) {
            v = _mm256_load_si256(ibuf++);
            r = _mm256_cmpeq_epi8(v, eols);
            ar = _mm256_and_si256(r, ones);

            sum = _mm256_add_epi8(sum, ar);
        }
        uchar *ptr = (uchar*)&sum;
        for(int vi = 0; vi < 32; ++vi)
            tsum += ptr[vi];
    }

    return tsum;
}





int main(int argc, const char* argv[])
{
    const char* filepath = argv[1];
    auto start = GetTickCount();

    //stream_read_plain_loop(filepath);
    //api_read_plain_loop(filepath);
    //map_and(filepath, loop_byte);
    map_and(filepath, simd_count);
    //map_and(filepath, simd_count_unwind);

    auto elapsed = GetTickCount() - start;
    cout << "took " << elapsed / 1000.0 << " sec" << endl;
}