/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>    
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>   
#include "AdhocServer.h"
#include "CommandList.h"
#include <time.h> 

// void delay(int number_of_mseconds)  //__attribute__((optimize("O0"))) 
// {
//     sleep 
//     // Converting time into milli_seconds 
//     volatile int milli_seconds = 1 * number_of_mseconds; 
  
//     // Stroing start time 
//     volatile clock_t start_time = clock(); 
  
//     // looping till required time is not acheived 
//     while (clock() < start_time + milli_seconds) 
//         ; 
// } 

// #ifndef __DEBUG__
// #define __DEBUG__ 1
// #endif
unsigned int con_count = 0;

int main(int argc , char *argv[])
{
    int socket_desc , c , read_size;
    struct sockaddr_in server , client;
    int cmd_val= 0; 
    int val;
    int  ret = 0;
    int i = 0;
    if (argc != 2) {
        printf("Please enter %s  <NUMBER OF BOTS>\n",argv[0]);
        exit(0);
    }
    NUM_CONNECTIONS = atoi(argv[1]);

    BOT_ID = (char *)malloc(NUM_CONNECTIONS * sizeof(char));
    memset(BOT_ID,0,NUM_CONNECTIONS);
    client_sock = (int *)malloc(NUM_CONNECTIONS * sizeof(int));
    memset(BOT_ID,0,NUM_CONNECTIONS * sizeof(int));

    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("\n");
    puts("******* Server control program (Adhoc networking course) *******");
    puts("\n");

    puts("* Socket created");

    int option = 1;
    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(TCP_PORT);

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("* Binding done");


    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection

    while(con_count != NUM_CONNECTIONS) {

        c = sizeof(struct sockaddr_in);
        puts("* Waiting for bots to connect");

        //accept connection from an incoming client
        client_sock[con_count] = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
        if (client_sock[con_count] < 0)
        {
            perror("accept failed");
            return 1;
        }
        printf("  - Accepted connection\n");
#ifdef __DEBUG__
        printf("Got new  sockfd %d\n",client_sock[con_count]);
#endif
        BOT_ID[con_count] = get_botID(con_count);
        printf("  - Bot with ID : <%d> Connected\n",BOT_ID[con_count]);
        con_count++;
    }
    // BOT_ID[con_count] = 3;
    // con_count++;
    // BOT_ID[con_count] = 14;
    // con_count++;
    printf("-----------------------------------------------------------\n");

    int master_node = BOT_ID[0];
    int slave_node = BOT_ID[con_count-1];

    FILE *fptr;
    unsigned int counter_rssi = 0;
    unsigned int counter_d = 0;
    int dis_val = 0;
    int idbot = 0;
    long rssi_val = 0;
    int done_measure = 1;
    
    while(done_measure) {
        printf("-----------------------------------------------------------\n");
        int counter_con_bot = 0;
        printf("Connected bot: ");
        while(counter_con_bot < con_count) {
            printf("%d",BOT_ID[counter_con_bot]);
            if(BOT_ID[counter_con_bot] == master_node) printf(" (master)");
            
            counter_con_bot++;
            if(counter_con_bot != con_count) printf(", ");
            else printf("\n");
        }
        printf("\nEnter Bot ID to send the packet\n");
        scanf("%d",&dst_id);
        // printf("Fetchng RSSI information \n");
        //         printf("RSSI reading : %ld\n",get_RSSI(src_id,dst_id));
        printf("Enter the command(1-12) to the bot-%d : \n",dst_id);


        printf("  1. Move forward \n"); 
        printf("  2. Move forward for time in seconds \n"); 
        printf("  3. Move reverse \n"); 
        printf("  4. Move reverse for time in seconds \n"); 
        printf("  5. Move left time\n"); 
        printf("  6. Move right time\n"); 
        printf("  7. Stop the bot\n"); 
 //       printf("  8. Get obstacle distance left \n"); 
 //       printf("  9. Get obstacle distance right\n"); 
        printf("  8. Get obstacle distance front\n"); 
        printf("  9. Get RSSI value\n");
        printf("  10. Get ID\n");
        printf("  11. Execute commands from file (cmd_file.txt)\n");
        printf("  12. Distance + RSSI Measurement\n");  
        printf("  13. Get distance from RSSI\n");
        printf("  14. Set bot master, current master: %d\n", master_node);
        printf("  15. Follow leader mode 1\n");
        printf("  16. Follow leader mode 2\n");
        printf(" Waiting for user input : "); 
        
        scanf("%d",&cmd_val);        

        switch(cmd_val) {

            case 1:
                send_forward_time(src_id, dst_id,0);
                break;
            case 2:
                printf("Enter the time in milliseconds : \n");
                scanf("%d",&val);
                send_forward_time(src_id, dst_id, val);
                break;
            case 3:
                send_reverse_time(src_id,dst_id,0);
                break;
            case 4:
                printf("Enter the time in milliseconds : \n");
                scanf("%d",&val);
                send_reverse_time(src_id,dst_id,val);
                break;
            case 5:
                printf("Enter the time for left turn in milliseconds : \n");
                scanf("%d",&val);
                send_rotate_left(src_id,dst_id,val);
                break;
            case 6:
                printf("Enter the time for right turn in milliseconds : \n");
                scanf("%d",&val);
                send_rotate_right(src_id,dst_id,val);
                break;
            case 7:
                printf("Sending command to stop the bot\n");
                stop_bot(src_id,dst_id);   
                break;
/*            case 8:
                printf("Fetchng left obstacle sensor information  \n");
                printf("Left Obtacle sensor reading : %d\n",get_obstacle_data(src_id,dst_id,ULTRASONIC_LEFT));
                break;
            case 9:
                printf("Fetchng right  obstacle sensor information \n");
                printf("Right Obtacle sensor reading : %d\n",get_obstacle_data(src_id,dst_id,ULTRASONIC_RIGHT));
                break; */
            case 8:
                printf("Fetchng front obstacle sensor information \n");
                printf("Front Obtacle sensor reading : %d\n",get_obstacle_data(src_id,dst_id,ULTRASONIC_FRONT));
                break;
            case 9:
                printf("Fetchng RSSI information \n");
                printf("RSSI reading : %ld\n",get_RSSI(src_id,dst_id));
                break;
            case 10:
                idbot = get_botID(con_count-1);
                printf("ID of the bot : %d\n",idbot);
                break;
            case 11:
                read_file();
                break;
            case 12:
                //create/append ressi_readings.txt
                fptr = fopen("rssi_readings.txt","a");
                if(fptr == NULL){
                    printf("ERROR!");
                    exit(1);
                }

                printf("Getting initial distance and RSSI data...\n");
                //Do measurement on 8 different distance
                while(counter_d < 8){
                    //Do RSSI readings 20x
                    while(counter_rssi < 20){
                        printf("Reading Ultrasonic\n");
                        dis_val = get_obstacle_data(src_id,dst_id,ULTRASONIC_FRONT);
                        // sleep(2);
                        printf("Reading RSSI..\n");
                        rssi_val = get_RSSI(src_id,dst_id);
                        // sleep(2);
                        printf("%d,%d,%ld\n", counter_d, dis_val, rssi_val);
                        
                        //output reading to txt file
                        fprintf(fptr, "%d,%d,%ld\n", counter_d, dis_val, rssi_val);
                        
                        counter_rssi++;
                    }
                    counter_rssi = 0;
                    
                    send_forward_time(src_id, dst_id, 1000); //move bot forward 1s
                    sleep(1);
                    counter_d++;
                }
                counter_d = 0;
                fclose(fptr);
                break;

            case 13:
                // rssi_val = get_RSSI(src_id, dst_id);
                // //run python script on terminal to get rssi
                // FILE *fppy;
                // char cmd[100];
                // char resp[100];
                // sprintf(cmd, "python knn.py rssi_readings.csv %ld",rssi_val);
                // fppy = popen(cmd, "r");
                // if (fppy == NULL){
                //     printf("Failed\n");
                //     exit(1);
                // }
                // while(fgets(resp, sizeof(resp), fppy) != NULL)
                //     printf("%s\n",resp);
                
                // pclose(fppy);
                break;

            case 14:
                //change master
                printf("List of bot:\n");
                int j = 0;
                int new_master;
                while(j < con_count) {
                    printf("%d",BOT_ID[j]);
                    if(BOT_ID[j] == master_node) printf(" (master)");
                    
                    j++;
                    if(j != con_count) printf(", ");
                    else printf("\n");
                }
                printf("\nEnter new master node: ");
                scanf("%d", &new_master);
                slave_node = master_node;
                master_node = new_master;
                break;

            case 15:
                //insert algorithm for mode 1 here
                break;

            case 16:
                //insert algorithm for mode 2 here
                break;
            default:
                printf("Unknown command received\n");
                break;


        }
        // system("clear");
		 printf("\r\n\r\n"); 
    }
    return 0;
}
