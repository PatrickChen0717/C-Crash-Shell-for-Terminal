#include <stdio.h>

void main(int argc, char *argv[]){
    int counter=0;
    while(counter <= 100){
        printf("count = %d\n",counter);
        sleep(1);
        counter++;
    }
}
