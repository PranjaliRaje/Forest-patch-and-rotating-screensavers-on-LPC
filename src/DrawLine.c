#include <cr_section_macros.h>
#include <NXP/crp.h>
#include "LPC17xx.h"
#include "ssp.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include<stdio.h>


#define LCD_D_C				23 //Just after pin 15
#define LCD_RST				24//next to 23
#define LCD_CS				16

#define PORTNUM			0

#define ST7735_TFTWIDTH  127
#define ST7735_TFTHEIGHT 159
#define ST7735_CASET   0x2A  //Column Address Set
#define ST7735_RASET   0x2B  //Row Address Set
#define ST7735_RAMWR   0x2C  //Memory Write

#define swap(x, y) { x = x + y; y = x - y; x = x - y; }

#define RED3 0xCD0000
#define GRAY72 0xB7B7B7
#define TEAL 0x388E8E
#define BLACK 0x000000
#define BLUE  0x33A1DE
#define GREEN 0x7FFF00
#define MAROON 0x800000
#define PURPLE 0x800080
#define BROWN 0xA52A2A
#define RED 0xFF4500
#define WHITE 0xFFFFFF
#define YELLOW 0xFFFF00
#define CYAN 0x00FFFF

int height = ST7735_TFTHEIGHT;
int width = ST7735_TFTWIDTH;
int cursor_x = 0, cursor_y = 0;
int rotation = 0;
int textsize  = 1;

int x_diff = 64;
int y_diff = 80;
struct coordinate_pnt
{
	int x;
	int y;
};

int cam_x = 100;
int cam_y = 100;
int cam_z = 100;

uint8_t src_addr[SSP_BUFSIZE];
uint8_t dest_addr[SSP_BUFSIZE];

int colstart = 0;
int rowstart = 0;

void lcd_delay(int ms);
void lcd_init();
void V_line(int16_t x, int16_t y0, int16_t y1, uint16_t color);
void H_line(int16_t x, int16_t y0, int16_t y1, uint16_t color);
void drawLine(float x0, float y0, float x1, float y1, uint16_t color);
void draw_pixel(int16_t x, int16_t y, uint16_t color);
void fill_rect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color);
void SPI_write(uint8_t c);
void write_cmd(uint8_t c);
void write_data(uint8_t c);
void write888(uint32_t color, uint32_t repeat);
void set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void draw_fast_h_line(int16_t x, int16_t y, int16_t w, uint32_t color);
void draw_fast_v_line(int16_t x, int16_t y,int16_t h, uint32_t color);

struct coordinate_pnt project_coordinates (int x_w, int y_w, int z_w)
{
	int scrn_x, scrn_y, Dist=100, x_diff=74, y_diff=50;
	double x_p, y_p, z_p, theta, phi, rho;
	struct coordinate_pnt screen;
	theta = acos(cam_x/sqrt(pow(cam_x,2)+pow(cam_y,2)));
	phi = acos(cam_z/sqrt(pow(cam_x,2)+pow(cam_y,2)+pow(cam_z,2)));
	//theta = 0.785;
	//phi = 0.785;
	rho= sqrt((pow(cam_x,2))+(pow(cam_y,2))+(pow(cam_z,2)));
	x_p = (y_w*cos(theta))-(x_w*sin(theta));
	y_p = (z_w*sin(phi))-(x_w*cos(theta)*cos(phi))-(y_w*cos(phi)*sin(theta));
	z_p = rho-(y_w*sin(phi)*cos(theta))-(x_w*sin(phi)*cos(theta))-(z_w*cos(phi));
            scrn_x = x_p*Dist/z_p;
	scrn_y = y_p*Dist/z_p;
	scrn_x = x_diff+scrn_x;
	scrn_y = y_diff-scrn_y;
	screen.x = scrn_x;
	screen.y = scrn_y;
	return screen;
}

void draw_coordinates ()
{
	struct coordinate_pnt lcd;
	int x1,y1,x2,y2, x3,y3,x4,y4;
	lcd = project_coordinates (0,0,0);
	x1=lcd.x;
	y1=lcd.y;
	lcd = project_coordinates (180,0,0);
	x2=lcd.x;
	y2=lcd.y;
	lcd = project_coordinates (0,180,0);
	x3=lcd.x;
	y3=lcd.y;
	lcd = project_coordinates (0,0,180);
	x4=lcd.x;
	y4=lcd.y;

	drawLine(x1,y1,x2,y2,0x00FF00FF);			//x axis  red
	drawLine(x1,y1,x3,y3,0x0000FF00);			//y axis  green
	drawLine(x1, y1, x4, y4,0x000000FF);  		//z axis  blue
}
void draw_cube(int start_pnt, int size)
{
	struct coordinate_pnt lcd;
	int x1,y1,x2,y2,x3,y3,x4,y4,x5,y5,x6,y6,x7,y7;
	cam_x = 120;
	cam_y = 120;
	cam_z = 120;

	lcd = project_coordinates(start_pnt,start_pnt,(size+start_pnt));
	x1=lcd.x;
	y1=lcd.y;
	lcd = project_coordinates((size+start_pnt),start_pnt,(size+start_pnt));
	x2=lcd.x;
	y2=lcd.y;
	lcd = project_coordinates((size+start_pnt),(size+start_pnt),(size+start_pnt));
	x3=lcd.x;
	y3=lcd.y;
	lcd = project_coordinates(start_pnt,(size+start_pnt),(size+start_pnt));
	x4=lcd.x;
	y4=lcd.y;
	lcd = project_coordinates((size+start_pnt),start_pnt,start_pnt);
	x5=lcd.x;
	y5=lcd.y;
	lcd = project_coordinates((size+start_pnt),(size+start_pnt),start_pnt);
	x6=lcd.x;
	y6=lcd.y;
	lcd = project_coordinates(start_pnt,(size+start_pnt),start_pnt);
	x7=lcd.x;
	y7=lcd.y;
	drawLine(x1, y1, x2, y2,0x00FF0000);
	drawLine(x2, y2, x3, y3,0x00FF0000);
	drawLine(x3, y3, x4, y4,0x00FF0000);
	drawLine(x4, y4, x1, y1,0x00FF0000);
	drawLine(x2, y2, x5, y5,0x00FF0000);
	drawLine(x5, y5, x6, y6,0x00FF0000);
	drawLine(x6, y6, x3, y3,0x00FF0000);
	drawLine(x6, y6, x7, y7,0x00FF0000);
	drawLine(x7, y7, x4, y4,0x00FF0000);
}
void fill_cube(int start_pnt,int size)
{
	struct coordinate_pnt s1;

	int i,j;
	size=size+start_pnt;

	for(i=0;i<size;i++)
	{
		for(j=0;j<size;j++)
		{
				s1=project_coordinates(j,i,size);	//top fill green
				draw_pixel(s1.x,s1.y,0x0064E986);

				s1=project_coordinates(i,size,j);	// right fill yellow
				draw_pixel(s1.x,s1.y,0x00FFFF00);

				s1=project_coordinates(size,j,i);	// left fill pink
				draw_pixel(s1.x,s1.y,0x00FF00FF);
		}
	}
}

void draw_square(int angle)
{
	int x0,y0,y1,x1,x2,y2,x3,y3,size,intColor=0,i=0;
	struct coordinate_pnt lcd;
	uint32_t color, colorArray [12]={0x00FF0000,0x0000FFFF,0x00FF007F,0x00FF8000,0x0000FF80,0x000000FF,0x00FFFF00,0x00330066,0x0000FF80,0x00FF00FF,0x0000FF00,0x000080FF};
	while(i<6)
	{
		i++;
		x0= 1+ rand() % (angle-10);
		y0=1+ rand() % (angle-10);
		size = 30 + rand() % (angle/4);
		if(intColor>12)
		{
			intColor =0;
		}
		color = colorArray[intColor];
		intColor++;
		x1=size+x0;
		if(x1>angle){
			x1=angle-1;
		}
		x2=x1;
		x3=x0;
		y1=y0;
		y2=size+y1;
		if(y2>angle){
			y2=angle-1;
		}
		y3=y2;

		lcd = project_coordinates (angle,x0,y0);
		x0=lcd.x;
		y0=lcd.y;
		lcd = project_coordinates (angle,x1,y1);
		x1=lcd.x;
		y1=lcd.y;
		lcd = project_coordinates (angle,x2,y2);
		x2=lcd.x;
		y2=lcd.y;
		lcd = project_coordinates (angle,x3,y3);
		x3=lcd.x;
		y3=lcd.y;

		drawLine(x0, y0, x1, y1,color);
		drawLine(x1, y1, x2, y2,color);
		drawLine(x2, y2, x3, y3,color);
		drawLine(x3, y3, x0, y0,color);

	}
}

void draw_tree(uint32_t color,int start_pnt, int size)
{
	int i=0, angle;
	struct coordinate_pnt lcd;
	int tree_branch[3][3]={{start_pnt+10,start_pnt+30,0.5*size},{start_pnt+20,start_pnt+30,0.3*size},{start_pnt+15,start_pnt+37,0.8*size}};
	while(i<3)
	{
		int x0, y0, y1, x1,xp0,xp1,yp0,yp1;
		angle = start_pnt+size;
		x0=tree_branch[i][0];
		x1=tree_branch[i][1];
		y0=tree_branch[i][2];
		y1=y0;
		i++;
		lcd = project_coordinates (y0,angle,x0);
		xp0=lcd.x;
		yp0=lcd.y;
		lcd = project_coordinates (y1,angle,x1);
		xp1=lcd.x;
		yp1=lcd.y;
		drawLine(xp0, yp0, xp1, yp1,0x00FF8000);		//level 0 straight line
		drawLine((xp0+1), (yp0+1), (xp1+1), (yp1+1),0x00FF8000);		//level 0 straight line
		drawLine((xp0-1), (yp0-1), (xp1-1), (yp1-1),0x00FF8000);		//level 0 straight line

		int it=0;
		for(it=0;it<5;it++){
			int16_t x2=(0.6*(x1-x0))+x1; 				// length of level 1 = 0.8 of previous level
			int16_t y2=y1;
			lcd = project_coordinates (y2,angle,x2);
			int xp2=lcd.x;
			int yp2=lcd.y;
			drawLine(xp1, yp1, xp2, yp2,color);		//level 1 straight line
			fill_rect(0,0,127,159,0x00000000);

			//for right rotated angle 30 degree
			int16_t xr= ((0.134*x1)+(0.866*x2)-(0.5*y2)+(0.5*y1));
			int16_t yr=((0.5*x2)-(0.5*x1)+(0.866*y2)-(0.866*y1)+y1);
			lcd = project_coordinates (yr,angle,xr);
			int xpr=lcd.x;
			int ypr=lcd.y;

			//for left rotated angle 30 degree
			int16_t xl=((0.134*x1)+(0.866*x2)+(0.5*y2)-(0.5*y1));
			int16_t yl=((0.5*x1)-(0.5*x2)+(0.134*y2)+(0.866*y1));
			lcd = project_coordinates (yl,angle,xl);
			int xpl=lcd.x;
			int ypl=lcd.y;

			drawLine(xp1, yp1, xpr, ypr,color);
			drawLine(xp1, yp1, xpl, ypl,color);
			fill_rect(0,0,127,159,0x00000000);
			//for branches on right rotated branch angle 30 degree
			int16_t xrLen = sqrt(pow((xr-x1),2)+pow((yr-y1),2)) ;	//length of right branch
			int16_t xrImag= (0.8*xrLen)+xr;					//imaginary vertical line x coordinate, y= yr
			int16_t xr1 = ((0.134*xr)+(0.866*xrImag)-(0.5*yr)+(0.5*yr));
			int16_t yr1 = ((0.5*xrImag)-(0.5*xr)+(0.866*yr)-(0.866*yr)+yr);
			lcd = project_coordinates (yr1,angle,xr1);
			int xpr1=lcd.x;
			int ypr1=lcd.y;
			//for right branch
			int16_t xrr,xrl,yrr,yrl;
			xrr = ((0.134*xr)+(0.866*xr1)-(0.5*yr1)+(0.5*yr));
			yrr = ((0.5*xr1)-(0.5*xr)+(0.866*yr1)-(0.866*yr)+yr);
			lcd = project_coordinates (yrr,angle,xrr);
			int xprr=lcd.x;
			int yprr=lcd.y;
			//for left branch
			xrl = ((0.134*xr)+(0.866*xr1)+(0.5*yr1)-(0.5*yr));
			yrl = ((0.5*xr)-(0.5*xr1)+(0.134*yr)+(0.866*yr1));
			lcd = project_coordinates (yrl,angle,xrl);
			int xprl=lcd.x;
			int yprl=lcd.y;
			//for branches on left rotated branch angle 30 degree
			int16_t xlImag= (0.8*xrLen)+xl;					//imaginary vertical line x coordinate, y= yr
			int16_t xl1 = ((0.134*xl)+(0.866*xlImag)+(0.5*yl)-(0.5*yl));
			int16_t yl1 = ((0.5*xl)-(0.5*xlImag)+(0.134*yl)+(0.866*yl));
			lcd = project_coordinates (yl1,angle,xl1);
			int xpl1=lcd.x;
			int ypl1=lcd.y;
			//for right branch
			int16_t xlr,xll,ylr,yll;
			xlr = ((0.134*xl)+(0.866*xl1)-(0.5*yl1)+(0.5*yl));
			ylr = ((0.5*xl1)-(0.5*xl)+(0.866*yl1)-(0.866*yl)+yl);
			lcd = project_coordinates (ylr,angle,xlr);
			int xplr=lcd.x;
			int yplr=lcd.y;
			//for left branch
			xll = ((0.134*xl)+(0.866*xl1)+(0.5*yl1)-(0.5*yl));
			yll = ((0.5*xl)-(0.5*xl1)+(0.134*yl)+(0.866*yl1));
			lcd = project_coordinates (yll,angle,xll);
			int xpll=lcd.x;
			int ypll=lcd.y;
			drawLine(xpr, ypr, xpr1, ypr1,color);
			drawLine(xpr, ypr, xprr, yprr,color);
			drawLine(xpr, ypr, xprl, yprl,color);
			drawLine(xpl, ypl, xpl1, ypl1,color);
			drawLine(xpl, ypl, xplr, yplr,color);
			drawLine(xpl, ypl, xpll, ypll,color);

			x0=x1;
			x1=x2;
		}
		fill_rect(0,0,127,159,0x00000000);
	}
}

void draw_shadow(double start_pnt, double size, double xShad, double yShad, double zShad)
{
	int i,j,k;
	int xs[8]={0}, ys[8]={0}, zs[8]={0};
	struct coordinate_pnt s5,s6,s7,s8;
	uint32_t color = 0x00000000;
	double x[8] = {start_pnt,(start_pnt+size),(start_pnt+size),start_pnt,start_pnt,(start_pnt+size),(start_pnt+size),start_pnt};
	double y[8] = {start_pnt, start_pnt, start_pnt+size, start_pnt+size, start_pnt, start_pnt, (start_pnt+size), (start_pnt+size) };
	double z[8] = {start_pnt, start_pnt, start_pnt, start_pnt, (start_pnt+size), (start_pnt+size), (start_pnt+size), (start_pnt+size)};

	for(i=0; i<8; i++){
		xs[i]=x[i]-((z[i]/(zShad-z[i]))*(xShad-x[i]));
		ys[i]=y[i]-((z[i]/(zShad-z[i]))*(yShad-y[i]));
		zs[i]=z[i]-((z[i]/(zShad-z[i]))*(zShad-z[i]));
	}
	s5 = project_coordinates (xs[4],ys[4],zs[4]);
	s6 = project_coordinates (xs[5],ys[5],zs[5]);
	s7 = project_coordinates (xs[6],ys[6],zs[6]);
	s8 = project_coordinates (xs[7],ys[7],zs[7]);
      for(k = 0;k < size; k = k+2){
    	for(j = 0; j < size;j = j+2){
	       drawLine((s5.x)-j, s5.y, (s8.x)-j, s8.y,color);
	       drawLine((s5.x)-(j+1), s6.y, (s8.x)-(j+1), s8.y,color);
    	}
    }
}



void draw_P(int start_pnt, int size)
{
	struct coordinate_pnt p1;
	int i,j;
	size=size+start_pnt;
		int map[size][size];
		// Draw P for Pranjali!
 for(i=0;i<size;i++)
		 {
		 	for(j=0;j<size;j++)
		 	{
		 		 if(i>=start_pnt+5 && j>=start_pnt+10 && j<=size-10 && i<=start_pnt+15) // up =
		 			map[i][j]=1;

		 		 else if(i>=start_pnt+15 && j>=size-20 && j<=size-10 && i<=start_pnt+40)//left |
		 			map[i][j]=1;
		 		 else if(i>=start_pnt+15 && j>=start_pnt+10 && j<=start_pnt+20 && i<=start_pnt+40)//right up
		 				 			map[i][j]=1;

		 		 else if(i>=start_pnt+30 &&  j>=size-20 && j<=size-10 && i<=size-5)// left down
		 				 			map[i][j]=1;
		 		 else if(i>=start_pnt+30 && j>=start_pnt+10 && j<=size-10 && i<=start_pnt+40) // mid =
		 			map[i][j]=1;
		 		 else
		 			map[i][j]=0;
		 	}
		 }
		 for(i=0;i<size;i++)
		 {
		 	for(j=0;j<size;j++)
		 	{
		 		if(map[i][j]==1)
		 		{
		 			p1 = project_coordinates(j,i,size);
		 			draw_pixel(p1.x,p1.y,0x008E35EF);
		 		}
		 		else if(map[i][j]==0)
		 		{
		 			p1 = project_coordinates(j,i,size);
		 		}
		 	}
		 }
}

int main(void)
{

	SSP0Init();
	lcd_init();
	for (int i = 0; i < SSP_BUFSIZE; i++ )
	{
		src_addr[i] = (uint8_t)i;
		dest_addr[i] = 0;
	}

	int size =70, start_pnt = 0;
	//Fill with white : no need for LCD Display Function
	fill_rect(0, 0, ST7735_TFTWIDTH, ST7735_TFTHEIGHT, WHITE);
	lcd_delay(20);
	draw_coordinates();
  	draw_cube(start_pnt,size);
  	fill_cube(start_pnt,size);
  	draw_P(start_pnt, size);
  	draw_square(size+start_pnt);


  	draw_shadow(start_pnt, size, -5000,0,5000);

	return 0;
}

void lcd_init()
{
	int i, portnum = 0;
	LPC_GPIO0->FIODIR |= (0x1 << LCD_CS);
	LPC_GPIO0->FIODIR |= (0x1 << LCD_D_C);
	LPC_GPIO0->FIODIR |= (0x1 << LCD_RST);

	LPC_GPIO0->FIOSET |= (0x1 << LCD_RST);

	lcd_delay(500); /*delay 500 ms */
	LPC_GPIO0->FIOCLR |= (0x1 << LCD_RST);
	lcd_delay(500); /* delay 500 ms */
	LPC_GPIO0->FIOSET |= (0x1 << LCD_RST);
	lcd_delay(500); /* delay 500 ms */

	for (i = 0; i < SSP_BUFSIZE; i++)
	{
		src_addr[i] = 0;
		dest_addr[i] = 0;
	}

	/* Sleep out */
	SSP_SSELToggle(portnum, 0);
	src_addr[0] = 0x11; /* Sleep out */
	SSPSend(portnum, (uint8_t *) src_addr, 1);
	SSP_SSELToggle(portnum, 1);

	lcd_delay(200);
	SSP_SSELToggle(portnum, 0);
	src_addr[0] = 0x29; /* Disp On */
	SSPSend(portnum, (uint8_t *) src_addr, 1);
	SSP_SSELToggle(portnum, 1);
	lcd_delay(200);
}
void lcd_delay(int ms)
{
	int count = 24000;
	int i;

	for (i = count * ms; i--; i > 0)
		;
}
void H_line(int16_t x0, int16_t x1, int16_t y, uint16_t color)
{

	int width;
	if (x0 < x1) {
		width = x1 - x0 + 1;
		set_addr_window(x0, y, x1, y);
	} else {
		width = x0 - x1 + 1;
		set_addr_window(x1, y, x0, y);
	}
	write_cmd(ST7735_RAMWR);
	write888(color, width);
}
void V_line(int16_t x, int16_t y0, int16_t y1, uint16_t color)
{

	int width;
	if (y0 < y1) {
		width = y1 - y0 + 1;
		set_addr_window(x, y0, x, y1);
	} else {
		width = y0 - y1 + 1;
		set_addr_window(x, y1, x, y0);
	}

	write_cmd(ST7735_RAMWR);
	write888(color, width);
}
void drawLine(float x0, float y0, float x1, float y1, uint16_t color)
{
	int16_t slope = abs(y1 - y0) > abs(x1 - x0);
	if (slope)
	{
		swap(x0, y0);
		swap(x1, y1);
	}

	if (x0 > x1)
	{
		swap(x0, x1);
		swap(y0, y1);
	}

	int16_t x_diff, y_diff;
	x_diff = x1 - x0;
	y_diff = abs(y1 - y0);

	int16_t err = x_diff / 2;
	int16_t ystep;

	if (y0 < y1)
	{
		ystep = 1;
	}
	else
	{
		ystep = -1;
	}

	for (; x0<=x1; x0++)
	{
		if (slope)
		{
			draw_pixel(y0, x0, color);
		}
		else
		{
			draw_pixel(x0, y0, color);
		}
		err -= y_diff;
		if (err < 0)
		{
			y0 += ystep;
			err += x_diff;
		}
	}

}
void draw_pixel(int16_t x, int16_t y, uint16_t color)
{
	if ((x < 0) || (x >= width) || (y < 0) || (y >= height))
		return;

	set_addr_window(x, y, x + 1, y + 1);
	write_cmd(ST7735_RAMWR);

	write888(color, 1);
}
void fill_rect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color)
{
	int16_t width, height;
	width = x1 - x0 + 1;
	height = y1 - y0 + 1;
	set_addr_window(x0, y0, x1, y1);
	write_cmd(ST7735_RAMWR);
	write888(color, width * height);
}
void SPI_write(uint8_t c)
{
	int portnum = 0;

	src_addr[0] = c;
	SSP_SSELToggle(portnum, 0);
	SSPSend(portnum, (uint8_t *) src_addr, 1);
	SSP_SSELToggle(portnum, 1);

}
void write_cmd(uint8_t c)
{
	LPC_GPIO0->FIOCLR |= (0x1 << LCD_D_C);
	SPI_write(c);
}
void write_data(uint8_t c)
{

	LPC_GPIO0->FIOSET |= (0x1 << LCD_D_C);
	SPI_write(c);
}
void write_word(uint16_t c)
{

	uint8_t d;

	d = c >> 8;
	write_data(d);
	d = c & 0xFF;
	write_data(d);
}
void write888(uint32_t color, uint32_t repeat)
{
	uint8_t red, green, blue;
	int i;
	red = (color >> 16);
	green = (color >> 8) & 0xFF;
	blue = color & 0xFF;
	for (i = 0; i < repeat; i++) {
		write_data(red);
		write_data(green);
		write_data(blue);
	}
}
void set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{

	write_cmd(ST7735_CASET);
	write_word(x0);
	write_word(x1);
	write_cmd(ST7735_RASET);
	write_word(y0);
	write_word(y1);

}
void draw_fast_h_line(int16_t x, int16_t y, int16_t w, uint32_t color)
{
    drawLine(x, y, x+w-1, y, color);
}
void draw_fast_v_line(int16_t x, int16_t y,int16_t h, uint32_t color)
{
  drawLine(x, y, x, y+h-1, color);
}
