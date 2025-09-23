/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#include <sys/endian.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "oshw.h"
#include "hpeif2.h"

#define MAX_NIC_INSTANCE 4

/**
 * Host to Network byte order (i.e. to big endian).
 *
 * Note that Ethercat uses little endian byte order, except for the Ethernet
 * header which is big endian as usual.
 */
uint16 oshw_htons (uint16 host)
{
   uint16 network = htons (host);
   return network;
}

/**
 * Network (i.e. big endian) to Host byte order.
 *
 * Note that Ethercat uses little endian byte order, except for the Ethernet
 * header which is big endian as usual.
 */
uint16 oshw_ntohs (uint16 network)
{
   uint16 host = ntohs (network);
   return host;
}

/* Create list over available network adapters.
 * @return First element in linked list of adapters
 */
ec_adaptert * oshw_find_adapters (void)
{
   ec_adaptert * adapter;
   ec_adaptert * prev_adapter = NULL;
   ec_adaptert * ret_adapter = NULL;
   int i = 0;

   /* List of supported HPE interface names */
   const char* supported_interfaces[] = {"ie1g", "rtl1g", "rtl1gl"};
   int num_supported = sizeof(supported_interfaces) / sizeof(supported_interfaces[0]);

   /* Probe each supported interface type up to MAX_NIC_INSTANCE */
   for (int interface_idx = 0; interface_idx < num_supported; interface_idx++)
   {
      for (int instance = 0; instance < MAX_NIC_INSTANCE; instance++)
      {
         char ifname[64];
         HPEHANDLE handle;
         HPESTATUS status;

         /* Create interface name with instance number */
         snprintf(ifname, sizeof(ifname), "%s%d", supported_interfaces[interface_idx], instance);

         /* Try to open the HPE interface with PROBE_ONLY to check if it exists */
         status = hpeOpen(ifname, SPEED_1000 | DUPLEX_FULL | PROBE_ONLY, NO_INTERRUPT, &handle);

         if (status == E_OK)
         {
            /* Close the handle immediately since we're only probing */
            hpeClose(handle);

            /* Allocate memory for adapter structure */
            adapter = (ec_adaptert *)malloc(sizeof(ec_adaptert));
            if (adapter == NULL)
            {
               /* Return what we have so far */
               return ret_adapter;
            }

            /* Link the adapter to the list */
            if (i)
            {
               prev_adapter->next = adapter;
            }
            else
            {
               ret_adapter = adapter;
            }

            /* Initialize the adapter structure */
            adapter->next = NULL;

            /* Copy interface name */
            strncpy(adapter->name, ifname, EC_MAXLEN_ADAPTERNAME);
            adapter->name[EC_MAXLEN_ADAPTERNAME-1] = '\0';

            /* Create description */
            snprintf(adapter->desc, EC_MAXLEN_ADAPTERNAME, "%s", ifname);
            adapter->desc[EC_MAXLEN_ADAPTERNAME-1] = '\0';

            prev_adapter = adapter;
            i++;
         }
      }
   }

   return ret_adapter;
}

/** Free memory allocated memory used by adapter collection.
 * @param[in] adapter = First element in linked list of adapters
 * EC_NOFRAME.
 */
void oshw_free_adapters (ec_adaptert * adapter)
{
   ec_adaptert * next_adapter;
   /* Iterate the linked list and free all elements holding
    * adapter information
    */
   if(adapter)
   {
      next_adapter = adapter->next;
      free (adapter);
      while (next_adapter)
      {
         adapter = next_adapter;
         next_adapter = adapter->next;
         free (adapter);
      }
   }
}
