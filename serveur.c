#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#define MAX_DATA 80
#define MAX_NAME 10


// structure permettant de sauvegarder les informations de l'utilisateur
// et les faire passer en argument pour la fonction communication

struct user_info 
{
    in_addr_t adr ;
    int fd_socket;
    
};

// on utilise le message comme varible globale du programme

    char last_msg[MAX_NAME+4+MAX_DATA+1] ;//pseudo+adr_ip+message+'\0'

// initilisation du verrou qui empeche l'acces en concurrence au message

    pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;


void* communication (void * arg ){

    // lire les informations de l'utilisateur
    struct user_info  ui = (*((struct user_info *) arg));    

    char pseudo[MAX_NAME+1];//+1 pour le caractere de fin '\0'
    char reponse[30];
    char msg[MAX_DATA+5];//+4 pour le debut du message PUT' ',+1 pour le pour le caractere de fin '\0'
    int len = 0 ;

    // lire le pseudo du client

    len = recv(ui.fd_socket,pseudo,MAX_NAME,0);

    // verifier que le pseudo est bien de longeur 10

    while ( len != 10)
    {
        printf("Pseudo introduit ne respecte pas la taille de 10 caracteres avec compris \n");
        len = recv(ui.fd_socket,pseudo,MAX_NAME,0);

    }

    pseudo[MAX_NAME] = '\0';

    sprintf(reponse,"HELLO %s",pseudo);

    // Envoyer le message de bienvenue a l'utilisateur 

    send(ui.fd_socket,reponse,strlen(reponse),0);


    len = recv(ui.fd_socket,msg,MAX_DATA+4,0);

    msg[len] ='\0';


    while (len > 0){

        if( strcmp(msg,"GET") == 0 && len == 3 ){// si l'utilisateur envoie "GET"



            // prendre le verrou pour lire le dernier message dans la variable last_msg
            // et empecher l'acces en  concurrence sur le message
            pthread_mutex_lock(&verrou);

                if( last_msg[0] == '\0' ){ // message vide

                        send(ui.fd_socket,"NOP",strlen("NOP"),0);

                }else{// il existe un message


                        // envoyer les donnees du dernier message au client
                        send(ui.fd_socket,last_msg,MAX_NAME+4+MAX_DATA+1,0);

                        // vider le dernier message
                        memset(last_msg,'\0',MAX_NAME+4+MAX_DATA+1);

                }

            //liberer le verrou apres avoirfini la lecture/ecriture
            pthread_mutex_unlock(&verrou);

        }else if ( (strncmp("PUT ",msg,4) == 0) && len == MAX_DATA+4 ){

            // stocker le nouveau message du client

            // prendre le verrou pour empecher tout acces en concurence sur le message

            pthread_mutex_lock(&verrou);


                // remplissage du message avec les informations du client

                memcpy(last_msg,pseudo,10*sizeof(char));
                memcpy(last_msg+MAX_NAME,&ui.adr,4*sizeof(char));
                memcpy(last_msg+MAX_NAME+4,msg+4,(MAX_DATA+1)*sizeof(char));

                printf("nouveau message recu\n");
            
            // liberer le verrou apres la fin de l'ecriture

            pthread_mutex_unlock(&verrou);

            // Envoyer au client que son message a bien ete pris en compte
            send(ui.fd_socket,"MOK",strlen("MOK"),0);

        }else{// le message recu ne correspond pas au protocole 

            printf("Format de message incorrect \n ");
        }


        // attendre un nouveau message de la part du client 
        len = recv(ui.fd_socket,msg,MAX_DATA+4,0);
        msg[len] ='\0';

    }

    free(arg);
    close(ui.fd_socket);
    return NULL ;
}

int main(int argc, char **argv){

    // verifier si on a bien le numero de port en parametre
    
    if ( argc !=2 ){

        printf("Veuillez donner le numero de port en parametre \n");
        exit(1);
    }


    //  creation de la socket serveur 

    int err;
    struct sockaddr_in asock;

    asock.sin_family = AF_INET;
    asock.sin_port = htons(atoi(argv[1]));
    asock.sin_addr.s_addr = htonl(INADDR_ANY);

    // creer la socket du serveur 

    int fd_socket = socket(PF_INET,SOCK_STREAM,0);

    // binding 

    err = bind(fd_socket,(struct sockaddr *)&asock,sizeof(struct sockaddr_in));

    // check binding failure

    if (err < 0){

        printf("Erreur bind \n");
        exit(1);
    }


    // start the server

    err = listen(fd_socket,0);

    // check for errors

    if (err < 0){

        printf("Erreur bind \n");
        exit(1);
    }

    memset(last_msg,'\0',MAX_NAME+4+MAX_DATA+1);

    while(1){

        // receiving connections 

        int * fd_socket1 = (int *) malloc(sizeof(int));
        struct sockaddr_in asock1;
        socklen_t taille = sizeof(struct sockaddr_in);
        struct user_info * inf = (struct user_info *) malloc(sizeof(struct user_info));

        // attendre une nouvelle connexion au serveur 

        (* fd_socket1) =accept(fd_socket,(struct sockaddr *)&asock1,&taille);

        // remplir la structure inf avec les informations du client qui vient de se connecter
        inf -> adr = asock1.sin_addr.s_addr;
        inf -> fd_socket = (*fd_socket1);

        // creation du thread pour permettre le traitment parallele de plusieurs clients

        pthread_t th ;
        printf("un nouveau client arrive \n");
        pthread_create(&th,NULL,communication,inf);

    }


    return 0;

}