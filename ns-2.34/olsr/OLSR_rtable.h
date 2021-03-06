 /***************************************************************************
  *   Copyright (C) 2004 by Francisco J. Ros                                *
  *   fjrm@dif.um.es                                                        *
  *                                                                         *
  *   Modified by Weverton Cordeiro                                         *
  *   (C) 2007 wevertoncordeiro@gmail.com                                   *
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   (at your option) any later version.                                   *
  *                                                                         *
  *   This program is distributed in the hope that it will be useful,       *
  *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
  *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
  *   GNU General Public License for more details.                          *
  *                                                                         *
  *   You should have received a copy of the GNU General Public License     *
  *   along with this program; if not, write to the                         *
  *   Free Software Foundation, Inc.,                                       *
  *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
  ***************************************************************************/
 
 ///
 /// \file  OLSR_rtable.h
 /// \brief  Header file for routing table's related stuff.
 ///
 
 #ifndef __OLSR_rtable_h__
 #define __OLSR_rtable_h__
 
 #include <olsr/OLSR_repositories.h>
 #include <trace.h>
 #include <map>
 
 ///
 /// \brief Defines rtable_t as a map of OLSR_rt_entry, whose key is the destination address.
 ///
 /// The routing table is thus defined as pairs: [dest address, entry]. Each element
 /// of the pair can be accesed via "first" and "second" members.
 ///
 typedef std::map<nsaddr_t, OLSR_rt_entry*> rtable_t;
 
 class OLSR;
 
 ///
 /// \brief This class is a representation of the OLSR's Routing Table.
 ///
 class OLSR_rtable {
   rtable_t  rt_;  ///< Data structure for the routing table.
 
 public:
 
   OLSR_rtable();
   ~OLSR_rtable();
 
   void    clear();
   void    rm_entry(nsaddr_t dest);
   OLSR_rt_entry*  add_entry(nsaddr_t dest, nsaddr_t next, nsaddr_t iface, u_int32_t dist);
   OLSR_rt_entry*  lookup(nsaddr_t dest);
   OLSR_rt_entry*  find_send_entry(OLSR_rt_entry*);
   u_int32_t  size();
   void    print(Trace*);
   void    print_debug(OLSR* agent);
 };
 
 #endif


