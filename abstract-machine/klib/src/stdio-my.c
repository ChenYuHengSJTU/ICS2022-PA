#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  int i=0,p=0;
  va_list ap;
  va_start(ap,fmt);
  for(;*(fmt+i)!='\0';++i){
    if(*(fmt+i)=='%'){
      switch(*(fmt+i+1)){
        case 's':
          char* out=va_arg(ap,char*);
          int j=0;
          while(*(out+j)!='\0'){
            putch(*(out+j));
            p++;
            j++;
          }
          break;
        case 'd':
          int n=va_arg(ap,int);
          if(n<0){
            putch('-');
            n=-n;
            p++;
          }
          uint32_t base=1;
          while(n/base){
            base*=10;
          }
          base/=10;
          while(base){
            putch('0'+n/base);
            n-=(n/base)*base;
            base/=10;
            p++;
          }
          break;
        case 'c':
          char ch=va_arg(ap,int);
          putch(ch);
          p++;
          break;
      }
      i++;
      continue;
    }
    putch(*(fmt+i));
    ++p;
  }
  putch('\0');
  return ++p;
  // panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  int i=0,p=0;
  va_list ap;
  va_start(ap,fmt);
  for(;*(fmt+i)!='\0';++i){
    if(*(fmt+i)=='%'){
      switch(*(fmt+i+1)){
        case 's':
          char* _out=va_arg(ap,char*);
          // printf("%s",_out);
          int j=0;
          for(;(*(out+p+j)=*(_out+j))!='\0';++j){
            // printf("%d\t",j);
            // printf("%s\n",out);
          }
          p+=j;
          break;
        case 'd':
          int n=va_arg(ap,int);
          if(n<0){
            *(out+p++)='-';
            n=-n;
          }
          uint32_t base=1;
          while(n/base){
            base*=10;
            // printf("%d ",base);
          }
          base/=10;
          while(base){
            *(out+p)='0'+n/base;
            n-=(n/base)*base;
            base/=10;
            p++;
            // printf("%d ",n);
          }
          break;
      }
      i++;
      continue;
    }
    *(out+p)=*(fmt+i);
    ++p;
  }
  *(out+p++)='\0';
  return p;
//   panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {

  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
