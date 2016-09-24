#include "ui.h"
#include "core.h"

Eina_Bool
system_check_changes(void *data)
{
    (void) data;
    system_get_disks();

    return ECORE_CALLBACK_RENEW;
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
    ecore_init(); 
    elm_init(argc, argv);

    system_get_disks();

    elm_window_create();
   
    /* scan for disks real-time */ 
    timer = ecore_timer_add(3.0, system_check_changes, NULL);

    ecore_main_loop_begin();
   
    ecore_shutdown();
    elm_shutdown();

    return (0);
}

ELM_MAIN()

