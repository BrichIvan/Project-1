#include "stm32f3xx.h"

uint8_t mode = 0;
 
 
 
void startToogle(void);  //
void startPWM(void);

 
void cfgRCC(void);  //YES
void SetLedOn(void);  //YES(?)
void setUpTimPin(void);  
void configureTimerForSimpleCounter(void);
void configureTimerForPWM(void);




	int main(void){
			cfgRCC();
		
		
		
			//SetLedOn();
			//startToogle();
			startPWM();
		
		while(1){
			


			}
	}
	
	void startToogle(void){
		SetLedOn();  
		configureTimerForSimpleCounter(); 
		mode=1;  
	}
	
	void startPWM(void){
		setUpTimPin();
		configureTimerForPWM();
		mode=0;
	}
	
//
	void cfgRCC(void){ //Clock and Timer Initialization
	
			RCC->CR |= RCC_CR_HSION; //HSI clock enable
			while(!RCC_CR_HSIRDY);  //Waiting for HSI clock is ready
			RCC->CFGR |=RCC_CFGR_SW_HSI; //HSI selected as system clock
		  RCC->AHBENR |= RCC_AHBENR_GPIOBEN;  //GPIOB clock enable
			RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; //TIM2 timer clock enable
	
	}
	
	void SetLedOn(void){ //Set LED on
		
		GPIOB->MODER &= ~GPIO_MODER_MODER3_Msk; //Zeroing of MODER3 register
		GPIOB->MODER |= 0x01<<GPIO_MODER_MODER3_Pos; //Set MODER3 into 01 (General purpose output mode)
		GPIOB->BSRR = GPIO_BSRR_BS_3; //Sets the corresponding ODRx bit   (why dont we set ODR bit)
	}
	void setUpTimPin(void){
		GPIOB->MODER &= ~GPIO_MODER_MODER3_Msk;
		GPIOB->MODER |= 0x02<<GPIO_MODER_MODER3_Pos; //configure PB3 as alternative function pin
		GPIOB->AFR[0] &=~GPIO_AFRL_AFRL3_Msk ;			//clear AFRL3 register. This register contains 2 levels . 1 - AFRL(0-7 pins) , 2 - AFRH(8-15 pins);
		GPIOB->AFR[0] |=0x01<<GPIO_AFRL_AFRL3_Pos ;	//our alternative function is AF1. So we are writing it in AFRL3.
	}
	void configureTimerForSimpleCounter(void){
		TIM2->PSC = 0x00FF; // configure prescaler   f CK_PSC / 256
		TIM2->ARR =0xFFFF;	// configure reload counter on 65535
		NVIC_EnableIRQ(TIM2_IRQn); // allow to interrupt TIM2 in the controller(core)
		TIM2->EGR |= TIM_EGR_UG;	//initialize event. It will create hardware UIF(update interrupt flag) when the counter resets.
		TIM2->SR = 0;  					// clear all status registers. We are interesting in clearing UIF(update interrupt flag). 
		TIM2->DIER |= TIM_DIER_UIE; //allow to interrupt TIM2 in the periphery of TIM2
		TIM2->CR1 |= TIM_CR1_CEN;		//start the counter
	}
	
	void configureTimerForPWM(void){
		TIM2->ARR =0xFFFF;
		TIM2->CCMR1 |= TIM_CCMR1_OC2M_1| TIM_CCMR1_OC2M_2; // configure chanel 2 of TIM2 as the PWM 
		TIM2->CCER |= TIM_CCER_CC2E;	//enble to capture/compare on chanel 2 of TIM2
		NVIC_EnableIRQ(TIM2_IRQn);
		TIM2->EGR |= TIM_EGR_UG;
		TIM2->DIER |= TIM_DIER_UIE;
		TIM2->SR = 0;
		TIM2->CR1 |=TIM_CR1_CEN;
		
	}
	
	uint16_t width=0;
	uint8_t CNT = 0;

void TIM2_IRQHandler(void){
		NVIC_ClearPendingIRQ(TIM2_IRQn); //clear interrupt flags in the core
		TIM2->SR = 0;										// clear interrupt flags in the TIM2
		
				if(mode){
					if(GPIOB->ODR & 1<<3) {   // if PB3 set
						GPIOB->BRR =1<<3;				// reset
					}
					else{
						GPIOB->BSRR =1<<3; 			// set
					}
				}
				else{
					
					if (width >=0xFF00){
						CNT = 1;
					}
					if (width <=0x00FF)
						CNT = 0;
					}
				  if (CNT) {
						TIM2->CCR2 = width;			// set pulse time("1"). Change from "0" to "1"
						width -=0xFF;
					}
					else {
						TIM2->CCR2 = width;			// set pulse time("1"). Change from "0" to "1"
						width +=0xFF;
					}
				
	}
