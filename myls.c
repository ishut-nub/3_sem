#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <getopt.h>
#include <limits.h>
#include <stdlib.h>

int fl_l = 0;
int fl_a = 0;
int fl_i = 0;
int fl_n = 0;
int fl_R = 0;


void show(char *dir, char *name)
{
    char path[PATH_MAX];
    struct stat st;
    sprintf(path, "%s/%s", dir, name);
    if (lstat(path, &st) == -1) {
        perror(path);
        return;
    }
    if (fl_i) {
        printf("%lu ", (unsigned long)st.st_ino);
    }
    if (fl_l) {
        char mode[11] = "----------";
        if (S_ISDIR(st.st_mode))
            mode[0] = 'd';
        if (S_ISLNK(st.st_mode))
            mode[0] = 'l';
        if (S_ISCHR(st.st_mode))
            mode[0] = 'c';
        if (S_ISBLK(st.st_mode))
            mode[0] = 'b';
        if (S_ISFIFO(st.st_mode))
            mode[0] = 'p';
        if (S_ISSOCK(st.st_mode))
            mode[0] = 's';
        if (st.st_mode & S_IRUSR) mode[1] = 'r';
        if (st.st_mode & S_IWUSR) mode[2] = 'w';
        if (st.st_mode & S_IXUSR) mode[3] = 'x';

        if (st.st_mode & S_IRGRP) mode[4] = 'r';
        if (st.st_mode & S_IWGRP) mode[5] = 'w';
        if (st.st_mode & S_IXGRP) mode[6] = 'x';

        if (st.st_mode & S_IROTH) mode[7] = 'r';
        if (st.st_mode & S_IWOTH) mode[8] = 'w';
        if (st.st_mode & S_IXOTH) mode[9] = 'x';

        printf("%s ", mode);
        printf("%lu ", (unsigned long)st.st_nlink);
        if (fl_n == 0) {
            struct passwd *pw;
            pw = getpwuid(st.st_uid);
            if (pw != NULL)
                printf("%s ", pw->pw_name);
            else
                printf("%u ", st.st_uid);
        } else {
            printf("%u ", st.st_uid);
        }
        if (fl_n == 0) {
            struct group *gr;
            gr = getgrgid(st.st_gid);

            if (gr != NULL)
                printf("%s ", gr->gr_name);
            else
                printf("%u ", st.st_gid);
        } else {
            printf("%u ", st.st_gid);
        }

        printf("%ld ", (long)st.st_size);
        char date[32];
        struct tm *tm_info;

        tm_info = localtime(&st.st_mtime);
        strftime(date, sizeof(date), "%b %e %H:%M", tm_info);

        printf("%s ", date);
    }

    printf("%s", name);
    if (fl_l && S_ISLNK(st.st_mode)) {
        char buf[PATH_MAX];
        int len;

        len = readlink(path, buf, sizeof(buf) - 1);

        if (len != -1) {
            buf[len] = '\0';
            printf(" -> %s", buf);
        }
    }
    printf("\n");
}
int cmp(const void *a, const void *b)
{
    char **s1 = (char **)a;
    char **s2 = (char **)b;

    return strcmp(*s1, *s2);
}
void list(char *dir)
{
    DIR *d;
    struct dirent *entry;
    char *names[4096];
    int n = 0;

    d = opendir(dir);
    if (d == NULL) {
        perror(dir);
        return;
    }
    while ((entry = readdir(d)) != NULL) {
        if (fl_a == 0 && entry->d_name[0] == '.')
            continue;
        names[n] = strdup(entry->d_name);
        n++;
        if (n == 4096)
            break;
    }
    closedir(d);
    qsort(names, n, sizeof(char *), cmp);

    if (fl_R)
        printf("%s:\n", dir);
    for (int i = 0; i < n; i++) {
        show(dir, names[i]);
    }
    if (fl_R) {
        for (int i = 0; i < n; i++) {
            if (strcmp(names[i], ".") == 0)
                continue;
            if (strcmp(names[i], "..") == 0)
                continue;
            char path[PATH_MAX];
            struct stat st;

            sprintf(path, "%s/%s", dir, names[i]);
            if (lstat(path, &st) == 0) {

                if (S_ISDIR(st.st_mode)) {
                    printf("\n");
                    list(path);
                }
            }
        }
    }
    for (int i = 0; i < n; i++) {
        free(names[i]);
    }
}
int main(int argc, char **argv)
{
    struct option opts[] = {
        {"all", 0, 0, 'a'},
        {"inode", 0, 0, 'i'},
        {"numeric-uid-gid", 0, 0, 'n'},
        {"recursive", 0, 0, 'R'},
        {0, 0, 0, 0}
    };
    int c;
    while ((c = getopt_long(argc, argv, "1lainR", opts, NULL)) != -1) {
        if (c == 'l')
            fl_l = 1;
        else if (c == 'a')
            fl_a = 1;
        else if (c == 'i')
            fl_i = 1;
        else if (c == 'n') {
            fl_n = 1;
            fl_l = 1;
        }
        else if (c == 'R')
            fl_R = 1;
        else if (c == '1')
            ;
        else
            return 1;
    }
    if (optind == argc) {
        list(".");
    }
    for (int i = optind; i < argc; i++) {
        if (argc - optind > 1 && fl_R == 0)
            printf("%s:\n", argv[i]);
        list(argv[i]);
        if (i < argc - 1)
            printf("\n");
    }
    return 0;
}

