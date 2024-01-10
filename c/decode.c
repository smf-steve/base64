
#define byte char
int decode(byte * in_buff, byte * out_buff);


#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>

#define _4K_ (4096)

#define size_in_buff   (3072) //  1024 * 3 == 3072 
#define size_out_buff  (3072) //   768 * 4 == 3072 

#define offset_A (  0 + 'A')
#define offset_a (-26 + 'a')
#define offset_0 (-52 + '0')

#define ascii2bin(index)               \
switch ( index ) {                     \
case 'A' ... 'Z' : index -=  0 + 'A' ; \
  break;                               \
case 'a' ... 'z' : index -= -26 + 'a'; \
  break;                               \
case '0' ... '9' : index -= -52 + '0'; \
  break;                               \
case '+'         : index = 62;         \
  break;                               \
case '/'         : index = 63;         \
  break;                               \
}


int decode (byte * in_buff, byte * out_buff) {
  int b0;  // The three output bytes
  int b1;
  int b2;
  int c0;  // The four input characters
  int c1;
  int c2;
  int c3;

  int num;              // The number of bytes to process in each BLOCK
  int in_count;         // The input byte number being processed within the BLOCK
  int out_count;        // The output byte number being processed within the BLOCK
  int output_bytes;     // The number of output bytes for the current ROW

  do {
    in_count  = 0;
    out_count = 0;

    num = read(0, (void *) in_buff, size_in_buff);
    if (num == 0) break;

    // Process four bytes at a time
    // but don't process the last row

    // Process four bytes at a time -- but don't process the last row
    // The last row may need to have padding removed.
    output_bytes = 3;
    for (in_count=0 ; in_count < num; in_count += 4) {

      // Load the first ASCII characters
      c0 = in_buff[in_count + 0];
      if (c0 == '\n' ) {
        // skip over any blank lines
        break;
      }
      // Load the next 3 ASCII characters
      c0 = in_buff[in_count + 0];
      c1 = in_buff[in_count + 1];
      c2 = in_buff[in_count + 2];
      c3 = in_buff[in_count + 3];

      // Unmap the ASCII characters, creating four groups of 6-bit values

      ascii2bin(c0);
      ascii2bin(c1);

      if (c3 != '=') {
        ascii2bin(c3);
      } else {
        c3 = 0;
        output_bytes = 1;
      }
      if (c2 != '=') {
        ascii2bin(c2);
      } else {
        c2 = 0;
        output_bytes = 2;
      }

      // Merge the four 6-bit values into a single 24-bit value 
      c0 = (c0 << 18) | (c1 << 12) | (c2 << 6) | c3;

      // Chop the 24-bit value into three groups of 8-bits
      b0 = (c0 >> 16);          //  b0 = (c0 >> 16) & 0xFF;
      b1 = (c0 >> 8) & 0xFF;    //  b1 = (c0 >>  8) & 0xFF;
      b2 = c0 & 0xFF;           //  b2 = (c0 >>  0) & 0xFF;

      // Store the individual 8-bit values
      out_buff[out_count + 0] = b0;
      out_buff[out_count + 1] = b1;
      out_buff[out_count + 2] = b2;

      // Here we need to print out only the correct number of chars
      out_count = out_count + output_bytes;
    }
    write(1, (void *) out_buff, out_count);

  } while (1==1);
  return 0;
}

