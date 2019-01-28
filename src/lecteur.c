#include "../include/audio.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main (int argc, char *argv[])
{
	FILE *fichier = NULL;
	char filename[50];
	void * buf;
	int s = scanf("%s", filename);
	if(s < 0)
	{
		printf("Scan impossible\n");
		exit(1);
	}
	
	fichier = fopen(filename,  "r");
	if(fichier == NULL)
	{
		printf("Impossible d'ouvrir le fichier\n");
		exit(1);
	}
	
	int sample_rate, sample_size, channels;
	int fd;
	int fd1;
	fd = aud_readinit(filename, &sample_rate, &sample_size, &channels);
	buf=malloc(sample_size);
	fd1 = aud_writeinit(sample_rate, sample_size, channels);
	if(fd < 0 || fd1 < 0)
	{
		printf("Lecture/écriture du fichier impossible\n");
		exit(1);
	}
	while(read(fd, buf, sample_size) > 0)
	{	
		write(fd1,buf,sample_size);
	}	
	fclose(fichier);
	
	return 0;
	
}
	
