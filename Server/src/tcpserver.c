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
#include <sys/time.h>

//#define __DEBUG__ 1

#define KNNWINDOW 4
#define DELTA 3 //3 cm error margin
#define READINGF 20 //frequency for rssi measurement
#define BOTLENGTH 20 //length of the bot

const int  K = 10; //K of the KNN
const int MaxRange = 300; //maximum reading of the ultrasonic reader is 254


//Applications 
void test_application();
void bot_follow_bot();
void bot_mimick_bot();
void calibrate_mode(int ind);
int slaveDistanceGapKNN(long * leaderRSSI, long * slaveRSSI, int mode); //for filtering and getting distance using knn algo
void sort (long * v, int n); //for bubble sorting


//bot_follow_bot and bot_mimic_bot application variables
int fw_mult = 5; // multiplier from distance to time for forward motion
int rv_mult = 5; // multiplier from distance to time for reverse motion

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

    master_node = BOT_ID[0];
    slave_node = BOT_ID[con_count - 1];
    printf("-----------------------------------------------------------\n\n\n");
    
    char end_loop = 0;
    
    while(!end_loop)
    {
        printf("===========================================================\n");
        printf(" APPLICATION\n");
        printf("-----------------------------------------------------------\n");
        printf(" CONNECTED BOTS: ");
        int j = 0;
        while(j < con_count) {
            printf("%d", BOT_ID[j]);
            if(BOT_ID[j] == master_node) printf(" (leader)");        
            j++;
            if(j != con_count) printf(", ");
            else printf("\n");
        }
        printf("-----------------------------------------------------------\n");
        printf("  1. Test Application\n");
        printf("  2. Calibration for mode 1\n");
        printf("  3. Bot following Bot\n");
        printf("  4. Calibration for mode 2\n");
        printf("  5. Bot mimicking Bot\n");
        printf("  6. Change leader\n");
        printf("  0. Exit\n");
        printf("===========================================================\n");
        printf(" Select Application: ");
        scanf("%d", &cmd_val);

        switch(cmd_val)
        {
            case 1:  test_application();
                     break;
            case 2:  calibrate_mode(1);
                     break;
            case 3:  bot_follow_bot();
                     break; 
            case 4:  calibrate_mode(2);
                     break;
            case 5:  bot_mimick_bot();
                     break;
            case 6:
                
                printf("\nEnter new leader node: ");
                int new_master;
                scanf("%d", &new_master);
                slave_node = master_node;
                master_node = new_master;
                break;
            case 0:  printf("Exiting App ... \n");
                     end_loop = 1;
                     break;
            default: printf("wrong command!\n");
                     break;
        }
    printf("\r\n\r\n"); 
    
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
        int ledno;
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
        printf("  13. Toggle LED\n");
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
                #ifdef __DEBUG__
                struct timeval tv1, tv2;
                gettimeofday(&tv1, NULL);
                #endif
                printf("Front Obtacle sensor reading : %d\n",get_obstacle_data(src_id,dst_id,ULTRASONIC_FRONT));
                #ifdef __DEBUG__
                gettimeofday(&tv2, NULL);
                printf("Total time: %f seconds\n", (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));
                #endif
                break;
            case 9:
                printf("Fetchng RSSI information \n");
                #ifdef __DEBUG__
                struct timeval tv3, tv4;
                gettimeofday(&tv3, NULL);
                #endif
                printf("RSSI reading : %ld\n",get_RSSI(src_id,dst_id));
                #ifdef __DEBUG__
                gettimeofday(&tv4, NULL);
                printf("Total time: %f seconds\n", (double) (tv4.tv_usec - tv3.tv_usec) / 1000000 + (double) (tv4.tv_sec - tv3.tv_sec));
                #endif
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
                
                fptr = fopen("rssi_us_readings.csv","a");
                if(fptr == NULL){
                    printf("ERROR!");
                    exit(1);
                }
                unsigned int counter_d = 0;
                unsigned int counter_rssi = 0;
                int dis_val = 0;
                long rssi_val = 0;
                printf("Getting initial distance and RSSI data...\n");
                //Do measurement on 8 different distance
                while(counter_d < 8){
                    //Do RSSI readings 20x
                    send_toggle_led(src_id, dst_id, 1);
                    
                    while(counter_rssi < 20){
                        send_toggle_led(src_id, dst_id, 2);
                        //printf("Reading Ultrasonic\n");
                        dis_val = get_obstacle_data(src_id,dst_id,ULTRASONIC_FRONT);
                        // sleep(2);
                        //printf("Reading RSSI..\n");
                        rssi_val = get_RSSI(src_id,dst_id);
                        // sleep(2);
                        
                        //output reading to txt file
                        fprintf(fptr, "%ld,%d\n", rssi_val, dis_val);
                        
                        counter_rssi++;
                        send_toggle_led(src_id, dst_id, 2);
                        
                        usleep(100000);
                        
                    }
                    
                    counter_rssi = 0;
                    
                    send_forward_time(src_id, dst_id, 1000); //move bot forward 1s
                    send_toggle_led(src_id, dst_id, 1);
                    
                    usleep(100000);
                    //printf("%d\n",counter_d);
                    counter_d++;
                }
                counter_d = 0;
                fclose(fptr);
                break;
            
            case 13:
                printf("Input LED (1-4): ");
                
                scanf("%d", &ledno);
                send_toggle_led(src_id, dst_id, ledno);
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
 * \brief Measure RSSI and corresponding distance and store it as .csv for leader & slave bots
 * 
 * \param ind is the experiment number 
 *            1 - bot follow bot 
 *            2 - bot mimic bot
 */
void calibrate_mode(int ind){
    char ans;
    printf("Ready to calibrate ? (y/n): ");
    scanf(" %c", &ans);
    if(ans == 'N' || ans == 'n') return;

    FILE *fptr, *fptr2;
    char filename1[30];
    char filename2[30];
    sprintf(filename1, "calibration_master_%d.csv", ind);
    sprintf(filename2, "calibration_slave_%d.csv", ind);
    fptr = fopen(filename1,"a");
    if(fptr == NULL){
        printf("ERROR!");
        exit(1);
    }

    fptr2 = fopen(filename2,"a");
    if(fptr2 == NULL){
        printf("ERROR!");
        exit(1);
    }
    unsigned int counter_d = 0;
    unsigned int counter_rssi = 0;
    int dis_val1 = 0;
    int dis_val2 = 0;
    long rssi_val1 = 0;
    long rssi_val2 = 0;
    printf("Backward (ms): ");
    unsigned long dis = 0;
    scanf(" %ld", &dis);
    
    printf("Getting initial distance and RSSI data...\n");

    while(counter_d < 5){ //counter_d -> # distance reading iteration
        //toggle led to indicate distance reading
        send_toggle_led(src_id, master_node, 1);
        send_toggle_led(src_id, slave_node, 1);
    
        while(counter_rssi < 10){ //counter_rssi -> # rssi reading iteration
            //toggle led to indicate rssi reading
            send_toggle_led(src_id, master_node, 2);
            send_toggle_led(src_id, slave_node, 2);
            printf("Reading Ultrasonic\n");
            dis_val1 = get_obstacle_data(src_id,master_node,ULTRASONIC_FRONT);
            usleep(100000);
            dis_val2 = get_obstacle_data(src_id,slave_node,ULTRASONIC_FRONT);
            // sleep(2);
            printf("Reading RSSI..\n");
            usleep(100000);
            rssi_val1 = get_RSSI(src_id,master_node);
            usleep(100000);
            rssi_val2 = get_RSSI(src_id,slave_node);
            // sleep(2);
            
            //output reading to txt file
            fprintf(fptr, "%ld,%d\n", rssi_val1, dis_val1);
            fprintf(fptr2, "%ld,%d\n", rssi_val2, dis_val2);
            
            counter_rssi++;
            // send_toggle_led(src_id, master_node, 2);
            // send_toggle_led(src_id, slave_node, 2);
            
            usleep(100000);
            
        }
        
        counter_rssi = 0;
            send_reverse_time(src_id, master_node, dis-20); //move bot backward 1s
            send_reverse_time(src_id, slave_node, dis);
        
        // send_toggle_led(src_id, master_node, 1);
        // send_toggle_led(src_id, slave_node, 1);
        
        
        printf("Iteration: %d\n",counter_d);
        counter_d++;
        sleep(1);
    }
    counter_d = 0;
    
    fclose(fptr);
    fclose(fptr2);
}

/*! 
 * \Brief Slave Bot follows the Leader Bot while both of them are on the same side 
 *        The Slave is placed behind the Leader
 * 
 */
void bot_follow_bot()
{
    FILE *fp;
    char filename[40];
    sprintf(filename, "perf1.csv");
    fp = fopen(filename,"a");
    if(fp == NULL){
        printf("ERROR!");
        exit(1);
    }


    int val = 500; // 500 millisecond, for forward and reverse movement
    char end_loop = 0; 
    int cmd_val = 0;
    long leaderRSSI[KNNWINDOW] = { 0 };
    long slaveRSSI[KNNWINDOW] = { 0 };
    int distanceFromLeader;
    unsigned int requiredGap = 20; // current gap to maintain between the leader and the master
    
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

    printf("Input required gap (cm): ");
    scanf("%d", &requiredGap);

    int logging = 0;
    printf("Log data? (1/0)\n");
    scanf("%d", &logging);

    //check if node input is correct
    j = 0;
    int flagm = 0;
    int flags = 0;
    while(j < con_count){
        if(master_node == BOT_ID[j]) flagm = 1;
        if(slave_node == BOT_ID[j]) flags = 1;
        j++;
    }
    
    if(flagm != 1 && flags != 1){
        printf("Wrong bot input\n");
        return;
    }
    flagm = 0;
    flags = 0;

    printf("\n\n\n");
    
    printf("Select Master Action\n");
    printf("====================\n");
    printf("1. Move Forward\n");
    printf("2. Move Reverse\n");
    printf("3. Increase reverse multiplier\n");
    printf("4. Decrease reverse multiplier\n");
    printf("5. Increase forward multiplier\n");
    printf("6. Decrease forward multiplier\n");
    printf("0. Exit\n");
    
    //timed input
    while(!end_loop)
    {
        if( poll(&mypoll, 1, 1000) )
        {
            scanf("%d", &cmd_val);
            // printf("Insert duration:");
            //scanf("%d",&val);
        
            switch(cmd_val)
            {
                case 1: send_forward_time(src_id, master_node, val);
                        break;
                case 2: send_reverse_time(src_id, master_node, val);
                        break;
                case 3: rv_mult+=1;
                        printf("rv multiplier: %d\n",rv_mult);
                        break;
                case 4: rv_mult-=1;
                        printf("rv multiplier: %d\n",rv_mult);
                        break;
                case 5: fw_mult+=1;
                        printf("fw multiplier: %d\n",fw_mult);
                        break;
                case 6: fw_mult-=1;
                        printf("fw multiplier: %d\n",fw_mult);
                        break;
                case 0: printf("Exiting App ... \n"); 
                        stop_bot(src_id,slave_node); 
                           end_loop = 1;
                           break; 
            }
       
        }

        //printf("reading\n");
        //measure RSSI of leader and servant
        for(int i = 0; i < KNNWINDOW; i++)
        {
            leaderRSSI[i] = get_RSSI(src_id,master_node);
            slaveRSSI[i] = get_RSSI(src_id,slave_node);
        }
        //printf("sending\n");
        //if difference gives positive, we move forward; if difference give negative, we move backward
        distanceFromLeader = slaveDistanceGapKNN(leaderRSSI, slaveRSSI, 1);
        int us_reading1 = get_obstacle_data(src_id,slave_node,ULTRASONIC_FRONT);
        int us_reading2 = get_obstacle_data(src_id,master_node,ULTRASONIC_FRONT);
        printf("d_rssi: %d, d_us1: %d, d_us2: %d\n", distanceFromLeader,us_reading1, us_reading2);
        if(logging == 1) fprintf(fp, "%d,%d,%d,%d\n",distanceFromLeader, us_reading1, us_reading2, requiredGap);
        //only process if valid distance has been computed both for the leader and the slave
        if (distanceFromLeader < MaxRange)
        {
            distanceFromLeader = requiredGap - distanceFromLeader;  //(assuming the forward direction is towards AP)
        
            //if the Gap is too huge we need to move closer to the leader ... else the Gap is too small so we need to retreat back
            if(distanceFromLeader > DELTA) send_reverse_time(src_id,slave_node,(int)((distanceFromLeader-DELTA)*rv_mult));
            else if ((-1*distanceFromLeader) > DELTA) send_forward_time(src_id,slave_node,(int)((-distanceFromLeader-DELTA)*fw_mult));    
        }
        
    } 
    
    // usleep((1/READINGF)*1000000); //delay 
    fclose(fp);
}


/*! 
 * \Brief Slave Bot mimicks the Leader Bot from an opposit side of the AP
 * 
 */
void bot_mimick_bot()
{
    FILE *fp;
    char filename[40];
    sprintf(filename, "perf2.csv");
    fp = fopen(filename,"a");
    if(fp == NULL){
        printf("ERROR!");
        exit(1);
    }

    int val = 500; // 500 millisecond, for forward and reverse movement
    char end_loop = 0; 
    int cmd_val = 0;
    long leaderRSSI[KNNWINDOW] = { 0 };
    long slaveRSSI[KNNWINDOW] = { 0 };
    int distanceFromLeader;
    unsigned int requiredGap = 0; // current gap to maintain between the leader and the master
    
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
    int logging = 0;
    printf("Log data? (1/0)\n");
    scanf("%d", &logging);

    //check if input is correct
    j = 0;
    int flagm = 0;
    int flags = 0;
    while(j < con_count){
        if(master_node == BOT_ID[j]) flagm = 1;
        if(slave_node == BOT_ID[j]) flags = 1;
        j++;
    }
    
    if(flagm != 1 && flags != 1){
        printf("Wrong bot input\n");
        return;
    }
    flagm = 0;
    flags = 0;
    printf("\n\n\n");
    
    printf("Select Master Action\n");
    printf("====================\n");
    printf("1. Move Forward\n");
    printf("2. Move Reverse\n");
    printf("3. Increase reverse multiplier\n");
    printf("4. Decrease reverse multiplier\n");
    printf("5. Increase forward multiplier\n");
    printf("6. Decrease forward multiplier\n");
    
    printf("0. Exit\n");
    
    //timed input
    while(!end_loop)
    {
        if( poll(&mypoll, 1, 1000) )
        {
            scanf("%d", &cmd_val);
            // printf("Insert duration:");
            //scanf("%d",&val);
        
            switch(cmd_val)
            {
                case 1: send_forward_time(src_id, master_node, val);
                        break;
                case 2: send_reverse_time(src_id, master_node, val);
                        break;
                case 3: rv_mult+=1;
                        printf("rv multiplier: %d\n",rv_mult);
                        break;
                case 4: rv_mult-=1;
                        printf("rv multiplier: %d\n",rv_mult);
                        break;
                case 5: fw_mult+=1;
                        printf("fw divisor: %d\n",fw_mult);
                        break;
                case 6: fw_mult-=1;
                        printf("fw divisor: %d\n",fw_mult);
                        break;
                case 0: printf("Exiting App ... \n"); 
                        stop_bot(src_id,slave_node); 
                           end_loop = 1;
                           break; 
            }
       
        }
        printf("reading\n");
        //measure RSSI of leader and servant
        for(int i = 0; i < KNNWINDOW; i++)
        {
            leaderRSSI[i] = get_RSSI(src_id,master_node);
            slaveRSSI[i] = get_RSSI(src_id,slave_node);
        }
        printf("sending\n");
        //if difference gives positive, we move forward; if difference give negative, we move backward
        distanceFromLeader = slaveDistanceGapKNN(leaderRSSI, slaveRSSI, 2);
        int us_reading1 = get_obstacle_data(src_id,slave_node,ULTRASONIC_FRONT);
        int us_reading2 = get_obstacle_data(src_id,master_node,ULTRASONIC_FRONT);
        printf("d_rssi: %d, d_us1: %d, d_us2: %d\n", distanceFromLeader,us_reading1, us_reading2);
        if(logging == 1) fprintf(fp, "%d,%d,%d,%d\n",distanceFromLeader, us_reading1, us_reading2, requiredGap);
        //only process if valid distance has been computed both for the leader and the slave
        if (distanceFromLeader < MaxRange)
        {
            distanceFromLeader = requiredGap - distanceFromLeader;  //(assuming the forward direction is towards AP)
        
            //if the Gap is too huge we need to move closer to the leader ... else the Gap is too small so we need to retreat back
            if(distanceFromLeader > DELTA) send_reverse_time(src_id,slave_node,(int)((distanceFromLeader-DELTA)*rv_mult));
            else if ((-1*distanceFromLeader) > DELTA) send_forward_time(src_id,slave_node,(int)((-distanceFromLeader-DELTA)*fw_mult));    
        }
        
    } 
    
    // usleep((1/READINGF)*1000000); //delay
    fclose(fp);
}


/*!
 * \breif computes the distance of a slave from the AP compared to the master
 *
 * \param leaderRSSI an array of RSSI readings for the leader as defined by the KNNWINDOW
 * \param slaveRSSI an array of RSSI readings for the salve as defined by the KNNWINDOW
 *
 * \return 300 if the readings are too noisy and the distance fo the slave from the AP compared to the maaster if not
 *
 * \todo We need to add one more argument to indicate the application invoking this, so that we can use appropriet file for the slave depending on wheather it is mimick or follow
 */  
int slaveDistanceGapKNN(long * leaderRSSI, long * slaveRSSI, int mode) //leaderRSSI and slaveRSSI array of size KNNWINDOW
{

    //For invoking python command
    FILE *fp;
    char path[1024];
    char command[100];
    //KNNWINDOW 
    // both from leaderRSSI and slaveRSSI, remove extreme value
    long removalThreshold = 8;
    int countValidLeader= (int)(KNNWINDOW);
    int countValidSlave = (int)(KNNWINDOW);
    sort(leaderRSSI, countValidLeader);
    sort(slaveRSSI, countValidSlave);
    if(KNNWINDOW > 1){
        //max outlier
        if( (leaderRSSI[0] - leaderRSSI[1]) < -removalThreshold )
        {
            memmove(&leaderRSSI[0], &leaderRSSI[1], (countValidLeader-1)*sizeof(*leaderRSSI));
            countValidLeader--;
        }
        
        if( (slaveRSSI[0] - slaveRSSI[1]) < -removalThreshold )
        {
            memmove(&slaveRSSI[0], &slaveRSSI[1], (countValidSlave-1)*sizeof(*slaveRSSI));
            countValidSlave--;
        }
        
        //least outlier
        if( (leaderRSSI[countValidLeader-2] - leaderRSSI[countValidLeader-1]) < -removalThreshold )
        {
            leaderRSSI[countValidLeader-1] = leaderRSSI[countValidLeader-2];
            countValidLeader--;
        }
            
        if( (slaveRSSI[countValidSlave-2] - slaveRSSI[countValidSlave-1]) < -removalThreshold )
        {
            slaveRSSI[countValidSlave-1] = slaveRSSI[countValidSlave-2];
            countValidSlave--;
        }
    }
    #ifdef __DEBUG__
    for(int i = 0; i < countValidLeader; i++){
        printf("%ld ", leaderRSSI[i]);
    }
    printf("\n");
    for(int i = 0; i < countValidSlave; i++){
        printf("%ld ", slaveRSSI[i]);
    }
    printf("\n");
    #endif
    
    //abort if the reading is unstable
    if(countValidSlave < 1 || countValidLeader < 1)
    {
        return MaxRange; //the maximum value the ultrasonic reader will give is 254
    }
    
    //Finding the distance for the remaining RSSI
    int slaveDistance = 0; 
    int leaderDistance = 0;
    
    // for both the leader and slave calculate distance from the KNN table for each member
    //for leader
    for(int i = 0; i < countValidLeader; i++)
    {
        sprintf(command, "python knn.py calibration_master_%d.csv %d %d", mode, K, (int)leaderRSSI[i]);
        fp = popen(command, "r");
        if (fp ==NULL){
            printf("Failed\n");
            exit(1);
        }
        while(fgets(path, sizeof(path), fp) != NULL)
            leaderDistance += atoi(path);    
        pclose(fp);
    }

    //average the reading
    leaderDistance /= countValidLeader;
    printf("leaderDistance: %d\n",leaderDistance);
    
    //for slave
    for(int i = 0; i < countValidSlave; i++)
    {
        sprintf(command, "python knn.py calibration_slave_%d.csv %d %d", mode, K, (int)slaveRSSI[i]);
        fp = popen(command, "r");
        if (fp ==NULL){
            printf("Failed\n");
            exit(1);
        }
        while(fgets(path, sizeof(path), fp) != NULL)
            slaveDistance += atoi(path);    
        pclose(fp);
    }
    slaveDistance /= countValidSlave;
    printf("slaveDistance: %d\n",slaveDistance);
    // again remove the estreme and calculate the average of the results bot for leader and slave
    // .. we will use this step if necessary]
        
    return (int)(slaveDistance - leaderDistance);
}

/*!
 * \breif bubble sort from stackoverflow
 *        arrangs in decending order
 *
 * \param v pointer to the array to be sorted
 *
 * \param n the size of the array
 */

void sort (long * v, int n)
{
    int i,k;
    int temp;

    for(i = 0; i<n-1; i++) {
        for(k = 0; k<n-1-i; k++) {
            if(v[k] > v[k+1]) {
                temp = v[k];
                v[k] = v[k+1];
                v[k+1] = temp;
            }
        }
    }
}