#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"

// local headers
#include <iostream>

int main(int argc, char *argv[])
{
	Disk disk_run;
	StaticBuffer bufferCache;
	OpenRelTable cache;

	return FrontendInterface::handleFrontend(argc, argv);
}