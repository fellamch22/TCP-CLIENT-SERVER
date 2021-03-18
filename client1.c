#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#define MAX_DATA 80
#define MAX_NAME 10


/* 
    cette fonction genere aleatoirement une message de longeur 80 avec le bon format
    qui respecte le protocole du serveur "PUT MESSAGE_DE_LONGEUR_80"
*/
char * random_msg(){

        char * msg = (char *)malloc((MAX_DATA+5)*sizeof(char));

        char letters[26]="abcdefghijklmnopqrstuvwxyz";

        strcpy(msg,"PUT ");

        for (int i = 4 ; i < MAX_DATA+4 ; i++){

            msg[i] = letters[rand()%25];
        }

        msg[MAX_DATA+4] ='\0';


        return msg;

}

/*
    programme client1
*/

int main(int argc, char**argv){

    // verifier si on a bien le numero de port en parametre

    if(argc != 2){

        printf("Veuillez donner le numero de port en parametre \n");
        exit(1);
    }
    
    int err;

    struct sockaddr_in asock;
    struct sockaddr_in * asock1;
    int fd_socket ;
    asock.sin_family = AF_INET;
    asock.sin_port = htons(atoi(argv[1]));

    struct addrinfo * first_info;
    struct addrinfo hints;

    memset(&hints,0,sizeof(struct addrinfo));
    hints.ai_family= PF_UNSPEC;

     // Recheche de l'adresse ip de lulu 


    err = getaddrinfo("lulu.informatique.univ-paris-diderot.fr",NULL,&hints,&first_info);

    if(err < 0){

        perror("Erreur getaddrinfo");
        exit(1);

    }

    struct addrinfo * info = first_info;
    int found = 0 ;

    while(info != NULL ){

        if((info->ai_addr)->sa_family == PF_INET){

            asock1 = (struct sockaddr_in *) info->ai_addr;
            found = 1;

        }

        info = info->ai_next;
    }

    // lancer une erreur si on echoue a trouver l'adresse 

    if ( !found){

        printf("IPv4 Adress not found\n");
        exit(1);
    }

    // recuperer l'adresse retrouvee

    asock.sin_addr = asock1->sin_addr;


    // creation de la socket 

    fd_socket = socket(PF_INET,SOCK_STREAM,0);

    // connexion au serveur 

    err = connect(fd_socket,(struct sockaddr *)&asock,sizeof(struct sockaddr_in));

    // verification des erreurs de connexion 

    if ( err < 0 ){

        printf("Erreur de connexion %d\n",err);
        close(fd_socket);
        exit(1);

    }


    char pseudo[MAX_NAME+1];
    char *  msg;
    char rep[4];
    int len;

    // envoyer le pseudo

    sprintf(pseudo,"JolieLoulu");

    send(fd_socket,pseudo,10,0);

    // reception du message de bienvenue de la part du serveur

    len = recv(fd_socket,rep,95,0);

    rep[len] = '\0';

    // afficher le message recu 

    printf("received : %s\n",rep);



    for (int i = 0 ; i < 5 ; i ++ ){

        // envoie du message au serveur 

        msg = random_msg();
        send(fd_socket,msg,MAX_DATA+4,0);
        free(msg);

        // reception de la reponse du serveur
        memset(rep,'\0',3);

        len = recv(fd_socket,rep,3,0);

        rep[len] = '\0';

        printf("received : %s\n",rep);

        sleep(2);

    }

    // fermeture de la socket 
    close(fd_socket);

    // liberation de la memoire de first_info

    freeaddrinfo(first_info);


    return 0;

}