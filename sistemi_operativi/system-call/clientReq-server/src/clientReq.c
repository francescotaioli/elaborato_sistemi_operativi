#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#include "../inc/errExit.h"
#include "../inc/request_response.h"

char *pathToServerFIFO = "/tmp/vr422009.fifo_server";
char *basePathToClientFIFO = "/tmp/vr422009.fifo_client."; // to handle multiple process
char pathToClientFIFO [35]; //extended path for the fif
int serverFIFO, clientFIFO;

void closeFIFOs(){
    //printf("deleting fifos...\n");
    // remove client fifo from the file system
    if (unlink(pathToClientFIFO) != 0)
        errExit("unlink from the file system -> failed");
    if(serverFIFO == -1)return; //probably server not running
    if (close(serverFIFO) != 0 || close(clientFIFO) != 0)
        errExit("close of server and client fifo -> failed");
}
int main (int argc, char *argv[]) {
    atexit(closeFIFOs);
    printf("Benvenuto in clientReq!\n");
    printf("digita uno fra i sequenti servizi\n\tStampa, Salva e invia\n\n");

    char userIdentifier[26];
    printf("Inserisci il nome utente(25 caratteri max): ");
    scanf( "%25s", userIdentifier);
    int character ;
    //discarding the extra character
    while((character = getchar()) != '\n' && character != EOF);

    char serviceName[7];
    printf("Inserisci il nome del servizio(6 caratteri max): ");
    scanf("%6s", serviceName);
    //discarding the extra character
    while((character = getchar()) != '\n' && character != EOF);

    //get the service tolower
    for(int i = 0; serviceName[i]; i++)
        serviceName[i] = tolower(serviceName[i]);


    // get the extended path for the fifo ( base path + pid )
    sprintf(pathToClientFIFO, "%s%d", basePathToClientFIFO, getpid());

    // create the fifo for the response from the server
    // rw- -w- ---
    if (mkfifo(pathToClientFIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
        errExit("creation of client fifo -> failed");

    // opening the server fifo to instantiate a request
    serverFIFO = open(pathToServerFIFO, O_WRONLY);
    if (serverFIFO == -1)
        errExit("it's server running?");

    // prepare a request
    struct Request request;
    request.clientPid = getpid();
    strcpy(request.userIdentifier , userIdentifier);
    strcpy(request.serviceName , serviceName);


    // send the request through server fifo
    if (write(serverFIFO, &request, sizeof(struct Request))
            != sizeof(struct Request))
        errExit("write to fifo server -> failed");

    // open my fifo to read the response
    clientFIFO = open(pathToClientFIFO, O_RDONLY);
    if (clientFIFO == -1)
        errExit("open the client fifo -> failed");

    // read the response from the server
    struct Response response;
    if (read(clientFIFO, &response, sizeof(struct Response))
            != sizeof(struct Response))
        errExit("read response from the server -> failed");


    printf("\ncodice identificativo: %s\n", userIdentifier);
    printf("servizio: %s\n", serviceName);
    printf("chiave rilasciata del server: %d\n", response.key);

}
