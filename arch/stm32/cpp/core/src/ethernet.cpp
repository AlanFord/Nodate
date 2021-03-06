/*
	ethernet.cpp - Implementation of the Ethernet class.
	
	Revision 0
	
	Features:
			- 
			
	Notes:
			- 
			
	2021/02/17, Maya Posch
*/


#include <ethernet.h>


#ifdef NODATE_ETHERNET_ENABLED

#include <nodate.h>

// Preprocessor definitions for this implementation.
#include <mpu_def.h>

#include <cstring>

Timer timer;


// TODO: ETH interrupt for RX.
#ifdef __stm32f7
extern "C" {
	void ETH_IRQHandler();
	void ETH_WKUP_IRQHandler();
}


__weak void ETH_TxCpltCallback() {
	// To be overridden by custom function.
}


__weak void ETH_RxCompleteCallback() {
	// To be overridden by custom function.
}


__weak void ETH_ErrorCallback() {
	// To be overridden by custom function.
}


void ETH_IRQHandler() {
	if ((ETH->DMASR & ETH_DMASR_RS) == ETH_DMASR_RS) {
		// Frame received.
		// Receive complete callback.
		ETH_RxCompleteCallback();
    
		// Clear the Eth DMA Rx IT pending bits.
		ETH->DMASR = ETH_DMASR_RS;
		
		// TODO: Set device state to Ready.
	}
	else if ((ETH->DMASR & ETH_DMASR_TS) == ETH_DMASR_TS) {
		// Frame transmitted
		// Transfer complete callback.
		ETH_TxCpltCallback();
    
		// Clear the Eth DMA Tx IT pending bits.
		ETH->DMASR = ETH_DMASR_TS;

		// TODO: Set device state to Ready.
	}
  
	// Clear the interrupt flags.
	ETH->DMASR = ETH_DMASR_NIS;
  
	// ETH DMA Error.
	if ((ETH->DMASR & ETH_DMASR_AIS) == ETH_DMASR_AIS) {
		// Ethernet Error callback.
		ETH_ErrorCallback();

		// Clear the interrupt flags.
		ETH->DMASR = ETH_DMASR_AIS;
  
		// TODO: Set device state to Ready.
	}
}


void ETH_WKUP_IRQHandler() {
	// TODO: Implement along with wake-up feature.
}
#endif


// DMA Descriptors & TX/RX buffers.
struct ETH_DMADescTypeDef {
  __IO uint32_t   Status;           // DES 0 - Status
  uint32_t   ControlBufferSize;     // DES 1 - Control and Buffer1, Buffer2 lengths
  uint32_t   Buffer1Addr;           // DES 2 - Buffer1 address pointer
  uint32_t   Buffer2NextDescAddr;   // DES 3 - Buffer2 or next descriptor address pointer
  
  // Enhanced Ethernet DMA PTP Descriptors
  uint32_t   ExtendedStatus;        // DES 4 - Extended status for PTP receive descriptor
  uint32_t   Reserved1;             // DES 5 - Reserved
  uint32_t   TimeStampLow;          // DES 6 - Time Stamp Low value for transmit and receive
  uint32_t   TimeStampHigh;         // DES 7 - Time Stamp High value for transmit and receive
};


#if defined __stm32f7
/* ETH_DMADescTypeDef*		DMARxDscrTab 	= (ETH_DMADescTypeDef*) SRAM2_BASE;
ETH_DMADescTypeDef* 	DMATxDscrTab 	= (ETH_DMADescTypeDef*) SRAM2_BASE + 0x80;
uint8_t* 				Rx_Buff 		= (uint8_t*) SRAM2_BASE + 0x100;
uint8_t* 				Tx_Buff 		= (uint8_t*) SRAM2_BASE + 0x17D0; */
extern "C" {
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RXBUFNB] __attribute__((section(".RxDescSection")));/* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TXBUFNB] __attribute__((section(".TxDescSection")));/* Ethernet Tx DMA Descriptors */
static uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __attribute__((used)) __attribute__((section(".RxArraySection")));
static uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE] __attribute__((used)) __attribute__((section(".TxArraySection")));
}
#endif

// -D__stm32f7=1
__attribute__((used)) void Ethernet::dmaRxDescListInit(bool interruptMode) {
#if defined __stm32f7
	// Set up new RX DMA descriptors and buffer.
	//ETH_DMADescTypeDef descDef[ETH_RXBUFNB];
	//uint8_t rxBuff[ETH_RXBUFNB][ETH_RX_BUF_SIZE];
	
	// Fill the RX DMA descriptors.
	for (uint32_t i = 0; i < ETH_RXBUFNB; ++i) {
		// Set up new descriptor.
		//ETH_DMADescTypeDef& dd = descDef[i];
		ETH_DMADescTypeDef& dd = DMARxDscrTab[i];
		dd.Status = ETH_DMARXDESC_OWN;
		dd.ControlBufferSize = ETH_DMARXDESC_RCH | ETH_RX_BUF_SIZE;
		//dd.Buffer1Addr = (uint32_t) (&rxBuff[i * ETH_RX_BUF_SIZE]);
		dd.Buffer1Addr = (uint32_t) &((&Rx_Buff[0][0])[i * ETH_RX_BUF_SIZE]);
		if (interruptMode) {
			dd.ControlBufferSize &= ~ETH_DMARXDESC_DIC;
		}
		
		// Initialize the next descriptor with the Next Descriptor Polling Enable.
		if ((i + 1) < ETH_RXBUFNB) {
			// Set next descriptor address register with next descriptor base address.
			//dd.Buffer2NextDescAddr = (uint32_t) &(descDef[i + 1]);
			dd.Buffer2NextDescAddr = (uint32_t) &(DMARxDscrTab[i + 1]);
		}
		else {
			// Set next descriptor address register to beginning of descriptor list.
			//dd.Buffer2NextDescAddr = (uint32_t) &(descDef[0]);
			dd.Buffer2NextDescAddr = (uint32_t) &(DMARxDscrTab[0]);
		}
	}
	
	//memcpy(DMARxDscrTab, &descDef, sizeof(descDef));
	//memcpy(Rx_Buff, &rxBuff, sizeof(rxBuff));
	
	// Set Receive Descriptor List Address Register.
	ETH->DMARDLAR = (uint32_t) DMARxDscrTab;
#endif
}

__attribute__((used)) void Ethernet::dmaTxDescListInit(bool interruptMode, bool hardwareChecksum) {
#if defined __stm32f7
	// TODO: Set the ETH peripheral state to BUSY.
	
	// Set up new RX DMA descriptors and buffer.
	//ETH_DMADescTypeDef descDef[ETH_TXBUFNB];
	//uint8_t txBuff[ETH_TXBUFNB][ETH_TX_BUF_SIZE];
	
	// Fill the RX DMA descriptors.
	for (uint32_t i = 0; i < ETH_TXBUFNB; ++i) {
		// Set up new descriptor.
		ETH_DMADescTypeDef& dd = DMATxDscrTab[i];
		dd.Status = ETH_DMATXDESC_TCH;
		//dd.Buffer1Addr = (uint32_t) (&txBuff[i * ETH_TX_BUF_SIZE]);
		dd.Buffer1Addr = (uint32_t) (&Tx_Buff[i * ETH_TX_BUF_SIZE]);
		if (hardwareChecksum) {
			dd.Status |= ETH_DMATXDESC_CIC_TCPUDPICMP_FULL;
		}
		
		if ((i + 1) < ETH_TXBUFNB) {
			//dd.Buffer2NextDescAddr = (uint32_t) &(descDef[i + 1]);
			dd.Buffer2NextDescAddr = (uint32_t) &(DMATxDscrTab[i + 1]);
		}
		else {
			//dd.Buffer2NextDescAddr = (uint32_t) &(descDef[0]);
			dd.Buffer2NextDescAddr = (uint32_t) &(DMATxDscrTab[0]);
		}
	}
	
	//memcpy(&DMATxDscrTab, &descDef, sizeof(descDef));
	//memcpy(&Tx_Buff, &txBuff, sizeof(txBuff));

	// Set Transmit Descriptor List Address Register.
	ETH->DMATDLAR = (uint32_t) &DMATxDscrTab;

	// TODO: Set ETH state to Ready.
#endif
}


// --- SETUP MPU ---
// Configure the memory protection for the TX/RX DMA descriptors and buffers.
bool Ethernet::setupMPU() {
#if defined __stm32f7
	// 1. Disable MPU.
	// Make sure outstanding transfers are done.
	__DMB();

	// Disable fault exceptions.
	SCB->SHCSR &= ~SCB_SHCSR_MEMFAULTENA_Msk;
  
	// Disable the MPU and clear the control register.
	MPU->CTRL = 0;
	
	// 2. Configure the MPU.
	// Set the Region number.
	MPU->RNR = MPU_REGION_NUMBER0;
	MPU->RBAR = SRAM2_BASE;
    MPU->RASR = ((uint32_t) MPU_INSTRUCTION_ACCESS_ENABLE	<< MPU_RASR_XN_Pos)   |
                ((uint32_t) MPU_REGION_FULL_ACCESS			<< MPU_RASR_AP_Pos)   |
                ((uint32_t) MPU_TEX_LEVEL1					<< MPU_RASR_TEX_Pos)  |
                ((uint32_t) MPU_ACCESS_SHAREABLE           	<< MPU_RASR_S_Pos)    |
                ((uint32_t) MPU_ACCESS_NOT_CACHEABLE		<< MPU_RASR_C_Pos)    |
                ((uint32_t) MPU_ACCESS_NOT_BUFFERABLE		<< MPU_RASR_B_Pos)    |
                ((uint32_t) 0x00							<< MPU_RASR_SRD_Pos)  |
                ((uint32_t) MPU_REGION_SIZE_16KB			<< MPU_RASR_SIZE_Pos) |
                ((uint32_t) MPU_REGION_ENABLE				<< MPU_RASR_ENABLE_Pos);
				
	MPU->RNR = MPU_REGION_NUMBER1;
	MPU->RBAR = SRAM2_BASE;
    MPU->RASR = ((uint32_t) MPU_INSTRUCTION_ACCESS_ENABLE	<< MPU_RASR_XN_Pos)   |
                ((uint32_t) MPU_REGION_FULL_ACCESS			<< MPU_RASR_AP_Pos)   |
                ((uint32_t) MPU_TEX_LEVEL0					<< MPU_RASR_TEX_Pos)  |
                ((uint32_t) MPU_ACCESS_SHAREABLE           	<< MPU_RASR_S_Pos)    |
                ((uint32_t) MPU_ACCESS_NOT_CACHEABLE		<< MPU_RASR_C_Pos)    |
                ((uint32_t) MPU_ACCESS_BUFFERABLE			<< MPU_RASR_B_Pos)    |
                ((uint32_t) 0x00							<< MPU_RASR_SRD_Pos)  |
                ((uint32_t) MPU_REGION_SIZE_256B			<< MPU_RASR_SIZE_Pos) |
                ((uint32_t) MPU_REGION_ENABLE				<< MPU_RASR_ENABLE_Pos);
	
	// 3. Enable the MPU.
	MPU->CTRL |= MPU_CTRL_ENABLE_Msk;
  
	// Enable fault exceptions.
	SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
  
	// Ensure MPU setting take effects.
	__DSB();
	__ISB();
						
	return true;
#else
	return false;
#endif
}


// --- WRITE PHY REGISTER ---
bool Ethernet::writePhyRegister(uint16_t reg, uint32_t value) {
#if defined __stm32f7
	// TODO: check for busy status on ETH.
	
	// Get the ETHERNET MACMIIAR value's CR value.
	uint32_t tmpreg = 0;
	tmpreg = ETH->MACMIIAR & ((uint32_t) 0x7 << ETH_MACMIIAR_CR_Pos); // Mask CR bits.
	
	// Prepare the MII address register value.
	tmpreg |= (((uint32_t) PHY_ADDRESS << 11) & ETH_MACMIIAR_PA);	// Set the PHY device address
	tmpreg |= (((uint32_t) reg << 6 ) & ETH_MACMIIAR_MR);			// Set the PHY register address
	tmpreg |= ETH_MACMIIAR_MW;										// Set the write mode
	tmpreg |= ETH_MACMIIAR_MB;										// Set the MII Busy bit
  
	// Write the value into the MII Data Register.
	ETH->MACMIIDR = (uint16_t) value;
  
	// Write the new value into the MII Address register.
	ETH->MACMIIAR = tmpreg;
	
	// Check for the Busy flag.
	while((tmpreg & ETH_MACMIIAR_MB) == ETH_MACMIIAR_MB) { }
	
	// TODO: set state to ready.
						
	return true;
#else
	return false;
#endif
}


// --- READ PHY REGISTER ---
bool Ethernet::readPhyRegister(uint16_t reg, uint32_t &value) {
#if defined __stm32f7
	// TODO: check for busy status on ETH.
	
	// Get the ETHERNET MACMIIAR value's CR value.
	uint32_t tmpreg = 0;
	tmpreg = ETH->MACMIIAR & ((uint32_t) 0x7 << ETH_MACMIIAR_CR_Pos); // Mask CR bits.
	
	// Prepare the MII address register value.
	tmpreg |= (((uint32_t) PHY_ADDRESS << 11) & ETH_MACMIIAR_PA);	// Set the PHY device address
	tmpreg |= (((uint32_t) reg << 6 ) & ETH_MACMIIAR_MR);			// Set the PHY register address
	tmpreg &= ~ETH_MACMIIAR_MW;										// Set the read mode
	tmpreg |= ETH_MACMIIAR_MB;										// Set the MII Busy bit
  
	// Write the new value into the MII Address register.
	ETH->MACMIIAR = tmpreg;
	
	// Check for the Busy flag.
	while((tmpreg & ETH_MACMIIAR_MB) == ETH_MACMIIAR_MB) { }
	
	// TODO: set state to ready.
						
	return true;
#else
	return false;
#endif
}


bool Ethernet::macDmaConfig(uint32_t speed, uint32_t duplexMode, bool hardwareChecksum, bool interruptMode,
																		uint8_t macAddress[6]) {
#if defined __stm32f7
	// TODO: check for busy status on ETH.
	
	// >> MAC CR <<
	uint32_t tmpreg = ETH->MACCR;
	
	// Clear WD, PCE, PS, TE and RE.
	tmpreg &= ~(ETH_MACCR_WD | ETH_MACCR_IPCO | ETH_MACCR_APCS | ETH_MACCR_TE | ETH_MACCR_RE);
	
	uint32_t checksumOffload = ETH_CHECKSUMOFFLOAD_ENABLE;
	if (!hardwareChecksum) { checksumOffload = ETH_CHECKSUMOFFLOAD_DISABLE; }
	
	// Set the WD bit according to ETH Watchdog value.
	tmpreg |= (uint32_t) (ETH_WATCHDOG_ENABLE | 
						   ETH_JABBER_ENABLE | 
						   ETH_INTERFRAMEGAP_96BIT |
						   ETH_CARRIERSENSE_ENABLE |
						   speed | 
						   ETH_RECEIVEOWN_ENABLE |
						   ETH_LOOPBACKMODE_DISABLE |
						   duplexMode | 
						   checksumOffload |    
						   ETH_RETRYTRANSMISSION_DISABLE | 
						   ETH_AUTOMATICPADCRCSTRIP_DISABLE | 
						   ETH_BACKOFFLIMIT_10 | 
						   ETH_DEFFERRALCHECK_DISABLE);
						   
	ETH->MACCR = (uint32_t) tmpreg;
	
	// Wait until the write operation will be taken into account.
    // This takes at least four TX_CLK/RX_CLK clock cycles.
	tmpreg = ETH->MACCR;
	timer.delay(ETH_REG_WRITE_DELAY);
	ETH->MACCR = tmpreg; 
	
	// >> MAC FFR <<
	ETH->MACFFR = (uint32_t)(ETH_RECEIVEAll_DISABLE | 
								ETH_SOURCEADDRFILTER_DISABLE |
								ETH_PASSCONTROLFRAMES_BLOCKALL |
								ETH_BROADCASTFRAMESRECEPTION_ENABLE | 
								ETH_DESTINATIONADDRFILTER_NORMAL |
								ETH_PROMISCUOUS_MODE_DISABLE |
								ETH_MULTICASTFRAMESFILTER_PERFECT |
								ETH_UNICASTFRAMESFILTER_PERFECT);
								
	// Wait until write operation will be registered.
	tmpreg = ETH->MACFFR;
	timer.delay(ETH_REG_WRITE_DELAY);
	ETH->MACFFR = tmpreg;
	
	// >> MAC HTHR & MAC HTLR <<
	ETH->MACHTHR = (uint32_t) 0x0;
	ETH->MACHTLR = (uint32_t) 0x0;
	
	// >> MAC FCR <<
	tmpreg = 0;
	uint32_t pauseTime = 0x0;
	tmpreg |= (uint32_t) ((pauseTime << 16) | 
                        ETH_ZEROQUANTAPAUSE_DISABLE |
                        ETH_PAUSELOWTHRESHOLD_MINUS4 |
                        ETH_UNICASTPAUSEFRAMEDETECT_DISABLE | 
                        ETH_RECEIVEFLOWCONTROL_DISABLE |
                        ETH_TRANSMITFLOWCONTROL_DISABLE); 
						
	ETH->MACFCR = (uint32_t) tmpreg;
	
	// Wait for write operation being noticed: four TX/RX clock cycles.
	tmpreg = ETH->MACFCR;
	timer.delay(ETH_REG_WRITE_DELAY);
	ETH->MACFCR = tmpreg;
	
	// >> MAC VLANTR <<
	uint32_t VLANTagIdentifier = 0x0;
	ETH->MACVLANTR = (uint32_t)(ETH_VLANTAGCOMPARISON_16BIT | 
                                    VLANTagIdentifier);
									
	tmpreg = ETH->MACVLANTR;
    timer.delay(ETH_REG_WRITE_DELAY);
    ETH->MACVLANTR = tmpreg;
	
	// >> DMA INITIALISATION <<
	// Configure the DMA with default settings.
	tmpreg = 0;
	tmpreg |= (uint32_t)(ETH_DROPTCPIPCHECKSUMERRORFRAME_ENABLE | 
                         ETH_RECEIVESTOREFORWARD_ENABLE |
                         ETH_FLUSHRECEIVEDFRAME_ENABLE |
                         ETH_TRANSMITSTOREFORWARD_ENABLE | 
                         ETH_TRANSMITTHRESHOLDCONTROL_64BYTES |
                         ETH_FORWARDERRORFRAMES_DISABLE |
                         ETH_FORWARDUNDERSIZEDGOODFRAMES_DISABLE |
                         ETH_RECEIVEDTHRESHOLDCONTROL_64BYTES |
                         ETH_SECONDFRAMEOPERARTE_ENABLE);
						 
	ETH->DMAOMR = (uint32_t) tmpreg;
	
	tmpreg = ETH->DMAOMR;
    timer.delay(ETH_REG_WRITE_DELAY);
    ETH->DMAOMR = tmpreg;
	
	// >> DMA BMR <<
	uint32_t DescriptorSkipLength = 0x0;
	ETH->DMABMR = (uint32_t) (ETH_ADDRESSALIGNEDBEATS_ENABLE | 
							  ETH_FIXEDBURST_ENABLE |
							  ETH_RXDMABURSTLENGTH_32BEAT |    /* !! if 4xPBL is selected for Tx or Rx it is applied for the other */
							  ETH_TXDMABURSTLENGTH_32BEAT |
							  ETH_DMAENHANCEDDESCRIPTOR_ENABLE |
							  (DescriptorSkipLength << 2) |
							  ETH_DMAARBITRATION_ROUNDROBIN_RXTX_1_1 |
							  ETH_DMABMR_USP); /* Enable use of separate PBL for Rx and Tx */
							  
	// Wait until the write operation will be taken into account:
    // at least four TX_CLK/RX_CLK clock cycles.
    tmpreg = ETH->DMABMR;
    timer.delay(ETH_REG_WRITE_DELAY);
    ETH->DMABMR = tmpreg;
	
	if (interruptMode) {
		// Enable RX interrupt.
		ETH->DMAIER |= ETH_DMAIER_NISE | ETH_DMAIER_RIE;
	}
	
	// Initialize MAC address in ethernet MAC
	// High register contains the last two bytes, low register the first (LSB) bytes.
	ETH->MACA0LR |= ((uint32_t) macAddress[5] << 8) | (uint32_t) macAddress[4];
	ETH->MACA0HR |= ((uint32_t) macAddress[3] << 24) | 
						((uint32_t) macAddress[2] << 16) | 
						((uint32_t) macAddress[1] << 8) | 
						macAddress[0];
						
	// Set up DMA descriptors.
	dmaRxDescListInit(interruptMode);
	dmaTxDescListInit(interruptMode, hardwareChecksum);
	
	// Set up the memory protection.
	if (!setupMPU()) { return false; }
						
	return true;
#else
	return false;
#endif
}


// --- START ETHERNET ---
bool Ethernet::startEthernet(Ethernet_MII &ethDef) {
	// TODO:
	
	return true;
}


// --- START ETHERNET ---
bool Ethernet::startEthernet(Ethernet_RMII &ethDef) {
	// Set up the pins.
#if defined __stm32f7
	// NOTE: targeting the Nucleo-F746ZG development board here specifically.

	// Port A: REF_CLK, MDIO, CRS_DV.
	Rcc::enablePort(RCC_PORT_A);
	GPIO::set_af(ethDef.REF_CLK.port, ethDef.REF_CLK.pin, ethDef.REF_CLK.af);
	GPIO::set_output_parameters(ethDef.REF_CLK.port, ethDef.REF_CLK.pin, GPIO_FLOATING,
								GPIO_PUSH_PULL, GPIO_HIGH);
	GPIO::set_af(ethDef.MDIO.port, ethDef.MDIO.pin, ethDef.MDIO.af);
	GPIO::set_output_parameters(ethDef.MDIO.port, ethDef.MDIO.pin, GPIO_FLOATING,
								GPIO_PUSH_PULL, GPIO_HIGH);
	GPIO::set_af(ethDef.CRS_DV.port, ethDef.CRS_DV.pin, ethDef.CRS_DV.af);
	GPIO::set_output_parameters(ethDef.CRS_DV.port, ethDef.CRS_DV.pin, GPIO_FLOATING,
								GPIO_PUSH_PULL, GPIO_HIGH);
								
	// Port B: TXD1.
	Rcc::enablePort(RCC_PORT_B);
	GPIO::set_af(ethDef.TXD1.port, ethDef.TXD1.pin, ethDef.TXD1.af);
	GPIO::set_output_parameters(ethDef.TXD1.port, ethDef.TXD1.pin, GPIO_FLOATING,
								GPIO_PUSH_PULL, GPIO_HIGH);
	
	// Port C: MDC, RXD0, RXD1.
	Rcc::enablePort(RCC_PORT_C);
	GPIO::set_af(ethDef.MDC.port, ethDef.MDC.pin, ethDef.MDC.af);
	GPIO::set_output_parameters(ethDef.MDC.port, ethDef.MDC.pin, GPIO_FLOATING,
								GPIO_PUSH_PULL, GPIO_HIGH);
	GPIO::set_af(ethDef.RXD0.port, ethDef.RXD0.pin, ethDef.RXD0.af);
	GPIO::set_output_parameters(ethDef.RXD0.port, ethDef.RXD0.pin, GPIO_FLOATING,
								GPIO_PUSH_PULL, GPIO_HIGH);
	GPIO::set_af(ethDef.RXD1.port, ethDef.RXD1.pin, ethDef.RXD1.af);
	GPIO::set_output_parameters(ethDef.RXD1.port, ethDef.RXD1.pin, GPIO_FLOATING,
								GPIO_PUSH_PULL, GPIO_HIGH);
								
	// Port G: RX_ER, TX_EN, TXD0.
	Rcc::enablePort(RCC_PORT_G);
	GPIO::set_af(ethDef.RX_ER.port, ethDef.RX_ER.pin, ethDef.RX_ER.af);
	GPIO::set_output_parameters(ethDef.RX_ER.port, ethDef.RX_ER.pin, GPIO_FLOATING,
								GPIO_PUSH_PULL, GPIO_HIGH);
	GPIO::set_af(ethDef.TX_EN.port, ethDef.TX_EN.pin, ethDef.TX_EN.af);
	GPIO::set_output_parameters(ethDef.TX_EN.port, ethDef.TX_EN.pin, GPIO_FLOATING,
								GPIO_PUSH_PULL, GPIO_HIGH);
	GPIO::set_af(ethDef.TXD0.port, ethDef.TXD0.pin, ethDef.TXD0.af);
	GPIO::set_output_parameters(ethDef.TXD0.port, ethDef.TXD0.pin, GPIO_FLOATING,
								GPIO_PUSH_PULL, GPIO_HIGH);
	
	// Enable Ethernet global interrupt.
	uint32_t prioritygroup = NVIC_GetPriorityGrouping();
	NVIC_SetPriority(ETH_IRQn, NVIC_EncodePriority(prioritygroup, 0x7, 0));
	NVIC_EnableIRQ(ETH_IRQn);

	// Enable ETH clock.
	if (!Rcc::enable(RCC_ETH)) {
		return false;
	}
	
	// Configure the ETH peripheral.
	// * Set RMII interface in SYSCFG.
	// * Reset ETH DMA.
	// * Initialise MAC.
	// * Initialise PHY.
	// * Configure DMA.
	if (!Rcc::enable(RCC_SYSCFGCOMP)) {
		return false;
	}
	
	// Set RMII interface for ETH.
	SYSCFG->PMC &= ~(SYSCFG_PMC_MII_RMII_SEL);
	SYSCFG->PMC |= SYSCFG_PMC_MII_RMII_SEL;
	
	// Reset DMA.
	ETH->DMABMR |= ETH_DMABMR_SR;
	while ((ETH->DMABMR & ETH_DMABMR_SR) != (uint32_t) RESET) { }
	
	// >> Initialise MAC. <<
	// Get the ETHERNET MACMIIAR value's CR value.
	uint32_t tempreg = 0;
	tempreg = ETH->MACMIIAR & ((uint32_t) 0x7 << ETH_MACMIIAR_CR_Pos); // Mask CR bits.
	// Clear CSR Clock Range CR[2:0] bits
	//tempreg &= ~ETH_MACMIIAR_CR;
	
	uint32_t &hclk = SystemCoreClock;
	// Set CR bits depending on hclk value
	if ((hclk >= 20000000) && (hclk < 35000000)) {
		tempreg |= (uint32_t) ETH_MACMIIAR_CR_Div16;	// CSR Clock Range between 20-35 MHz
	}
	else if((hclk >= 35000000) && (hclk < 60000000)) {
		tempreg |= (uint32_t) ETH_MACMIIAR_CR_Div26;	// CSR Clock Range between 35-60 MHz
	}  
	else if((hclk >= 60000000) && (hclk < 100000000)) {
		tempreg |= (uint32_t) ETH_MACMIIAR_CR_Div42;	// CSR Clock Range between 60-100 MHz
	}  
	else if((hclk >= 100000000) && (hclk < 150000000)) {
		tempreg |= (uint32_t) ETH_MACMIIAR_CR_Div62;	// CSR Clock Range between 100-150 MHz
	}
	else { /* ((hclk >= 150000000) && (hclk <= 216000000)) */
		tempreg |= (uint32_t) ETH_MACMIIAR_CR_Div102;    // CSR Clock Range between 150-216 MHz
	}
  
	// Write to ETHERNET MAC MIIAR: Configure the ETHERNET CSR Clock Range
	ETH->MACMIIAR = (uint32_t) tempreg;
	
	// >> Initialise PHY <<
	// Reset PHY. Send command to the PHY.
	if (!writePhyRegister(PHY_BCR, PHY_RESET)) {
		return false;
	}
	
	// Wait for PHY_RESET_DELAY to ensure the PHY is ready.
	timer.delay(PHY_RESET_DELAY);
	
	// If auto-negotiation is enabled, wait for linked state. 
	uint32_t phyreg = 0;
	if (ethDef.autonegotiate) {
		do {
			readPhyRegister(PHY_BSR, phyreg);
		}
		while (((phyreg & PHY_LINKED_STATUS) != PHY_LINKED_STATUS));
		
		if (!writePhyRegister(PHY_BCR, PHY_AUTONEGOTIATION)) {
			return false;
		}
		
		// Wait for auto-negotiation to be completed.
		do {
			readPhyRegister(PHY_BSR, phyreg);
		}
		while (((phyreg & PHY_AUTONEGO_COMPLETE) != PHY_AUTONEGO_COMPLETE));
		
		// Read auto-negotiation result.
		if (!readPhyRegister(PHY_SR, phyreg)) {
			return false;
		}
		
		// Set the negotiated duplex mode.
		if ((phyreg & PHY_DUPLEX_STATUS) != (uint32_t) RESET) {
			ethDef.duplexMode = ETH_MODE_FULLDUPLEX;
		}
		else {
			ethDef.duplexMode = ETH_MODE_HALFDUPLEX;
		}
		
		// Set the speed negotiated.
		uint32_t speed = 0;
		if ((phyreg & PHY_SPEED_STATUS) == PHY_SPEED_STATUS) {
			ethDef.speed = ETH_SPEED_10M;
		}
		else {
			ethDef.speed = ETH_SPEED_100M;
		}
	}
	else {
		// Auto-negotiation is disabled. Set parameters manually.
		if (!writePhyRegister(PHY_BCR, ((uint16_t) (ethDef.duplexMode >> 3) | 
										(uint16_t) (ethDef.speed >> 1)))) {
			return false;
		}
		
		// Delay by PHY_CONFIG_DELAY.
		timer.delay(PHY_CONFIG_DELAY);
	}
	
	// Configure the DMA for the MAC.
	macDmaConfig(ethDef.speed, ethDef.duplexMode, ethDef.hardwareChecksum, 
					ethDef.interruptMode, ethDef.macAddress);
				
	
	// Enable transmit state machine of the MAC for transmission on the MII.
	ETH->MACCR |= ETH_MACCR_TE;
	  
	__IO uint32_t tmpreg = 0;
	tmpreg = ETH->MACCR;
	timer.delay(ETH_REG_WRITE_DELAY);
	ETH->MACCR = tmpreg;
  
	// Enable receive state machine of the MAC for reception from the MII.
	ETH->MACCR |= ETH_MACCR_RE;

	tmpreg = 0;
	tmpreg = ETH->MACCR;
	timer.delay(ETH_REG_WRITE_DELAY);
	ETH->MACCR = tmpreg;
  
	// Flush Transmit FIFO.
	ETH->DMAOMR |= ETH_DMAOMR_FTF;
	
	tmpreg = 0;
	tmpreg = ETH->DMAOMR;
	timer.delay(ETH_REG_WRITE_DELAY);
	ETH->DMAOMR = tmpreg;
  
	// Start DMA transmission.
	ETH->DMAOMR |= ETH_DMAOMR_ST;
  
	// Start DMA reception.
	ETH->DMAOMR |= ETH_DMAOMR_SR;
	
	return true;
#else
	
	return false;
#endif
}



/* Ethernet DMA Rx descriptors Frame length Shift */
#define  ETH_DMARXDESC_FRAMELENGTHSHIFT            ((uint32_t)16)

// --- RECEIVE DATA ---	
bool Ethernet::receiveData(uint8_t* buffer, uint32_t &length) {
#if defined __stm32f7
	// Scan descriptors owned by host.
	// We wish to 
	uint32_t descriptorscancounter = 0;
	uint32_t segcount = 0;
	ETH_DMADescTypeDef* indexDesc = DMARxDscrTab;
	ETH_DMADescTypeDef* startDesc = 0;
	ETH_DMADescTypeDef* lastDesc = 0;
	while (((indexDesc->Status & ETH_DMARXDESC_OWN) == (uint32_t) RESET) && 
										(descriptorscancounter < ETH_RXBUFNB)) {
		// For security.
		descriptorscancounter++;
    
		// Check if first segment in frame.
		if ((indexDesc->Status & (ETH_DMARXDESC_FS | ETH_DMARXDESC_LS)) == (uint32_t) ETH_DMARXDESC_FS) {
			startDesc = indexDesc;
			segcount = 1;   
			
			// Point to next descriptor 
			indexDesc = (ETH_DMADescTypeDef*) indexDesc->Buffer2NextDescAddr;
		}
		else if ((indexDesc->Status & (ETH_DMARXDESC_LS | ETH_DMARXDESC_FS)) == (uint32_t) RESET) {
			// Intermediate segment found.
			segcount++;
			
			// Point to next descriptor.
			indexDesc = (ETH_DMADescTypeDef*) indexDesc->Buffer2NextDescAddr;
		}
		else {
			// Last segment.
			lastDesc = indexDesc;
		  
			// Increment segment count.
			segcount++;
		  
			// Check if last segment is first segment: in this case one segment contains the frame.
			if (segcount == 1) {
				startDesc = indexDesc;
			}
		  
			// Get the Frame Length of the received packet: subtract 4 bytes of the CRC.
			length = ((indexDesc->Status & ETH_DMARXDESC_FL) >> ETH_DMARXDESC_FRAMELENGTHSHIFT) - 4;
		  
			// Allocate buffer if nullptr, else ensure a buffer of size length bytes exists.
			// De-allocating of buffer is done by caller.
			if (buffer == 0) {
				buffer = (uint8_t*) calloc(length, 1);
				if (buffer == 0) {
					// TODO: report allocation failed.
					return false;
				}
			}
			else {
				// Use realloc to ensure we got sufficient space.
				buffer = (uint8_t*) realloc(buffer, length);
				if (buffer == 0) {
					// TODO: report allocation failed.
					return false;
				}
			}
		  
			// Point to next descriptor.
			indexDesc = (ETH_DMADescTypeDef*) indexDesc->Buffer2NextDescAddr;
		}
	}
  
	// Copy the buffer data from each descriptor into the buffer.
	// We copy starting wih the buffer data in the first descriptor and end with the last descriptor.
	uint32_t offset = 0;
	uint32_t bytesLeft = length;
	for (indexDesc = startDesc; indexDesc != lastDesc; 
			indexDesc = (ETH_DMADescTypeDef*) indexDesc->Buffer2NextDescAddr) {
		memcpy(buffer + offset, ((uint8_t*) indexDesc->Buffer1Addr), ETH_RX_BUF_SIZE);
		// TODO: check res value.
		
		offset += ETH_RX_BUF_SIZE;
		bytesLeft -= ETH_RX_BUF_SIZE;
	}
	
	// For the last descriptor, copy the remaining data.
	memcpy(buffer + offset, ((uint8_t*) indexDesc->Buffer1Addr), bytesLeft);
	
	// Release descriptors to DMA 
	// Point to first descriptor.
	indexDesc = startDesc;
	 
	// Set Own bit in Rx descriptors: gives the buffers back to DMA 
	for (uint32_t i = 0; i < segcount; i++) {  
		indexDesc->Status |= ETH_DMARXDESC_OWN;
		indexDesc = (ETH_DMADescTypeDef*) indexDesc->Buffer2NextDescAddr;
	 }
		
	 // Clear segment count.
	 segcount = 0;
	  
	// When Rx Buffer unavailable flag is set: clear it and resume reception.
	if ((ETH->DMASR & ETH_DMASR_RBUS) != (uint32_t) RESET) {
		// Clear RBUS ETHERNET DMA flag 
		ETH->DMASR = ETH_DMASR_RBUS;
		
		// Resume DMA reception
		ETH->DMARPDR = 0;
	}
	
	return true;
#else
	
	return false;
#endif
}


bool Ethernet::sendData(uint8_t* buffer, uint32_t len) {
#if defined __stm32f7
	// Copy the data from the buffer into available DMA descriptors.
	ETH_DMADescTypeDef* indexDesc = DMATxDscrTab;
	
	uint32_t bytesLeft = len;
	uint32_t offset = 0;
	bool fs = true;
	while (len > 0) {
		// Check that the Tx descriptor is available (OWN not set) for write, otherwise return
		// false.
		if ((indexDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t) RESET) {
			// TODO: report exact error.
			return false;
		}
		
		// Copy the buffer data into the Tx descriptor's buffer.
		uint32_t bytesToCopy = (bytesLeft > ETH_TX_BUF_SIZE)? ETH_TX_BUF_SIZE : bytesLeft;
		memcpy(((ETH_DMADescTypeDef*) indexDesc->Buffer1Addr), buffer + offset, bytesToCopy);
		offset += bytesToCopy;
		bytesLeft -= bytesToCopy;
		
		// Set descriptors.
		// Clear FIRST and LAST segment bits.
		indexDesc->Status &= ~(ETH_DMATXDESC_FS | ETH_DMATXDESC_LS);
		
		if (fs) {
			// Set the first segment bit.
			indexDesc->Status |= ETH_DMATXDESC_FS;
			fs = false;
		}
		
		if (bytesLeft == 0) {
			// Set the last segment bit.
			indexDesc->Status |= ETH_DMATXDESC_LS;
			indexDesc->ControlBufferSize = (bytesToCopy & ETH_DMATXDESC_TBS1);
		}
		else {
			// Buffer size.
			indexDesc->ControlBufferSize = (ETH_TX_BUF_SIZE & ETH_DMATXDESC_TBS1);
		}
		
		// Set OWN bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA.
		indexDesc->Status |= ETH_DMATXDESC_OWN;
	}
  
	// When Tx Buffer unavailable flag is set: clear it and resume transmission
	if ((ETH->DMASR & ETH_DMASR_TBUS) != (uint32_t) RESET) {
		// Clear TBUS ETHERNET DMA flag.
		ETH->DMASR = ETH_DMASR_TBUS;
		
		// Resume DMA transmission.
		ETH->DMATPDR = 0;
	}
	
	// When Transmit Underflow flag is set, clear it and issue a Transmit Poll Demand to resume transmission 
	if ((ETH->DMASR & ETH_DMASR_TUS) != (uint32_t) RESET) {
		// Clear TUS ETHERNET DMA flag
		ETH->DMASR = ETH_DMASR_TUS;
    
		// Resume DMA transmission
		ETH->DMATPDR = 0;
	}
  
	return true;
#else
	
	return false;
#endif
}

#endif
