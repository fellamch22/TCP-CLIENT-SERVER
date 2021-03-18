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

struct noeud
{
    char data[MAX_DATA+MAX_NAME+5];
    struct noeud * next ;
};

struct list 
{
    struct noeud * tete;
};

struct user_info 
{
    in_addr_t adr ;
    int fd_socket;
    
};

// on utilise la pile de stockage des messages comme varible globale du programme

    struct list stack ;

// initilisation du verrou qui empeche l'acces en concurrence a la liste des messages

    pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;


void* communication (void * arg ){


    struct user_info  ui = (*((struct user_info *) arg));    

    char pseudo[MAX_NAME+1];//+1 pour le caractere de fin '\0'
    char reponse[30];
    char msg[MAX_DATA+5];//+1 pour le caractere de fin '\0'

    recv(ui.fd_socket,pseudo,MAX_NAME,0);

    pseudo[MAX_NAME] = '\0';

    sprintf(reponse,"HELLO %s",pseudo);

    send(ui.fd_socket,reponse,strlen(reponse),0);

    int len;

    len = recv(ui.fd_socket,msg,MAX_DATA+4,0);
    msg[len] ='\0';


    while (len > 0){

        if( strcmp(msg,"GET") == 0 ){

            // recuperer le dernier message
            struct noeud * last_msg ;

            // prendre le verrou pour modifier la liste de messages
            pthread_mutex_lock(&verrou);

                if( stack.tete == NULL ){ //pile vide -> aucun message

                        send(ui.fd_socket,"NOP",strlen("NOP"),0);

                }else{

                        last_msg = stack.tete;

                        // remplacer les dernier message de la pile par l'avant dernier
                        stack.tete = stack.tete->next;

                        // envoyer les donnees au client
                        send(ui.fd_socket,last_msg->data,strlen(last_msg->data),0);

                        // liberer le noeud 

                        free(last_msg);

                }

            pthread_mutex_unlock(&verrou);

        }else if (strncmp("PUT",msg,3) == 0){

            // stocker le message 

            // prendre le verrou our empecher tout acces en concurence sur la pile

            pthread_mutex_lock(&verrou);

                struct noeud * n = (struct noeud *) malloc(sizeof(struct noeud ));

                // remplissage du noeud

                sprintf(n->data,"%s%x%s",pseudo,htons(ui.adr),msg+4);
                // modification  du chainage et mettre le nouveau message en tete de la pile
                printf("stack : %s \n",n->data);
                n->next = stack.tete;
                stack.tete = n ;

            pthread_mutex_unlock(&verrou);

            send(ui.fd_socket,"MOK",strlen("MOK"),0);

        }

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

    stack.tete = NULL;

    while(1){

            // receiving connections 


        int * fd_socket1 = (int *) malloc(sizeof(int));
        struct sockaddr_in asock1;
        socklen_t taille = sizeof(struct sockaddr_in);
        struct user_info * inf = (struct user_info *) malloc(sizeof(struct user_info));;

        (* fd_socket1) =accept(fd_socket,(struct sockaddr *)&asock1,&taille);

        inf -> adr = asock1.sin_addr.s_addr;
        inf -> fd_socket = (*fd_socket1);
        // creation du thread pour permettre le traitment parallele de plusieurs clients

        pthread_t th ;
        printf("aw dja \n");
        pthread_create(&th,NULL,communication,inf);

    }


    return 0;

}