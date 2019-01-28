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

#define couleur(param) printf("\033[%sm", param)
#define SIZEOFBUFF 16

void changementVolume(void * buff, int volume);

int main (char argc, char *argv[])
{
	// localhost (127.0.0.1) & nom_music
	if(argc < 3 || argc > 5)
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
	
	// Initialisation Socket	
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

	// Reception help
	if(strcmp("help", argv[2]) == 0)
	{
		char * buffHelp[1024];
		
		printf("\n\nListe des musiques disponible sur le serveur : \n");
		
		while( strcmp((char *) buffHelp, "Fin de transmission") != 0 )
		{
			printf("%s\n", (char *) buffHelp);
			recvfrom(fd, buffHelp, 1024, 0, (struct sockaddr*) &dest, &fromlen);
		}
		couleur("36;1");
		printf("\nFiltre 1 : \033[4mmono\033[0;36;1m : Permet de passer de stereo à mono\n");
		printf("Filtre 2 : \033[4mspeed\033[0;36;1m \033[34;1m[+/- entier]\033[0;36;1m : Permet d'augmenter ou de baisser la vitesse de lecture\n");
		printf("Filtre 3 : \033[4msaturation\033[0;36;1m : Joue la musique avec de la saturation\n");
		printf("Filtre 4 : \033[4mvolume\033[0;36;1m \033[34;1m[+/- entier]\033[0;36;1m : Permet d'augmenter ou de baisser le volume\n");
		printf("Filtre 5 : \033[4mvolume_random\033[0;36;1m : Permet de jouer la musique avec une variation de volume sinusoïdale\n");
		printf("Filtre 6 : \033[4m8bit\033[0;36;1m : Joue la musique avec un effet 8bit\n\n");
		couleur("0");
		exit(1);
	}
	
	
	// Reception des informations de la musique
	receive = recvfrom(fd, tab, sizeof(tab), 0, (struct sockaddr*) &dest, &fromlen);
	if(receive < 0)
	{
		perror("Erreur reception\n");
		exit(1);
	}
	
	int volume = 0;
	int volumeRandom = 0;
	int bit = 0;
	void* buffBit = malloc(16);
	int boucle = 0;
	int volumeBit = 1;
	int echo = 0;
	
	sample_rate = tab[0];
	sample_size = tab[1];
	channels = tab[2];
	
	// Gestion des filtres
	if(argc >= 4)
	{
		if(strcmp("mono", argv[3]) == 0) // Mono
		{ 	
			channels = 1;
			sample_rate *= 2;
		}	
		else if(strcmp("speed", argv[3]) == 0) // Speed
		{ 
			printf("Speed\n");
			if(argc == 5)
			{
				if(atof(argv[4]) > 0) { sample_rate = (int) (tab[0] * atof(argv[4])); }
				else if(atof(argv[4]) < 0) { sample_rate = (int) (tab[0] / (-atof(argv[4]))); }
			}
		}
		else if(strcmp("saturation", argv[3]) == 0) // Saturation
		{
			sample_size = tab[1]/2;
		}
		else if(strcmp("volume", argv[3]) == 0) // Volume
		{
			if(argc == 5)
			{
				volume = atoi(argv[4]);
			}
		}
		else if(strcmp("volume_random", argv[3]) == 0) // volume_Random
		{
			volumeRandom = 1;
		}
		else if(strcmp("8bit", argv[3]) == 0) // 8bit
		{
			bit = 1;
		}
		else
		{
			printf("Erreur sur le nom du filtre mais lecture normale de la musique\n");
		}
	}

	printf("Receive : \n");
	printf("Sample_rate : %d\n", sample_rate);
	printf("Sample_size : %d\n", sample_size);
	printf("Channels: %d\n\n", channels);
	
	// Gestion si un client est déjà connecté au serveur
	if(channels > 10 || channels < 0)
	{
		printf("Impossible, le serveur est occupé, réessayer plus tard\n\n");
		exit(1);
	}
	
	// Initialisation 
	fd1 = aud_writeinit(sample_rate, sample_size, channels);
	void *buff = malloc(sample_size);

	
	// Timeout
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	
	int r = 2;
	int monter = 1;
	int cpt = 0;
	
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
		
		if(volume != 0) // Changement Volume
		{
			changementVolume(buff, volume);
			
		}
		else if(volumeRandom == 1) // Volume Random
		{
			if(cpt > 3000)
			{
				if(monter == 1)
				{
					r--;
					if(r == -3) { monter = 0; }
				}
				else
				{
					r++;
					if(r == 3) { monter = 1; }
				}
				cpt = 0;
			}
			else
			{
				cpt++;
			}
			changementVolume(buff, r);
		}
		else if(bit == 1) // 8bit
		{
			if(boucle > 0)
			{
				strcpy(buff, buffBit);
				volumeBit--;
				changementVolume(buff, volumeBit);
				boucle--;
			}
			else
			{
				strcpy(buffBit, buff);
				boucle = 2;
				volumeBit = 1;
			}	 
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
	// Quand on reçoit le message de fin, on coupe le client 
	
	
	// Close
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
	

	return 0;
}

/**
 * @param buff : Tableau contenant les échantillons de musique
 * @param volume : Valeur pour le changement de volume
 * Modifie la hauteur des échantillons
 */
void changementVolume(void * buff, int volume)
{
	char * buffTmp = buff;
	for(int i = 0 ; i < SIZEOFBUFF ; i++)
	{
		if(volume > 0) { buffTmp[i] = buffTmp[i] * volume; }
		else if(volume < 0) { buffTmp[i] = buffTmp[i] / volume; }
	}
}
