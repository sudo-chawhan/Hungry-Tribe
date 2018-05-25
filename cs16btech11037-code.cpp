#include <semaphore.h>
#include <thread>
#include <vector>
#include <iostream>
#include <atomic>
#include <stdio.h>
#include <random>
#include <ctime>
#include <unistd.h>
#include <time.h>
#include <chrono>
#include <pthread.h>
#include <fstream>

int n,M,K,t11,t12,t21,t22,servings_count,total_out;
float avg_time = 0.0;

using namespace std;

void *tribers_work(void *i);     // writer thread
void *cooks_work(void *i);

sem_t request_for_food;         // for requesting food only M threads could enter
sem_t servings_count_mutex;     // for servings count as it is global
sem_t pot_empty;                // wait if pot is empty
sem_t pot_full;                 // wait if pot is full
sem_t total_out_mutex;          // for total out is global
sem_t request_cook;             // to request cook to cook


FILE *output_log = fopen("Tribe_log.txt", "w");  //open a file to wirte to it
FILE *output_avg = fopen("Average_time.txt", "w");  //open a file to append to it

int main()
{

    sem_init(&servings_count_mutex,0,1);   
    sem_init(&pot_empty,0,0);   
    sem_init(&pot_full,0,0); 
    sem_init(&total_out_mutex,0,1); 
    sem_init(&request_cook,0,1); 

    ifstream inFile ;
    inFile.open("inputs.txt");    //we read from the input file
    inFile>>K>>M>>n>>t11>>t12>>t21>>t22;
    servings_count = M;

    sem_init(&request_for_food,0,M);   


    pthread_t triber[K],cook[1];
    pthread_attr_t triber_attr[K],cook_attr[1];


    /* -- create K threads one for each triber -- */
    for (int i = 0; i < K; ++i) {
        pthread_attr_init(&triber_attr[i]);
                pthread_create(&triber[i],&triber_attr[i],tribers_work,(void*)(intptr_t)(i+1));
    }
   
    // cooks thread
    pthread_attr_init(&cook_attr[0]);
    pthread_create(&cook[0],&cook_attr[0],cooks_work,(void*)(intptr_t)(1));
    
    // joining the threads
    for(int i=0;i<K;i++)
        pthread_join(triber[i],NULL);
    
    pthread_join(cook[0],NULL);

    // calculate avg times
    fprintf(output_avg,"\nTotal average waiting time = %f milliseconds\n",avg_time/K);

    sem_destroy(&request_for_food);
    sem_destroy(&servings_count_mutex);   
    sem_destroy(&pot_empty);   
    sem_destroy(&pot_full); 
    sem_destroy(&total_out_mutex); 
    sem_destroy(&request_cook);

    return 0;


}


double ran_exp(int t){
    default_random_engine generator;
    exponential_distribution<double> distribution(1.0/t);
    return distribution(generator);
}


void *tribers_work(void * param){

    int id = (intptr_t)param;

    time_t my_time;
    struct tm *timeinfo;
    float eat_time_total = 0.0;

    for(int i=1; i<=n ;i++){

        // Hungry phase
        // check if pot is empty 
        // if it is then sem_wait for it to refill

        auto reqTime = std::chrono::system_clock::now();      
        time (&my_time);
        timeinfo = localtime (&my_time);
        fprintf(output_log,"time%02d:%02d: T%d becomes hungry\n",timeinfo->tm_min,timeinfo->tm_sec,id);

        
        sem_wait(&request_for_food);        // at max only M threads could enter

        sem_wait(&request_cook);            // on request dont let other threads to enter

        if(servings_count==0){              // if pot is empty
            
            //ask cook to fill the pot
            sem_post(&pot_empty);
            // wait for the pot to get filled
            sem_wait(&pot_full);
        }

        sem_post(&request_cook);            // only others threads to now enter

        
        // Eating phase

        auto enterTime = std::chrono::system_clock::now(); 
        time (&my_time);
        timeinfo = localtime (&my_time);
        fprintf(output_log,"time%02d:%02d: T%d eats from the pot\n",timeinfo->tm_min,timeinfo->tm_sec,id);

        float eat_time = (std::chrono::duration_cast<std::chrono::microseconds>(enterTime-reqTime).count())/(1000.0);
        eat_time_total += eat_time;

        sem_wait(&servings_count_mutex);    
        servings_count--;                       // decrement servings_count as we have just eaten one
        sem_post(&servings_count_mutex);
        
        usleep(ran_exp(t11)*1000);

        sem_post(&request_for_food);



        auto sleep_time = std::chrono::system_clock::now();
        time (&my_time);
        timeinfo = localtime (&my_time);
        fprintf(output_log,"time%02d:%02d: T%d now sleeps\n",timeinfo->tm_min,timeinfo->tm_sec,id);

        //Community service phase
        usleep(ran_exp(t12)*1000);          // perform community services
        

    }

    // On thead finish


    auto exit_time = std::chrono::system_clock::now();
    time (&my_time);
    timeinfo = localtime (&my_time);
    fprintf(output_log,"time%02d:%02d: T%d has eaten n times. Hence, exits.\n",timeinfo->tm_min,timeinfo->tm_sec,id);

    fprintf(output_avg,"average waiting time for T%d = %f milliseconds\n",id,eat_time_total/n);

    // for calculating avg waiting time
    sem_wait(&total_out_mutex);
    avg_time += eat_time_total/n;
    total_out ++;
    sem_post(&total_out_mutex);

    if(total_out==K){
        sem_post(&pot_empty);
    } 

    pthread_exit(0);
}

void *cooks_work(void * param){


    while(total_out!=K){

        sem_wait(&pot_empty);               // wait until pot is empty

        usleep(ran_exp(t21)*1000);          // put servings in the pot

        sem_wait(&servings_count_mutex);   
        servings_count = M;                 // fill the pot
        sem_post(&servings_count_mutex);

        sem_post(&pot_full);                // tell others that the pot is now full

        usleep(ran_exp(t22)*1000);          // now sleep
    }

    pthread_exit(0);
}
