#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX 280           // Định nghĩa MAX là 280
#define FIRST_SNT 2       // Định nghĩa FIRST_SNT là 2 (2->280)

// Hàm tạo số tự nhiên từ FIRST_SNT đến MAX
int TaoSTN() 
{ 
    int tube[2]; 
    pipe(tube); // Tạo pipe (1 ghi, 0 đọc)

    if (fork() == 0) 
    { // Nếu là tiến trình con (tạo bản sao tiến trình cha)
        for (int i = FIRST_SNT; i <= MAX; i++) 
        {
            write(tube[1], &i, sizeof(int)); // Ghi số vào pipe
        }
        close(tube[1]); // Đóng đầu ghi sau khi gửi xong
        exit(0); // Kết thúc tiến trình con
    }

    close(tube[1]); // Đóng đầu ghi trong tiến trình cha
    return tube[0]; // Trả về đầu đọc của pipe
}

// Hàm lọc số nguyên tố
int LocSNT(int input, int SNT) 
{
    int temp; // biến tạm thời
    int tube[2]; // (1 ghi, 0 đọc)
    pipe(tube); // Tạo pipe mới cho mỗi lần lọc

    if (fork() == 0) 
    { // Nếu là tiến trình con
        // Đọc số từ input và ghi vào temp để so sánh với SNT
        while (read(input, &temp, sizeof(int)) > 0) 
        {
            if (temp % SNT != 0 || temp == SNT) 
            { // Kiểm tra xem số có chia hết cho SNT không
                write(tube[1], &temp, sizeof(int)); // Nếu không chia hết, ghi vào pipe
            }
        }
        close(tube[1]); // Đóng đầu ghi trong tiến trình con
        exit(0); // Kết thúc tiến trình con
    }

    close(input); // Đóng đầu đọc của pipe hiện tại trong tiến trình cha
    close(tube[1]); // Đóng đầu ghi mới trong tiến trình cha
    return tube[0]; // Trả về đầu đọc của pipe mới
}

int main()
{
    int SNT; // Khai báo biến SNT để lưu số nguyên tố
    int input = TaoSTN(); // Gọi hàm tạo dãy số từ 2 đến MAX

    // Đọc số nguyên từ pipe và lọc
    while (read(input, &SNT, sizeof(int)) > 0) 
    {
        printf("prime %d\n", SNT); // In số nguyên tố
        input = LocSNT(input, SNT); // Lọc các số chia hết cho SNT
    }
    
    close(input); // Đóng đầu đọc của pipe cuối cùng
    exit(0); // Kết thúc chương trình
}
