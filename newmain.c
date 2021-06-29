#include "mcc_generated_files/mcc.h"
#define I2C_freq 100000 //this is teh baud rate and is the speed at which data travels through a channel of communication. This value is suggested by the MCP23008 datasheet.

//defining characters on the seven segment display with their hexadecimal counterparts which indicate which segemnts light up.
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

unsigned char digits[] = {zero, one, two, three, four, five, six, seven, eight, nine, unlit, lit}; //creating an array based on the characters for the seven segement display.

//defining addess locations and setting for the communication/I2C/MCP23008. many are pre set hex numbers on the datasheet.
#define mcp23008_address_write 0x40 //these addresses are changed base on the hardware address pins (a0,a1,a2) and whether the address is read or write 
#define mcp23008_address_read 0x41 

#define MCP_IODIR 0x00//sets whether the pin is an input or an output. Hex value is 0 because all pins are outputs.
#define MCP_GPIO 0x09//reflects the logic level on the pins
#define MCP_IPOL 0x01//configures teh polarity of the GPIO port pins

#define MCP_IOCON 0x05// contains many bits which configure the device, including the slew rate of the SDA pin, enabling/disabling the hardware address pins
#define MCP_GPPU 0x06//this register controls the pull up resistors connected to the SDA and SCL pins. data sheet specifies this hex value.
#define MCP_OLAT 0x0A//controls access to output latches

//these set of registers are all to do with interupts, not being used for team 1s Derbot
#define MCP_DEFVAL 0x03 
#define MCP_INTCON 0x04 
#define MCP_INTF 0x07 
#define MCP_INTCAP 0x08 
#define MCP_GPINTEN 0x02

//Declairng and intialising funtions

void diagnostic(void);
void motor_fwd(unsigned char dir, unsigned char spd, bool STAT);
void movement(void);
//initialsing functions for the communication/I2C/MCP23008/PIC
void PIC_Initialise(void); //Pic
void I2C_Initialise(void); //i2c
void MCP23008_Initialise(void); //mcp
void IdleI2C(void); //Idlei2c
void StartI2C(void); //start i2c
void I2C_send_data(unsigned char device_address, unsigned char register_address, unsigned char register_data); //i2c send data 
void WriteI2C(unsigned char data); //writei2c 
void StopI2C(void); //stopi2c
void I2C(void); //this is the final I2C function called within the main

void rightPosition(void); //Declaring rightPosition, this function is used to move the servo to the right position
void leftPosition(void); //Declaring leftPosition, this function is used to move the servo to the left position
void adcFunction(void); //Declaring adcFucntion, this function will return the value from the ADC on pin AN3 

int adcResult; //Integer variable to hold the value of the ADC result from the adcFucntion() 
int UnLit = 0; //Integer variable to hold the number of un-lit objects detected 
int Lit = 0; //Integer variable to hold the number of lit objects detected
int LDRVAL = 40; //Integer variable which will compared to the ADC result to determine if a lit object is in front or not
int positionHolder = 0; //Integer variable used to escape the IF loop used

bool IsBumperL(void); //Declaring IsBumperL, this function returns true if the left bumper is triggered 
bool IsBumperR(void); //Declaring IsBumperR, this function returns true if the right bumper is triggered 
bool IsOptoL(void); //left opto triggered
bool IsOptoR(void); //right opto triggered
bool IsOptoFarR(void); //far right opto triggered

void main(void)
{

    SYSTEM_Initialize(); // Initialize the device - generated by MCC 
    I2C_Initialise(); //initialising communication
    MCP23008_Initialise(); //initialising the MCP23008 I/O expander
    //    diagnostic();
    while (1) // Loop forever
    {

        diagnostic();
        motor_fwd('L', 255, HIGH); //left motor full speed
        motor_fwd('R', 250, HIGH); //right motor 90% speed
        // initial motor conditions
        adcFunction(); //Calling function to get LDR value

        if (adcResult > LDRVAL) //IF the LDR does not see light...
        {
            adcFunction(); //Calling function to get LDR value
            leftPosition(); //Move servo to the left position

            if (IsOptoR()) // is right opto triggered
            { // 0 for opto-coupler is dark and 1 is light
                motor_fwd('L', 250, HIGH); //left motor full speed
                motor_fwd('R', 105, HIGH); //reduce right motor speed 


            }
            else if (IsOptoL()) //is left opto triggered 
            { // 0 for opto-coupler is dark and 1 is light
                motor_fwd('L', 105, HIGH); //reduce left motor speed
                motor_fwd('R', 250, HIGH); //right motor full speed


            }
            else if (IsOptoFarR()) //is far right opto triggered
            {
                __delay_ms(450); //wait for derbot to reach centre of square
                motor_fwd('L', 255, LOW); //set left motor off
                motor_fwd('R', 255, LOW); //set right motor off
                I2C(); //calling the I2C function to display the characters whent the derbot sends the stop signal
                break;
            }


            if (IsBumperR()) //Test if the right bumper is triggered
            {
                LED_LEFT_SetHigh(); //Set the back right LED HIGH
                UnLit++; //Un-lit variable is increased by 1
                __delay_ms(320); // The code is paused for 320mS to reduce the effects of button bounce           
                LED_LEFT_SetLow(); //Back right LED set to LOW again
            }
        }


        if (adcResult < LDRVAL) //IF the LDR sees light...
        {
            adcFunction(); //Calling function to get LDR value again
            positionHolder = 1; //Setting positionHolder variable to 1
            while (positionHolder == 1) //While loop for the positionHolder variable 
            {
                if (IsOptoR()) // is right opto triggered
                { // 0 for opto-coupler is dark and 1 is light
                    motor_fwd('L', 250, HIGH); //left motor full speed
                    motor_fwd('R', 105, HIGH); //reduce right motor speed 


                }
                else if (IsOptoL()) //is left opto triggered 
                { // 0 for opto-coupler is dark and 1 is light
                    motor_fwd('L', 105, HIGH); //reduce left motor speed
                    motor_fwd('R', 250, HIGH); //right motor full speed


                }
                else if (IsOptoFarR()) //is far right opto triggered
                {
                    __delay_ms(450); //wait for derbot to reach centre of square
                    motor_fwd('L', 255, LOW); //set left motor off
                    motor_fwd('R', 255, LOW); //set right motor off
                    I2C(); //calling the I2C function to display the characters whent the derbot sends the stop signal
                    break;
                }
                rightPosition(); //Moves the servo to the right position
                if (IsBumperL())
                {
                    LED_RIGHT_SetHigh(); //Set the back left LED HIGH
                    Lit++; //Lit variable is increased by 1
                    __delay_ms(320); // The code is paused for 320mS to reduce the effects of button bounce 
                    LED_RIGHT_SetLow(); //Back left LED set to LOW again
                    positionHolder = 0; //PositionHolder variable set to 0 to break the while loop
                }
                continue; //To break out of the while loop
            }
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
    if (dir == 'L') //left motor selected
    {
        CCPR2L = spd; //left motor speed
        LEFT_MTR_EN_PORT = STAT; // HIGH or LOW for left motor
    }
    else //left motor selected
    {

        CCPR1L = spd; // right motor speed
        RIGHT_MTR_EN_PORT = STAT; // HIGH or LOW for right motor
    }
}

void I2C_Initialise()//setting special function registers to start up the I2C
{

    SSP1STAT = 0x00; //serial synchronous port status register
    SSP1CON1 = 0x28; //serial synchronous port control register1
    SSP1CON2 = 0x00; //serial synchronous port control register2
    SSP1CON3 = 0x08; //serial synchronous port control register3
    SSP1ADD = 0x09; //serial synchronous port address and baud rate register
}

void MCP23008_Initialise(void)//creating the function for starting up the MCP23008
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

void I2C_send_data(unsigned char device_address, unsigned char register_address, unsigned char register_data)//configuring the settings required for sending messages through the I2C. this function uses many of the other functions created.
{
    //communication package, inlcluding start and stop bits, address checks and data transfer
    StartI2C();
    WriteI2C(device_address);
    IdleI2C();
    WriteI2C(register_address);
    IdleI2C();
    WriteI2C(register_data);
    IdleI2C();
    StopI2C();
}

void IdleI2C(void)//keeps the communication idle while 1 of 2 special function registers are active with specific settings
{

    while ((SSP1CON2 & 0x1F) || (SSP1STAT & 0x02));
}

void StartI2C(void)//creating the start bits required for communication packet
{
    PIR1bits.SSPIF = 0; //changing specific bits whithin the special function registers
    SSP1CON2bits.SEN = 1; //enabling the start condition bit

    while ((SSP1STAT & 0x04) || (SSP1CON2 & 0x1F)); //
    //    while (!PIR1bits.SSPIF);
    PIR1bits.SSPIF = 0;
}

void WriteI2C(unsigned char data)//writing data 
{
    SSP1BUF = data; //receive buffer/transmit register

    while (SSP1STATbits.BF);
}

void StopI2C()//creating the stop point of the package
{

    SSP1CON2bits.PEN = 1; //eanbling the stop condition bit
}

void I2C(void)//creating the function called within the main when teh stop signal is sent and the character diplay can occur
{

    //    for(int x=0; x<12; x++)
    {

        I2C_send_data(mcp23008_address_write, MCP_GPIO, digits[10]); // sending and displaying 10 of the array, U
        __delay_ms(1000); //waitng to allow for time to see the display
        I2C_send_data(mcp23008_address_write, MCP_GPIO, digits[UnLit]); //sending and displaying variable unlit, this value is from the amount of times the bumpers have been hit on the unlit side
        __delay_ms(1000);
        I2C_send_data(mcp23008_address_write, MCP_GPIO, digits[11]); // sending and displaying 11 of the array, L
        __delay_ms(1000);
        I2C_send_data(mcp23008_address_write, MCP_GPIO, digits[Lit]);
        __delay_ms(1000);
    }

}

//Servo position to the right

void rightPosition(void)
{
    SERVO_PWM_PORT = HIGH; //setting the output of the port to HIGH
    __delay_us(2000); //Holding it HIGH for 2000uS
    SERVO_PWM_PORT = LOW; //Setting the output to LOW
    __delay_us(19250); //Holding it LOW for 19250uS
}

//Servo position to the left

void leftPosition(void)
{
    SERVO_PWM_PORT = HIGH; //setting the output of the port to HIGH
    __delay_us(2320); //Holding it HIGH for 19250uS
    SERVO_PWM_PORT = LOW; //Setting the output to LOW
    __delay_us(19250); //Holding it LOW for 19250uS
}

//Analoge to Digital Conversion (ADC) function

void adcFunction(void)
{
    ADCON0bits.GO = 1; //Setting the 'start conversion' bit to 1 to start a new ADC reading
    while (ADCON0bits.GO == 1)//Pausing the code while the ADC conversion is taking place
    {
    }

    adcResult = ADRESH; //Store the value of ADRESH (the output of the ADC) to the adcResult variable
}

bool IsBumperL(void) // Return true when left switch is pressed
{

    return (BUMPER_LEFT_GetValue() == 0);
}

bool IsBumperR(void) // Return true when right switch is pressed
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

bool IsOptoFarR(void) // return true when right opto is dark (dark = 0, light = 1)
{
    return (OPTO_FAR_RIGHT_GetValue() == 0);
}

