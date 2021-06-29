
#include "mcc_generated_files/mcc.h"
#define I2C_freq 100000 

#define unlit 0x3e 
#define lit 0x38 
#define zero 0x3f 
#define one 0x6 
#define two 0x5b 
#define three 0x4f 
#define four 0x66 
#define five 0x6d 
#define six 0x7d 
#define seven 0x7 
#define eight 0x7f 
#define nine 0x67 

unsigned char digits[] = {unlit, lit, zero, one, two, three, four, five, six, seven, eight, nine};

#define mcp23008_address_write 0x40 //important changed based on a1,a0,a2
#define mcp23008_address_read 0x41 //important changed based on a1,a0,a2
#define MCP_IODIR 0x00//important 
#define MCP_GPIO 0x09//important 
#define MCP_IPOL 0x01 
#define MCP_GPINTEN 0x02 
#define MCP_DEFVAL 0x03 
#define MCP_INTCON 0x04 
#define MCP_IOCON 0x05 
#define MCP_GPPU 0x06 
#define MCP_INTF 0x07 
#define MCP_INTCAP 0x08 
#define MCP_OLAT 0x0A 

//Declairng and intialising funtions

void diagnostic(void);
void motor_fwd(unsigned char dir, unsigned char spd, bool STAT);
void movement(void);
void rightPosition(void);
void leftPosition(void);
void PIC_Initialise(void); //Pic
void I2C_Initialise(void); //i2c
void MCP23008_Initialise(void); //mcp
void IdleI2C(void); //Idlei2c
void StartI2C(void); //start i2c
void I2C_send_data(unsigned char device_address, unsigned char register_address, unsigned char register_data); //i2c send data requires 3 unsigned char 
void WriteI2C(unsigned char data); //writei2c requires unsigned char
void StopI2C(void); //stopi2c
void I2C_test(void);
void adcFunction(void);

int adcResult, intialResult;
int UnLit = 0;
int Lit = 0;
int LDRVAL = 40;

bool IsOptoL(void);
bool IsOptoR(void);
bool IsOptoFarR(void);
bool IsBumperL(void);
bool IsBumperR(void);

void main(void)
{

    SYSTEM_Initialize(); // Initialize the device - generated by MCC 
    I2C_Initialise();
    MCP23008_Initialise();
    diagnostic();
    while (1) // Loop for ever
    {

        //diagnostic();
        motor_fwd('L', 255, HIGH);
        motor_fwd('R', 250, HIGH);
        // initial motor conditions
        adcFunction(); //calling the ADC function to detect if there is light in front
        if (adcResult > LDRVAL)
        {
            leftPosition(); // the function to move the servo to the right is called

            if (IsBumperR())
            {
                LED_LEFT_SetHigh(); //if it is LOW then the back right LED is turned on
                UnLit++; //un-lit variable is increased by 1
                __delay_ms(5000); // the code is paused by 2 seconds
                LED_LEFT_SetLow(); //before the back right LED is turned off again
            }
        }
        else
        {
            rightPosition();

            if (IsBumperL())
            {
                // IF argument testing if the left bumper is LOW
                LED_RIGHT_SetHigh(); //if it is LOW then the back right LED is turned on
                Lit++; //un-lit variable is increased by 1
                __delay_ms(5000); // the code is paused by 2 seconds
                LED_RIGHT_SetLow(); //before the back right LED is turned off again

            }
            __delay_ms(4000);
        }



        if (IsOptoR())
        { // 0 for opto-coupler is dark and 1 is light
            motor_fwd('L', 250, HIGH);
            motor_fwd('R', 105, HIGH);
            adcFunction(); //calling the ADC function to detect if there is light in front

            if (adcResult > LDRVAL)
            {
                leftPosition(); // the function to move the servo to the right is called
            }
            else
            {
                rightPosition();
            }
            if (IsBumperL())
            {
                LED_RIGHT_SetHigh(); //if it is LOW then the back right LED is turned on
                UnLit++; //un-lit variable is increased by 1
                __delay_ms(2000); // the code is paused by 2 seconds
                LED_RIGHT_SetLow(); //before the back right LED is turned off again
            }

            if (IsBumperR())
            {
                // IF argument testing if the left bumper is LOW
                LED_LEFT_SetHigh(); //if it is LOW then the back right LED is turned on
                Lit++; //un-lit variable is increased by 1
                __delay_ms(2000); // the code is paused by 2 seconds
                LED_LEFT_SetLow(); //before the back right LED is turned off again

            }
            continue;
        }
        else if (IsOptoL())
        { // 0 for opto-coupler is dark and 1 is light
            motor_fwd('L', 105, HIGH);
            motor_fwd('R', 250, HIGH);
            adcFunction(); //calling the ADC function to detect if there is light in front
            if (adcResult > LDRVAL)
            {
                leftPosition(); // the function to move the servo to the right is called
            }
            else
            {
                rightPosition();
            }
            if (IsBumperL())
            {
                LED_RIGHT_SetHigh(); //if it is LOW then the back right LED is turned on
                UnLit++; //un-lit variable is increased by 1
                __delay_ms(2000); // the code is paused by 2 seconds
                LED_RIGHT_SetLow(); //before the back right LED is turned off again
            }

            if (IsBumperR())
            {
                // IF argument testing if the left bumper is LOW
                LED_LEFT_SetHigh(); //if it is LOW then the back right LED is turned on
                Lit++; //un-lit variable is increased by 1
                __delay_ms(2000); // the code is paused by 2 seconds
                LED_LEFT_SetLow(); //before the back right LED is turned off again

            }
            continue;

        }
        else if (IsOptoFarR())
        {
            __delay_ms(450);
            motor_fwd('L', 255, LOW);
            motor_fwd('R', 255, LOW);
            I2C_test();
            __delay_ms(20000);
            break;
        }

    }

}

void diagnostic(void) // Toggles leds for 0.3s each (Tcy = 1us)
{
    LED_LEFT_Toggle(); // PORTCbits.RC5 = 0
    LED_RIGHT_Toggle(); // PORTCbits.RC6 = 1
    __delay_ms(300); // 0.3 second delay
    LED_LEFT_Toggle(); // PORTCbits.RC5 = 1
    LED_RIGHT_Toggle(); // PORTCbits.RC6 = 0
}

void motor_fwd(unsigned char dir, unsigned char spd, bool STAT)
{
    if (dir == 'L')
    {
        CCPR2L = spd; //left motor speed
        LEFT_MTR_EN_PORT = STAT; // HIGH or LOW for left motor
    }
    else
    {
        CCPR1L = spd; // right motor speed
        RIGHT_MTR_EN_PORT = STAT; // HIGH or LOW for right motor
    }
}

void I2C_Initialise()
{
    SSP1STAT = 0x00;
    SSP1CON1 = 0x28;
    SSP1CON2 = 0x00;
    SSP1CON3 = 0x08;
    SSP1ADD = 0x09;
}

void MCP23008_Initialise(void)
{
    I2C_send_data(mcp23008_address_write, MCP_IODIR, 0x00);
    IdleI2C();
    I2C_send_data(mcp23008_address_write, MCP_IPOL, 0x00);
    IdleI2C();
    I2C_send_data(mcp23008_address_write, MCP_GPINTEN, 0x00);
    IdleI2C();
    I2C_send_data(mcp23008_address_write, MCP_IOCON, 0x3E);
    IdleI2C();
    I2C_send_data(mcp23008_address_write, MCP_GPPU, 0x00);
}

void I2C_send_data(unsigned char device_address, unsigned char register_address, unsigned char register_data)
{
    StartI2C();
    WriteI2C(device_address);
    IdleI2C();
    WriteI2C(register_address);
    IdleI2C();
    WriteI2C(register_data);
    IdleI2C();
    StopI2C();
}

void IdleI2C(void)
{
    while ((SSP1CON2 & 0x1F) || (SSP1STAT & 0x02));
}

void StartI2C(void)
{
    PIR1bits.SSPIF = 0;
    SSP1CON2bits.SEN = 1;
    while ((SSP1STAT & 0x04) || (SSP1CON2 & 0x1F));
    //    while (!PIR1bits.SSPIF);
    PIR1bits.SSPIF = 0;
}

void WriteI2C(unsigned char data)
{
    SSP1BUF = data;
    while (SSP1STATbits.BF);
}

void StopI2C()
{
    SSP1CON2bits.PEN = 1;
}

void I2C_test(void)
{
    int i;
    for (i = 0; i < sizeof (i) - 1; i++)
    {
        I2C_send_data(mcp23008_address_write, MCP_GPIO, digits[i]);
        __delay_ms(500);
        //        LED_LEFT_Toggle();
    }
}

//Servo position to the right

void rightPosition(void)//naming the function
{
    unsigned int i; //declaring an unsigned integer called 'i'
    for (i = 0; i < 50; i++)//starting a for loop, intialising 'i' to 0 and incrementing it by 1 upto 50
    {
        SERVO_PWM_PORT = 1; //setting the PWM signal HIGH
        __delay_us(1800); //holding it HIGH for 1200us
        SERVO_PWM_PORT = 0; //pulling the PWM signal LOW
        __delay_us(19200); //holding it LOW for 19200us
    }
}
//Servo position to the left

void leftPosition(void) //180 Degree
{
    unsigned int i;
    for (i = 0; i < 50; i++)
    {
        SERVO_PWM_PORT = 1;
        __delay_us(2150);
        SERVO_PWM_PORT = 0;
        __delay_us(19200);

    }
}

//Analoge to Digital Conversion (ADC) function

void adcFunction(void)
{
    __delay_us(7); //delaying the code by 7us to slow it down
    ADCON0bits.GO = 1; //setting the 'ADC begin' bit to HIGH
    while (ADCON0bits.GO == 1)//pausing the code while the ADC conversion is taking place
    {
    }

    //adcResult = (ADRESH << 8) + ADRESL; //Saving the 10-bit result of the ADC to the adcResult variable
    adcResult = ADRESH;

}

bool IsBumperL(void) // return true when right opto is dark (dark = 0, light = 1)
{
    return (BUMPER_LEFT_GetValue() == 0);
}

bool IsBumperR(void)
{
    return (BUMPER_RIGHT_GetValue() == 0);
}

bool IsOptoL(void) // return true when left opto is dark (dark = 0, light = 1)
{
    return (OPTO_LEFT_GetValue() == 0);
}

bool IsOptoR(void) // return true when right opto is dark (dark = 0, light = 1)
{
    return (OPTO_RIGHT_GetValue() == 0);
}

bool IsOptoFarR(void)
{
    return (OPTO_FAR_RIGHT_GetValue() == 0);
}
