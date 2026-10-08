#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>


int main() {
    DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("opendir fail");
        return 1;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }
    closedir(dir);
    return 0;
}
