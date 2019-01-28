
#include "../include/audio.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>

#define SIZEOFBUFF 16

int main (char argc, char *argv[])
{
	// localhost (127.0.0.1) & nom_music
	if(argc != 3)
	{
		printf("Nombre d'argument incorrect ! Il en faut 3 ou 5 pour les options !\n");
		exit(1);
	}
	
	// Reception des arguments
	char name[100];
	strcpy(name, argv[1]);
	char song[100];
	strcpy(song, argv[2]);
	
	
	// Déclaration des sockets 
	struct sockaddr_in addr; 
	struct sockaddr_in dest;
	socklen_t fromlen = sizeof(addr);
	int fd = socket(AF_INET, SOCK_DGRAM,0);
	if(fd < 0)
	{
		perror("Création de socket impossible\n");
		exit(1);
	}
		
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1900);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	
	// Initialisation des variables
	int sample_rate, sample_size, channels;
	int fdMusic;
	int fd1;
	int receive;
	int tab[3];

	// Envoie du titre de la musique 
	int send = sendto(fd, &song, sizeof(char[100]), 0, (struct sockaddr*) &addr, fromlen); 
	printf("Send : %s\n", song); 
	if(send < 0) 
	{
		perror("Send impossible\n");
		exit(1);
	}

		
	// Reception 
	receive = recvfrom(fd, tab, sizeof(tab), 0, (struct sockaddr*) &dest, &fromlen);
	if(receive < 0)
	{
		perror("Erreur reception\n");
		exit(1);
	}
	
	// Reception des informations de la musique
	sample_rate = tab[0];
	sample_size = tab[1];
	channels = tab[2];
	printf("Receive : \n");
	printf("Sample_rate : %d\n", sample_rate);
	printf("Sample_size : %d\n", sample_size);
	printf("Channels: %d\n\n", channels);
	
	// Gestion si un client est déjà connecté au serveur
	if(channels > 10 || channels < 0)
	{
		printf("Impossible, le serveur est occupé, réessayer plus tard ;)\n");
		exit(1);
	}
	
	// Initialisation 
	fd1 = aud_writeinit(sample_rate, sample_size, channels);
	void *buff = malloc(sample_size);
	
	// Timeout
	struct timeval timeout;
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	do
	{
		// Reception de l'échantillon
		setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
		receive = recvfrom(fd, buff, SIZEOFBUFF, 0, (struct sockaddr*) &dest, &fromlen);
		if(receive < 0)
		{
			perror("Erreur reception\n");
			exit(1);
		}
		// Lecture de l'échantillon
		write(fd1, buff, sample_size);
		if(fd < 0 || fd1 < 0)
		{				
			perror("Lecture/écriture du fichier impossible\n");
			exit(1);
		}
		
		// Envoie d'un acquittement au serveur
		if(strcmp(buff, "Fin de transmiss") != 0)
		{
			send = sendto(fd, "Check", sizeof("Check")+1, 0, (struct sockaddr*) &addr, fromlen);
			if(send < 0)
			{
				perror("Erreur\n");
				exit(1);
			}
		}
	}while(strcmp(buff, "Fin de transmiss") != 0);
	// Quand on reçoit le message de fin, on coupe le serveur 
	
	int end = close(fd);
	if(end < 0)
	{
		perror("Error close fd\n");
		exit(1);
	}
	
	end = close(fd1);
	if(end < 0)
	{
		perror("Error close fd1\n");
		exit(1);
	}
	end = close(fdMusic);
	if(end < 0)
	{
		perror("Error close fdMusic\n");
	}

	return 0;
}
