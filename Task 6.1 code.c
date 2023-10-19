#include "DSP2833x_Device.h"

// external function prototypes
extern void InitSysCtrl(void);
extern void InitPieCtrl(void);
extern void InitPieVectTable(void);
extern void InitCpuTimers(void);
extern void ConfigCpuTimer(struct CPUTIMER_VARS *, float, float);

// Prototype statements for functions found within this file.
void Gpio_select(void);

// Prototype for Interrupt Functions ...
interrupt void cpu_timer0_isr(void);


void main(void)
 {
    int counter=0;  // binary counter for digital output

    InitSysCtrl();  // Basic Core Init from DSP2833x_SysCtrl.c

    EALLOW;
    SysCtrlRegs.WDCR= 0x00AF;   // Re-enable the watchdog
    EDIS;           // 0x00AF  to NOT disable the Watchdog, Prescaler = 64

    DINT;               // Disable all interrupts
    Gpio_select();      // GPIO9, GPIO11, GPIO34 and GPIO49 as output
                        // to 4 LEDs at Peripheral Explorer)
    InitPieCtrl();      // basic setup of PIE table; from DSP2833x_PieCtrl.c
    InitPieVectTable(); // default ISR's in PIE

    EALLOW;
    PieVectTable.TINT0 = &cpu_timer0_isr;
    EDIS;


    InitCpuTimers();    // basic setup CPU Timer0, 1 and 2–

    ConfigCpuTimer(&CpuTimer0,150,100000); // CPU - Timer0 at 100 milliseconds

    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    IER |=1;

    EINT;
    ERTM;

    CpuTimer0Regs.TCR.bit.TSS = 0;  // start timer0

    while(1)
    {
            while(CpuTimer0.InterruptCount == 0);
            CpuTimer0.InterruptCount = 0;

            EALLOW;
            SysCtrlRegs.WDKEY = 0x55;   // service WD #1
            EDIS;

            counter++;
            if(counter&1) GpioDataRegs.GPASET.bit.GPIO9 = 1;
                else GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;
            if(counter&2) GpioDataRegs.GPASET.bit.GPIO11 = 1;
                else GpioDataRegs.GPACLEAR.bit.GPIO11 = 1;
            if(counter&4) GpioDataRegs.GPBSET.bit.GPIO34 = 1;
                else GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;
            if(counter&8) GpioDataRegs.GPBSET.bit.GPIO49 = 1;
                else GpioDataRegs.GPBCLEAR.bit.GPIO49 = 1;
    }
}

void Gpio_select(void)
{
    EALLOW;
    GpioCtrlRegs.GPAMUX1.all = 0;        // GPIO15 ... GPIO0 = General Puropse I/O
    GpioCtrlRegs.GPAMUX2.all = 0;        // GPIO31 ... GPIO16 = General Purpose I/O
    GpioCtrlRegs.GPBMUX1.all = 0;        // GPIO47 ... GPIO32 = General Purpose I/O
    GpioCtrlRegs.GPBMUX2.all = 0;        // GPIO63 ... GPIO48 = General Purpose I/O
    GpioCtrlRegs.GPCMUX1.all = 0;        // GPIO79 ... GPIO64 = General Purpose I/O
    GpioCtrlRegs.GPCMUX2.all = 0;        // GPIO87 ... GPIO80 = General Purpose I/O


    GpioCtrlRegs.GPADIR.all = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO9 = 1;    // peripheral explorer: LED LD1 at GPIO9
    GpioCtrlRegs.GPADIR.bit.GPIO11 = 1;    // peripheral explorer: LED LD2 at GPIO11

    GpioCtrlRegs.GPBDIR.all = 0;        // GPIO63-32 as inputs
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;    // peripheral explorer: LED LD3 at GPIO34
    GpioCtrlRegs.GPBDIR.bit.GPIO49 = 1; // peripheral explorer: LED LD4 at GPIO49
    GpioCtrlRegs.GPCDIR.all = 0;        // GPIO87-64 as inputs
    EDIS;

}


interrupt void cpu_timer0_isr(void)
{
    CpuTimer0.InterruptCount++;
    EALLOW;
    SysCtrlRegs.WDKEY = 0xAA;   // service WD #2
    EDIS;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
