#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;
#include <cctype>
#include <cstdlib>
#include <cstring>

#define SHM_SIZE 1024 // Size of the shared memory segment

void writeToSharedMemory(int shmid, const char *data, int size) {
  char *shm = (char *)shmat(shmid, NULL, 0);
  if (shm == (char *)-1) {
    cout << "eroor in writing" << endl;
    exit(1);
  }
  memcpy(shm, data, size);
  shmdt(shm);
}

void readFromSharedMemory(int shmid, char *buffer, int size) {
  char *shm = (char *)shmat(shmid, NULL, 0);
  if (shm == (char *)-1) {
    cout << "eorro in reading" << endl;
    exit(1);
  }
  std::memcpy(
      buffer, shm,
      size - 1); // Read size - 1 bytes to avoid reading the null terminator
  buffer[size - 1] = '\0'; // Ensure null termination
  shmdt(shm);
}

int main(int ag, char *agr[]) {

  if (ag != 2) {
    cout << "error contents not given" << endl;
    return 0;
  }

  int shmid;
  key_t key = 1234; // Key for the shared memory segment
  int *shm;         // Pointer to the shared memory segment

  const char *filename = agr[1];

  // Open file and read its content
  ifstream file(filename);
  if (!file) {
    cout << "Error: Unable to open file " << filename << endl;
    return 1;
  }

  string data;
  char ch;
  while (file.get(ch)) {
    if (isalpha(ch)) {
      data += ch;
    }
  }

  // Create a shared memory segment
  if ((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) < 0) {
    cout << " erroe in creation shmget" << endl;
    exit(1);
  }

  writeToSharedMemory(shmid, data.c_str(), data.size());

  // Attach the shared memory segment to the process's address space
  if ((shm = (int *)shmat(shmid, NULL, 0)) == (int *)-1) {
    cout << "eorro in attachments" << endl;
    exit(1);
  }

  pid_t pid = fork();

  if (pid == 0) { // Child process
    // Read data from shared memory
    char *buffer = new char[data.size() + 1];
    readFromSharedMemory(shmid, buffer, data.size());

    // Change the case of each character
    for (int i = 0; buffer[i] != '\0'; ++i) {
      if (islower(buffer[i]))
        buffer[i] = toupper(buffer[i]);
      else if (isupper(buffer[i]))
        buffer[i] = tolower(buffer[i]);
      exit(0);
    }

    writeToSharedMemory(shmid, buffer, data.size());

    delete[] buffer;
  } else if (pid > 0) { // Parent process
    wait(NULL);

    // Read data from shared memory
    char *buffer = new char[data.size() + 1];
    readFromSharedMemory(shmid, buffer, data.size());

    // Write changed data back to file
    ofstream outfile(filename);
    outfile << buffer;

    delete[] buffer;
  } else {
    perror("fork");
    return 1;
  }

  // Remove the shared memory segment
  if (shmctl(shmid, IPC_RMID, NULL) == -1) {
    cout << "eroor here. " << endl;
    exit(1);
  }

  cout << "Shared memory segment removed. " << endl;

  return 0;
}