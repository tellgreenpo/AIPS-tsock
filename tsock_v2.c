/* librairie standard ... */
#include <stdlib.h>
/* pour getopt */
#include <unistd.h>
/* déclaration des types de base */
#include <sys/types.h>
/* constantes relatives aux domaines, types et protocoles */
#include <sys/socket.h>
/* constantes et structures propres au domaine UNIX */
#include <sys/un.h>
/* constantes et structures propres au domaine INTERNET */
#include <netinet/in.h>
/* structures retournées par les fonctions de gestion de la base de
données du réseau */
#include <netdb.h>
/* pour les entrées/sorties */
#include <stdio.h>
/* pour la gestion des erreurs */
#include <errno.h>

// Fonction donnée par le sujet afin de construire les messages
void construire_message(char *message, char motif, int lg) {
	int i;
	for (i=5;i<lg;i++) message[i] = motif;
};

void message_final(char *pmsg,int i,int lg){
	char snum[5];
	char *p=snum;
	int multiple = 1;
	while (i/10>0){
		multiple++;
		i = i/10;
	};
	// on met les petits tirets
	for (int j=0;j<(5-multiple);j++){
		pmsg[j] = '-';
	};

	sprintf(snum,"%i",i);

	// on mets le numero du message (i)
	for (int j=(5-multiple);j<5;j++){
		pmsg[j]=*p;
		p++;
	};

	// on met genere les characteres qui suivent
	construire_message(pmsg,(char)((i%26)+97),lg);
}

void afficher_message(char *message, int lg) {
	int i;
	printf("[");
	for (i=0;i<lg;i++) printf("%c",message[i]);
	printf("]");
	printf("\n");
};

void affiche(char *message, int lg, int source,int n){
	if(source){
		printf("SOURCE: envoi n°%i ",n);
		afficher_message(message,lg);
	}else{
		printf("PUITS: envoi n°%i ",n);
		afficher_message(message,lg);
	}
}


void main (int argc, char **argv)
{
	int c;
	extern char *optarg;
	extern int optind;
	int nb_message = 10; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int source = -1 ; /* 0=puits, 1=source */
	int mode = 1; // 0=UDP 1=TCP, TCP est par default
	int co_max = 5; // Nombre de clients en attente max pour le serveur en TCP ==> 2 par default
	int lg_mesg = 30; //Longueur du message par default 30 octets 
	while ((c = getopt(argc, argv, "pn:sul:")) != -1) {
		switch (c) {
		case 'p':
			if (source == 1) {
				printf("usage: cmd [-p|-s][-n ##]\n");
				exit(1);
			}
			source = 0;
			break;

		case 's':
			if (source == 0) {
				printf("usage: cmd [-p|-s][-n ##]\n");
				exit(1) ;
			}
			source = 1;
			break;

		case 'n':
			nb_message = htons(atoi(optarg));
			break;

		case 'u':
		//Utilisation de UDP
			mode = 0;
			break;
		
		case 'l':
			lg_mesg = htons(atoi(optarg));
			break;

		default:
			printf("usage: cmd [-p|-s][-n ##]\n");
			break;
		}
	}

	if (source == -1) {
		printf("usage: cmd [-p|-s][-n ##]\n");
		exit(1) ;
	}

	// On est source
	if (source == 1){
		printf("Mode source");

		if (mode == 0){// On utilise UDP
			printf(" | Protocle UDP\n");
			// On creer un socket local sock_dgram
			int socketS; 
			if((socketS = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))==-1){
				printf("Echec de la creation socket\n");
				exit(-1);
			};
			//construction adresse du socket auquel on veux s'adresser
			struct sockaddr_in adr_distant;
			struct hostent *hp;
			memset((char*)&adr_distant,0,sizeof(adr_distant));
			adr_distant.sin_family = AF_INET;
			adr_distant.sin_port = htons(atoi(argv[argc-1]));
			if ((hp=gethostbyname(argv[argc-2]))==NULL){
				printf("erreur gethostbyname\n");
				exit(1);
			}
			memcpy((char*)&(adr_distant.sin_addr.s_addr),hp->h_addr,hp->h_length);
			// appel a la primitive sendto()
			char pmsg[lg_mesg]; // 5 premiers octets pour le numero du message
			// envoi des messages
			printf("SOURCE: lg_msg_emis=%i, port=%s, nb_envois=%i, TP=udp, dest=%s \n",lg_mesg,argv[argc-1],nb_message,argv[argc-2]);
			for (int i=0;i<nb_message;i++){
				// On construit le message
				message_final(pmsg,i,lg_mesg);
				affiche(pmsg,lg_mesg,source,i);
				sendto(socketS,pmsg,lg_mesg,0,(struct sockaddr *)&adr_distant,sizeof(adr_distant));
			};
			close(socketS);
		}else{
			// TCP en source²
			printf(" | Protocole TCP\n");
			// Creation socket local
			int socketS;
			if((socketS = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==-1){
				printf("Echec de la creation socket\n");
				exit(-1);
			};
			// Creation de l'adresse du socket auquel on souhaite s'adresser
			struct sockaddr_in adr_distant;
			struct hostent *hp;
			memset((char*)&adr_distant,0,sizeof(adr_distant));
			adr_distant.sin_family = AF_INET;
			adr_distant.sin_port = htons(atoi(argv[argc-1]));
			if ((hp=gethostbyname(argv[argc-2]))==NULL){
				printf("erreur gethostbyname\n");
				exit(1);
			}
			memcpy((char*)&(adr_distant.sin_addr.s_addr),hp->h_addr,hp->h_length);
			// Demande de connexion
			if(connect(socketS,(struct sockaddr *)&adr_distant,sizeof(adr_distant))==-1){
				printf("Echec de la connexion au serveur");
			}
			// Dialogue avec le serveur
			char pmsg[lg_mesg]; // 5 premiers octets pour le numero du message
			// envoi des messages
			printf("SOURCE: lg_msg_emis=%i, port=%s, nb_envois=%i, TP=tcp, dest=%s \n",lg_mesg,argv[argc-1],nb_message,argv[argc-2]);
			for (int i=0;i<nb_message;i++){
				// On construit le message
				message_final(pmsg,i,lg_mesg);
				affiche(pmsg,lg_mesg,source,i);
				if(write(socketS,pmsg,lg_mesg)==-1){
					printf("Echec de l'envoi\n");
				};
			};
			shutdown(socketS,1);
			close(socketS);

		};

	}else{
		// Puit
		printf("Mode puit");
		if (mode == 0){
			printf(" | Protocole UDP\n");
			//creer socket local
			int socketR; 
			if((socketR = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))==-1){
				printf("Echec de la creation socket\n");
				exit(-1);
			};
			//construire addresse
			struct sockaddr_in adr_local;
			memset((char*)&adr_local,0,sizeof(adr_local));
			adr_local.sin_family = AF_INET;
			adr_local.sin_port = htons(atoi(argv[argc-1]));
			adr_local.sin_addr.s_addr = INADDR_ANY;
			//binder l'adresse
			int lg_adr_local = sizeof(adr_local);
			if (bind(socketR,(struct sockaddr *)&adr_local,lg_adr_local)==-1){
				printf("Echec du bind\n");
				exit(1);
			}
			//recvfrom()
			//On est ouvert a tous les paquets emis jusqu'à terminaison de tsock quand on est en mode puit
			char pmsg[lg_mesg];
			struct sockaddr * padr_em;
			printf("PUIT: lg_msg_emis=%i, port=%s, nb_reception=infini, TP=udp\n",lg_mesg,argv[argc-1]);
			while(1){
				recvfrom(socketR,pmsg,lg_mesg,0,padr_em,sizeof(adr_local));
				affiche(pmsg,lg_mesg,source,-1);
			}
		}else{
			// TCP
			printf(" | Protocole TCP\n");
			// Creation socket localint socketR; 
			int socketR, socketC;
			int lg_adr_client;
			if((socketR = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==-1){
				printf("Echec de la creation socket\n");
				exit(-1);
			};
			//construire addresse
			struct sockaddr_in adr_local, adr_client;
			lg_adr_client = sizeof(adr_client);
			memset((char*)&adr_local,0,sizeof(adr_local));
			adr_local.sin_family = AF_INET;
			adr_local.sin_port = htons(atoi(argv[argc-1]));
			adr_local.sin_addr.s_addr = INADDR_ANY;
			//binder l'adresse
			int lg_adr_local = sizeof(adr_local);
			if (bind(socketR,(struct sockaddr *)&adr_local,lg_adr_local)==-1){
				printf("Echec du bind\n");
				exit(1);
			};
			// Dimensionement de la liste d'attente
			if(listen(socketR,co_max)==-1){
				printf("Echec du listen()\n");
				exit(1);
			};
			// Acceptation de toute demande
			if((socketC=accept(socketR,(struct sockaddr*)&adr_client,&lg_adr_client))==-1){
				printf("Echec du accept()\n");
				exit(1);
			};
			// Affichage
			int lg_rec;
			char pmsg[lg_mesg];
			int max=10; // par default on fixe a 10 messages
			int i;
			printf("PUIT: lg_msg_emis=%i, port=%s, nb_reception=%i, TP=tcp\n",lg_mesg,argv[argc-1],max);
			for(i=0;i<max;i++){
				if((lg_rec=read(socketC,pmsg,lg_mesg))<0){
					printf("Echec du read()\n");
					exit(1);
				};
				affiche(pmsg,lg_mesg,source,i);
			};
			shutdown(socketR,0);
			close(socketR);
		};

	};
	if (nb_message != -1) {
		if (source == 1)
			printf("nb de tampons à envoyer : %d\n", nb_message);
		else
			printf("nb de tampons à recevoir : %d\n", nb_message);
	} else {
		if (source == 1) {
			nb_message = 10 ;
			printf("nb de tampons à envoyer = 10 par défaut\n");
		} else
		printf("nb de tampons à envoyer = infini\n");

	}
}

