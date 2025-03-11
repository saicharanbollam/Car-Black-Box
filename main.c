/*
 * NAME: B SAI CHARAN REDDY
 * DATE: 08/11/2024
 * DESCRIPTION: CAR BLACK BOX PROJECT
 */




#include <xc.h>
#include "main.h"
#include "clcd.h"
#include "matrix_keypad.h"
#include "adc.h"
#include "ds1307.h"
#include "i2c.h"
#include "external_eeprom.h"
#include "uart.h"

unsigned char entry_count = 0, start_index = 0;
char *arr[] = {"ON", "GN", "GR", "G1", "G2", "G3", "G4", "G5", "C "};
char *menu1[] = {"VIEW LOG       ", "SET TIME       ", "DOWNLOAD LOG   ", "CLEAR LOG      "};
static unsigned int flag = 0, flag1 = 0, delay = 0, delay1 = 0, flag2 = 0;
unsigned char clock_reg[3];
unsigned char time[9], time1[9], data[10] = {0};
unsigned long int adc_reg_val;
unsigned long int speed;
static unsigned char address = 0, temp = 0;
static unsigned int i = 0, j = 0, count = 0, last_count = -1, mcount = 0, mmcount = 0, vcount = 0, field = 0;
int arr1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static unsigned int hours = 0, minutes = 0, seconds = 0;
int arr2[10][11] = {0};
int set_flag;
int set_count;
unsigned char key;
static void init_config(void) {
    init_clcd();
    init_matrix_keypad();
    init_adc();
    init_i2c();
    init_ds1307();
    init_uart();


}


void display_time(void) {
    clcd_print(time, LINE2(0));
}

void get_time(void) {
    clock_reg[0] = read_ds1307(HOUR_ADDR);
    clock_reg[1] = read_ds1307(MIN_ADDR);
    clock_reg[2] = read_ds1307(SEC_ADDR);

    time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);
    time[1] = '0' + (clock_reg[0] & 0x0F);
    time[2] = ':';
    time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);
    time[4] = '0' + (clock_reg[1] & 0x0F);
    time[5] = ':';
    time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);
    time[7] = '0' + (clock_reg[2] & 0x0F);
    time[8] = '\0';
}

void main(void) {
    init_config();

    while (1) {
        check_matrix_keypad();
        if (flag == 0) {
            get_time();
            display_time();
            adc_reg_val = read_adc(CHANNEL4);
            speed = (adc_reg_val) / 10.23;
            if (speed >= 99) {
                speed = 99;
            }
            clcd_putch((speed / 10) + '0', LINE2(14));//printing speed
            clcd_putch((speed % 10) + '0', LINE2(15));
        }
    }
}


void check_matrix_keypad(void) {

    key = read_switches(STATE_CHANGE);

    if (flag == 0) {//dashboard will show when flag=0
        clcd_print("  TIME    EV  SP", LINE1(0));
        clcd_print(arr[count], LINE2(10));

        if (count != last_count && count != 0) {
            //we have use only 100 bytes of address to store 10 events
            //after temp is greater than 9 reset to zero because we have store only 10 events means 1 event takes 10 bytes
            //in f write function i am multiply  with 10  and store 10 bytes of memory
            if (temp > 9) {
                temp = 0;
            }
            f_write(temp);
            temp++;
            vcount++;
            last_count = count;
        }
    }
    if (flag == 1) {// main menu will display when flag=1
        if (i == 0) {
            clcd_putch('*', LINE1(0));
            clcd_putch(' ', LINE2(0));
        } else if (i == 1) {
            clcd_putch(' ', LINE1(0));
            clcd_putch('*', LINE2(0));
        }
        clcd_print(menu1[mcount], LINE1(1));//displaying main menu based on mcount and i variable
        clcd_print(menu1[mcount + 1], LINE2(1));
    }
    if (key != ALL_RELEASED) {
        if (key == MK_SW11 && flag == 0) {
            flag = 1;//to go main menu
            mcount = 0;
            i = 0;
            c_display();
        } else if (key == MK_SW11 && flag == 1) {
            flag = 2; //to go inside of menu(like inside of inside of view log,download log,clear log,set time)
            mmcount = 0;
            c_display();
            address = 0;

        }


        if (flag == 0) {
            //used for incrementing or decrementing count nothing but evrnts are adding
            if ((key == MK_SW1 && count == 8) || (key == MK_SW2 && count == 8)) {
                count = 1;
            } else if (key == MK_SW1 && count < 7) {
                count++;
            } else if (key == MK_SW2 && count > 1) {
                count--;
            } else if (key == MK_SW3) {
                count = 8;//collision eill occur when we press switch 3
            }
        } else if (flag == 1) {
            //mcount is used to change the menu items to display in clcd
            if (key == MK_SW1 && mcount >= 0) {
                if (i == 1) {
                    i = 0;// variable i is used for position of star in main menu
                } else if (mcount > 0) {
                    mcount--;
                    i = 0;
                }
            } else if (key == MK_SW2 && mcount <= 2) {
                if (i == 0) {
                    i = 1;
                } else if (mcount < 2) {
                    i = 1;
                    mcount++;
                }
            } else if (key == MK_SW12) {
                flag = 0;
                c_display();
            }
        } else if (flag == 2) {
            if (mcount == 0 && i == 0) {
                //displaying counts based on vcount and count values
                if (vcount == 0 || count == 0) {
                    c_display();
                    clcd_print("NO LOGS", LINE1(0));
                    clcd_print("TO DISPLAY :(", LINE2(2));
                    for (unsigned long int a = 0; a < 500000; a++);
                    flag=1;
                    
                } else  {
                    c_display();

                    clcd_print("# VIEW LOG", LINE1(0));
                    f_read(address);
                    if (key == MK_SW2 && address < (vcount - 1)) {
                        address++;
                        if(address>=9)
                        {
                            address=9;
                        }  
                        f_read(address);
                    } else if (key == MK_SW1 && address > 0) {
                        address--;

                        f_read(address);
                    }
                }
            } else if ((mcount == 0 && i == 1) || (mcount == 1 && i == 0)) {
                //to saved only one time from rtc for editing the set time
                for (int i = 0; i < 9; i++) {
                    time1[i] = time[i];
                }
                flag2 = 1;
                 if (key == MK_SW11)
                    set_count++;

                if (set_count == 2) {
                    clock_reg[0] = ((hours / 10) << 4) | (hours % 10);
                    clock_reg[1] = ((minutes / 10) << 4) | (minutes % 10);
                    clock_reg[2] = ((seconds / 10) << 4) | (seconds % 10);
                    //used for write time into dashboard
                    write_ds1307(HOUR_ADDR, clock_reg[0]);
                    write_ds1307(MIN_ADDR, clock_reg[1]);
                    write_ds1307(SEC_ADDR, clock_reg[2]);
                    field = 0;
                    set_flag = 0;
                    set_count = 0;
                    flag = 0,flag2=0;;
                    mcount=0,i=0,mmcount=0;


                }
                
               
            } else if ((mcount == 2 && i == 0) || (mcount == 1 && i == 1)) {
                //downloading logs into tera term based on events saved on address
                c_display();
                if (count == 0 || vcount == 0) {
                    clcd_print("NO LOGS", LINE1(0));
                    clcd_print("TO DOWNLOAD :(", LINE2(2));
                    for (unsigned long int a = 0; a < 500000; a++);
                } else {
                    
                    clcd_print("DOWNLOADING...", LINE1(0));
                    clcd_print("Through UART...", LINE2(0));
                    for (unsigned long int a = 0; a < 500000; a++);
                    download_logs();//calling download logs
                    
                    
                   
                }
               flag = 1;
               mcount = 0, i = 0; 
            } else if (mcount == 2 && i == 1) {
                c_display();
                clcd_print("Clearing  Logs...", LINE1(0));
                clcd_print("Just a minute", LINE2(0));
                for (unsigned long int a = 0; a < 500000; a++);
                clear_logs();
                flag = 1;
                mcount = 0, i = 0;
            }
            if (key == MK_SW12 && flag == 2) {
                //going back to main menu based on flag=1 value
                set_count = 0;
                set_flag = 0;                   
                flag = 1;
                field = 0;
                flag2 = 0;
                mcount=0,i=0;
                c_display();
            }

        }
    }
    if (flag2 == 1 && flag == 2) {
        
         if (set_flag == 0) {
            set_flag = 1;
            hours = ((time1[0] - 48)*10) + (time1[1] - 48);
            minutes = ((time1[3] - 48)*10) + (time1[4] - 48);
            seconds = ((time1[6] - 48)*10) + (time1[7] - 48);
            
        }
        
        clcd_print("HH:MM:SS", LINE1(0));


        if (field == 0) {

            if ((delay++) < 500) {
                //used for blinking purpose
               clcd_putch(' ', LINE2(0));
                clcd_putch(' ', LINE2(1));
                

            } else if ((delay++) < 1000) {
                
                clcd_putch((hours / 10) + '0', LINE2(0));
                clcd_putch((hours % 10) + '0', LINE2(1));
                clcd_putch(':', LINE2(2));
                clcd_putch((minutes / 10) + '0', LINE2(3));
                clcd_putch((minutes % 10) + '0', LINE2(4));
                clcd_putch(':', LINE2(5));
                clcd_putch((seconds / 10) + '0', LINE2(6));
                clcd_putch((seconds % 10) + '0', LINE2(7));
            } else
                delay = 0;


        } else if (field == 1) {

            if ((delay++) < 500) {
                //used for blinking purpose
               clcd_putch(' ', LINE2(3));
                clcd_putch(' ', LINE2(4));
                
            } else if ((delay++) < 1000) {
               
                clcd_putch((hours / 10) + '0', LINE2(0));
                clcd_putch((hours % 10) + '0', LINE2(1));
                clcd_putch(':', LINE2(2));
                clcd_putch((minutes / 10) + '0', LINE2(3));
                clcd_putch((minutes % 10) + '0', LINE2(4));
                clcd_putch(':', LINE2(5));
                clcd_putch((seconds / 10) + '0', LINE2(6));
                clcd_putch((seconds % 10) + '0', LINE2(7));


            } else
                delay = 0;
        } else if (field == 2) {

            if ((delay++) < 500) {
                clcd_putch(' ', LINE2(6));//used for blinking purpose
                clcd_putch(' ', LINE2(7));
           
            } else if ((delay++) < 1000) {
                
                clcd_putch((hours / 10) + '0', LINE2(0));
                clcd_putch((hours % 10) + '0', LINE2(1));
                clcd_putch(':', LINE2(2));
                clcd_putch((minutes / 10) + '0', LINE2(3));
                clcd_putch((minutes % 10) + '0', LINE2(4));
                clcd_putch(':', LINE2(5));
                clcd_putch((seconds / 10) + '0', LINE2(6));
                clcd_putch((seconds % 10) + '0', LINE2(7));

            } else
                delay = 0;
        }


    }

    if (key == MK_SW2 && flag2 == 1 && flag == 2) {
        switch_field();//used for switching field
    } else if (key == MK_SW1 && flag2 == 1 && flag == 2) {
        
        increment_field();//used for incrementing values of hours,min,seconds
    }
}

void f_write(unsigned char entry_index) {

   
    unsigned int base_address = (entry_index % 10) * 10;

//writing into external eeprom based on address value
    write_external_eeprom(base_address + 0, time[0]);
    write_external_eeprom(base_address + 1, time[1]);
    write_external_eeprom(base_address + 2, time[3]);
    write_external_eeprom(base_address + 3, time[4]);
    write_external_eeprom(base_address + 4, time[6]);
    write_external_eeprom(base_address + 5, time[7]);
    write_external_eeprom(base_address + 6, arr[count][0]);
    write_external_eeprom(base_address + 7, arr[count][1]);
    write_external_eeprom(base_address + 8, (speed / 10) + '0');
    write_external_eeprom(base_address + 9, (speed % 10) + '0');


    if (entry_count < 10) {
        entry_count++;
    } else {

        start_index = (start_index + 1) % 10;//used to shift old entries into first after exceding 10 events then new entries overwrite old entries
    }
}

void f_read(unsigned int index) {

    //below two lines are used to find index when entry count is exceeds 10
    if (index >= entry_count) {
        index = entry_count - 1;
    }
    //based on entry count start index value gets so to shift events after it exceeds 10 
    //read index will help to print old events first then old events
    unsigned int read_index = (start_index + index) % 10;
    unsigned int base_address = read_index * 10;
//reading data from external eeprom to data[i] variable to print in clcd and used to print in tera term
    for (int i = 0; i < 10; i++) {
        data[i] = read_external_eeprom(base_address + i);
    }

//used to display in clcd of events based on events counts and index 
    clcd_putch((arr1[index] + '0'), LINE2(0));
    clcd_putch(data[0], LINE2(2));
    clcd_putch(data[1], LINE2(3));
    clcd_putch(':', LINE2(4));
    clcd_putch(data[2], LINE2(5));
    clcd_putch(data[3], LINE2(6));
    clcd_putch(':', LINE2(7));
    clcd_putch(data[4], LINE2(8));
    clcd_putch(data[5], LINE2(9));
    clcd_putch(data[6], LINE2(11));
    clcd_putch(data[7], LINE2(12));
    clcd_putch(data[8], LINE2(14));
    clcd_putch(data[9], LINE2(15));
}

void c_convert() {
    //converting one dimensional to two dimensional
    for (unsigned int entry_index = 0; entry_index <= entry_count; entry_index++) {
        f_read(entry_index);
        for (int i = 0; i < 10; i++) {
            arr2[entry_index][i] = data[i];
        }
        arr2[entry_index][11] = '\0';
    }


}

void download_logs(void) {

    c_convert();//calling for converting two from one dimensional
    puts("NO. TIME   EV SP");
    puts("\n\r");
    for (int i = 0; i < entry_count; i++) {
        putch((i % 10) + '0');
        putch(' ');
        for (int j = 0; arr2[i][j] != '\0'; j++) {
            putch(arr2[i][j]);
            if (j == 1 || j == 3) {
                putch(':');
            }
            if (j == 5 || j == 7) {
                putch(' ');
            }
        }
        puts("\n\r");
    }

}

void clear_logs(void) {
    //used for clearing logs
    for (int entry = 0; entry < 10; entry++) {
        for (int i = 0; i < 10; i++) {
            write_external_eeprom(entry * 10 + i, 0xFF);
        }
    }
    vcount = 0;
    address = 0;
    temp = 0;
    entry_count = 0;
    start_index = 0;
}

void c_display() {
    CLEAR_DISP_SCREEN;//to clear clcd display using macro
}

void increment_field() {

    if (field == 0) {
        hours = (hours + 1) % 24;//incrementing hour field
        
      

    } else if (field == 1) {
        minutes = (minutes + 1) % 60;//incrementing minute field
        
        
    } else if (field == 2) {
        seconds = (seconds + 1) % 60;//incrementing seconds  field
        
       
    }

}

void switch_field() {
    field = ((field + 1) % 3);//this is used to change field
}
