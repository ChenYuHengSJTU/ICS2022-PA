#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  int i=0;
  for(;*(s+i)!='\0';++i){}
  return i;
  // panic("Not implemented");
}

char *strcpy(char *dst, const char *src) {
  // int i=0;
  // for(;(*(dst+i)=*(src+i))!='\0';++i){}
  // return dst;
  size_t i = 0;
  for(; src[i] != '\0';++i){
    // printf("%d ",i);
    dst[i] = src[i];
  }
  dst[i] = '\0';
  return dst;
  // panic("Not implemented");
}

char *strncpy(char *dst, const char *src, size_t n) {
  int i=0;
  for(;i<n;++i){
    *(dst+i)=*(src+i);
  }
  return dst;
  // panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  int i=0,j=0;
  for(;*(dst+i)!='\0';++i){}
  for(;(*(dst+i)=*(src+j))!='\0';++i,++j){}
  return dst;
  // panic("Not implemented");
}

char *strncat(char *dst, const char *src, size_t n) {
  size_t pos_dst = strlen(dst);
  size_t pos_src = 0;
  for(; (pos_src) < n && (src[pos_src] != '\0'); pos_src++, pos_dst++)
    dst[pos_dst] = src[pos_src];
  dst[pos_dst] = '\0';

  return dst;
}

int strcmp(const char *s1, const char *s2) {
  // int len1=strlen(s1),len2=strlen(s2);
  // if(len1<len2) return -1;
  // else if(len1>len2) return 1;
  // int i=0;
  // for(;i<len1;++i){
  //   if(*(s1+i)<*(s2+i)) return -1;
  //   else if(*(s1+i)>*(s2+i)) return 1;
  // }
  // return 0;
  if(s1==NULL){
    if(s2==NULL) return 0;
    else return -1;
  }
  if(s2==NULL){
    if(s1==NULL) return 0;
    else return 1;
  }
  int i;
  for (i = 0; s1[i] != '\0' && s2[i] != '\0'; ++i){
    if (s1[i] != s2[i])
      return (int)(s1[i]) - (int)(s2[i]);
  }
  return (int)(s1[i]) - (int)(s2[i]);
  // panic("Not implemented");
}

int strncmp(const char *s1, const char *s2, size_t n) {
  int i=0;
  for(;i<n;++i){
    if(*(s1+i)<*(s2+i)) return -1;
    else if(*(s1+i)>*(s2+i)) return 1;
  }
  return 0;
  // panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  int i=0;
  for(;i<n;++i){
    *(char*)(s+i)=c;
  }
  return s;
  // panic("Not implemented");
}

void *memmove(void *dst, const void *src, size_t n) {
  // return dst;
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  // int i=0;
  // for(;i<n&&*(char*)(out+i)!='\0';++i){
  //   *(char*)(out+i)=*(char*)(in+i);
  // }
  // return out;
  char *char_out = (char *)out;
  const char *char_in = (const char *)in;
  for (int i = 0; i < n; ++i){
    char_out[i] = char_in[i];
  }

  return out;
  // panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  int i=0;
  for(;i<n;++i){
    if(*(char*)(s1+i)<*(char*)(s2+i)) return -1;
    else if(*(char*)(s1+i)>*(char*)(s2+i)) return 1;
  }
  return 0;
  // panic("Not implemented");
}

#endif
