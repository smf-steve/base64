// base64.c: an implementation of base64 
//   - assumes
// usage:
//    base64    // encodes ASCII string on stdin into base64
//    base64 -d // decodes binary string on stdin into ASCII-bytes 

// # Method: 
// #    1. read a block of information
// #       1. load load three bytes from the input location (totaling 24 bits;  3 * 8 == 24)
// #       1. merge bytes into one register      
// #       1. chop the bytes into 4 6-bit chunks
// #       1. map each of the 4 6-bit chunks into an 8-bit ASCII value
// #       1. store each of these 8-bit ASCII values to the output location
// #    2. handle padding
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>

#define byte char

#define _4K_ (4096)

#define size_in_buff   (3072) //  1024 * 3 == 3072 
#define size_out_buff  (3072) //   768 * 4 == 3072 
#define num_in_buff
#define num_out_buff 

byte in_buffer[ _4K_ ];
byte out_buffer[ _4K_ ];

#define map(index)                         \
   switch ( index ) {                      \
     case 0 ... 25  : index +=   0 + 'A' ; \
                      break;               \
     case 26 ... 51 : index += -26 + 'a';  \
                      break;               \
     case 52 ... 61 : index += -52 + '0';  \
                      break;               \
     case 62        : index = '+';         \
                      break;               \
     case 63        : index = '/';         \
                      break;               \
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

     // process three bytes at a time
     // but don't process the last row, which has 1, 2, or 3 byte
     int tmp;
     for (in_count=0 ; tmp=in_count + 3, tmp < num; in_count += 3) {

        // load 24 bits, three bytes
        b0 = in_buff[in_count + 0];
        b1 = in_buff[in_count + 1];
        b2 = in_buff[in_count + 2];

        // merge bytes
        b0 =  (b0 << 16) | (b1 << 8) | b2;
        //b0 = (((b0 << 8) | b1 )<< 8) | b2;   // better since I can reuse a reg and shift value in ARM

        // chop bytes
        c0 = (b0 >> 18);           //  c0 = (b0 >> 18) & 0x3F;
        c1 = (b0 >> 12) & 0x3F;    //  c1 = (b0 >> 12) & 0x3F;
        c2 = (b0 >>  6) & 0x3F;    //  c2 = (b0 >>  6) & 0x3F;
        c3 = (b0 & 0x3F);          //  c3 = (b0 >>  0) & 0x3F;

        // map bytes
        map(c0);
        map(c1);
        map(c2);
        map(c3);

        // store bytes
        out_buff[out_count + 0] = c0;
        out_buff[out_count + 1] = c1;
        out_buff[out_count + 2] = c2;
        out_buff[out_count + 3] = c3;        

        out_count = out_count + 4;
     }

     // Handle the last row that might need padding
     // This is a bug.. if we have a large file..
     // I.e., always go into this section
     if (size_in_buff != num) {
       b0 = in_buff[in_count + 0];

       bytes = num - in_count;
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
       // merge bytes
       b0 = (b0 << 16) | (b1 << 8) | b2;

       // chop bytes
       c0 = (b0 >> 18);           //  c0 = (b0 >> 18) & 0x3F;
       c1 = (b0 >> 12) & 0x3F;    //  c1 = (b0 >> 12) & 0x3F;
       c2 = (b0 >>  6) & 0x3F;    //  c2 = (b0 >>  6) & 0x3F;
       c3 = (b0 & 0x3F);          //  c3 = (b0 >>  0) & 0x3F;

       // map bytes
       map(c0);
       map(c1);
       // map(c2);
       // map(c3);

       // store bytes
       out_buff[out_count + 0] = c0;
       out_buff[out_count + 1] = c1;
       switch (bytes) {
         case 3:
             map(c2);
             map(c3);
             out_buff[out_count + 2] = c2;
             out_buff[out_count + 3] = c3;
             break;
         case 2:
             map(c2);
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




#define unmap(index)                       \
   switch ( index ) {                      \
     case 0 ... 25  : index +=   0 + 'A' ; \
                      break;               \
     case 26 ... 51 : index += -26 + 'a';  \
                      break;               \
     case 52 ... 61 : index += -52 + '0';  \
                      break;               \
     case 62        : index = '+';         \
                      break;               \
     case 63        : index = '/';         \
                      break;               \
   }




int decode (byte * in_buff, byte * out_buff) {
  int b0;  // The three output bytes
  int b1;
  int b2;
  int c0;  // The four input characters
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

    // process four bytes at a time
    // but don't process the last row
    int tmp;
    for (in_count=0 ; tmp=in_count + 4, tmp < num; in_count += 4) {

      // load 4 bytes
      c0 = in_buff[in_count + 0];
      c1 = in_buff[in_count + 1];
      c2 = in_buff[in_count + 2];
      c3 = in_buff[in_count + 3];

      // unmap bytes
      unmap(c0);
      unmap(c1);
      unmap(c2);
      unmap(c3);

      // 
      // merge bytes
      b0 =  (c0 << 18) | (c1 << 12) | (c2 << 6) | c3;

      // chop bytes
      b0 = (b0 >> 16);          //  c0 = (b0 >> 16) & 0xFF;
      b1 = (b0 >> 8) & 0xFF;    //  c1 = (b0 >>  8) & 0xFF;
      b3 = b0 & 0xFF;           //  c2 = (b0 >>  0) & 0xFF;

      // store bytes
      out_buff[out_count + 0] = b0;
      out_buff[out_count + 1] = b1;
      out_buff[out_count + 2] = b2;

      out_count = out_count + 3;
    }

    {
      // Handle the last row that might need padding

      // load 4 bytes
      c0 = in_buff[in_count + 0];
      c1 = in_buff[in_count + 1];
      c2 = in_buff[in_count + 2];
      c3 = in_buff[in_count + 3];

      bytes = 3;
      // unmap bytes
      unmap(c0);
      unmap(c1);

      if (c3 != '=') {
        unmap(c3);
      else
        c3 = 0;
        bytes = 1;
      }
      if (c2 != '=') {
        unmap(c2);
      else
        c2 = 0;
        bytes = 2;
      }
      hexad : group of six
      octad : group of eight

      // merge bytes
      b0 =  (c0 << 18) | (c1 << 12) | (c2 << 6) | c3;

      // chop bytes
      b0 = (b0 >> 16);          //  c0 = (b0 >> 16) & 0xFF;
      b1 = (b0 >> 8) & 0xFF;    //  c1 = (b0 >>  8) & 0xFF;
      b3 = b0 & 0xFF;           //  c2 = (b0 >>  0) & 0xFF;

      // store bytes
      out_buff[out_count + 0] = b0;
      out_buff[out_count + 1] = b1;
      out_buff[out_count + 2] = b2;

      // Here we need to print out only the correct number of chars
      out_count = out_count + bytes;
     }
     write(1, (void *) out_buff, out_count);
   } while (1==1);
   return 0;
}




int main() {
  encode(in_buffer, out_buffer);
  printf("\n");
  return 0;
}



