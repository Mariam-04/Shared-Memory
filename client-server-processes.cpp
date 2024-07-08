#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;
#define SHM_SIZE 1024 // Size of the shared memory segment

// Function to calculate factorial
int factorial(int n) {
  if (n <= 1)
    return 1;
  else
    return n * factorial(n - 1);
}

int main() {
  // Create a shared memory segment
  key_t key = ftok("shmfile", 'R');
  int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
  if (shmid < 0) {
    perror("shmget");
    return 1;
  }

  // Fork a child process
  pid_t pid = fork();

  if (pid < 0) {
    perror("fork");
    return 1;
  } else if (pid == 0) { // Child process (producer)
    // Generate and write random integers to shared memory
    int *shm = (int *)shmat(shmid, NULL, 0);
    if (shm == (int *)-1) {
      perror("shmat");
      exit(1);
    }

    cout << "Producer process generating random integers: " << endl;
    for (int i = 0; i < 5; ++i) {
      shm[i] = rand() % 10 + 1; // Random integers between 1 and 10
      cout << "Produced integer: " << shm[i] << endl;
    }

    shmdt(shm); // Detach shared memory
  } else {      // Parent process (consumer)
    wait(NULL); // Wait for the child process to finish

    // Read integers from shared memory and calculate factorial
    int *shm = (int *)shmat(shmid, NULL, 0);
    if (shm == (int *)-1) {
      perror("shmat");
      exit(1);
    }

    cout << "Consumer process reading from shared memory and calculating "
            "factorial: "
         << endl;
    for (int i = 0; i < 5; ++i) {
      int num = shm[i];
      int fact = factorial(num);
      cout << "Factorial of " << num << " is: " << fact << endl;
    }

    shmdt(shm); // Detach shared memory

    // Remove the shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
      perror("shmctl");
      return 1;
    }
  }

  return 0;
}