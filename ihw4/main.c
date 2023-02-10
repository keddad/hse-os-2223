#include <stdio.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    char buffer[64];

    FILE *input_file = fopen(argv[1], "rb");
    FILE *output_file = fopen(argv[2], "wb");

    while (1)
    {
        int read = fread(buffer, 1, sizeof(buffer), input_file);

        if (!read)
        {
            break;
        }

        fwrite(buffer, 1, read, output_file);
    }

    struct stat orig_stat;
    stat(argv[1], &orig_stat);
    chmod(argv[2], orig_stat.st_mode);
}