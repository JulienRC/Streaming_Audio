# **************************************************************************
# L3Informatique						System
# 			PROJET

# * Group 	: GROUPE 3.1
# * Name 1 : David RENARD CALZANT
# * Name 2 : Julien ROYON CHALENDARD
# *
# **************************************************************************#

# Taper make dans le cmd
# Link
#

bin/lecteur: obj/lecteur.o obj/audio.o obj/audioserver.o obj/audioclient.o
	gcc -o bin/lecteur obj/lecteur.o obj/audio.o
	gcc -o bin/audioserver obj/audioserver.o obj/audio.o
	gcc -o bin/audioclient obj/audioclient.o obj/audio.o


# Objets of TP Lists
#

obj/lecteur.o: src/lecteur.c
	gcc -o obj/lecteur.o -c src/lecteur.c

obj/audio.o: src/audio.c
	gcc -o obj/audio.o -c src/audio.c
	
obj/audioserver.o: src/audioserver.c
	gcc -o obj/audioserver.o -c src/audioserver.c
	
obj/audioclient.o: src/audioclient.c
	gcc -o obj/audioclient.o -c src/audioclient.c

#
# Remove files
#

clean :
	rm -rf obj/*.o
	
mrproper : clean
	rm -rf bin/lecteur
	rm -rf bin/audioserver
	rm -rf bin/audioclient
