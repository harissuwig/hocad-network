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
#include <poll.h>

#define KNNWIND0W 4
#define DELTA 3 //3 cm error margin

//#define __DEBUG__ 1

//Applications 
void test_application();
void bot_follow_bot();
void bot_mimick_bot();
double slaveDistanceGapKNN(long * leaderRSSI, long * slaveRSSI, unsigned int requiredGap);

unsigned int con_count = 0;
int master_node = 0;
int slave_node = 0;

int main(int argc , char *argv[])
{
    int socket_desc , c , read_size;
    struct sockaddr_in server , client;
    int cmd_val= 0; 
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
    // BOT_ID[0] = 3;
    // con_count = 2;
    // BOT_ID[1] = 14;
    master_node = BOT_ID[0];
    slave_node = BOT_ID[con_count - 1];
    printf("-----------------------------------------------------------\n");
    
    char end_loop = 0;
    
    while(!end_loop)
    {
        printf(" Application\n");
        printf("==================\n");
        printf("  1. Test Application\n");
        printf("  2. Bot following Bot\n");
        printf("  3. Bot mimicking Bot\n");
        printf("  0. Exit\n");
        printf("==================\n");
        printf(" Select Application: ");
        scanf("%d", &cmd_val);
    
        switch(cmd_val)
        {
            case 1:  test_application();
                     break;
            case 2:  bot_follow_bot();
                     break; 
            case 3:  bot_mimick_bot();
                     break;
            case 0:  printf("Exiting App ... \n");
                     end_loop = 1;
                     break;
            default: printf("wrong command!\n");
                     break;
        }

    
    }
    
    //free malloced variables
    return 0;
}

/*! 
 * \Brief test basic functionalities of both Bots and connection between them 
 */

void test_application()
{
        int val;
        int cmd_val = 0;
        char end_loop = 0;

        while(!end_loop) {

        printf("Enter Bot ID to send the packet\n");
        scanf("%d",&dst_id);
    
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
        printf("  0.  Exit this App ...\n");
        printf(" Waiting for user input : "); 

        scanf("%d",&cmd_val);        
        FILE *fptr;
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
                printf("Enter the time for right turn in millisecond : \n");
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
                printf("Fetchng ID\n");
                printf("ID of the bot : %d\n",BOT_ID[bot_num]);
                break;
            case 11:
                read_file();
                break;
            case 12:
                //create/append rssi_readings.csv
                
                fptr = fopen("rssi_readings.csv","a");
                if(fptr == NULL){
                    printf("ERROR!");
                    exit(1);
                }
                unsigned int counter_d = 0;
                unsigned int counter_rssi = 0;
                int dis_val1 = 0;
                int dis_val2 = 0;
                long rssi_val1 = 0;
                long rssi_val2 = 0;
                printf("Getting initial distance and RSSI data...\n");
                //Do measurement on 8 different distance
                while(counter_d < 8){
                    //Do RSSI readings 20x
                    while(counter_rssi < 20){
                        printf("Reading Ultrasonic\n");
                        dis_val1 = get_obstacle_data(src_id,master_node,ULTRASONIC_FRONT);
                        dis_val2 = get_obstacle_data(src_id,slave_node,ULTRASONIC_FRONT);
                        // sleep(2);
                        printf("Reading RSSI..\n");
                        rssi_val1 = get_RSSI(src_id,master_node);
                        rssi_val2 = get_RSSI(src_id,slave_node);
                        // sleep(2);
                        
                        //output reading to txt file
                        fprintf(fptr, "%ld,%ld,%d,%d\n", rssi_val1, rssi_val2, dis_val1, dis_val2);
                        
                        counter_rssi++;
                    }
                    
                    counter_rssi = 0;
                    
                    send_forward_time(src_id, dst_id, 1000); //move bot forward 1s
                    sleep(1);
                    printf("%d\n",counter_d);
                    counter_d++;
                }
                counter_d = 0;
                fclose(fptr);
                break;
            case 0: 
                end_loop = 1; 
                break;
            default:
                printf("Unknown command received\n");
                break;

        }
		 printf("\r\n\r\n"); 
    }
    
    system("clear");

}

/*! 
 * \Brief test basic functionalities of both Bots and connection between them 
 * 
 */
void bot_follow_bot()
{
    int val = 1000; // 1000 millisecond, for forward and reverse movement
    char end_loop = 0; 
    int cmd_val = 0;
    long leaderRSSI[KNNWIND0W] = { 0 };
    long slaveRSSI[KNNWIND0W] = { 0 };
    double distanceFromLeader;
    double DistanceToTimeScale = 1; // scale converstion from distance to time of movement scale 
    unsigned int requiredGap = 100; // current gap to maintain between the leader and the maste
    
    //variables for timed input
    struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };
    
    printf("\n\n\n");
    if(con_count != 2)
    {
       printf("You need exactly two number of Bots for this application");
       return;
    }
    printf("List of connected bot: ");
    int j = 0;
    int new_master;
    while(j < con_count) {
        printf("%d", BOT_ID[j]);
        if(BOT_ID[j] == master_node) printf(" (leader)");        
        j++;
        if(j != con_count) printf(", ");
        else printf("\n");
    }

    printf("Select the leader bot: ");
    scanf("%d", &master_node);
    
    printf("Select the slave bot: ");
    scanf("%d", &slave_node);
    printf("\n\n\n");
    
    printf("Select Master Action\n");
    printf("====================\n");
    printf("1. Move Forward\n");
    printf("2. Move Reverse\n");
    
    //timed input
    while(!end_loop)
    {
        if( poll(&mypoll, 1, 1000) )
        {
            scanf("%d", &cmd_val);
        
            switch(cmd_val)
            {
                case 1: send_forward_time(src_id, master_node, val);
                        break;
                case 2: send_reverse_time(src_id,master_node, val);
                        break;
                default: printf("Exiting App ... \n"); 
                           end_loop = 1;
                           break; 
            }
       
        }
        
        //measure RSSI of leader and servant
        for(int i = 0; i < KNNWIND0W; i++)
        {
            leaderRSSI[i] = get_RSSI(src_id,master_node);
            slaveRSSI[i] = get_RSSI(src_id,slave_node);
        }
        
        //if difference gives positive, we move forward; if difference give negative, we move backward
        distanceFromLeader = slaveDistanceGapKNN(leaderRSSI, slaveRSSI, requiredGap);
        
        
        if(distanceFromLeader > DELTA)
        {
            send_forward_time(src_id,slave_node,distanceFromLeader*DistanceToTimeScale);
        }
        else if ((-1*distanceFromLeader) > DELTA)
        {
            send_reverse_time(src_id,slave_node,distanceFromLeader*DistanceToTimeScale);
        }
    } 
    
    
}

void bot_mimick_bot()
{
    printf("\n\n\n");
    if(con_count != 2)
    {
       printf("You need exactly two number of Bots for this application");
       return;
    }
    printf("List of connected bot: ");
    int j = 0;
    int new_master;
    while(j < con_count) {
        printf("%d", BOT_ID[j]);
        if(BOT_ID[j] == master_node) printf(" (leader)");
        
        j++;
        if(j != con_count) printf(", ");
        else printf("\n");
    }

    printf("Select the leader bot: ");
    scanf("%d",&master_node);
    
    printf("Select the slave bot: ");
    scanf("%d", &slave_node);
    printf("\n\n\n");


}


double slaveDistanceGapKNN(long * leaderRSSI, long * slaveRSSI, unsigned int requiredGap) //leaderRSSI and slaveRSSI array of size KNNWINDOW
{
//KNNWIND0W 
// both from leaderRSSI and slaveRSSI, remove extreme value
// for both the leader and slave calculate distance from the KNN table for each member
// again remove the estreme and calculate the average of the results bot for leader and slave
// Return requiredGap - (slave - distance)  (assuming the forward direction is towards AP)
return 0.0;
}
