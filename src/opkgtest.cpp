#include <iostream>

#include "lib/ipkg/Ipkg.h"

int main(int argc, char **argv)
{
	Ipkg *ipkg = new Ipkg();
	ipkg->update();
	ipkg->join();
	ipkg->categoryInit();
	ipkg->categoryDeinit();
	delete ipkg;
	return 0;
}
