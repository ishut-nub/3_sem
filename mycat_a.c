#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#define buffer 5

int main(int argc, char* argv[])
{
    char buf[buffer] = {0};
    if (argc == 1)
    {
        ssize_t bytes = read(0, buf, buffer);
        while (bytes > 0)
        {
            write(1, buf, bytes);
            bytes = read(0, buf, buffer);
        }
    }
    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            int fd = open(argv[i], O_RDONLY);
            if (fd < 0)
            {
                perror("oshibka");
                return 0;
            }
            ssize_t bytes = read(fd, buf, buffer);
            while (bytes > 0)
            {
                write(1, buf, bytes);
                bytes = read(fd, buf, buffer);
            }
            close(fd);
        }

    }
    return 0;
}

