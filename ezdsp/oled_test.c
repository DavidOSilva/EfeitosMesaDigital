//////////////////////////////////////////////////////////////////////////////
// * File name: oled_test.c
// *                                                                          
// * Description:  OSD9616 OLED Test.
// *                                                                          
// * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/ 
// * Copyright (C) 2011 Spectrum Digital, Incorporated
// *                                                                          
// *                                                                          
// *  Redistribution and use in source and binary forms, with or without      
// *  modification, are permitted provided that the following conditions      
// *  are met:                                                                
// *                                                                          
// *    Redistributions of source code must retain the above copyright        
// *    notice, this list of conditions and the following disclaimer.         
// *                                                                          
// *    Redistributions in binary form must reproduce the above copyright     
// *    notice, this list of conditions and the following disclaimer in the   
// *    documentation and/or other materials provided with the                
// *    distribution.                                                         
// *                                                                          
// *    Neither the name of Texas Instruments Incorporated nor the names of   
// *    its contributors may be used to endorse or promote products derived   
// *    from this software without specific prior written permission.         
// *                                                                          
// *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     
// *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT       
// *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR   
// *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT    
// *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,   
// *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT        
// *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,   
// *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY   
// *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT     
// *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE   
// *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.    
// *                                                                          
//////////////////////////////////////////////////////////////////////////////

#include"ezdsp5502.h"

#include"lcd.h"

/*
 *
 *  Int16 oled_test()
 *
 *      Testing function for the OSD9616 display
 *
 */

Int16 letterVals[135] = {
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x7C,
  0x0A,
  0x09,
  0x0A,
  0x7C,
  0x36,
  0x49,
  0x49,
  0x49,
  0x7F,
  0x22,
  0x41,
  0x41,
  0x41,
  0x3E,
  0x1C,
  0x22,
  0x41,
  0x41,
  0x7F,
  0x41,
  0x49,
  0x49,
  0x49,
  0x7F,
  0x01,
  0x09,
  0x09,
  0x09,
  0x7F,
  0x3A,
  0x49,
  0x49,
  0x41,
  0x3E,
  0x7F,
  0x08,
  0x08,
  0x08,
  0x7F,
  0x00,
  0x00,
  0x7F,
  0x00,
  0x00,
  0x01,
  0x01,
  0x3F,
  0x41,
  0x21,
  0x63,
  0x14,
  0x08,
  0x08,
  0x7F,
  0x40,
  0x40,
  0x40,
  0x40,
  0x7F,
  0x7F,
  0x06,
  0x18,
  0x06,
  0x7F,
  0x7F,
  0x20,
  0x18,
  0x06,
  0x7F,
  0x3E,
  0x41,
  0x41,
  0x41,
  0x3E,
  0x06,
  0x09,
  0x09,
  0x09,
  0x7F,
  0x5E,
  0x21,
  0x51,
  0x41,
  0x3E,
  0x46,
  0x29,
  0x19,
  0x09,
  0x7F,
  0x32,
  0x49,
  0x49,
  0x49,
  0x26,
  0x01,
  0x01,
  0x7F,
  0x01,
  0x01,
  0x3F,
  0x40,
  0x40,
  0x40,
  0x3F,
  0x0F,
  0x30,
  0x40,
  0x30,
  0x0F,
  0x3F,
  0x40,
  0x38,
  0x40,
  0x3F,
  0x63,
  0x14,
  0x08,
  0x14,
  0x63,
  0x07,
  0x08,
  0x70,
  0x08,
  0x07,
  0x43,
  0x45,
  0x49,
  0x51,
  0x61
};

Int16 printLetterT(char c) {
  int b = 0;
  switch (c) {
  case 'A':
    b = 1;
    break;
  case 'B':
    b = 2;
    break;
  case 'C':
    b = 3;
    break;
  case 'D':
    b = 4;
    break;
  case 'E':
    b = 5;
    break;
  case 'F':
    b = 6;
    break;
  case 'G':
    b = 7;
    break;
  case 'H':
    b = 8;
    break;
  case 'I':
    b = 9;
    break;
  case 'J':
    b = 10;
    break;
  case 'K':
    b = 11;
    break;
  case 'L':
    b = 12;
    break;
  case 'M':
    b = 13;
    break;
  case 'N':
    b = 14;
    break;
  case 'O':
    b = 15;
    break;
  case 'P':
    b = 16;
    break;
  case 'Q':
    b = 17;
    break;
  case 'R':
    b = 18;
    break;
  case 'S':
    b = 19;
    break;
  case 'T':
    b = 20;
    break;
  case 'U':
    b = 21;
    break;
  case 'V':
    b = 22;
    break;
  case 'W':
    b = 23;
    break;
  case 'X':
    b = 24;
    break;
  case 'Y':
    b = 25;
    break;
  case 'Z':
    b = 26;
    break;
  default:
    b = 0;
    break;
  }
  // w r i t e l e t t e r t o LCD
  int a = 0;
  for (a = 0; a < 5; a++) {
    osd9616_send(0x40, letterVals[(b * 5) + a]);
  }
  osd9616_send(0x40, 0x00);
  return 0;
}

void spaceBlank(int num) {
  int i = 0;
  for (i = 0; i < num; i++) {
    osd9616_send(0x40, 0x00);
  }

}
Int16 lcdPage1(int selection){
    /* Fill page 1*/
      osd9616_send(0x00, 0x00); // Set low column address
      osd9616_send(0x00, 0x10); // Set high column address
      osd9616_send(0x00, 0xb0 + 1); // Set page for page 0 to page 5
      spaceBlank(128);

      /* Write to page 1*/
      osd9616_send(0x00, 0x00); // Set low column address
      osd9616_send(0x00, 0x10); // Set high column address
      osd9616_send(0x00, 0xb0 + 1); // Set page for page 0 to page 5
      spaceBlank(15);

    switch(selection){
    case 0:
        printLetterT('D');
        printLetterT('E');
        printLetterT('T');
        printLetterT('C');
        printLetterT('E');
        printLetterT('L');
        printLetterT('E');
        printLetterT('S');
        spaceBlank(3);

        printLetterT('T');
        printLetterT('O');
        printLetterT('N');

        spaceBlank(15);
        break;
    case 1:
        printLetterT('D');
        printLetterT('E');
        printLetterT('T');
        printLetterT('C');
        printLetterT('E');
        printLetterT('L');
        printLetterT('E');
        printLetterT('S');
        spaceBlank(15);


        break;

    }


      return 0;
}

Int16 lcdPage0(int effect) {
  Int16 i;
  Uint16 cmd[10]; // For multibyte commands

  osd9616_init(); // Initialize  Display

  osd9616_send(0x00, 0x2e); // Deactivate Scrolling

  /* Fill page 0 */
  i = osd9616_send(0x00, 0x00); // Set low column address
  osd9616_send(0x00, 0x00); // Set high column address
  osd9616_send(0x00, 0xb0 + 0); // Set page for page 0 to page 5

  spaceBlank(128);

  /* Write to page 0 */
  osd9616_send(0x00, 0x00); // Set low column address
  osd9616_send(0x00, 0x10); // Set high column address
  osd9616_send(0x00, 0xb0 + 0); // Set page for page 0 to page 5

  spaceBlank(10); //margem das letras

  switch (effect) {
  case 3:
    printLetterT('M');
    printLetterT('O');
    printLetterT('O');
    printLetterT('R');

    spaceBlank(5);

    printLetterT('V');
    printLetterT('E');
    printLetterT('R');

    spaceBlank(5);

    // printLetter(0x3E, 0x49, 0x49, 0x49); // 3
    printLetter(0x40, 0x7F, 0x42, 0x00); // 1
    printLetter(0x3E, 0x41, 0x41, 0x3E); // 0

    spaceBlank(23);
    break;
  case 6:
    printLetterT('B');
    printLetterT('A');

    spaceBlank(2);
    printLetterT('E');
    printLetterT('G');
    printLetterT('A');
    printLetterT('T');
    printLetterT('S');

    spaceBlank(2);

    printLetterT('V');
    printLetterT('E');
    printLetterT('R');
    spaceBlank(2);
   // printLetter(0x71, 0x51, 0x52, 0x3C); // 6
    printLetter(0x46, 0x49, 0x51, 0x62); // 2
    printLetter(0x3E, 0x41, 0x41, 0x3E); // 0

    // spaceBlank(10);
    break;
  case 9:

    printLetterT('C');

    spaceBlank(5);
    printLetterT('E');
    printLetterT('G');
    printLetterT('A');
    printLetterT('T');
    printLetterT('S');

    spaceBlank(5);

    printLetterT('V');
    printLetterT('E');
    printLetterT('R');
    spaceBlank(5);
   // printLetter(0xFE, 0x51, 0x51, 0x4E); // 9
     printLetter(0x3E, 0x49, 0x49, 0x49); // 3
    printLetter(0x3E, 0x41, 0x41, 0x3E); // 0

    spaceBlank(43);
    break;
  case 12:

    printLetterT('E');

    spaceBlank(5);
    printLetterT('E');
    printLetterT('G');
    printLetterT('A');
    printLetterT('T');
    printLetterT('S');

    spaceBlank(2);

    printLetterT('V');
    printLetterT('E');
    printLetterT('R');
    spaceBlank(2);
    printLetter(0xFF, 0x08, 0x08, 0x0F);  // 4 corrigido
    printLetter(0x3E, 0x41, 0x41, 0x3E); // 0

    spaceBlank(43);
    break;
  case 15:
    printLetterT('G');

    spaceBlank(5);
    printLetterT('E');
    printLetterT('G');
    printLetterT('A');
    printLetterT('T');
    printLetterT('S');

    spaceBlank(2);

    printLetterT('V');
    printLetterT('E');
    printLetterT('R');
    spaceBlank(2);
    printLetter(0x31, 0x49, 0x49, 0x2F); // 5
    printLetter(0x3E, 0x41, 0x41, 0x3E); // 0

    spaceBlank(43);
    break;
  case 18:

    printLetterT('S');
    printLetterT('U');
    printLetterT('R');
    printLetterT('O');
    printLetterT('H');
    printLetterT('C');
    spaceBlank(3);
  //  printLetter(0x36, 0x49, 0x49, 0x36); // 8
     printLetter(0x71, 0x51, 0x52, 0x3C); // 6
    printLetter(0x3E, 0x41, 0x41, 0x3E); // 0

    spaceBlank(43);
    break;
  case 21:
    printLetterT('E');
    printLetterT('C');
    printLetterT('I');
    printLetterT('O');
    printLetterT('V');
    spaceBlank(3);
    printLetterT('O');
    printLetterT('I');
    printLetterT('D');
    printLetterT('A');
    printLetterT('R');
    spaceBlank(3);

   // printLetter(0x40, 0x7F, 0x42, 0x00); // 1
   // printLetter(0x46, 0x49, 0x51, 0x62); // 2
    printLetter(0xFF, 0x01, 0x01, 0x0F);  // 7 corrigido
    printLetter(0x3E, 0x41, 0x41, 0x3E); // 0

    spaceBlank(43);
    break;
  case 24:
    printLetterT('L');
    printLetterT('A');
    printLetterT('C');
    printLetterT('O');
    printLetterT('V');
    spaceBlank(3);

    printLetter(0xFF, 0x91, 0x91, 0xFF);  // 8 corrigido
    printLetter(0x3E, 0x41, 0x41, 0x3E); // 0
    spaceBlank(43);
    break;

  }
  osd9616_send(0x00, 0x2e); // Deactivate Scrolling

       /* Set vertical and horizontal scrolling */
       cmd[0] = 0x00;
       cmd[1] = 0x29; // Vertical and Right Horizontal Scroll
       cmd[2] = 0x00; // Dummy byte
       cmd[3] = 0x00; // Define start page address
       cmd[4] = 0x03; // Set time interval between each scroll step
       cmd[5] = 0x01; // Define end page address
       cmd[6] = 0x01; // Vertical scrolling offset
       // osd9616_multiSend( cmd, 7 );
       // osd9616_send(0x00,0x2f);

       /* Keep first 8 rows from vertical scrolling  */
       cmd[0] = 0x00;
       cmd[1] = 0xa3; // Set Vertical Scroll Area
       cmd[2] = 0x08; // Set No. of rows in top fixed area
       cmd[3] = 0x08; // Set No. of rows in scroll area
       osd9616_multiSend(cmd, 4);

  return 0;
}
