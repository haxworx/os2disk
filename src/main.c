#include "ui.h"
#include "core.h"

/*
distro_t distributions[] = {
    {"test", "http://enform.haxlab.org/files/default.edj"},
    {"Debian GNU/Linux v8.4 (i386/amd64)", "http://gensho.acc.umu.se/debian-cd/8.4.0/multi-arch/iso-cd/debian-8.4.0-amd64-i386-netinst.iso"},
    {"FreeBSD v10.3 (x86)", "http://ftp.freebsd.org/pub/FreeBSD/releases/ISO-IMAGES/10.3/FreeBSD-10.3-RELEASE-i386-memstick.img"},
    {"FreeBSD v10.3 (amd64)", "http://ftp.freebsd.org/pub/FreeBSD/releases/ISO-IMAGES/10.3/FreeBSD-10.3-RELEASE-amd64-memstick.img"},
    {"NetBSD v7.0 (i386)", "http://mirror.planetunix.net/pub/NetBSD/iso/7.0/NetBSD-7.0-i386.iso"},
    {"NetBSD v7.0 (amd64)", "http://mirror.planetunix.net/pub/NetBSD/iso/7.0/NetBSD-7.0-amd64.iso"},
    {"OpenBSD v6.0 (i386)", "http://mirror.ox.ac.uk/pub/OpenBSD/6.0/i386/install60.fs"},
    {"OpenBSD v6.0 (amd64)", "http://mirror.ox.ac.uk/pub/OpenBSD/6.0/amd64/install60.fs"},
    {NULL, NULL},
};

char *storage[] = {
    "custom file...",
    "/dev/sd1c",
    "/dev/null",
    "file.img",
    NULL,
};

*/
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

