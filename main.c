#include "stm32f3xx.h"
#include "math.h"

#define pi 3.14159265

uint8_t mode = 1;
 
 
 
void cfgRCC(void); 
void startPWM(void);

void SetLedOn(void);
void setUpTimPin(void);
void configureTimerForPWM(void);
void extiInitialization(void);
void TIM7Config(void);




	int main(void){
			cfgRCC();
			extiInitialization();
		  TIM7Config();
		
			//SetLedOn();
			startPWM();
		
		while(1){


			}
	}

	
	void startPWM(void){
		setUpTimPin();
		configureTimerForPWM();
	}
	
//
	void cfgRCC(void){ //Clock and Timer Initialization
	
			RCC->CR |= RCC_CR_HSION; //HSI clock enable
			while(!RCC_CR_HSIRDY);  //Waiting for HSI clock is ready
			RCC->CFGR |=RCC_CFGR_SW_HSI; //HSI selected as system clock
		  RCC->AHBENR |= RCC_AHBENR_GPIOBEN;  //GPIOB clock enable
		  GPIOB->MODER &= ~GPIO_MODER_MODER3_Msk; //Zeroing of MODER3 register
		  GPIOB->MODER |= 0x01<<GPIO_MODER_MODER3_Pos; //Set MODER3 into 01 (General purpose output mode)
			RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; //TIM2 timer clock enable
		  RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;  //TIM7 timer clock enable
	}
	
  void extiInitialization (void){	
			RCC->AHBENR |= RCC_AHBENR_GPIOAEN;  //GPIOA clock enable
			GPIOA->MODER &= ~GPIO_MODER_MODER3;  //GPIO Mode
			GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR3_Msk;  //Zeroing PUPDR3 register
			GPIOA->PUPDR |= 0x02<<GPIO_PUPDR_PUPDR3_Pos;  //Pull down   IS it right ???
		  
		
			//RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;  // Checked during previous step
	    //SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR1_EXTI3;  // Checked during previous step
	    SYSCFG->EXTICR[3] = SYSCFG_EXTICR1_EXTI3_PA;  // Connect External Line to the GPIO

	    EXTI->IMR |= EXTI_IMR_MR3;  // Interrupt Mask Register (enabled)
	    EXTI->RTSR |= EXTI_RTSR_TR3;  //Rising edge
		  
			NVIC_EnableIRQ(EXTI3_IRQn);  // Enable interrupt
			NVIC_SetPriority(EXTI3_IRQn, 0);  // Set priority for that interrupt
	}
	
	void SetLedOn(void){ //Set LED ON
		GPIOB->MODER &= ~GPIO_MODER_MODER3_Msk; //Zeroing of MODER3 register
		GPIOB->MODER |= 0x01<<GPIO_MODER_MODER3_Pos; //Set MODER3 into 01 (General purpose output mode)
		GPIOB->BSRR = GPIO_BSRR_BS_3; //Sets the corresponding ODRx bit   (why dont we set ODR bit)
	}
	
	void setUpTimPin(void){
		GPIOB->MODER &= ~GPIO_MODER_MODER3_Msk;  // Zeroing MODER3 register
		GPIOB->MODER |= 0x02<<GPIO_MODER_MODER3_Pos; //configure PB3 as alternative function pin
		GPIOB->AFR[0] &=~GPIO_AFRL_AFRL3_Msk ;			//clear AFRL3 register. This register contains 2 levels . 1 - AFRL(0-7 pins) , 2 - AFRH(8-15 pins);
		GPIOB->AFR[0] |=0x01<<GPIO_AFRL_AFRL3_Pos ;	//our alternative function is AF1. So we are writing it in AFRL3.
	}

	void configureTimerForPWM(void){
		
		TIM2->ARR =0xFFFF;  // Set auto reload value
		TIM2->CCMR1 |= TIM_CCMR1_OC2M_1| TIM_CCMR1_OC2M_2; // configure chanel 2 of TIM2 as the PWM 
		TIM2->CCER |= TIM_CCER_CC2E;	//enble to capture/compare on channel 2 of TIM2
		NVIC_EnableIRQ(TIM2_IRQn);
		TIM2->EGR |= TIM_EGR_UG;
		TIM2->DIER |= TIM_DIER_UIE;  // let the enterrupt be
		TIM2->SR = 0;  //Reset status register
		TIM2->CR1 |= TIM_CR1_CEN;  // Set ON TIM2
	}
	
	uint16_t width=0;

void TIM7Config (void){
	TIM7->PSC = 8000 - 1;  // frequency set
	TIM7->ARR = 5000 - 1;  // auto-reload set
	NVIC_EnableIRQ(TIM7_IRQn);
	TIM7->CR1 |= TIM_CR1_URS;  //
	TIM7->EGR |= TIM_EGR_UG;  //
	TIM7->DIER |= TIM_DIER_UIE;  // let the enterrupt be
	TIM7->CR1 |= TIM_CR1_OPM; // One-pulse mode
	TIM7->SR = 0;
	TIM7->CR1 &= ~TIM_CR1_CEN;  //start counter
	
}	
	
void TIM2_IRQHandler(void){
	
		NVIC_ClearPendingIRQ(TIM2_IRQn); //clear interrupt flags in the core
		TIM2->SR = 0;										// clear interrupt flags in the TIM2
	
		TIM2->CCR2 = rint(32767.5*(sin(2*pi/65535*width-pi/2)+1));			// set pulse time("1"). Change from "0" to "1"
		width +=0xFF;
}

void TIM7_IRQHandler(void){
	
	TIM7->SR = 0;  //Clear status pin
	
	if (mode) {
		mode = 0;
		TIM2->CR1 &= ~TIM_CR1_CEN;  // Set off TIM 2
		GPIOB->MODER &= ~GPIO_MODER_MODER3_Msk; //Zeroing of MODER3 register
		GPIOB->MODER |= 0x01<<GPIO_MODER_MODER3_Pos; //Set MODER3 into 01 (General purpose output mode)
		GPIOB-> ODR = GPIO_ODR_3;  // Set led pin
		TIM7->ARR = 3000 - 1;
		TIM7->CR1 |= TIM_CR1_CEN;  // Set TIM7 count
	}
	else {
		mode = 1;
		GPIOB->ODR &= ~GPIO_ODR_3;	 // Reset
		GPIOB->MODER &= ~GPIO_MODER_MODER3_Msk;  // Zeroing MODER3 register
		GPIOB->MODER |= 0x02<<GPIO_MODER_MODER3_Pos; //configure PB3 as alternative function pin
		GPIOB->AFR[0] &=~GPIO_AFRL_AFRL3_Msk ;			//clear AFRL3 register. This register contains 2 levels . 1 - AFRL(0-7 pins) , 2 - AFRH(8-15 pins);
		GPIOB->AFR[0] |=0x01<<GPIO_AFRL_AFRL3_Pos ;	//our alternative function is AF1. So we are writing it in AFRL3.
		
		TIM2->CR1 |= TIM_CR1_CEN;  //  Set on TIM 2
		TIM7->ARR = 5000 - 1;
		
	}
	
}

void EXTI3_IRQHandler (void){
		if (EXTI->PR & EXTI_PR_PR3) {
			TIM7->CR1 |= TIM_CR1_CEN;  //start TIM7 counter
			//TIM7->SR = 0;
			EXTI->PR |= EXTI_PR_PR3;  //Cleared flag
		}
}	
