#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* DJGPP specific headers */
#include <dos.h>
#include <conio.h>
#include <go32.h>
#include <sys/farptr.h>

#define WIND  1
#define	BUF_W 320
#define BUF_H 75

/* Look-up table for VGA color palette */
static const uint8_t color_table[20] = {
	0x00, 0x6f, 0x6f, 0x6f, 0x29,
	0x29, 0x29, 0x2a, 0x2a, 0x2a,
	0x2b, 0x2b, 0x2b, 0x2c, 0x2c,
	0x2c, 0x0e, 0x0e, 0x0e, 0x0f
};

void 
SetFire(uint8_t *buf, int8_t intensity)
{
	memset(buf+(BUF_W*(BUF_H-1)), intensity, BUF_W);
}

void
IncFire(uint8_t *buf, int8_t inc)
{
	int8_t intensity = buf[BUF_W*BUF_H-1]+inc;

	if (intensity>0x13) intensity = 0x13;
	if (intensity<0)	intensity = 0;

	SetFire(buf, intensity);
}

void
ToggleFire(uint8_t *buf, uint8_t *on)
{
	if (*on) {
		SetFire(buf, 0);
		*on = 0;
	} else {
		SetFire(buf, 0x13);
		*on = 1;
	}
}

void
InitScene(uint8_t *buf)
{
	_farsetsel(_dos_ds); /* For accessing far-pointer */
	textmode(0x13); /* 320x200, 256-color video mode */

	memset(buf, 0, BUF_W*(BUF_H-1));
	memset(buf+(BUF_W*(BUF_H-1)), 0x13, BUF_W);
}

void
GenFire(uint8_t *buf)
{
	for (unsigned x=0; x<BUF_W; x++)
		for (unsigned y=0; y<BUF_H; y++) {
			unsigned i = x+(BUF_W*y);
			unsigned dec = buf[BUF_W+i]-(rand()%2);

			if (BUF_W+i >= BUF_W*BUF_H) continue;
			
			if (!buf[BUF_W+i]) {
				buf[i] = 0;
				continue;
			}

			if (dec<0) dec=0;
			if (WIND) buf[i-(rand()&1)] = dec;
			else	  buf[i]			= dec;

		}
}

void
WriteVideo(uint8_t *buf)
{
	for (unsigned y=0; y<BUF_H; y++)
		for (unsigned p=y*BUF_W, x=0; x<BUF_W; x++, p++)
			_farnspokeb(0xA0000+p+BUF_W*125, color_table[buf[p]]);
}

int
main(void)
{
	uint8_t fb[BUF_W*BUF_H];
	uint8_t on;
	char ch;

	srand(time(NULL));

	InitScene(fb);
	on = 1;

	for (;;) {
		if (kbhit()) {
			ch = getch();
			
			if (ch=='i') IncFire(fb, 1);
			if (ch=='k') IncFire(fb,-1);
			if (ch=='t') ToggleFire(fb, &on);
			if (ch=='q') break;
		}

		GenFire(fb);
		delay(25);
		WriteVideo(fb);
	}

	textmode(C80); /* Default DOS text-mode */

	return EXIT_SUCCESS;
}
