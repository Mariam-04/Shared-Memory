#include <cstdlib>
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

#define SHM_SIZE 5 * sizeof(int) // Size of the shared memory segment

int main() {
  int shmid;
  key_t key = 1234; // Key for the shared memory segment
  int *shm;         // Pointer to the shared memory segment

  // Create a shared memory segment
  if ((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) < 0) {
    perror("shmget");
    exit(1);
  }

  // Attach the shared memory segment to the process's address space
  if ((shm = (int *)shmat(shmid, NULL, 0)) == (int *)-1) {
    perror("shmat");
    exit(1);
  }

  // Prompt the user to enter 5 integers and write them to shared memory
  cout << "Parent process: Enter 5 integers:" << std::endl;
  for (int i = 0; i < 5; i++) {
    cin >> shm[i]; // Write integers to shared memory
  }

  // Create a child process
  pid_t pid = fork();

  if (pid == -1) {
    perror("fork");
    exit(1);
  }

  if (pid == 0) { // Child process (reader)
    cout << "Child process: Reading integers from shared memory and "
            "displaying squares:"
         << endl;
    for (int i = 0; i < 5; i++) {
      cout << "Square of " << shm[i] << ": " << shm[i] * shm[i] << endl;
    }

    // Detach the shared memory segment
    if (shmdt(shm) == -1) {
      perror("shmdt");
      exit(1);
    }

    exit(0);
  } else { // Parent process
    // Wait for the child process to finish
    wait(NULL);

    // Detach the shared memory segment
    if (shmdt(shm) == -1) {
      perror("shmdt");
      exit(1);
    }

    // Remove the shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
      perror("shmctl");
      exit(1);
    }
    std::cout << "Shared memory segment removed." << std::endl;
  }

  return 0;
}