#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

bool starting_position(int read_fd, int write_fd)
{
    // We want to *seqientially* exchange messages
    // We need to select the first process to write the message
    // It will be the process with the biggest PID.
    pid_t current_pid = getpid(), other_pid;

    size_t written = write(write_fd, &current_pid, sizeof(pid_t));

    if (written != sizeof(pid_t))
    {
        printf("Can\'t write PID!\n");
        exit(1);
    }

    size_t read_pid_size = read(read_fd, &other_pid, sizeof(pid_t));

    if (read_pid_size != sizeof(pid_t))
    {
        printf("Can\'t read PID!\n");
        exit(1);
    }

    printf("Current PID: %d Other PID: %d\n", current_pid, other_pid);

    return current_pid > other_pid; // We are the top => we write first
}

int main(int argc, char *argv[])
{
    // Create channels
    mknod(argv[1], S_IFIFO | 0666, 0);
    mknod(argv[2], S_IFIFO | 0666, 0);

    srand(getpid()); // Ensure messages differ

    int read_fd, write_fd;

    // read_fd without O_NONBLOCK won't open untill someone opens it for write, which gives us are deadlock
    if ((read_fd = open(argv[1], O_RDONLY | O_NONBLOCK)) < 0)
    {
        printf("Can\'t open FIFO to read from!\n");
        exit(1);
    }

    if ((write_fd = open(argv[2], O_WRONLY)) < 0)
    {
        printf("Can\'t open FIFO to write to!\n");
        exit(1);
    }

    // Now we can fall back to blocked reading, otherwise nothing works as I expect
    // I think we could just open in O_RDWR, but this works too
    fcntl(read_fd, F_SETFL, fcntl(read_fd, F_GETFL) & (~O_NONBLOCK));

    bool is_writer = starting_position(read_fd, write_fd);

    while (1)
    {
        if (is_writer)
        {
            int message = rand();

            size_t written = write(write_fd, &message, sizeof(int));

            if (written != sizeof(int))
            {
                printf("Can\'t write message!\n");
                exit(1);
            }

            printf("Send message %d\n", message);
        }
        else
        {
            int message;

            size_t read_bytes = read(read_fd, &message, sizeof(int));

            printf("Read message: %d\n", message);

            if (read_bytes != sizeof(int))
            {
                printf("Can\'t read message!\n");
                exit(1);
            }
        }

        sleep(1);
        is_writer = !is_writer;
    }

    close(read_fd);
    close(write_fd);

    unlink(argv[1]);
    unlink(argv[2]);

    printf("Writer exit\n");
    return 0;
}
