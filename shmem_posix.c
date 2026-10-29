#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>


int main()
{
    char *src_path = "src.txt";
    char *dst_path = "dst.txt";

    int src = open(src_path, O_RDONLY);
    struct stat st;
    fstat(src, &st);
    size_t sz = st.st_size;
    printf("%ld\n", sz);

    int dst = open(dst_path, O_RDWR | O_CREAT | O_TRUNC, 0644);

    ftruncate(dst, sz);

    void *s = mmap(NULL, sz, PROT_READ, MAP_SHARED, src, 0);
    void *d = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, dst, 0);

    memcpy(d, s, sz);

    munmap(s, sz);
    munmap(d, sz);

    close(src);
    close(dst);

    return 0;
}
