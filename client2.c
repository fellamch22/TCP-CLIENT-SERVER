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


int main(int argc, char** argv){

    // verifier si on a bien le numero de port en parametre
    
    if(argc != 2){

        printf("Veuillez donner le numero de port en parametre \n");
        exit(1);
    }

    int err;

    struct sockaddr_in asock;
    struct sockaddr_in * asock1;
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


    int fd_socket ;

    // creation de la socket client
    fd_socket = socket(PF_INET,SOCK_STREAM,0);

    // connexion au serveur

    err = connect(fd_socket,(struct sockaddr *)&asock,sizeof(struct sockaddr_in));

    // verification des erreurs de connexion 
    if ( err < 0 ){

        printf("Erreur de connexion %d\n",err);
        close(fd_socket);
        exit(1);

    }
        
    // envoyer le pseudo

    send(fd_socket,"BelleFella",10,0);

    char msg[95];
    int len;

    // reception du message de bienvenue de la part du serveur

    len = recv(fd_socket,msg,95,0);
    msg[len] ='\0';

    // afficher le message recu 
    printf("received : %s\n",msg);

    // demander le dernier message stocke dans le serveur
    send(fd_socket,"GET",strlen("GET"),0);

    // reception de la reponse du serveur 
    len = recv(fd_socket,msg,MAX_DATA+MAX_NAME+5,0);
    msg[len] ='\0';

    // traitement et extraction des donnees

    if( len == 3 ){// recevoir NOP de la part du serveur

        printf("received : %s\n",msg);

    }else{

        char data[MAX_DATA+1];
        char pseudo_msg[MAX_NAME+1];
        in_addr_t number ;
        struct in_addr adress;

        

        // recuperation de l'@ ip 
        memcpy(&number,msg+10,4);
        adress.s_addr = number;

        // recuperation du pseudo
        strncpy(pseudo_msg,msg,10);
        pseudo_msg[MAX_NAME]='\0';
        strncpy(data,msg+MAX_NAME+4,MAX_DATA+1);

        // affichage du message recu

        printf("received : %s %s %s\n",pseudo_msg,inet_ntoa(adress),data);

    }

    // fermeture de la socket 
    close(fd_socket);


    return 0;
}