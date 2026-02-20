#define GVRAMWIDTH				512
#define GVRAMWIDTH_2				256

#define CRTC_BASE (0xE80000)
#define CRTC_R00 (CRTC_BASE)
#define CRTC_R01 (CRTC_BASE+0x2)
#define CRTC_R02 (CRTC_BASE+0x4)
#define CRTC_R03 (CRTC_BASE+0x6)
#define CRTC_R04 (CRTC_BASE+0x8)
#define CRTC_R05 (CRTC_BASE+0xA)
#define CRTC_R06 (CRTC_BASE+0xC)
#define CRTC_R07 (CRTC_BASE+0xE)
#define CRTC_R08 (CRTC_BASE+0x10)
#define CRTC_R09 (CRTC_BASE+0x12)
#define CRTC_R10 (CRTC_BASE+0x14)
#define CRTC_R11 (CRTC_BASE+0x16)
#define CRTC_R12 (CRTC_BASE+0x18)
#define CRTC_R13 (CRTC_BASE+0x1A)
#define CRTC_R14 (CRTC_BASE+0x1C)
#define CRTC_R15 (CRTC_BASE+0x1E)
#define CRTC_R16 (CRTC_BASE+0x20)
#define CRTC_R17 (CRTC_BASE+0x22)
#define CRTC_R18 (CRTC_BASE+0x24)
#define CRTC_R19 (CRTC_BASE+0x26)
#define CRTC_R20 (CRTC_BASE+0x28)
#define CRTC_R21 (CRTC_BASE+0x2A)
#define CRTC_R22 (CRTC_BASE+0x2C)
#define CRTC_R23 (CRTC_BASE+0x2E)
#define CRTC_MEM (CRTC_BASE+0x421)

short *crtc_r00 = (short*)CRTC_R00;	/* htotal*/
short *crtc_r01 = (short*)CRTC_R01;	/* hsync end*/
short *crtc_r02 = (short*)CRTC_R02;	/* hdisp start*/
short *crtc_r03 = (short*)CRTC_R03;	/* hdisp end*/

/* vertical control*/
short *crtc_r04 = (short*)CRTC_R04;	/* vtotal*/
short *crtc_r05 = (short*)CRTC_R05;	/* vsync end*/
short *crtc_r06 = (short*)CRTC_R06;	/* vdisp end*/
short *crtc_r07 = (short*)CRTC_R07;	/* vdisp start*/

/* external hsync adjust/hposition fine tune*/
short *crtc_r08 = (short*)CRTC_R08;	

/* raster numbers: raster interrupt(?)*/
short *crtc_r09 = (short*)CRTC_R09;	

/* text screen scrolling-*/
short *crtc_r10 = (short*)CRTC_R09;	/* x position*/
short *crtc_r11 = (short*)CRTC_R10;	/* y position*/

/* graphic screen scroll*/
short *crtc_r12 = (short*)CRTC_R12;	/* x0*/
short *crtc_r13 = (short*)CRTC_R13;	/* y0*/
short *crtc_r14 = (short*)CRTC_R14;	/* x1*/
short *crtc_r15 = (short*)CRTC_R15;	/* y1*/
short *crtc_r16 = (short*)CRTC_R16;	/* x2*/
short *crtc_r17 = (short*)CRTC_R17;	/* y2*/
short *crtc_r18 = (short*)CRTC_R18;	/* x3*/
short *crtc_r19 = (short*)CRTC_R19;	/* y3*/

/* graphic mode setting*/
volatile short *crtc_r20 = (short*)CRTC_R20;

/* tvram plane simultaneous access enable/tvram raster copy enable/fast clear gvram enable*/
volatile short *crtc_r21 = (short*)CRTC_R21;

/* raster copy source/destination*/
short *crtc_r22 = (short*)CRTC_R22;

/* text screen access mask pattern*/
short *crtc_r23 = (short*)CRTC_R23;

/* image capture/fast clear/raster copy control*/
volatile char *crtc_mem = (char*)CRTC_MEM;


#define VIDCON_BASE	(0xE82000)
#define GRAPHICS_PAL	(VIDCON_BASE)	/* graphics layers*/
#define TEXT_PAL	(VIDCON_BASE+0x200)	/* text, sprite, bg*/
#define VIDCON_R0	(VIDCON_BASE+0x400)	
#define VIDCON_R1	(VIDCON_BASE+0x500)	
#define VIDCON_R2	(VIDCON_BASE+0x600)	

short *vidcon_r0 = (short*)VIDCON_R0;	/* screen mode*/
short *vidcon_r1 = (short*)VIDCON_R1;	/* priority control*/
short *vidcon_r2 = (short*)VIDCON_R2;	/* on/off control / special priority*/

short *gpal = (short*)GRAPHICS_PAL;
short *tpal = (short*)TEXT_PAL;


#define GVRAM_BASE (0xC00000)
#define GVRAM0 (GVRAM_BASE)
#define GVRAM1 (GVRAM_BASE+0x80000)
#define GVRAM2 (GVRAM_BASE+0x100000)
#define GVRAM3 (GVRAM_BASE+0x180000)

short *gvram[2] = {(short*)GVRAM0, (short*)GVRAM1};


#define TVRAM_BASE	(0xE00000)
#define TVRAM_0		(TVRAM_BASE)	
#define TVRAM_1		(TVRAM_BASE+0x20000)
#define TVRAM_2		(TVRAM_BASE+0x40000)
#define TVRAM_3		(TVRAM_BASE+0x60000)

short *tvram0 = (short*)TVRAM_0;
short *tvram1 = (short*)TVRAM_1;
short *tvram2 = (short*)TVRAM_2;
short *tvram3 = (short*)TVRAM_3;