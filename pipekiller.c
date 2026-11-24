#include <stdio.h>
#include <string.h>

#define SIZE (70 * 1024)

int main(void) {
    char buffer[SIZE];

    if (fwrite(buffer, 1, SIZE, stdout) != SIZE) {
        return 1;
    }


    if (fwrite(buffer, 1, SIZE, stderr) != SIZE) {
        return 1;
    }

    return 0;
}
