#define byte char
int encode(byte * in_buff, byte * out_buff);

// Encoding Method: 
//    1. read a block of information
//       1. load load three bytes from the input location (totaling 24 bits;  3 * 8 == 24)
//       1. merge bytes into one register      
//       1. chop the bytes into 4 6-bit chunks
//       1. map each of the 4 6-bit chunks into an 8-bit ASCII value
//       1. store each of these 8-bit ASCII values to the output location
//    2. handle padding
//
// Decoding Method
//
//

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>

#include "base64.h"

#define offset_A (  0 + 'A')
#define offset_a (-26 + 'a')
#define offset_0 (-52 + '0')

#define bin2ascii(index)              \
switch ( index ) {                    \
case 0 ... 25  : index +=   0 + 'A' ; \
  break;                              \
case 26 ... 51 : index += -26 + 'a';  \
  break;                              \
case 52 ... 61 : index += -52 + '0';  \
  break;                              \
case 62        : index = '+';         \
  break;                              \
case 63        : index = '/';         \
  break;                              \
}


int encode (byte * in_buff, byte * out_buff) {
  int b0;  // The three input bytes
  int b1;
  int b2;
  int c0;  // The four output characters
  int c1;
  int c2;
  int c3;

  int num;       // The number of bytes to process
  int in_count;  // The input byte number being processed
  int out_count; // The output byte number being processed
  int bytes;     // The number of remaining bytes;  num % 3

  do {
    in_count  = 0;
    out_count = 0;

    num = read(0, (void *) in_buff, size_in_buff);
    if (num == 0) break;

    // Process three bytes at a time -- but don't process the last row
    // The last row may need to have padding added based upon the number
    // of bytes in that row: 1, 2, or 3 bytes
    int x = num - 3;
    for (in_count=0 ; in_count < x; in_count += 3) {

      // Load 24 bits, which is three bytes
      b0 = in_buff[in_count + 0];
      b1 = in_buff[in_count + 1];
      b2 = in_buff[in_count + 2];

      // Merge the bytes into a single 24-bit value
      b0 =  (b0 << 16) | (b1 << 8) | b2;      // b0 = (((b0 << 8) | b1 )<< 8) | b2;   
                                              // Rewrite for the ARM architecture

      // Chop 24-bit value into 4 groups of 6-bits
      c0 = (b0 >> 18);           //  c0 = (b0 >> 18) & 0x3F;
      c1 = (b0 >> 12) & 0x3F;    //  c1 = (b0 >> 12) & 0x3F;
      c2 = (b0 >>  6) & 0x3F;    //  c2 = (b0 >>  6) & 0x3F;
      c3 = (b0 & 0x3F);          //  c3 = (b0 >>  0) & 0x3F;

      // Map the 6-bit binary values into ASCII
      bin2ascii(c0);
      bin2ascii(c1);
      bin2ascii(c2);
      bin2ascii(c3);

      // Store the individual ASCII values
      out_buff[out_count + 0] = c0;
      out_buff[out_count + 1] = c1;
      out_buff[out_count + 2] = c2;
      out_buff[out_count + 3] = c3;        

      out_count = out_count + 4;
    }

    // Handle the last row, which might need padding
    // Note this could be the body of the loop in total
    //   The last row is processed separately to simplify 
    //   the explanation of the code -- for teaching purposes.
    {

      // Determine the number of bytes in the row
      // For the last row it is
      bytes = num - in_count;

      // For any row, it is:
      // bytes = (num - in_count ) % 3;
      
      // Load up to 24 bits, which is three bytes
      b0 = in_buff[in_count + 0];

      switch (bytes) {
        case 3:  // i.e., three bytes to process
          b1 = in_buff[in_count + 1];
          b2 = in_buff[in_count + 2];
          break;
        case 2:
          b1 = in_buff[in_count + 1];
          b2 = 0;
          break;
        case 1:
          b1 = 0;
          b2 = 0;
          break;
      }
 
      // Merge the bytes into a single 24-bit value
      b0 =  (b0 << 16) | (b1 << 8) | b2;      // b0 = (((b0 << 8) | b1 )<< 8) | b2;   
                                              // Rewrite for the ARM architecture

      // Chop 24-bit value into 4 groups of 6-bits
      c0 = (b0 >> 18);           //  c0 = (b0 >> 18) & 0x3F;
      c1 = (b0 >> 12) & 0x3F;    //  c1 = (b0 >> 12) & 0x3F;
      c2 = (b0 >>  6) & 0x3F;    //  c2 = (b0 >>  6) & 0x3F;
      c3 = (b0 & 0x3F);          //  c3 = (b0 >>  0) & 0x3F;

      // Map the 6-bit binary values into ASCII
      bin2ascii(c0);
      bin2ascii(c1);
      // bin2ascii(c2);
      // bin2ascii(c3);

      // Store the individual ASCII values
      out_buff[out_count + 0] = c0;
      out_buff[out_count + 1] = c1;
      switch (bytes) {
        case 3:
          bin2ascii(c2);
          bin2ascii(c3);
          out_buff[out_count + 2] = c2;
          out_buff[out_count + 3] = c3;
          break;
       case 2:
          bin2ascii(c2);
          out_buff[out_count + 2] = c2;
          out_buff[out_count + 3] = '=';
          break;
       case 1:
          out_buff[out_count + 2] = '=';
          out_buff[out_count + 3] = '=';
          break;
       }
       out_count = out_count + 4;
    }

     write(1, (void *) out_buff, out_count);

  } while (1==1);
  return 0;
}

