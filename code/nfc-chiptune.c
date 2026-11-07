/*
 * Example for using Advanced Control Timer (TIM1) for PWM generation
 * 03-28-2023 E. Brombaugh
 */

#include "ch32v003fun.h"
#include "ch32v003_GPIO_branchless.h"
#include <stdio.h>


#define T2PERIOD    3125
// mask for the CCxP bits
// when set PWM outputs are held HIGH by default and pulled LOW
// when zero PWM outputs are held LOW by default and pulled HIGH
#define TIM2_DEFAULT 0xff
//#define TIM2_DEFAULT 0x00

#define NFC_I2C_ADDR     0x55
#define NFC_PAGE_SIZE    16
#define NFC_FIRST_PAGE   0x20
#define NFC_LAST_PAGE    0x2F
#define I2C_WRITE        0x0
#define I2C_READ         0x1

static uint16_t cycleCount = 0;
static uint16_t tuneIndex = 0;
static int16_t  tuneCount = 0;
static uint16_t timerPeriod = 0;


static const uint16_t notes[12] = {
	45867, // C0
	43293, // C#0
	40863, // D0
	38569, // Eb0
	36405, // E0
	34361, // F0
	32433, // F#0
	30613, // G0
	28894, // G#0
	27273, // A0
	25742, // A#0
	24297  // B0
};

// C=0, C#=1, D=2, Eb=3, E=4, F=5, F#=6, G=7, G#=8, A=9, Bb=A, B=B
// Nible Encoding:
// 0x[whole][1/16ths][octave][note]

uint16_t tune[128] = {
	// 0x023B, // 1/8  B3
	// 0x0344, // 3/16 E4
	// 0x013B, // 1/16 B3
	// 0x0244, // 1/8  E4
	// 0x0647, // 3/8  G4
	// 0x0644, // 3/8  E4
	// 0x0347, // 3/16 G4
	// 0x0144, // 1/16 E4
	// 0x0247, // 1/8  G4
	// 0x064B, // 3/8  B4
	// 0x0647, // 3/8  G4
	// 0x034B, // 3/16 B4
	// 0x0147, // 1/16 G4
	// 0x024B, // 1/8  B4
	// 0x0652, // 3/8  D5
	// 0x0642, // 3/8  D4
	0x0347, // 3/16 G4
	0x0142, // 1/16 D4
	0x0247, // 1/8  G4
	0x184B, // 1.5  B4
	0x10FF, // 1/4  rest
	0xF000  // loop back to index 0
};

void TIM2_IRQHandler( void ) __attribute__((interrupt));
void TIM2_IRQHandler( void ) 
{
	// Count to 15
	// Update Tune on 0
	cycleCount += 1;
	if (cycleCount > 15) {
		cycleCount = 0;
		tuneCount -= 1;
		if (tuneCount < 0) {
			tuneIndex += 1;
			tuneCount = tune[tuneIndex] >> 8;
			if ((tuneCount & 0xF0)==0xF0) {
				tuneIndex = tune[tuneIndex] & 0xFFF;
			    tuneCount = tune[tuneIndex] >> 8;
			}
			if ((tune[tuneIndex] & 0xF)==0xF) {
	            TIM1->ATRLR = 32;
	            TIM1->CH2CVR = 0x0;
			} else { 
	            timerPeriod = notes[(tune[tuneIndex]&0xF)] >> ((tune[tuneIndex]>>4)&0xF);
	            TIM1->ATRLR = timerPeriod;
	            TIM1->CH2CVR = timerPeriod >> 1;
			}
		}
	}
	// Acknowledge the interrupt
	TIM2->INTFR = 0;
}


/*
 * initialize TIM1 for PWM
 */
void t1pwm_init( void )
{
	// Enable GPIOC, GPIOD and TIM1
	RCC->APB1PCENR |= 	RCC_APB1Periph_TIM2 ;
	RCC->APB2PCENR |= 	RCC_APB2Periph_GPIOA |
						RCC_APB2Periph_TIM1;
	
	// PA1 is T1CH2, 10MHz Output alt func, push-pull
	GPIOA->CFGLR &= ~(0xf<<(4*1));
	GPIOA->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*1);
	
	// PA2 is T1CH2N, 10MHz Output alt func, push-pull
	GPIOA->CFGLR &= ~(0xf<<(4*2));
	GPIOA->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*2);

	// Reset TIM1 to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;
	
	// Reset TIM2 to init all regs
	RCC->APB1PRSTR |= RCC_APB1Periph_TIM2;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_TIM2;

	// Prescaler 
	TIM1->PSC = 0x0000;
	
	// set TIM2 clock prescaler divider 
	TIM2->PSC = 0x0000;

	// set PWM total cycle width
	TIM2->ATRLR = T2PERIOD;
	
	// Enable CH2 output, positive pol
	TIM1->CCER |= TIM_CC2E | TIM_CC2P;
	
	// Enable CH2N output
	TIM1->CCER |= TIM_CC2NE | TIM_CC2NP;
	
	// CH2 Mode is output, PWM1 (CC1S = 00, OC1M = 110)
	TIM1->CHCTLR1 |= TIM_OC2M_2 | TIM_OC2M_1;
	// for channel 1 and 2, let CCxS stay 00 (output), set OCxM to 110 (PWM I)
	// enabling preload causes the new pulse width in compare capture register only to come into effect when UG bit in SWEVGR is set (= initiate update) (auto-clears)

	// CTLR1: default is up, events generated, edge align
	// enable auto-reload of preload
	TIM2->CTLR1 |= TIM_ARPE;

	// Enable TIM1 outputs
	TIM1->BDTR |= TIM_MOE;
	
	// Enable TIM1
//	TIM1->CTLR1 |= TIM_CEN;

	// initialize counter
	TIM2->SWEVGR |= TIM_UG;

	// Enable TIM2
	TIM2->CTLR1 |= TIM_CEN;

    // Enble TIM2 Update Interrupt
	TIM2->DMAINTENR = TIM_UIE;

}

void i2c_init(void) {
    // Enable I2C1 & AFIO clocks
    RCC->APB1PCENR |= RCC_I2C1EN;
    RCC->APB2PCENR |= RCC_AFIOEN;

    // Reset I2C
    RCC->APB1PRSTR |= RCC_I2C1RST;
    RCC->APB1PRSTR &= ~RCC_I2C1RST;

//  No DMA, No INT, 8 MHz clock
	I2C1->CTLR2 = 0x03;
//  100 KHz:  8 MHz / (2* 100 KHz) = 40
	I2C1->CKCFGR = 15;
//  Enable I2C
	I2C1->CTLR1 = I2C_CTLR1_PE;

}

int i2c_read_page(uint8_t page, uint8_t *buf) {
	int byte_count = 0;
// Wait for bus free (stop completed)
	while ((I2C1->STAR2 & I2C_STAR2_BUSY));
// Send start
	I2C1->CTLR1 |= I2C_CTLR1_START;
	while (!(I2C1->STAR1 & I2C_STAR1_SB));
// Send address for write
	I2C1->DATAR = (NFC_I2C_ADDR << 1) | I2C_WRITE;
    while (!(I2C1->STAR1 & I2C_STAR1_ADDR));
    (void)I2C1->STAR2;  // Clear ADDR
// Send page to read
    I2C1->DATAR = page;
	while (!(I2C1->STAR1 & I2C_STAR1_TXE));

// Restart
	I2C1->CTLR1 |= I2C_CTLR1_START | I2C_CTLR1_ACK;
	while (!(I2C1->STAR1 & I2C_STAR1_SB));
// Send address for read
	I2C1->DATAR = (NFC_I2C_ADDR << 1) | I2C_READ;
    while (!(I2C1->STAR1 & I2C_STAR1_ADDR));
    (void)I2C1->STAR2;  // Clear ADDR
// Read bytes
    while(byte_count < NFC_PAGE_SIZE) {
		if (byte_count == NFC_PAGE_SIZE -1) {
			I2C1->CTLR1 &= ~I2C_CTLR1_ACK;
		}
		while (!(I2C1->STAR1 & I2C_STAR1_RXNE));
		buf[byte_count] = I2C1->DATAR;
		byte_count += 1;
	}
// Send stop
	I2C1->CTLR1 = I2C_CTLR1_STOP | I2C_CTLR1_PE;

    return byte_count;
}

/*
 * entry
 */
int main()
{

// Add wait for I2C signal to be high to allow for connecting debugger before changing clock speed
	RCC->APB2PCENR |= 	RCC_APB2Periph_GPIOC;

// SDA is PC1, 10MHz, input floating (ext pull-up)
	GPIOC->BSHR = (0x01<<1);
	GPIOC->CFGLR &= ~(0xf<<(4*1));
	GPIOC->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_FLOATING)<<(4*1);

// Wait for D0 to rise high before starting
	while ((GPIOC->INDR & (0x01<<1)) == 0x0) {
		__NOP;
	}
// Using slow system clock that will make debug port unusable after System Init
	SystemInit();

// SDA is PC1, 10MHz Output alt func, open-drain
	GPIOC->CFGLR &= ~(0xf<<(4*1));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF)<<(4*1);
	
// SCL is PC2, 10MHz Output alt func, open-drain
	GPIOC->CFGLR &= ~(0xf<<(4*2));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF)<<(4*2);

// Init I2C
    i2c_init();

// Read 4 pages
    uint8_t page = NFC_FIRST_PAGE;
	tuneIndex = 0;
    while (page <= NFC_LAST_PAGE) {
		i2c_read_page(page, (uint8_t *)&tune[tuneIndex]);
		tuneIndex += 8;
		page += 1;
	}

// Turn clock down to 750 KHz for playing tune
    RCC->CFGR0 = RCC_HPRE_DIV32;
// init TIM1 for PWM
//	printf("initializing tim1...");
	t1pwm_init();
//	printf("done.\n\r");

	tuneIndex = 0;
	tuneCount = tune[tuneIndex] >> 8;
	timerPeriod = notes[(tune[tuneIndex]&0xF)] >> ((tune[tuneIndex]>>4)&0xF);
	TIM1->ATRLR = timerPeriod;
	TIM1->CH2CVR = timerPeriod >> 1;
	// Reload immediately
	TIM1->SWEVGR |= TIM_UG;
	// Enable TIM1
	TIM1->CTLR1 |= TIM_CEN;

	// enable interrupt
	NVIC_EnableIRQ( TIM2_IRQn );

//	printf("looping...\n\r");
	while(1)
	{
		// __WFI(); // Wait for Interrupt
		Delay_Ms( 5 );
//		GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), low);
		Delay_Ms( 5 );
//		GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_D, 4), low);
		Delay_Ms( 5 );
//		GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_C, 4), high);
		Delay_Ms( 5 );
//		GPIO_digitalWrite(GPIOv_from_PORT_PIN(GPIO_port_D, 4), high);

	}
}
