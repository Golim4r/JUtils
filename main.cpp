#include <iostream>
#include <thread>
#include "JUtils.h"

char endl = '\n';

int main() {
	JIpsManager im;
	JDurationManager dm;
  JTimedIterationManager tim(16);
	dm.start();
  tim.start();
	//std::this_thread::sleep_for(std::chrono::seconds(1));
	
	for (int i=0; i < 10; ++i) {
		im.update(true);
		//std::this_thread::sleep_for(std::chrono::milliseconds(16));
    tim.wait();
    dm.lap();
		//std::cout << im.ips() << '\n';
	}
	dm.stop();
	dm.print();
	
  return 0;
}
