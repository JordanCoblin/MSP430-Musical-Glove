#include "msp430.h"


#define     TXD                   BIT1                      // TXD on P1.1
#define     RXD                   BIT2                      // RXD on P1.2

#define     Bitime                13*4
// duration of 1 bit, measured in timer A clock cycles.
// the main clock runs at 1MHz. TimerA runs at 1MHz/8.
//For 2400 bits per second, we want 52 timer ticks.

unsigned char BitCnt;
unsigned int TXByte;
unsigned int output;

void ConfigureTimerUart(void);
void Transmit(void);

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD; // Stop watchdog

	BCSCTL1 = CALBC1_1MHZ;                    // Set range
	DCOCTL = CALDCO_1MHZ;
	BCSCTL2 &= ~(DIVS_3);                     // SMCLK = DCO = 1MHz

	ADC10CTL0 = ADC10SHT_2 + ADC10ON; // ADC10CTL0 is register. ADC10SHT_2 bit sets the ADC10’s sample and hold time at 16xADC10CLKs - 16 clock cycle sampling time, ADC10ON this bit turns the ADC10 converter on.
	P1DIR |= BIT4 ; // Set P1.4 as output (for now)
	P1DIR |= TXD;

	output = 0; // initialize output to be 0

	TA0CCTL0 = CCIE; // enable interrupts
	TA0CTL = TASSEL_2 + MC_2; // SMLCK, continuous mode

	__enable_interrupt();

	for (;;)
	{
		ADC10CTL0 &= ~ENC;

		// First Accelerometer (tri ax breakout)
		ADC10CTL1 = INCH_0; // ADC10CTL1 is register. Input A0 = Pin 1.0
		ADC10AE0 |= BIT0; // PA.0 ADC option select - enable analog input channel
		ADC10CTL0 |= ENC + ADC10SC; // Enable conversion, Sampling and conversion start
		while (ADC10CTL1 & ADC10BUSY); // ADC10BUSY? wait for ADC to complete
		if (ADC10MEM < 0x32) { // // ADC converts analog signal to 10 bit representation and stores in ADC10MEM - 0x32 represents trigger voltage
			P1OUT |= BIT4; // LED testing
			output |= 0x1;
		}
		else {
			P1OUT &= ~BIT4; // LED testing
			output &= ~0x1;
		}

		ADC10CTL0 &= ~ENC; // Must clear ENC in order to change ADC input

		// Second accelerometer
		ADC10CTL1 = INCH_6; // ADC10CTL1 is register. Input A6 = Pin 1.6
		ADC10AE0 |= BIT6; // PA.6 ADC option select - enable analog input channel
		ADC10CTL0 |= ENC + ADC10SC; // Enable conversion, Sampling and conversion start
		while (ADC10CTL1 & ADC10BUSY); // ADC10BUSY? wait for ADC to complete
		if (ADC10MEM < 0x1d3) { // 0x1d3
			output |= 0x2;
		}
		else {
			output &= ~0x2;
		}

		ADC10CTL0 &= ~ENC;

		// Third accelerometer
		ADC10CTL1 = INCH_3; // ADC10CTL1 is register. Input A3 = Pin 1.3
		ADC10AE0 |= BIT3; // PA.3 ADC option select - enable analog input channel
		ADC10CTL0 |= ENC + ADC10SC; // Enable conversion, Sampling and conversion start
		while (ADC10CTL1 & ADC10BUSY); // ADC10BUSY? wait for ADC to complete
		if (ADC10MEM < 0x1e0) { // trigger voltage
			output |= 0x4;
		}
		else {
			output &= ~0x4;
		}

		ADC10CTL0 &= ~ENC;

		// Fourth accelerometer (gyro/accelerometer)
		ADC10CTL1 = INCH_5; // ADC10CTL1 is register. Input A5 = Pin 1.5
		ADC10AE0 |= BIT5; // PA.5 ADC option select - enable analog input channel
		ADC10CTL0 |= ENC + ADC10SC; //  Enable conversion, Sampling and conversion start
		while (ADC10CTL1 & ADC10BUSY); // ADC10BUSY? wait for ADC to complete

		if (ADC10MEM > 0x340) { // trigger voltage
			output |= 0x8;
		}
		else {
			output &= ~0x8;
		}

		ADC10CTL0 &= ~ENC;


		// Second function of second accelerometer - tilting
		ADC10CTL1 = INCH_7; // ADC10CTL1 is register. Input A7 = Pin 1.7
		ADC10AE0 |= BIT7; // PA.7 ADC option select - enable analog input channel
		ADC10CTL0 |= ENC + ADC10SC; // Enable conversion, Sampling and conversion start
		while (ADC10CTL1 & ADC10BUSY); // ADC10BUSY? wait for ADC to complete
		if (ADC10MEM < 0x242) { // Change to a lower octave
			output |= 0x10;
			output &= ~0x20;
		}
		else if (ADC10MEM > 0x353){ // Change to a higher octave
			output |= 0x20;
			output &= ~0x10;
		}
		else {
			output &= ~0x10;
			output &= ~0x20;
		}


		ConfigureTimerUart();
		TXByte = (unsigned char) output;
		Transmit();

		__delay_cycles(50000); // delay to align sampling
	}
}


void ConfigureTimerUart(){
  TACCTL0 = OUT;                 // TXD Idle as Mark
  TACTL = TASSEL_2 + MC_2 + ID_3;// set SMCLK as source, divide by 8, continuous mode
  P1SEL |= (TXD+RXD);
  P1DIR |= TXD;
}

 /*using the serial port requires Transmit(),
   the TIMERA0_VECTOR, ConfigureTimerUart()
   and variables Bitcnt, TXbyte, Bitime */

// Function Transmits Character from TXByte


void Transmit()
{

  BitCnt = 0xA;           // Load Bit counter, 8 data + Start/Stop bit
  TXByte |= 0x100;        // Add mark stop bit to TXByte
  TXByte = TXByte << 1;   // Add space start bit

  TACCR0 = TAR+ Bitime;
  TACCTL0 =  CCIS0 + OUTMOD0 + CCIE;   // TXD = mark = idle, enable interrupts
  // OUTMOD0 sets output mode 1: SET which will
  // have the CCR bit (our TX bit) to go high when the timer expires
  while ( TACCTL0 & CCIE );                   // Wait for TX completion
}


// Timer A0 interrupt service routine -
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  TACCR0 += Bitime;                         // Add Offset to TACCR0
  if ( BitCnt == 0){
    P1SEL &= ~(TXD+RXD);
    TACCTL0 &= ~ CCIE ;                   // All bits TXed, disable interrupt
  }
  else{
    // in here we set up what the next bit will be: when the timer expires
    // next time.
    // In TimerConfigUart, we set OUTMOD0 for output mode 1 (set).
    // Adding OUTMOD2 gives output mode 5 (reset).

    // The advantage to doing this is that the bits get set in hardware
    // when the timer expires so the timing is as accurate as possible.
    TACCTL0 |=  OUTMOD2;                  // puts output unit in 'set' mode
    if (TXByte & 0x01)
      TACCTL0 &= ~ OUTMOD2;               // puts output unit in reset mode
    TXByte = TXByte >> 1; // shift down so the next bit is in place.
    BitCnt --;
  }

}
