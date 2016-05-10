#define BCM2708_PERI_BASE	0x3f000000 // Peripheral Base
#define GPIO_BASE			(BCM2708_PERI_BASE + 0x200000)
 
#define INP_GPIO(g)			*(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g)			*(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a)	*(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

