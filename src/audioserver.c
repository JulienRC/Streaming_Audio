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
#include <time.h>
#include <arpa/inet.h>

#define SIZEOFBUFF 1024

int main()
{
	struct sockaddr_in addr; 
	struct sockaddr_in dest;
	struct sockaddr_in stop;
	socklen_t from = sizeof(struct sockaddr_in);
	
	int fd = socket(AF_INET, SOCK_DGRAM,0);
	if(socket < 0)
	{
		perror("Création de socket impossible\n");
		exit(1);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1900);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	// Connexion
	int port = bind(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
	if(port < 0)
	{
		perror("Bind impossible\n");
		exit(1);
	}
	
	// Initialisation des variables
	int sample_rate, sample_size, channels;
	int fdMusic;
	int fd1;
	int sendError = 0;
	
	
	// Timeout
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	
	// Pour permettre au serveur de tourner en permanence pour avoir les requêtes des clients
	while(1)
	{
		
		void * buff = malloc(SIZEOFBUFF);
		
		printf("%s", (char*)buff);
		
		// Reception du nom de la musique
		setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
		int receive = recvfrom(fd, buff, SIZEOFBUFF, 0, (struct sockaddr*) &dest, &from);
		printf("Receive : %s\n", (char*)buff);
		if(receive < 0)
		{
			perror("Receive impossible\n");
			exit(1);
		}

		printf("Connexion d'un client\n");
		printf("IP Client : %s\n", (char *) inet_ntoa(addr.sin_addr));
		printf("Send : %s\n", (char*)buff);

		// Initialisation des variables de la musique
		fdMusic = aud_readinit(buff, &sample_rate, &sample_size, &channels);
				
		int send;

		int tab[3];
		tab[0] = sample_rate;
		tab[1] = sample_size;
		tab[2] = channels;
			
		// Envoi des informations de la musique au client
		send = sendto(fd, tab, sizeof(tab), 0, (struct sockaddr*) &dest, from);
		if(send < 0)
		{
			perror("Send impossible\n");
			exit(1);
		}
		
		// Gestion des échantillons
		int nbTransmissionErreur = 0;
		int nbTransmission = 0;
		
		int buffClient[1024];
			
		
		// Lecture de la musique dans le descripteur
		while(read(fdMusic, buff, sample_size) > 0)
		{
			// Envoie au client des échantillons
			send = sendto(fd, buff, SIZEOFBUFF+1, 0, (struct sockaddr*) &dest, from);
			if(send < 0)
			{
				printf("Send impossible\n");
				nbTransmissionErreur++; // On a mal reçu un paquet d'échantillon
			}
			else
			{
				// Reception de l'acquittement 
				timeout.tv_sec = 5;
				setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
				receive = recvfrom(fd, buffClient, SIZEOFBUFF, 0, (struct sockaddr*) &dest, &from);
				if(receive < 0)
				{
					perror("Error recv\n");
					sendError = 1;
					break;
				}
			}
				nbTransmission++;
		}
		// On envoie au client que la musique est fini 
		if(sendError == 0)
		{
			send = sendto(fd, "Fin de transmission", sizeof("Fin de transmission")+1, 0, (struct sockaddr*) &dest, from);
			if(send < 0)
			{
				printf("Fin de tranmission impossible\n");
			}
		}
	
		// Affichage du nombre de transmission
		printf("Transmission Totale : %d\n", nbTransmission);
		printf("Transmission Erreur : %d\n\n\n", nbTransmissionErreur);
		
		timeout.tv_sec = 0;
		buff = NULL;
		free(buff);
	}
	// Fermeture des descripteurs de fichier
	int end = close(fd);
	if(end < 0)
	{
		perror("Close impossible\n");
		exit(1);
	}
	
	end = close(fd1);
	if(end < 0)
	{
		perror("Error fd1\n");
	}
	
	end = close(fdMusic);
	if(end < 0)
	{
		perror("Close impossible\n");
		exit(1);
	}
	return 0;
}
