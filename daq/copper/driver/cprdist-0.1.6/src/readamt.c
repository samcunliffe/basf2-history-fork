#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>
#include "copper.h"

unsigned long buffer[1024 * 1024];

void
show_event(unsigned long * head, int len)
{
    int i;
    for (i=0; i<(len/4); i+= 8) {
        printf("%08d %08x %08x %08x %08x %08x %08x %08x %08x\n", i,
            buffer[i], buffer[i+1], buffer[i+2], buffer[i+3],
            buffer[i+4], buffer[i+5], buffer[i+6], buffer[i+7]);
    }
}

unsigned long
xor(unsigned long * start, int wordlen)
{
    unsigned long ret = 0;
    int i;
    while (wordlen--) {
        ret ^= *(start++);
    }
    return ret;
}

int cprfd;

main()
{
    int event=0;
    int ret, i = 0;
    fd_set rfds, efds;
    int amtfd[4];

    cprfd = open("/dev/copper/copper", O_RDONLY);

    if (cprfd < 0) {
        perror("open");
        exit(0);
    }

    amtfd[0] = open("/dev/copper/amt3:a", O_RDWR);
    amtfd[1] = open("/dev/copper/amt3:b", O_RDWR);
    amtfd[2] = open("/dev/copper/amt3:c", O_RDWR);
    amtfd[3] = open("/dev/copper/amt3:d", O_RDWR);

    printf("amt = %d:%d:%d:%d\n", amtfd[0], amtfd[1], amtfd[2], amtfd[3]);

    while (1) {
	int redo;
#if 0
	/* If you want to test select() */
        FD_ZERO(&rfds); FD_SET(cprfd, &rfds);
        FD_ZERO(&efds); FD_SET(cprfd, &efds);
        ret = select(1+fd, &rfds, NULL, &efds, NULL);
        printf("select returned %d\n", ret);

        if (FD_ISSET(cprfd, &rfds))
            printf("fd %d is readable\n", fd);

        if (FD_ISSET(cprfd, &efds))
            printf("fd %d is error\n", fd);
#endif

	do {
		ret = read(cprfd, (char *)buffer, sizeof(buffer));
		if (ret < 0)
			perror("read");
	} while (ret == -1 && errno == EINTR);

	if (ret == 0) { /* EOF indicates end of this run */
		break;
	}

        i++;

        if (i % 1000 == 0)
        {
            write(2, ".", 1);
        }

	{
	    char * p = (char *)buffer;
	    struct copper_header * header = (struct copper_header *)p;
	    struct copper_footer * footer =
		(struct copper_footer *)(p + ret - sizeof(struct copper_footer));

	    if (header->magic != COPPER_DRIVER_HEADER_MAGIC) {
		printf("bad fotter %x\n", header->magic);
		break;
	    }

	    if (footer->magic != COPPER_DRIVER_FOOTER_MAGIC) {
		printf("bad fotter %x\n", footer->magic);
		break;
	    }

	    if (header->event_number != event) {
		printf("bad copper evn = %x should be %x\n", buffer[1], event);
	    }
	}

        if (event < 10) {
            printf("xor = %08x\n", xor((unsigned long *)buffer, ret/4));
            show_event(buffer, ret);
        }
        event++;
    }

    if (ret > 0) {
        printf("last event\n");
        show_event(buffer, ret);
    }
}
