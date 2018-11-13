#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(){
    FILE *fp;
    char path[1024];
    char command[100];
    
    int number;
    int k;
    printf("Insert RSSI: ");
    scanf("%d", &number);
    printf("Insert k: ");
    scanf("%d",&k);
    sprintf(command, "python knn.py rssi_readings_inside.csv %d %d",k,number);
    
    //printf("%s\n",command);
    fp = popen(command, "r");
    if (fp ==NULL){
        printf("Failed\n");
        exit(1);
    }
    while(fgets(path, sizeof(path), fp) != NULL)
        printf("%d",atoi(path));
    
    pclose(fp);

    return 0;
}