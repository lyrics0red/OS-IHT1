#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

const int buf_size = 5000;

int main(int argc, char** argv) {
  if (argc != 4) {
      printf("Incorrect number of parameters\n");
      exit(-1);
  }
  FILE* input;
  FILE* output;
  size_t mes_size, size1;
  char* instr = argv[1];
  char* outstr = argv[2];
  size_t N = atoi(argv[3]);
  char str[buf_size];
  char str1[buf_size];
  char str2[buf_size];
  char res[buf_size];
  int fd, result;  size_t size;
  char name1[] = "fifo.one";
  char name2[] = "fifo.two";
  mknod(name1, S_IFIFO | 0666, 0);
  result = fork();  
  if(result < 0) {
    printf("Can\'t fork child\n");    
    exit(-1);
  } else if (result > 0) {
    int fdin = open(instr, O_RDONLY);
    if (fdin < 0) {
        printf("Can\'t open input file\n");
        exit(-1);
    }
    mes_size = read(fdin, str, buf_size);
    if (mes_size == -1) {
        printf("Can\'t read this file\n");
        exit(-1);
    }
    if ((fd = open(name1, O_WRONLY)) < 0) {
    	printf("Can\'t open FIFO for writing\n");
    	exit(-1);
    }    
    size = write(fd, str, mes_size);
    if (size != mes_size) {
        printf("Can\'t write all string to FIFO\n");
        exit(-1);
    }
    if(close(fd) < 0) {
      printf("parent: Can\'t close FIFO\n");      
      exit(-1);
    }    
  } else { 
      int fdtmp;
      mknod(name2, S_IFIFO | 0666, 0);
      result = fork();
      if (result < 0) {
        printf("Can\'t fork child\n");    
        exit(-1);
      } else if (result > 0) {
        if(close(fd) < 0){
          printf("child: Can\'t close FIFO\n"); 
          exit(-1);
        }
        if ((fd = open(name1, O_RDONLY)) < 0) {
            printf("Can\'t open FIFO for reading\n");
            exit(-1);
        }
        size = read(fd, str1, buf_size); 
        if(size < 0){
            printf("Can\'t read string FIFO\n");    
            exit(-1);
        }    
        if(close(fd) < 0) {        
          printf("child: Can\'t close FIFO\n"); 
          exit(-1);
        }
        if ((fdtmp = open(name2, O_WRONLY)) < 0) {
          printf("child: Can\'t open FIFO\n"); 
          exit(-1);
        }
        size_t len = 0;
        for (int i = size - N; i >= 0; --i) {
            for (int j = i; j < i + N; ++j) {
                if (j == i + N - 1) {
                    res[len] = str1[j];
                    len++;
                } else if (str1[j] < str1[j + 1]) {
                    res[len] = str1[j];
                    len++;
                } else {
                    len = 0;
                    break;
                }
            }
            if (len == N) {
                break;
            }
        }
        if (len != N) {
            printf("There are no such substring\n");
            exit(-1);
        }
        size1 = write(fdtmp, res, N);
        if (N != size1) {
          printf("Can\'t write all string to FIFO\n");
          exit(-1);
        }
        if(close(fdtmp) < 0) {
          printf("child: Can\'t close writing side of FIFO\n");      
          exit(-1);
        }  
      } else {
        if(close(fdtmp) < 0){
          printf("child: Can\'t close writing side of FIFO\n"); 
          exit(-1);
        }
        if ((fdtmp = open(name2, O_RDONLY)) < 0) {
            printf("Can\'t open FIFO for readind\n");
            exit(-1);
        }
        size = read(fdtmp, str2, buf_size);    
        if(size < 0){
            printf("Can\'t read string from FIFO\n");    
            exit(-1);
        } 
        if(close(fdtmp) < 0) {
          printf("parent: Can\'t close writing side of FIFO\n");      
          exit(-1);
        }  
        remove(outstr);
        int fdout = open(outstr, O_CREAT | O_WRONLY, 0666);
        if (fdout < 0) {
            printf("Can\'t open output file\n");
            exit(-1);
        }
        write(fdout, str2, size);
      }
  }
  return 0;
}
