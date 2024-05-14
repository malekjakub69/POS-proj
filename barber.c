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
#include <sys/queue.h>

#define CUSTOMERS 50
#define CHAIRS 5
// service time 40 ms is too fast, so I changed it to 80 ms for any rejection
#define SERVICE_TIME 80
#define MIN_INTERVAL 10
#define MAX_INTERVAL 100

#define true 1
#define false 0

// Prototype of functions
void *barber(void *arg);
void *customer(void *arg);
void barber_sleep();
void barber_service(int customer_number);
void customer_entry(int number);
void customer_wait(int number);
void customer_served(int number);
void customer_exit(int number);
void customer_reject(int number);

// Values for the barber and customers synchronization
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t barber_cond = PTHREAD_COND_INITIALIZER;
int customers_waiting = 0;

// Define the structure of the queue
typedef struct entry
{
    int id;
    pthread_cond_t cond;
    TAILQ_ENTRY(entry)
    entries;
} entry;

TAILQ_HEAD(tailhead, entry)
head = TAILQ_HEAD_INITIALIZER(head);

int served_customers = 0;
int rejected_customers = 0;
int barber_cutting = 0;

int main()
{
    srand((unsigned)time(NULL));

    pthread_t barber_thread, customer_threads[CUSTOMERS];
    pthread_cond_init(&barber_cond, NULL);

    // Create barber thread
    pthread_create(&barber_thread, NULL, barber, NULL);

    // Simulate the arrival of customers
    for (long i = 0; i < CUSTOMERS; i++)
    {
        pthread_create(&customer_threads[i], NULL, customer, (void *)i);
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
    pthread_cond_destroy(&barber_cond);

    printf("----------------------\n");
    printf("Customers served: %d\n", served_customers);
    printf("Customers declined: %d\n", rejected_customers);

    return 0;
}
void *barber(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);
        while (customers_waiting == 0)
        {
            barber_sleep();
            pthread_cond_wait(&barber_cond, &mutex);
        }

        entry *customer = TAILQ_FIRST(&head);
        TAILQ_REMOVE(&head, customer, entries);
        customers_waiting--;

        barber_service(customer->id);

        pthread_cond_signal(&customer->cond); // Notify the customer haircut is done

        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void *customer(void *arg)
{
    struct entry *my_entry = malloc(sizeof(struct entry));
    pthread_cond_init(&my_entry->cond, NULL);
    my_entry->id = *(int *)arg;

    customer_entry(my_entry->id);

    pthread_mutex_lock(&mutex);
    if (customers_waiting < 5)
    { // Only 5 chairs available
        customers_waiting++;
        TAILQ_INSERT_TAIL(&head, my_entry, entries);

        customer_wait(my_entry->id);

        pthread_cond_signal(&barber_cond);          // Wake barber if sleeping
        pthread_cond_wait(&my_entry->cond, &mutex); // Wait for the haircut to be done

        customer_served(my_entry->id);
    }
    else
    {
        customer_reject(my_entry->id);
    }

    customer_exit(my_entry->id);
    pthread_mutex_unlock(&mutex);
    pthread_cond_destroy(&my_entry->cond);
    return NULL;
}

// Functions for printing messages barber sleep
void barber_sleep()
{
    printf("Barber sleep.\n");
    fflush(stdout);
}

// Function for printing messages barber service
void barber_service(int customer_number)
{
    printf("Barber cutting hair of customer %d.\n", customer_number);
    fflush(stdout);

    usleep(SERVICE_TIME * 1000);

    printf("Barber finished cutting hair.\n");
    fflush(stdout);
}

// Functions for printing messages customer entry
void customer_entry(int number)
{
    printf("Customer %d entered.\n", number);
    fflush(stdout);
}

// Functions for printing messages customer wait
void customer_wait(int number)
{
    printf("Customer %d waiting.\n", number);
}

// Functions for printing messages customer service
void customer_served(int number)
{
    printf("Customer %d served.\n", number);
    fflush(stdout);
}

void customer_exit(int number)
{
    printf("Customer %d exited.\n", number);
    fflush(stdout);
}

void customer_reject(int number)
{
    printf("Customer %d rejected.\n", number);
    fflush(stdout);
}
