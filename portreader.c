/*
 * Copyright (c) 2010 Rafael F. Zalamena <rzalamena@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <limits.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DBUG 0
#define SHWCHR 0

static void			usage(char *);
static void			dbgm(const char *);

static void
usage(char *argv)
{
		fprintf(stderr, "%s [<mem or port> [[<address - dec or hex> <range>] <value - dec or hex>]]\n", argv);
		exit(1);
}

static void
dbgm(const char *msg)
{
		if (DBUG)
			fprintf(stderr, "[D] %s\n", msg);
}

int
main(int argc, char *argv[])
{
		char		in[12], conv[5];
		unsigned short	range;
		int		portfd, it;
		long		test;
		unsigned int	addrin, value;

		ssize_t		m;
		off_t		n;

		printf("\n");

		if (argc > 5)
			usage(argv[0]);

		if (argc >= 2)
		{
			strncpy(in, argv[1], sizeof(in));
			in[sizeof(in) - 1] = '\0';
		}

		if (strcmp(in, "mem"))
		{
			portfd = open("/dev/port", O_RDWR);
			dbgm("Reading from io ports");
		}
		else
		{
			portfd = open("/dev/mem", O_RDWR);
			dbgm("Reading from memory");
		}
		
		if (portfd == -1)
			err(0, "Error opening the memory/port file");

		if (argc >= 3)
		{
			strncpy(in, argv[2], sizeof(in));
			in[sizeof(in) - 1] = '\0';
		}
		else
		{
			printf("What memory address do you want to read?\n");
			fgets(in, sizeof(in), stdin);
			in[sizeof(in) - 1] = '\0';
		}


		dbgm("addrin read:");
		dbgm(in);

		addrin = strtoul(in, NULL, 16);

		if (DBUG)
		{
			dbgm("addrin hex:");
			fprintf(stderr, "[D] 0x%08x\n", addrin);
		}

		/* seeks and set the exact address */
		n = lseek(portfd, addrin, SEEK_SET);
		if (n >= 0)
			dbgm("Seek OK");
		else
			err(0, "Seek error");

		/* clear the value to avoid trash in the output */
		memset(&value, 0x0, sizeof(value));
		m = read(portfd, &value, sizeof(value));

		if (m == -1)
			err(0, "read error");

		printf("0x%08x contains 0x%08x\n", addrin, value);

		if (SHWCHR)
		{
			memset(conv, 0x0, sizeof(conv));

			/* LITTLE-ENDIAN */
			memcpy(conv, &value, sizeof(value));
			conv[5] = conv[0];
			conv[0] = conv[3];
			conv[3] = conv[5];

			conv[5] = conv[1];
			conv[1] = conv[2];
			conv[2] = conv[5];

			/* BIG-ENDIAN
			memcpy(conv, &value, sizeof(value));
			*/

			conv[sizeof(conv) - 1] = '\0';
			printf("0x%08x contains %c%c%c%c\n", addrin, conv[0], conv[1], conv[2], conv[3]);
		}

		/* read a range of values */
		if (argc >= 4)
		{
			test = strtoul(argv[3], NULL, 16);
			if (test > USHRT_MAX || test < 0)
			{
				fprintf(stderr, "Range is too big or negative\n");
				exit(1);
			}
			range = test;				

			if (range == 0)
				printf("No range, skipping this part\n");
			else
			{
				printf("\nStart of range\n---\n");
				for (it = 1; it <= range; it = it + 1)
				{
					memset(&value, 0x0, sizeof(value));
					m = read(portfd, &value, sizeof(value));
					if (m == -1)
						err(0, "read error");

					printf("0x%08x contains 0x%08x\n", addrin + (0x04 * it), value);
					if (SHWCHR)
					{
						memset(conv, 0x0, sizeof(conv));

						/* LITTLE-ENDIAN */
						memcpy(conv, &value, sizeof(value));
						conv[5] = conv[0];
						conv[0] = conv[3];
						conv[3] = conv[5];

						conv[5] = conv[1];
						conv[1] = conv[2];
						conv[2] = conv[5];

						/* BIG-ENDIAN
						 * memcpy(conv, &value, sizeof(value));
						 */

						conv[sizeof(conv) - 1] = '\0';
						printf("0x%08x contains %c%c%c%c\n", addrin + (0x04 * it), conv[0], conv[1], conv[2], conv[3]);
					}
				}
				printf("---\nEnd of range\n\n");
			}
		}



		if (argc == 5)
		{
			strncpy(in, argv[4], sizeof(in));
			in[sizeof(in) - 1] = '\0';
		}
		else
		{
			printf("\nWhat do you want to write at 0x%08x?\n", addrin);
			fgets(in, sizeof(in), stdin);
			in[sizeof(in) - 1] = '\0';
		}

		dbgm("read value:");
		dbgm(in);

		memset(&value, 0x0, sizeof(value));
		value = strtoul(in, NULL, 16);

		if (DBUG)
		{
			dbgm("read value hex:");
			fprintf(stderr, "[D] 0x%08x\n", value);
		}

		n = lseek(portfd, addrin, SEEK_SET);
		if (n >= 0)
			dbgm("Seek OK");
		else
			err(0, "Seek error");

		/* overwrites the memory */
		printf("Writting 0x%08x at 0x%08x...\n", value, addrin);
		m = write(portfd, &value, sizeof(value));
		if (m == -1)
			err(0, "write error");
		if (m == 0)
			dbgm("nothing was written");

		n = lseek(portfd, addrin, SEEK_SET);
		if (n >= 0)
			dbgm("Seek OK");
		else
			err(0, "Seek error");

		memset(&value, 0x0, sizeof(value));
		m = read(portfd, &value, sizeof(value));

		if (m == -1)
			err(0, "reading error");

		printf("The address 0x%08x now contains the value 0x%08x.\n", addrin, value);
		printf("\n");
		exit(0);
}

