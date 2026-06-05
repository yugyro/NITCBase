#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
#include <stdio.h>


int main(int argc, char *argv[]) {
  /* Initialize the Run Copy of Disk */
  Disk disk_run;
  // StaticBuffer buffer;
  // OpenRelTable cache;
  unsigned char buffer[BLOCK_SIZE];
  char message[BLOCK_SIZE];
  Disk::readBlock(buffer,0);
  memcpy(message,buffer,2048);
  for(int i=0;i<BLOCK_SIZE;i++)
  {
    std::cout<<(int)message[i] << " ";
  }

  return 0;

  // return FrontendInterface::handleFrontend(argc, argv);
}