#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char* format_name(char *path) {
	char *p;
	for (p = path + strlen(path); p >= path && *p != '/'; p--);
	return p + 1;
}

int open_directory(char *path) {
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		fprintf(2, "find: cannot open [%s]\n", path);
	}
	return fd;
}

// Read directory entries and handle errors
int read_directory(int fd, struct dirent *de) {
	return read(fd, de, sizeof(*de)) == sizeof(*de);
}

// Check if the entry is the current or parent directory
int is_current_or_parent_dir(const char *name) {
	return !strcmp(name, ".") || !strcmp(name, "..");
}

// Recursively find the target name in the directory
void find(char *path, char *target_name) {
	char buf[512], *p;
	struct dirent de; // thông tin từng mục (tên, số inode)
	struct stat st; // thông tin trạng thái (loại, kích thước,...)
	int fd = open_directory(path);
	
	if (fd < 0) return;
	if (fstat(fd, &st) < 0) { // lấy thông tin trạng thái 
		fprintf(2, "find: cannot stat %s\n", path);
		close(fd);
		return;
	}

	if (st.type == T_FILE && !strcmp(format_name(path), target_name)) { // nếu st.type là loại tệp và lấy tên tệp trong đường dẫn so sánh với tên tệp truyền vào 
		printf("%s\n", path);
		close(fd);
		return;
	} 
	
	if (st.type != T_DIR) { // nếu không là thư mục thì thoát
		close(fd);
		return;
	}
	
	if (strlen(path) + DIRSIZ + 2 > sizeof(buf)) { // nếu path + tên thư mục vượt quá bộ đệm buf
		fprintf(2, "find: path too long\n");
		close(fd);
		return;
	}
	
	strcpy(buf, path); 
	p = buf + strlen(buf);// sao chép đường dẫn và di chuyển đến cuối đường dẫn 
	*p++ = '/'; // thêm '/' để cbi nối tên các mục con 

	while (read_directory(fd, &de)) { //đọc từng mục trong thư mục 
		if (de.inum == 0 || is_current_or_parent_dir(de.name)) continue; // bỏ qua các thư mục không hợp lệ 
		
		memmove(p, de.name, DIRSIZ); // sao chép tên mục vào vị trí tiếp theo trong buf
		p[DIRSIZ] = 0;
		
		find(buf, target_name); // gọi đệ quy để tiếp tục tìm kiếm 
	}
	close(fd);
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		fprintf(2, "usage: find path filename\n");
		exit(1);
	}

	find(argv[1], argv[2]);
	exit(0);
}