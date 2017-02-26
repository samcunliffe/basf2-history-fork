/*ca2nsm.c*/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cadef.h"
#include "dbDefs.h"
#include "epicsString.h"
#include "cantProceed.h"

#include "ca2nsm.h"
#include "ca2nsm_callback.h"

int main(int argc, char **argv)
{
  if (argc > 1) {
    init_ca2nsm(argv[1]);
    while (1) {
      //SEVCHK(
      ca_pend_event(0.1);
      //,"ca_pend_event");
      add_PVs();
    }
  } else {
    printf("Usage %s : <config>\n", argv[0]);
  }
  return 0;
}
