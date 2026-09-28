#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#define buffer 5

int main(int argc, char* argv[])
{
    char buf[buffer] = {0};
    if (argc == 1)
    {
        size_t bytes = fread(buf, sizeof(char), buffer, stdin);
        while (bytes > 0)
        {
            fwrite(buf, sizeof(char), bytes, stdout);
            bytes = fread(buf, sizeof(char), buffer, stdin);
        }
    }
    
    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            FILE *file = fopen(argv[i],"r");
            if (file == NULL)
            {
                perror("oshibka");
                return 0;
            }
            size_t bytes = fread(buf, sizeof(char), buffer, file);
            while (bytes > 0)
            {
                fwrite(buf, sizeof(char), bytes, stdout);
                bytes = fread(buf, sizeof(char), buffer, file);
            }
            fclose(file);
        }

    }
    return 0;
}

