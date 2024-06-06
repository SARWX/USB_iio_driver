#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define LENGTH 5

int main()
{
    int fd = open ("./test.txt", O_RDONLY);
    if (fd == -1)
    {
        /* ошибка */
        printf("ERROR IN OPEN FUNCTION");
        return(-1);
    }
    else 
    {
        printf("fd = %d\n", fd);
        printf("reading:\n");
        char *buf;

        /* считываем пару байт в 'word' из 'fd' */
        int nr = read (fd, buf, (sizeof (char)) * LENGTH);
            if (nr == -1)
            {
                /* ошибка */
                printf("ERROR IN READ FUNCTION");
                return(-1);
            }
            else
            {
                printf("nr = %d\n", nr);    
                printf("read string = %s\n", buf);    
            }

    }   
}