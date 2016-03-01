/*
Напишите эхо-сервер (IPv4, TCP). Это сервер, который возвращает то, что ему прислано. Вопросы (с кодом и без) можно задавать в комментариях.

Тестировать сервер можно с помощью утилит telnet и nc - выбирайте по вкусу.
 */

#include <stdio.h>

#include <unistd.h> //for close()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char **argv) {
    int MasterSocket = socket(
        AF_INET,     //IPv4
        SOCK_STREAM, //TCP
        IPPROTO_TCP);

    struct sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(12345);
    SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(MasterSocket, (struct sockaddr *)(&SockAddr), sizeof(SockAddr)) != 0) {
        printf("Can not bind port\n");
        return -1;
    }

    if (listen(MasterSocket, SOMAXCONN) != 0) {
        printf("Can not listen() socket\n");
        return -1;
    }

    while(1) {
        int SlaveSocket =accept(MasterSocket, 0, 0);
        if (SlaveSocket >= 0) {
            printf("New client accepted\n");
        } else if (SlaveSocket == -1) {
            printf("Problem with accept()\n");
            return -1;
        }

        while(1) {
            
            char Buffer[5] = {0, 0, 0, 0, 0};
            ssize_t msgsize = recv(SlaveSocket, Buffer, 4, MSG_NOSIGNAL);
            if (msgsize == -1) { // error
                printf("Error in recv()\n");
                return -1;
            } else if (msgsize == 0) { //seems EOF
                printf("Client correctly close connection\n");
                if (shutdown(SlaveSocket, SHUT_RDWR) == -1) {
                    printf("Problem with shutdown() function\n");
                    return -1;
                }

                if (close(SlaveSocket) == -1) {
                    printf("Problem with close() function\n");
                    return -1;
                }
                break; //exit
            }

            printf("%s", Buffer);
            if (send(SlaveSocket, Buffer, 4, MSG_NOSIGNAL) == -1) {
                printf("Problems with send()\n");
                return -1;
            }
        } 
    }

    printf("%s\n", "Exit program");
    return 0;
}

