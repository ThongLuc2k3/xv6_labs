#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: xargs command [args...]\n");
        exit(1);
    }

    int start_idx = 1;
    if (strcmp(argv[1], "-n") == 0)
        start_idx = 3;
    else
        start_idx = 1;

    char *args[MAXARG];
    for (int i = start_idx; i < argc; i++) {
        args[i - start_idx] = argv[i];
    }

    char input[500];
    int pos = argc - start_idx;

    while (1) {
        int n = 0;
        while (read(0, &input[n], 1) == 1) { // read(0,..) với 0 là stdin đầu vào, 1 là ra, 2 là lỗi chuẩn.
            if (input[n] == '\n') { 
                input[n] = '\0';  // Đổi '\n' thành '\0' để kết thúc chuỗi
                args[pos] = input; // Đặt dòng đầu vào làm tham số cuối của lệnh
                args[pos + 1] = 0; // Kết thúc danh sách đối số

                int child = fork();
                if (child < 0) {
                    printf("Cannot create process\n");
                    exit(1);
                } else if (child == 0) {
                    exec(args[0], args);  // Thực thi lệnh với đối số
                    printf("Exec failed\n");
                    exit(1);
                } else {
                    n = -1;
                    memset(input, 0, sizeof(input));
                    wait(0); // Chờ tiến trình con hoàn thành
                }
            }
            n++;
            if (n == 500) {
                printf("Command out of range\n");
                exit(1);
            }
        }
        if (n == 0) break;  // Kết thúc nếu không đọc được gì từ stdin
    }
    exit(0);
}