#include <chrono>
#include <functional>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>

#define BUFFER_SIZE 10

using namespace std;

int buffer[BUFFER_SIZE];

sem_t full;
sem_t empty_sem;
pthread_mutex_t mutex;
int reader_head = 0;
int writer_head = 0;

double producer_wait_time = 0.0;
int producer_wait_count = 0;

double consumer_wait_time = 0.0;
int consumer_wait_count = 0;

void signalHandler(int signum)
{
    if (producer_wait_count > 0)
        cout << "Average wait time for producers: " << (producer_wait_time / producer_wait_count) << " seconds" << endl;

    if (consumer_wait_count > 0)
        cout << "Average wait time for consumers: " << (consumer_wait_time / consumer_wait_count) << " seconds" << endl;

    if (consumer_wait_time > producer_wait_time)
        cout << "Producers are overworking, consumers are underworking\n";
    else
        cout << "Producers are overworking, consumers are underworking\n";
    sem_destroy(&empty_sem);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);
    exit(0);
}

void *producer(void *arg)
{
    while (true)
    {
        int value = rand() % 100;
        auto start_wait = std::chrono::high_resolution_clock::now();
        sem_wait(&empty_sem);
        auto end_wait = std::chrono::high_resolution_clock::now();

        // Calculate wait time
        std::chrono::duration<double> wait_duration = end_wait - start_wait;
        producer_wait_time += wait_duration.count();
        producer_wait_count++;
        pthread_mutex_lock(&mutex);
        cout << "Producer #" << *(static_cast<int *>(arg)) << " produced " << value << " at head pos. " << writer_head
             << endl;
        buffer[writer_head] = value;
        writer_head = (writer_head + 1) % BUFFER_SIZE;
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
        usleep(rand() % 1000000 + 300000);
    }

    return nullptr;
}

void *consumer(void *arg)
{
    while (true)
    {
        auto start_wait = std::chrono::high_resolution_clock::now();
        sem_wait(&full);
        auto end_wait = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> wait_duration = end_wait - start_wait;
        consumer_wait_time += wait_duration.count();
        consumer_wait_count++;

        pthread_mutex_lock(&mutex);
        int value = buffer[reader_head];
        cout << "\tConsumer #" << *(static_cast<int *>(arg)) << " consumed " << value << " at head pos. " << reader_head
             << endl;
        reader_head = (reader_head + 1) % BUFFER_SIZE;
        pthread_mutex_unlock(&mutex);
        sem_post(&empty_sem);
        usleep(rand() % 1500000 + 30000000);
    }

    return nullptr;
}

int main()
{
    signal(SIGINT, signalHandler);
    sem_init(&empty_sem, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    pthread_t producers[4];
    pthread_t consumers[4];
    for (int i = 0; i < 4; i++)
    {
        int *val = new int(i);
        pthread_create(&producers[i], NULL, producer, val);
        pthread_create(&consumers[i], NULL, consumer, val);
    }

    for (int i = 0; i < 4; i++)
    {
        pthread_join(producers[i], NULL);
        pthread_join(consumers[i], NULL);
    }

    sem_destroy(&empty_sem);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    return 0;
}
