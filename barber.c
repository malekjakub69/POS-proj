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
int barber_busy = false;

// Prototype of functions
void *
barber(void *arg);
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
    while (true)
    {
        pthread_mutex_lock(&mutex);

        while (waiting == 0) // testování podmínky v cyklu
        {
            barber_sleep();
            pthread_cond_wait(&customer_waiting, &mutex);
        }

        waiting--;
        barber_busy = true;

        pthread_cond_signal(&barber_available);
        pthread_mutex_unlock(&mutex);

        barber_service();
    }
}

void *customer(void *arg)
{
    int number = (int)(long)arg;

    customer_entry(number);

    pthread_mutex_lock(&mutex);
    // If there is a free chair, the customer sits down
    if (waiting < CHAIRS)
    {
        waiting++;

        customer_wait(number);

        pthread_cond_signal(&customer_waiting);

        while (!barber_busy) // testování podmínky v cyklu
        {
            pthread_cond_wait(&barber_available, &mutex);
        }

        barber_busy = false;
        pthread_mutex_unlock(&mutex);

        served_customers++;
        customer_service(number);
    }
    else
    {
        // If there are no free chairs, the customer leaves the barbershop
        rejected_customers++;
        customer_reject(number);
        pthread_mutex_unlock(&mutex);
    }
    customer_exit(number);

    return NULL;
}

void barber_service()
{
    printf("Barber is cutting hair.\n");
    usleep(SERVICE_TIME * 1000);
}

void barber_sleep()
{
    printf("Barber sleep.\n");
}

void customer_entry(int number)
{
    printf("Customer %d entry.\n", number);
}

void customer_wait(int number)
{
    printf("Customer %d wait.\n", number);
}

void customer_service(int number)
{
    printf("Customer %d service.\n", number);
}

void customer_reject(int number)
{
    printf("Customer %d rejected.\n", number);
}

void customer_exit(int number)
{
    printf("Customer %d rejected.\n", number);
}