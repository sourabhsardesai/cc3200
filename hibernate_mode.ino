/*
Hybrid code for putting cc3200 in hibernate mode

Author - Sourabh Sardesai


*/
#include <stdio.h>
#include <stdint.h>
#include <inc/hw_types.h>
#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <driverlib/prcm.h>
#include <driverlib/wdt.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <driverlib/interrupt.h>
#include <driverlib/utils.h>
#include "hwspinlock.h"

#include "spi.h"



#define INSTR_READ_STATUS       0x05
#define INSTR_DEEP_POWER_DOWN   0xB9
#define STATUS_BUSY             0x01
#define MAX_SEMAPHORE_TAKE_TRIES (4000000)

int Utils_SpiFlashDeepPowerDown(void)
{
    unsigned long ulStatus=0;

    //
    // Acquire SSPI HwSpinlock.
    //
    if (0 != MAP_HwSpinLockTryAcquire(HWSPINLOCK_SSPI, MAX_SEMAPHORE_TAKE_TRIES))
    {
        return -1;
    }

    //
    // Enable clock for SSPI module
    //
    MAP_PRCMPeripheralClkEnable(PRCM_SSPI, PRCM_RUN_MODE_CLK);

    //
    // Reset SSPI at PRCM level and wait for reset to complete
    //
    MAP_PRCMPeripheralReset(PRCM_SSPI);
    while(MAP_PRCMPeripheralStatusGet(PRCM_SSPI)== false)
    {
    }

    //
    // Reset SSPI at module level
    //
    MAP_SPIReset(SSPI_BASE);

    //
    // Configure SSPI module
    //
    MAP_SPIConfigSetExpClk(SSPI_BASE,PRCMPeripheralClockGet(PRCM_SSPI),
                     20000000,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVELOW |
                     SPI_WL_8));

    //
    // Enable SSPI module
    //
    MAP_SPIEnable(SSPI_BASE);

    //
    // Enable chip select for the spi flash.
    //
    MAP_SPICSEnable(SSPI_BASE);

    //
    // Wait for spi flash.
    //
    do
    {
        //
        // Send the status register read instruction and read back a dummy byte.
        //
        MAP_SPIDataPut(SSPI_BASE,INSTR_READ_STATUS);
        MAP_SPIDataGet(SSPI_BASE,&ulStatus);

        //
        // Write a dummy byte then read back the actual status.
        //
        MAP_SPIDataPut(SSPI_BASE,0xFF);
        MAP_SPIDataGet(SSPI_BASE,&ulStatus);
    }
    while((ulStatus & 0xFF )== STATUS_BUSY);

    //
    // Disable chip select for the spi flash.
    //
    MAP_SPICSDisable(SSPI_BASE);

    //
    // Start another CS enable sequence for Power down command.
    //
    MAP_SPICSEnable(SSPI_BASE);

    //
    // Send Deep Power Down command to spi flash
    //
    MAP_SPIDataPut(SSPI_BASE,INSTR_DEEP_POWER_DOWN);

    //
    // Disable chip select for the spi flash.
    //
    MAP_SPICSDisable(SSPI_BASE);

    //
    // Release SSPI HwSpinlock.
    //
    //MAP_HwSpinLockRelease(HWSPINLOCK_SSPI);

    //
    // Return as Pass.
    //
    return 0;
}

//****************************************************************************
//
//! Used to trigger a hibernate cycle for the SOC
//!
//! Note:This routine should only be exercised after all the network
//!      processing has been stopped. To stop network processing use sl_stop API
//! \param None
//!
//! \return None
//
//****************************************************************************
void Utils_TriggerHibCycle(void)
{
    //
    // Configure the HIB module RTC wake time
    //
    MAP_PRCMHibernateIntervalSet(330);

    //
    // Enable the HIB RTC
    //
    MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_SLOW_CLK_CTR);

    //
    // Enter HIBernate mode
    //
    MAP_PRCMHibernateEnter();
}



void EnterHIBernate()
{
#define SLOW_CLK_FREQ           (32*1024)
    //
    // Configure the HIB module RTC wake time
    //
    MAP_PRCMHibernateIntervalSet(5000* SLOW_CLK_FREQ);

    //
    // Enable the HIB RTC
    //
    MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_GPIO4 | PRCM_HIB_SLOW_CLK_CTR );
    //  MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_GPIO4 );
    Serial.println("HIB: Entering HIBernate...\n\r");
    //DBG_PRINT("HIB: Entering HIBernate...\n\r");
    delay(500);
    
    //
    // powering down SPI Flash to save power 
    //
    Utils_SpiFlashDeepPowerDown();
    //
    // Enter HIBernate mode
    //
    MAP_PRCMHibernateEnter();
}


int buttonState;
void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(9600);
 // pinMode(3, INPUT);
 Serial.println("wakeup");
}
int a=11;
void loop() 
{
  // put your main code here, to run repeatedly: 
 // Serial.println(a);
  //buttonState=digitalRead(3);
 // Serial.println(buttonState);
  //delay(500);

  EnterHIBernate();
  Serial.println("after wake up");
 // Serial.println(a);
}
