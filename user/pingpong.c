#include "kernel/types.h"  
#include "kernel/stat.h"   
#include "user/user.h"     // Thư viện hàm hệ thống người dùng

int main(int argc, char *argv[]) {
    int p1[2], p2[2];  // Two pipes: p1 for parent-to-child, p2 for child-to-parent
    char byte = 'x';   // Byte to send

    // Create both pipes
    pipe(p1);
    pipe(p2);

    int pid_t = fork();

    if (pid_t == 0) {  // Child process
        // Close unused ends
        close(p1[1]);  // Close write end of p1 (parent-to-child pipe)
        close(p2[0]);  // Close read end of p2 (child-to-parent pipe)

        // Read from p1 (parent-to-child pipe)
        read(p1[0], &byte, 1);
        close(p1[0]);  // Close read end after reading

        // Print received message
        printf("%d: received ping\n", getpid());

        // Write response to p2 (child-to-parent pipe)
        write(p2[1], &byte, 1);
        close(p2[1]);  // Close write end after writing

        exit(0);  // End child process
    } else {  // Parent process
        // Close unused ends
        close(p1[0]);  // Close read end of p1
        close(p2[1]);  // Close write end of p2

        // Send byte to child through p1
        write(p1[1], &byte, 1);
        close(p1[1]);  // Close write end after writing

        // Wait for response from child through p2
        read(p2[0], &byte, 1);
        close(p2[0]);  // Close read end after reading

        // Print received message
        printf("%d: received pong\n", getpid());

        exit(0);  // End parent process
    }
}
