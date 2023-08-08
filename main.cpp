#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include<iostream>
using namespace std;

int main(int argc, char *argv[]) {
   Disk disk_run;

  unsigned char buffer[BLOCK_SIZE];
  Disk::readBlock(buffer,0);
  for(int i=0;i<6;i++){
  cout<<(int)buffer[i];
  }
  
  char message[] = "hello";
  memcpy(buffer + 20, message, 6);
  Disk::writeBlock(buffer, 7000);
  

//   unsigned char buffer2[BLOCK_SIZE];
//   char message2[6];
//   Disk::readBlock(buffer2, 7000);
//   memcpy(message2, buffer2 + 20, 6);
//   std::cout << message2;
// for(int i=0;i<6;i++){
//   cout<<buffer2[i];
// }

  return 0;
}