#include <unistd.h>
#include <sys/io.h>
#include <stdio.h>
#include <time.h>

/* 

    gblnk - for transferring images between your gameboy camera and your PC
    Copyright (C) 2002 Chris McCormick
    Removing Allegro and Batch Mode (C) 2014 Difegue

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


*/

/* Set up our parallel port address */
#define lptbase 0x378
#define DATA	lptbase
#define STATUS	lptbase+1
#define CONTROL lptbase+2

/* how long to wait until we know the data is transferred it */
/* if your pictures are incomplete increase this val */
#define TIMEOUT 2

int main(int argc, char **argv)
	{
	/* our incoming data buffer */
	unsigned char buffer[0x1760];

	/* byte to be read, and a counter */
	int inbyte, count, x , y, col, row, c;

	/* our timeout waiters */
	int zerowait;
	int datastartwait;
	
	/* how big is the next block of picture data? */
	int readSize;
	/* the beginning of the next block to read */
	int start;

	/* map of gameboy photo colors */
	int colmap[4];

	/* test if they've put in the right number of arguments */
	if (argc < 2)
		{
		printf("Usage: %s filename1 filename 2 filename 3...\n", argv[0]);
		return -1;
		}
	
	printf("Creating our bitmap.\n");
	/*  initialize our temporary bitmap to 8 bit 
		One 160*144 int array. 
		Since GBC Pictures only use shades of gray(r=g=b), there's no need to keep 3 variables for r, g and b! 
	*/
	FILE *f;
	unsigned char *img = NULL;
	int w = 160; //A standard GameBoy Camera printout is 160*144.
	int h = 144;
	int i,j;
	int filesize = 54 + 3*w*h;  //w=160 is your image width, h=144 is image height
	int picture [160][144];

	
	/* create color map */
	printf("Creating color map.\n");
	colmap[3] = 0;
	colmap[2]= 127;
	colmap[1]= 191;
	colmap[0] = 255;
	
	//booleans used for ui things
	int ui,ui2;
	
	//IT BEGINS
	int a;
	for (a = 1; a < argc; a++) //batch mode
	{
	
	//variable resets
	ui = 0;
	ui2 = 0;
	count = 0;
	x = 0;
	y = 0;
	col=0;
	row=0;
	c=0;
	readSize=640;
	start=0;
	zerowait = 0;
	datastartwait = 0;
	img = NULL;
	memset(buffer, 0, sizeof buffer);
	memset(picture, 0, sizeof picture);
	
	printf("----------------------------------\n");
	printf("Preparing transfer to %s.\n",argv[a]);
	
	/* Get permission to write to the parallel port - possible security hazard. */
	/* if anyone knows how I should fix this Please tell me! */
	printf("Getting permission to access the parallel port.\n");
	if (ioperm(lptbase, 3, 1))
		{
		printf("Error getting parallel port permission.\n");
		printf("You must have write access to the parallel port (log in as root?)\n");
		return 1;
		}
	
	printf("Resetting Link Cable...\n\n");
	
	/* get the time right now */
	datastartwait = time(NULL);
	do 
		{
		outb(0x24, CONTROL);

		/* get the time right now */
		zerowait = time(NULL);
		while ((inb(STATUS) & 0x20) == 0)
			{ 
			/* make sure this doesn't ever take longer than 5 seconds */
			if ((time(NULL) - zerowait) > 5)
				{
				/* lose permission to parallel port */
				ioperm(lptbase, 3, 0);
				printf("Timeout while resetting the parallel port.\n");
				return 1;
				}
			}
		inbyte = inw(DATA) & 0xF;
		
		/*printf("%x\n",inbyte);*/
		/* check for a timeout */
		if ((time(NULL) - datastartwait) > 10)
			{
			/* lose permission to parallel port */
			ioperm(lptbase, 3, 0);
			printf("Timeout while resetting the parallel port.\n");
			return 1;
			}
		}
	while (inbyte != 4);
	
	outb(0x22, CONTROL);
	while ((inb(STATUS) & 0x20) != 0)
		{
		}	
	outb(0x26, CONTROL);
	
	/* Start the actual data receive cycle */
	printf("Awaiting data transfer...\n");
	printf("(Press Ctrl+C to cancel)\n");
	datastartwait = time(NULL) + 60;
	do 	
		{
		//printf("Output 0x26 to control\n");
		outb(0x26, CONTROL);
		//printf("Wait for bit 5 of status port to become 1\n");
		while ( ((inb(STATUS) & 0x20) == 0) && ((time(NULL) - datastartwait) < TIMEOUT)){};
		//printf("Read lower 4 bits of data port, store to lower 4 bits of received byte\n");
		if (ui == 0) {printf("Transferring."); ui = 1;} 
		inbyte = inw(DATA) & 0xF;
		//printf("Output 0x22 to control\n");
		outb(0x22, CONTROL);
		//printf("Wait for bit 5 of status port to become 0\n");
		while ( ((inb(STATUS) & 0x20) != 0) && ((time(NULL) - datastartwait) < TIMEOUT)){};
		//printf("Output 0x26 to control\n");
		outb(0x26, CONTROL);
		//printf("Wait for bit 5 of status port to become 1\n");
		while ( ((inb(STATUS) & 0x20) == 0) && ((time(NULL) - datastartwait) < TIMEOUT)){};
		//printf("Read lower 4 bits of data port, store to upper 4 bits of received byte\n");
		inbyte |= (inw(DATA) & 0xF) << 4;
		//printf("Output 0x22 to control\n");
		outb(0x22, CONTROL);
		//printf("Wait for bit 5 of status port to become 0\n");
		while ( ((inb(STATUS) & 0x20) != 0) && ((time(NULL) - datastartwait) < TIMEOUT)){};
		//printf("Repeat.\n");
		
		/* store our last byte */
		buffer[count++] = inbyte;

		/* progress meter */
		if (ui2 == 160) 
			{
			printf("."); 
			ui2 = 0; 
			fflush(stdout); //Real-time printing!
			} else ui2++;

		/* reset our timeout value */
		if ((time(NULL) - datastartwait) < TIMEOUT)
			datastartwait = time(NULL);
		}
	while ( (time(NULL) - datastartwait) < TIMEOUT);

	for (c = 0; c < count; c++)
		{
		/* if we encounter a header */
		if ((buffer[c] == 136) & (buffer[c+1] == 51))
			{
			/* if we get any of these headers, jump forward 10 */
			if (buffer[c+2] == 1) c=c+10;
			if (buffer[c+2] == 2) c=c+10;
			if (buffer[c+2] == 15) c=c+10;
			
			/* if this header is a length header */
			if (buffer[c+2] == 4)
				{
				/* get in a block of data */
				readSize = (buffer[c+5] * 256) + buffer[c+4];
				/* skip forward 6 */
				c=c+6;
				/* set the start point for this chunk of data */
				start = c;
				}
	/*			printf("\n\n"); */
			}
		else
			{
	/*			printf("%x ", buffer[c]); */
			
			/* go two bytes at a time down the page for 16 byte chunks. */
			/* go across the page until we get 320 'tiles' of two by eight */
			/* go down the page */
			if (((c-start) % 320) == 0)
				{
				row++;
				col=0;
				y=0;
				}
			else if (((c-start) % 16) == 0)
				{
				col++;
				y=0;
				}
			else if (((c-start) % 2) == 0)
				{
				y++;
				}
			x=(c-start)%2;

			/* draw in blocks of two (ignore second byte */
			if (x==0)
				{
				/* decode the tiles */
				picture[(x + col*2)*4 + 0][ y + row*8] = colmap[((buffer[c+1] & 0x80) >> 6) + ((buffer[c] & 0x80) >> 7)];
				picture[(x + col*2)*4 + 1][ y + row*8] = colmap[((buffer[c+1] & 0x40) >> 5) + ((buffer[c] & 0x40) >> 6)];
				picture[(x + col*2)*4 + 2][ y + row*8] = colmap[((buffer[c+1] & 0x20) >> 4) + ((buffer[c] & 0x20) >> 5)];
				picture[(x + col*2)*4 + 3][ y + row*8] = colmap[((buffer[c+1] & 0x10) >> 3) + ((buffer[c] & 0x10) >> 4)];

				picture[(x + col*2)*4 + 4][ y + row*8] = colmap[((buffer[c+1] & 0x08) >> 2) + ((buffer[c] & 0x08) >> 3)];
				picture[(x + col*2)*4 + 5][ y + row*8] = colmap[((buffer[c+1] & 0x04) >> 1) + ((buffer[c] & 0x04) >> 2)];
				picture[(x + col*2)*4 + 6][ y + row*8] = colmap[((buffer[c+1] & 0x02) >> 0) + ((buffer[c] & 0x02) >> 1)];
				picture[(x + col*2)*4 + 7][ y + row*8] = colmap[((buffer[c+1] & 0x01) << 1) + ((buffer[c] & 0x01) >> 0)];
				}
			}
		}
	/* write this bitmap to file */
	
	//if( img )
		//free( img );
	img = (unsigned char *)malloc(3*w*h);
	memset(img,0,sizeof(img));

	for(i = 0; i<w; i++)
	{
		for(j = 0; j<h; j++)
	{
		img[(i+j*w)*3+2] = (unsigned char)(picture[i][j]);
		img[(i+j*w)*3+1] = (unsigned char)(picture[i][j]);
		img[(i+j*w)*3+0] = (unsigned char)(picture[i][j]);
	}
	}
	
	/* I have no idea why I have to redefine the BMP header every time, but ok.*/
		
	unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
	//BM = windows bitmap, 0000 = filesize, 0000 = reserved, 54 0 0 0 = offset
	
	unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
	unsigned char bmppad[3] = {0,0,0};

	//fill in filesize
	bmpfileheader[ 2] = (unsigned char)(filesize    );
	bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
	bmpfileheader[ 4] = (unsigned char)(filesize>>16);
	bmpfileheader[ 5] = (unsigned char)(filesize>>24);

	//fill in width and height
	bmpinfoheader[ 4] = (unsigned char)(       w    );
	bmpinfoheader[ 5] = (unsigned char)(       w>> 8);
	bmpinfoheader[ 6] = (unsigned char)(       w>>16);
	bmpinfoheader[ 7] = (unsigned char)(       w>>24);
	bmpinfoheader[ 8] = (unsigned char)(       h    );
	bmpinfoheader[ 9] = (unsigned char)(       h>> 8);
	bmpinfoheader[10] = (unsigned char)(       h>>16);
	bmpinfoheader[11] = (unsigned char)(       h>>24);
	
	
	f = fopen(argv[a],"wb");
	fwrite(bmpfileheader,1,14,f);
	fwrite(bmpinfoheader,1,40,f);

	for(i = 0; i<h; i++)
	{
		fwrite(img+(w*(h-i-1)*3),3,w,f); //write row number i
		fwrite(bmppad,1,(4-(w*3)%4)%4,f); //pad row size to a multiple of 4 bytes
	}
	fclose(f);
	
	/*
	// bonus: PGM export
	
	//FILE *f;
	//char *pgmheader = "P5\n160 144\n255";
	
	f = fopen(argv[args],"w");
	fwrite("P2\n160 144\n255",1,sizeof("P5\n160 144\n255"),f);
	//int i,j;
	//int w = 160;
	//int h = 144;
	
	for(j = 0; j<h; j++)
	{
		fwrite("\n",1,1,f);
		
		for(i = 0; i<w; i++)
		{
		// writing line 
		fprintf(f, "%d", picture[i][j]);
		fwrite(" ",1,1,f);
		}
	}
	fclose(f);*/
	
	
	printf("\nImage transferred successfully!\n");
	
	/* lose permission to parallel port */
	ioperm(lptbase, 3, 0);
	}
	printf("\n All images printed!\n");
	return 0;
	}

END_OF_MAIN();
