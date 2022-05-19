#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void *producer(void *args);
void *consumer(void *args);
void produce(void);
void consume(void);
int calculateItem(void);
void addItem(int item, int *id);
void removeItem(int *id);

int bufferSize;
int numLimit;
int currentNum = 0;
int *buffer;
int in = 0, out = 0;
// initialize semaphores and mutexes
sem_t bufferSema, resources, delay, currentNumSema;
pthread_mutex_t currentNumLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t inLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t outLock = PTHREAD_MUTEX_INITIALIZER;

int main() {
  int prodNumber;
  int consNumber;
  sem_init(&bufferSema, 0, 1);
  sem_init(&delay, 0, 0);

  scanf("%d %d %d %d", &prodNumber, &consNumber, &numLimit, &bufferSize);
  sem_init(&resources, 0, bufferSize);
  buffer = (int *)malloc(sizeof(int) * bufferSize);
  memset(buffer, -1, bufferSize * sizeof(int));
  pthread_t producers[prodNumber];
  int producersId[prodNumber];
  pthread_t consumers[consNumber];
  int consumersId[consNumber];

  for (int i = 0; i < prodNumber; i++) {
    producersId[i] = i;
    pthread_create(&(producers[i]), NULL, producer, &producersId[i]);
  }

  for (int i = 0; i < consNumber; i++) {
    consumersId[i] = i;
    pthread_create(&(consumers[i]), NULL, consumer, &consumersId[i]);
  }

  for (int i = 0; i < prodNumber; i++)
    pthread_join(producers[i], NULL);

  for (int i = 0; i < prodNumber; i++)
    pthread_join(consumers[i], NULL);

  return 0;
}

void *producer(void *args) {
  while (1) {
    int *id = (int *)args;
    int item = calculateItem();

    // obtain buffer semaphore and remove a resource
    sem_wait(&resources);
    sem_wait(&bufferSema);

    // add producer's value to the buffer
    addItem(item, id);
    // release buffer semaphore
    sem_post(&bufferSema);
    // warn consumers that buffer is not empty
    sem_post(&delay);
  }
  return args;
}

void *consumer(void *args) {
  int *id = (int *)args;
  while (1) {
    // check if buffer is not empty
    sem_wait(&delay);
    // obtain buffer semaphore
    sem_wait(&bufferSema);
    // consume an item from buffer
    removeItem(id);
    // release buffer semaphore and add a resource
    sem_post(&resources);
    sem_post(&bufferSema);
  }
  return args;
}

int calculateItem() {
  int returnValue = 2 * currentNum + 1;
  pthread_mutex_lock(&currentNumLock);
  if (currentNum == numLimit)
    currentNum = 0;
  else
    currentNum++;
  pthread_mutex_unlock(&currentNumLock);
  return returnValue;
}

void addItem(int item, int *id) {
  pthread_mutex_lock(&inLock);
  while (buffer[in] != -1)
    in = (in + 1) % bufferSize;
  printf("Produtor %d produzindo %d na posição %d\n", *id, item, in);
  buffer[in] = item;
  in = (in + 1) % bufferSize;
  pthread_mutex_unlock(&inLock);
}

void removeItem(int *id) {
  pthread_mutex_lock(&outLock);
  while (buffer[out] == -1)
    out = (out + 1) % bufferSize;
  printf("Consumidor %d consumindo %d na posição %d\n", *id, buffer[out], out);
  buffer[out] = -1;
  out = (out + 1) % bufferSize;
  pthread_mutex_unlock(&outLock);
}
