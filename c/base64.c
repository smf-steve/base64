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

     // process the bytes three at a time
     for (in_count=0 ; in_count + 3 <= num; in_count += 3) {
        // load three bytes
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

     // Handle the padding
     if (size_in_buff != num) {
       fprintf(stderr, "\n\n%d, %d\n", size_in_buff, num);
       b0 = in_buff[in_count + 0];
       b1 = in_buff[in_count + 1];
       b2 = in_buff[in_count + 2];

       bytes = num % 3;
       switch (bytes) {
         case 2:
             b2 = 0;
             // merge;
         case 1:
             b1 = 0;
             // merge
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

       // store bytes
       out_buff[out_count + 0] = c0;
       out_buff[out_count + 1] = c1;
       switch (bytes) {
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
     fprintf(stderr, "\n%d, %d\n", in_count, out_count);
     write(1, (void *) out_buff, out_count);
   } while (1==1);
   return 0;
}




int main() {
  encode(in_buffer, out_buffer);
  return 0;
}

/*
1020 = 12 * 85;



## Major Entry Points:
read 24 bytes -- 3 words
  - expand to 4 regs
  - extract 4\*4 6-bit sequence
  - contruct 16 bytes 
  - write out 4 words

1. base64(fd_in, fd_out)
   -- a0: in_buff
   -- a1: out_buff

   1020  immediate
   3     
   12    2^8 + 2^4
   c
   loop_count
   retval


   do {
     retval = read(0, buff, 1020);

     // Determine the number out outut buts
     out = retval / 3 * 4;

     // Proceses 12 bytes at a time
     if (retval >= 12) {
       c=0;
       for(; c < retval ;) {
          load12  load_3w_to_4r
            lw $t0
            lw $t1
            lw $t3

            $t0 = $t0 >>> 8
            $t1 = ($t0 << 16) | ($t1 >>> 16)
            $t2 = ($t1 << 8 ) | ($t3 >>> 24 )
            $t3 = $t3


            // Process 1 word out of 4
            chop3  $t0  -> $t4, t5, t6
            map3   t4, t5, t6
               // merge
               // sw

 			// Process 2 worke out of 4
            chop3 $t1 -> $t7 t8 t9
            map3  t7 t8 t9
         

                //--
               merge4 t0, t4, t5, t6, t7
               sw t0 0(a1)

            // Process 3 word out of 4
          	chop3 $t2 ->  t4, t5, t6
          	map3 t4 t5 t6
             	//--
             	merge3 t1, t8, t9, t4, t5
            	sw t1 4(a1)

          	// Process 4 word out of 4
            chop3 $t3 ->  t7, t8, t9
            map3 t7 t8 t9
                 //-
                merge4 t2 t6 t7 t8 t9

            sw t2 8(a1)
          
            merge3 t3 t6 t7 t8 t9
            sw t3 12()

          c+=4;
        }
       retval    = retval - c;

     }

     // Process 3 bytes at a time
     if (c < retval) {
        for(; c < retval ;) {
          load3
            lb $t0   0(in_buff)

            lb $t1   1(in_buff)
            $t0 = $t0 << 8;
            $t0 = $t0 | $t1

            lb $t2    2(in_buff)
            $t0 = $t0 << 8;
            $t0 = $t0 | $t2
          
          chop3
            $t2 = $t0 & 0x3F 
            $t1 = $t0 >> 6  & 0x3F
            $t0 = $t0 >> 12

          map $t0
          sb $t0  0(out_buff)

          map $t1
          sb $t1  1(out_buff)

          map $t2
          sb $t2  2(out_buff)

          c++;
       }
     }

     if (retval >= 0) {
     // need to calculate the number of bytes to print
     // ratio is 6 ->  hance:  retval / 3 * 4 + 
     padding retval % 3;
     write(1, buff, retval)
   } while (retval > 0);


1. load12:
   - loads 12 bytes into 3 registers
     - 3: lw operations into 
     - unpacks 3 register into 4 registers

1. load3
   - loads 3 bytes into 1 register
     - 3 lb operations 
     - merge 3 registers into 1 register


 1. chop: chops the register into 4 6-bit sequence
      split:  mask
      slide:  shift

 1. map:    

 1. merge: place 4 ASCII values into a single register
     - shift:  into correct position
     - meld:   and



 1. padding


====
Alternative algorithm...
   read in word..
   write algorithm to work with top 24bit.
   shift last byte to position three, llh
   write algorithm to work thh lower half
     -- shift 8 instead

   this algoirth wold flip back and forth between
      lw, lh, lw, lh, lw

Alternative algorithm
  - double the one above  read_24 at at time
    - 6 lw --> 8 registers
    - 8 output registers

*/
