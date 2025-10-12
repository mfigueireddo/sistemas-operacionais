#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    struct stat sb;
    off_t len;
    char *p;
    int fd;

    if (argc < 2) {
        fprintf(stderr, "usage: %s <file>\n", argv[0]);
        exit(1);
    }

    fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    if (fstat(fd, &sb) == -1) {
        perror("fstat");
        exit(1);
    }

    if (!S_ISREG(sb.st_mode)) {
        fprintf(stderr, "%s is not a file\n", argv[1]);
        exit(1);
    }

    const char *message = "Mensagem do arquivo de escrita";
    size_t message_len = strlen(message) + 1;

    // Garante que o arquivo tenha espaço suficiente
    if (ftruncate(fd, message_len) == -1) {
        perror("ftruncate");
        exit(1);
    }

    // Mapeia o arquivo para memória
    p = mmap(0, message_len, PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    if (close(fd) == -1) {
        perror("close");
        exit(1);
    }

    memcpy(p, message, message_len);

    if (munmap(p, message_len) == -1) {
        perror("munmap");
        exit(1);
    }

    return 0;
}
