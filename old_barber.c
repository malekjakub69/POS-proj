/**
 * Project: Sleeping Barber Problem
 * Author: Jakub Málek
 * Date: 24.4.2024
 * Description: The program simulates the Sleeping Barber Problem.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define CUSTOMERS 50
#define CHAIRS 5
// service time 40 ms is too fast, so I changed it to 80 ms for any rejection
#define SERVICE_TIME 80
#define MIN_INTERVAL 10
#define MAX_INTERVAL 100

#define true 1
#define false 0

#define print true
// Values for the barber and customers synchronization
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t barber_available = PTHREAD_COND_INITIALIZER;
pthread_cond_t customer_waiting = PTHREAD_COND_INITIALIZER;

int waiting = 0; // Number of customers waiting
int is_barber_sleeping = true;

// Prototype of functions
void *barber(void *arg);
void *customer(void *arg);
void barber_sleep();
void barber_service();
void customer_entry(int number);
void customer_wait(int number);
void customer_service(int number);
void customer_exit(int number);
void customer_reject(int number);

int served_customers = 0;
int rejected_customers = 0;
int barber_cutting = 0;

int main()
{
    pthread_t barber_thread, customer_threads[CUSTOMERS];
    srand((unsigned)time(NULL));

    // Create barber thread
    pthread_create(&barber_thread, NULL, barber, NULL);

    // Simulate the arrival of customers
    for (int i = 0; i < CUSTOMERS; i++)
    {
        pthread_create(&customer_threads[i], NULL, customer, (void *)(long)i);
        int interval = (rand() % (MAX_INTERVAL - MIN_INTERVAL + 1) + MIN_INTERVAL);
        usleep(interval * 1000); // Arrival of the next customer
    }

    // Wait for the end of the customer threads
    for (int i = 0; i < CUSTOMERS; i++)
    {
        pthread_join(customer_threads[i], NULL);
    }

    // End of the barber thread
    pthread_cancel(barber_thread);
    pthread_join(barber_thread, NULL);

    printf("----------------------\n");
    printf("Customers served: %d\n", served_customers);
    printf("Customers declined: %d\n", rejected_customers);

    return 0;
}

void *barber(void *arg)
{
    // Barber's work cycle
    while (true)
    {
        // Synchronize the barber with the customers
        pthread_mutex_lock(&mutex);
        // If there are no customers, the barber goes to sleep
        if (waiting == 0)
        {
            is_barber_sleeping = true;
            barber_sleep();
            // Waiting for the arrival of the customer
            pthread_cond_wait(&customer_waiting, &mutex);
        }
        is_barber_sleeping = false;

        // The barber is ready to serve the customer
        pthread_cond_signal(&barber_available);
        // The barber is cutting hair
        pthread_mutex_unlock(&mutex);

        barber_service();
        barber_cutting++;
    }

    return NULL;
}
void *customer(void *arg)
{
    int number = (int)(long)arg;

    pthread_mutex_lock(&mutex);
    customer_entry(number);
    // If there is a free chair, the customer sits down
    if (waiting < CHAIRS)
    {
        waiting++;
        // if the barber is sleeping, wake him up
        if (is_barber_sleeping)
        {
            pthread_cond_signal(&customer_waiting);
            is_barber_sleeping = false;
        }
        //
        pthread_mutex_unlock(&mutex);
        customer_wait(number);
        pthread_mutex_lock(&mutex);
        // The customer is waiting for the barber to be ready
        pthread_cond_wait(&barber_available, &mutex);

        // The barber is ready to serve the customer
        customer_service(number);
        served_customers++;
        // The customer leaves the barbershop
        waiting--;
        pthread_cond_signal(&customer_waiting);
        pthread_mutex_unlock(&mutex);

        customer_exit(number);
    }
    else
    {
        // If there are no free chairs, the customer leaves the barbershop
        rejected_customers++;
        customer_reject(number);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

// Functions for printing messages barber sleep
void barber_sleep()
{

    if (print)
    {
        printf("Barber sleep.\n");
        fflush(stdout);
    }
}

// Function for printing messages barber service
void barber_service()
{
    if (print)
    {
        printf("Barber cutting hair.\n");
        fflush(stdout);
    }
    usleep(SERVICE_TIME * 1000);
    if (print)
    {
        printf("Barber finished cutting hair.\n");
        fflush(stdout);
    }
}

// Functions for printing messages customer entry
void customer_entry(int number)
{

    if (print)
    {
        printf("Customer %d entered.\n", number);
        fflush(stdout);
    }
}

// Functions for printing messages customer wait
void customer_wait(int number)
{

    if (print)
    {
        printf("Customer %d waiting.\n", number);
        fflush(stdout);
    }
}

// Functions for printing messages customer service
void customer_service(int number)
{
    if (print)
    {
        printf("Customer %d served.\n", number);
        fflush(stdout);
    }
    usleep(SERVICE_TIME * 1000); // Simulace obsluhy zákazníka
    if (print)
    {
        printf("Customer %d finished.\n", number);
        fflush(stdout);
    }
}

void customer_exit(int number)
{
    if (print)
    {
        printf("Customer %d exited.\n", number);
        fflush(stdout);
    }
}

void customer_reject(int number)
{
    if (print)
    {
        printf("Customer %d rejected.\n", number);
        fflush(stdout);
    }
}
