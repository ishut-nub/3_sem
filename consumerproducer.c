
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>

#define BUF 1024

typedef struct {
	char data[BUF];
	size_t count;
} Chunk;


void producer(int fd, Chunk* buf, sem_t *full, sem_t *empty){
	while (true) {
		sem_wait(full);

		ssize_t bytes = read(fd, buf->data, BUF);
		if (bytes < 0) {
            perror("read failed");
			exit(1);
		}
        buf->count = bytes;

		sem_post(empty);

		if (buf->count == 0)
            break;
	}
}

void consumer(Chunk* buf, sem_t *full, sem_t *empty){
	while (true) {
		sem_wait(empty);

		if (buf->count == 0) break;

		ssize_t w = write(1, buf->data, buf->count);
		if (w < 0){
			perror("write failed");
			exit(1);
		}

		sem_post(full);
	}
}


int main(int argc, char* argv[]){
    sem_unlink("/hello");
    sem_unlink("/bye");

	Chunk* buf = mmap(NULL, sizeof(Chunk), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);

	sem_t* full = sem_open("/hello", O_CREAT | O_EXCL, 0666, 1);
	sem_t* empty = sem_open("/bye", O_CREAT | O_EXCL, 0666, 0);

	int fd = 0;
	if (argc > 1){
		fd = open(argv[1], O_RDONLY);
	}
	if (fd < 0){
		perror("open failed");
		exit(1);
	}

	pid_t p = fork();
	if (p < 0) {
		perror("fork failed");
		exit(1);
	}

	if (p == 0){
		producer(fd, buf, full, empty);
		if (fd != 0)
            close(fd);
		return 0;
	}

	consumer(buf, full, empty);

	wait(NULL);

	sem_close(full);
	sem_close(empty);
	sem_unlink("/hello");
	sem_unlink("/bye");
	munmap(buf, sizeof(Chunk));

	return 0;
}
