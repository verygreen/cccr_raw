/*
 * Simple CCCR processing program mostly aimed at Tesla EAP cameras output
 * for now.
 *
 * The files resultant from this tool can be converted into tiff files with
 * ImageMagick like follows:
 * 
 * convert -depth 16 -size 1280x960 gray:processed_image.raw processed_image.tiff
 * convert -depth 16 -size 640x480 gray:processed_red.raw processed_red.tiff
 *
 * All your base belong to us, 2017
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

/* I am being lazy for now and hardcoding my values here */
#define RAW_HEIGHT 964
#define RAW_WIDTH 1280
#define SKIP_TOP_ROWS 2
#define SKIP_BOTTOM_ROWS 2
#define RAW_BITS 16

#define REAL_RAW_HEIGHT (RAW_HEIGHT - SKIP_TOP_ROWS - SKIP_BOTTOM_ROWS)
/* We also assume little-endian here */

int main(int argc, char **argv)
{
	char *source = NULL;
	char *grayscale_file = NULL;
	char *red_file = NULL;
	int fd;
	uint16_t fullimage[REAL_RAW_HEIGHT][RAW_WIDTH];
	uint16_t redimage[REAL_RAW_HEIGHT/2][RAW_WIDTH/2];
	int bytes;
	int i, j;

	if (argc > 4 || argc < 3) {
		printf("Usage: %s rawfile outfile [outredfile]\n", argv[0]);
		return 1;
	}

	source = argv[1];
	grayscale_file = argv[2];
	if (argc == 4)
		red_file = argv[3];

	fd = open(source, O_RDONLY);
	if (fd == -1) {
		perror("Cannot open source raw file");
		return -1;
	}

	/* Skip the leading garbage */
	bytes = read(fd, fullimage, SKIP_TOP_ROWS * RAW_WIDTH * RAW_BITS / 8);
	if (bytes != SKIP_TOP_ROWS * RAW_WIDTH * RAW_BITS / 8) {
		if (bytes == -1)
			perror("Cannot read raw file");
		else
			printf("Short read on the raw file, only got %d bytes\n", bytes);
		return -2;
	}

	bytes = read(fd, fullimage, REAL_RAW_HEIGHT * RAW_WIDTH * RAW_BITS / 8);
	close(fd);

	if (bytes != REAL_RAW_HEIGHT * RAW_WIDTH * RAW_BITS / 8) {
		if (bytes == -1)
			perror("Cannot read raw file");
		else
			printf("Short read on the raw file, only got %d bytes\n", bytes);
		return -2;
	}
	/* Ignoring the rest of the file including the tailing garbage */

	/* Now we have our file, so lets mangle some bytes */
	for (j = 0 ; j < REAL_RAW_HEIGHT; j += 2) {
		for (i = 1; i < RAW_WIDTH ; i += 2) {
			int average;
			int avgcount;
			if (red_file)
				redimage[j/2][i/2] = fullimage[j][i];

			average = fullimage[j][i - 1];
			avgcount = 1;
			if (j > 0) {
				average += fullimage[j - 1][i];
				average += fullimage[j - 1][i - 1];
				avgcount += 2;
			}
			if (j + 1 < REAL_RAW_HEIGHT) {
				average += fullimage[j + 1][i];
				average += fullimage[j + 1][i - 1];
				avgcount += 2;
			}
			if (i + 1 < RAW_WIDTH) {
				average += fullimage[j][i + 1];
				avgcount ++;
			}
			if (j > 0 && i + 1 < RAW_WIDTH) {
				average += fullimage[j - 1][i + 1];
				avgcount ++;
			}
			if (j + 1 < REAL_RAW_HEIGHT && i + 1 < RAW_WIDTH) {
				average += fullimage[j + 1][i + 1];
				avgcount++;
			}

			average /= avgcount;
			/* Replace the bad red pixel with average values
			 * from around it. This does lose some resolution,
			 * but I don't have any smarter ideas at the moment.
			 */ 
			fullimage[j][i] = average;
		}
	}

	/* Now write the stuff out; */
	fd = open(grayscale_file, O_WRONLY|O_CREAT|O_TRUNC, 0644);
	if (fd == -1) {
		perror("Cannot create corrected rawfile");
		return -3;
	}
	write(fd, fullimage, sizeof(fullimage));
	close(fd);
	if (red_file) {
		fd = open(red_file, O_WRONLY|O_CREAT|O_TRUNC, 0644);
		if (fd == -1) {
			perror("Cannot create extracted red rawfile");
			return -3;
		}
		write(fd, redimage, sizeof(redimage));
		close(fd);
	}

	/* All done, yay! */
	return 0;
}
