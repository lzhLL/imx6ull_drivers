#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    int fd = 0;
    const char *file_name = argv[1];
    char read_buf[128] = {0} , write_buf[128] = {0};
    //int open(const char *pathname, int flags, mode_t mode);
    fd = open(file_name, O_RDWR);
    if (fd < 0)
    {
        printf("can't open %s file\r\n", file_name);
        return -1;
    }
    
    int ret = read(fd, read_buf, sizeof(read_buf));
    if (ret < 0)
    {
        printf("read %s file failed!\r\n", file_name);
        return -1;
    }
    
    ret = write(fd, write_buf, sizeof(write_buf));
    if (ret < 0)
    {
        printf("write file %s failed!\r\n", file_name);
    }
    
    close(fd);

    return 0;
}
