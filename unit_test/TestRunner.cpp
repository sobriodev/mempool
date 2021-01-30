#include "TestRunner.h"

/* ------------------------------------------------------------ */
/* ---------------------- Api functions ----------------------- */
/* ------------------------------------------------------------ */

int main(int argc, char** argv)
{
    return CommandLineTestRunner::RunAllTests(argc, argv);
}
