#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>

int main(int argc, char *argv[]) {
    struct stat sb;
    off_t len;
    char *p;
    int fd;
    struct timeval ini, fim;
    long tempo_mmap, tempo_read;

    if (argc < 2) {
        fprintf(stderr, "usage: %s <file>\n", argv[0]);
        exit(1);
    }

    fd = open(argv[1], O_RDONLY);
    fstat(fd, &sb);

    // a. Número de páginas de memória
    long page_size = sysconf(_SC_PAGESIZE);
    long num_pages = (sb.st_size + page_size - 1) / page_size;
    printf("Número de páginas: %ld\n", num_pages);

    // b. Tempo com mmap
    gettimeofday(&ini, NULL);
    p = mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    for (len = 0; len < sb.st_size; len++) (void)p[len];
    gettimeofday(&fim, NULL);
    tempo_mmap = (fim.tv_sec - ini.tv_sec) * 1000000 + (fim.tv_usec - ini.tv_usec);
    munmap(p, sb.st_size);

    // b. Tempo com read
    lseek(fd, 0, SEEK_SET);
    char buf[4096];
    gettimeofday(&ini, NULL);
    while (read(fd, buf, sizeof(buf)) > 0);
    gettimeofday(&fim, NULL);
    tempo_read = (fim.tv_sec - ini.tv_sec) * 1000000 + (fim.tv_usec - ini.tv_usec);

    printf("Tempo com mmap: %ld us\n", tempo_mmap);
    printf("Tempo com read: %ld us\n", tempo_read);

    close(fd);
    return 0;
}
