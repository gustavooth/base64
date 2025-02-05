#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"

const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const u8 bit_table[] = {128, 64, 32, 16, 8, 4, 2, 1};
void print_byte(u8* str, u32 len) {
  for (u32 i = 0; i < len; i++)
  {
    for (u32 j = 0; j < 8; j++)
    {
      if ((str[i] & bit_table[j]) != 0)
      {
        printf("1");
      }else {
        printf("0");
      }
    }
    printf(" ");
  }
  printf("\n");
}

void print_base64_block(u8* str, u32 len) {
  for (u32 i = 0; i < len; i++)
  {
    for (u32 j = 2; j < 8; j++)
    {
      if ((str[i] & bit_table[j]) != 0)
      {
        printf("1");
      }else {
        printf("0");
      }
    }
    printf(" ");
  }
  printf("\n");
}

b8 base64_index(char c, u8* out_index) {
  for (u8 i = 0; i < 64; i++)
  {
    if (c == base64_chars[i]) {
      *out_index = i;
      return true;
    }
  }
  
  return false;
}

b8 encode_base64(char* str, b8 verbose, char** out_base64_str_ptr) {
  u32 in_len = strlen(str);
  u32 padding = 3 - (in_len % 3);
  if (padding == 3) padding = 0;
  u32 out_len = (4 * (in_len + padding)) / 3;

  char* out_str = (char*)malloc(out_len);
  memset(out_str, 0, out_len);

  for (u32 i = 0; i < in_len / 3; i++)
  {
    u8* blocks = malloc(4);
    memset(blocks, 0, 4);

    u32 l = i * 3;
    u32 j = i * 4;

    blocks[0] = str[l] >> 2;
    out_str[j] =  base64_chars[blocks[0]];

    blocks[1] = str[l] << 6;
    blocks[1] >>= 2;
    blocks[1] |= (str[l+1] >> 4);
    out_str[j+1] =  base64_chars[blocks[1]];

    blocks[2] = str[l+1] << 4;
    blocks[2] >>= 2;
    blocks[2] |= (str[l+2] >> 6);
    out_str[j+2] =  base64_chars[blocks[2]];

    blocks[3] = str[l+2] & (u64)63;
    out_str[j+3] =  base64_chars[blocks[3]];

    if (verbose)
    {
      print_byte((u8*)&str[l], 3);
      print_base64_block(blocks, 4);
      printf("=============================\n");
    }
  }

  u32 rest = in_len % 3;

  if (rest)
  {
    u32 blocks_len = 0;
    if (rest == 1)
    {
      blocks_len = 2;
    }else {
      blocks_len = 3;
    }

    u8* blocks = malloc(blocks_len);
    memset(blocks, 0, blocks_len);

    u32 i = (out_len / 4) - 1;
    u32 l = i * 3;
    u32 j = i * 4;

    blocks[0] = str[l] >> 2;
    out_str[j] =  base64_chars[blocks[0]];

    if (blocks_len == 2)
    {
      blocks[1] = str[l] << 6;
      blocks[1] >>= 2;
      out_str[j+1] =  base64_chars[blocks[1]];
      out_str[j+2] =  '=';
      out_str[j+3] =  '=';
    }else {
      blocks[1] = str[l] << 6;
      blocks[1] >>= 2;
      blocks[1] |= (str[l+1] >> 4);
      out_str[j+1] =  base64_chars[blocks[1]];

      blocks[2] = str[l+1] << 4;
      blocks[2] >>= 2;
      out_str[j+2] =  base64_chars[blocks[2]];
      out_str[j+3] =  '=';
    }

    if (verbose) {
      print_byte((u8*)&str[l], rest);
      print_base64_block(blocks, blocks_len);
      printf("=============================\n");
    }
  }
  

  *out_base64_str_ptr = out_str;

  return true;
}

b8 decode_base64(char* input, b8 verbose, char** out_str_ptr) {
  u32 input_len = strlen(input);
  u32 output_len = (input_len / 4) * 3;
  u32 base = input_len / 4;

  for (u8 i = 0; i < 2; i++)
  {
    if (input[input_len-1] == '=') {
      input_len -= 1;
    }
  }
  
  char* out = (char*)malloc(output_len);

  for (u32 i = 0; i < base; i++)
  {
    char bytes[4] = {0};
    
    u32 l = i * 3;
    u32 j = i * 4;

    for (u32 k = 0; k < 4; k++) base64_index(input[j + k], (u8*)&bytes[k]);

    out[l] = bytes[0] << 2;
    out[l] |= (bytes[1] >> 4);

    out[l+1] = bytes[1] << 4;
    out[l+1] |= (bytes[2] >> 2);

    out[l+2] |= bytes[2] << 6;
    out[l+2] |= bytes[3];

    if (verbose)
    {
      print_base64_block((u8*)bytes, 4);
      print_byte((u8*)&out[l], 3);
      printf("=============================\n");
    }
  }
  
  *out_str_ptr = out;
  return true;
}

int main(int cnt, char** args) {
  if (cnt > 2)
  {
    b8 verbose = false;
    b8 mode = 0;
    u32 str_index;

    for (u32 i = 1; i < cnt; i++)
    {
      if (args[i][0] == '-')
      {
        if (args[i][1] == '-') {
          if (strcmp(&args[i][2], "verbose") == 0) verbose = true;
          else if (strcmp(&args[i][2], "v") == 0) verbose = true;
        } else {
          if (strcmp(&args[i][1], "encode") == 0) mode = 0;
          else if (strcmp(&args[i][1], "decode") == 0) mode = 1;
          else if (strcmp(&args[i][1], "e") == 0) mode = 0;
          else if (strcmp(&args[i][1], "d") == 0) mode = 1;
        }
      }else {
        str_index = i;
        break;
      }
    }

    char* output = 0;
    if (mode) decode_base64(args[str_index], verbose, &output);
    else encode_base64(args[str_index], verbose, &output);
    
    printf("%s\n", output);
    return 0;
  }
  
  printf("Model: -encode: Encode base64\n");
  printf("Model: -decode: Decode base64 to str\n");
  printf("Option: --verbose: Show bits\n\n");
  if (cnt > 0)
  {
    printf("Usage:%s <model> <options> <str>\n", args[0]);
  }else {
    printf("Usage:base64 <model> <options> <str>\n");
  }
  
  return -1;
}