//#define MATLAB
#define _FORTIFY_SOURCE 0
#include <iostream>
#include "core.h"



int main (int argc, char *argv[])
{

    try
    {
        //setup logging first
        
        core kernel;

        kernel.read_config_file("CHM.config") ;
        
        kernel.run();
    }
    catch( boost::exception& e)
    {
       LOG_ERROR << boost::diagnostic_information(e);
    }



	return 0;
}
