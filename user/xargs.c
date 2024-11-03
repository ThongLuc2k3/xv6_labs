#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: xargs command [args...]\n");
        exit(1);
    }

    char *args[MAXARG];
    for (int i = 1; i < argc; i++) {
        args[i - 1] = argv[i];  // Bỏ qua argv[0] để args[0] chứa lệnh chính
    }

    char input[500];
    int pos = argc - 1;

    while (1) {
        int n = 0;
        while (read(0, &input[n], 1) == 1) {
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
                    wait(0); // Chờ tiến trình con hoàn thành
                }
                break;
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
