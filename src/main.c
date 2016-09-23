#include "ui.h"
#include "core.h"

EAPI_MAIN int
elm_main(int argc, char **argv)
{
    ecore_init(); 
    elm_init(argc, argv);

    elm_window_create();

    elm_run();
   
    ecore_shutdown();
    elm_shutdown();

    return (0);
}

ELM_MAIN()

