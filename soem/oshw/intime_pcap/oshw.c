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
   struct ifaddrs *interfaces = NULL;
   struct ifaddrs *ifa = NULL;
   ec_adaptert * adapter;
   ec_adaptert * prev_adapter = NULL;
   ec_adaptert * ret_adapter = NULL;
   int i = 0;

   /* Get network interface addresses using INtime getifaddrs API */
   if (getifaddrs(&interfaces) != 0)
   {
      /* Error getting interface addresses */
      return NULL;
   }

   /* Iterate through all interfaces */
   for (ifa = interfaces; ifa != NULL; ifa = ifa->ifa_next)
   {
      /* Skip interfaces that are down or don't have an address */
      if (!(ifa->ifa_flags & IFF_UP) || ifa->ifa_addr == NULL)
      {
         continue;
      }

      /* Only include Ethernet interfaces (exclude loopback) */
      if (ifa->ifa_flags & IFF_LOOPBACK)
      {
         continue;
      }

      /* Only include Ethernet interfaces to avoid duplicates */
      if (ifa->ifa_addr->sa_family != AF_LINK)
      {
         continue;
      }

      /* Skip virtual network interfaces that start with "ven" */
      if (ifa->ifa_name && strncmp(ifa->ifa_name, "ven", 3) == 0)
      {
         continue;
      }

      /* Allocate memory for adapter structure */
      adapter = (ec_adaptert *)malloc(sizeof(ec_adaptert));
      if (adapter == NULL)
      {
         /* Clean up and return what we have so far */
         if (interfaces)
            freeifaddrs(interfaces);
         return ret_adapter;
      }

      /* If we got more than one adapter save link list pointer to previous
       * adapter. Else save as pointer to return.
       */
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
      if (ifa->ifa_name)
      {
         strncpy(adapter->name, ifa->ifa_name, EC_MAXLEN_ADAPTERNAME);
         adapter->name[EC_MAXLEN_ADAPTERNAME-1] = '\0';
      }
      else
      {
         adapter->name[0] = '\0';
      }

      /* Create description - use interface name and flags information */
      snprintf(adapter->desc, EC_MAXLEN_ADAPTERNAME, "%s",
               ifa->ifa_name ? ifa->ifa_name : "unknown");
      adapter->desc[EC_MAXLEN_ADAPTERNAME-1] = '\0';

      prev_adapter = adapter;
      i++;
   }

   /* Free the interfaces structure */
   if (interfaces)
      freeifaddrs(interfaces);

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