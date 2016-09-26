#include "ui.h"

Eina_Bool
system_check_changes(void *data)
{
    (void) data;
    int count = system_get_disks();
    if (count) {
        ecore_timer_del(timer);
        timer = NULL;
    }

    return ECORE_CALLBACK_RENEW;
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
    ecore_init(); 
    elm_init(argc, argv);

    if (!system_get_disks()) {
        /* scan for disks real-time */ 
        timer = ecore_timer_add(3.0, system_check_changes, NULL);
    }

    elm_window_create();

    ecore_main_loop_begin();
   
    ecore_shutdown();
    elm_shutdown();

    return (0);
}

ELM_MAIN()

