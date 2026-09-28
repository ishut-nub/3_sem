#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	write(1,"hello world\n", 12);
	int newline = 0;
	if (argc == 1)
	{
		printf("\n");
		return 0;
	}
	if (strcmp(argv[1],"-n") == 0)
		newline = 1;
	if (newline != 1)
		printf("%s ", argv[1]);
	for (int i = 2; i < argc-1; i++)
		printf("%s ", argv[i]);
	if (newline == 1)
		printf("%s", argv[argc-1]);
	else
		printf("%s\n", argv[argc-1]);
	return 0;
}
