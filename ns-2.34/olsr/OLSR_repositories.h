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
 /// \file  OLSR_repositories.h
 /// \brief  Here are defined all data structures needed by an OLSR node.
 ///
 
 #ifndef __OLSR_repositories_h__
 #define __OLSR_repositories_h__
 
 #include <olsr/OLSR_pkt.h>
 #include <olsr/OLSR_parameter.h>
 #include <math.h>
 #include <set>
 #include <vector>
 
 /// Gets current time from the scheduler.
 #define CURRENT_TIME  Scheduler::instance().clock()
 
 /// An %OLSR's routing table entry.
 typedef struct OLSR_rt_entry {
   nsaddr_t  dest_addr_;  ///< Address of the destination node.
   nsaddr_t  next_addr_;  ///< Address of the next hop.
   nsaddr_t  iface_addr_;  ///< Address of the local interface.
   u_int32_t  dist_;    ///< Distance in hops to the destination.
 
   inline nsaddr_t&  dest_addr()  { return dest_addr_; }
   inline nsaddr_t&  next_addr()  { return next_addr_; }
   inline nsaddr_t&  iface_addr()  { return iface_addr_; }
   inline u_int32_t&  dist()    { return dist_; }
 } OLSR_rt_entry;
 
 /// An Interface Association Tuple.
 typedef struct OLSR_iface_assoc_tuple {
   /// Interface address of a node.
   nsaddr_t  iface_addr_;
   /// Main address of the node.
   nsaddr_t  main_addr_;
   /// Time at which this tuple expires and must be removed.
   double    time_;
 
   inline nsaddr_t&  iface_addr()  { return iface_addr_; }
   inline nsaddr_t&  main_addr()  { return main_addr_; }
   inline double&    time()    { return time_; }
 } OLSR_iface_assoc_tuple;
 
 #define DEFAULT_LOSS_WINDOW_SIZE   10
 
 /// Maximum allowed sequence number.
 #define OLSR_MAX_SEQ_NUM     65535
 
 /// A Link Tuple.
 typedef struct OLSR_link_tuple {
   /// Interface address of the local node.
   nsaddr_t        local_iface_addr_;
   /// Interface address of the neighbor node.
   nsaddr_t        nb_iface_addr_;
   /// The link is considered bidirectional until this time.
   double          sym_time_;
   /// The link is considered unidirectional until this time.
   double          asym_time_;
   /// The link is considered lost until this time (used for link layer notification).
   double          lost_time_;
   /// Time at which this tuple expires and must be removed.
   double          time_;
 
   inline nsaddr_t&  local_iface_addr()  { return local_iface_addr_; }
   inline nsaddr_t&  nb_iface_addr()    { return nb_iface_addr_; }
   inline double&    sym_time()    { return sym_time_; }
   inline double&    asym_time()    { return asym_time_; }
   inline double&    lost_time()    { return lost_time_; }
   inline double&    time()      { return time_; }
 
   /// Link quality extension
   unsigned char   loss_bitmap_ [16];
   u_int16_t       loss_seqno_;
   int             loss_index_;
   int             loss_window_size_;
   int             lost_packets_;
   int             total_packets_;
   int             loss_missed_hellos_;
   double          next_hello_;
   double          hello_ival_;
 
   double          link_quality_;
   double          nb_link_quality_;
   double          etx_;
 
   /// Link Quality extension Methods
   inline void link_quality_init (u_int16_t seqno, int loss_window_size) {
     assert (loss_window_size > 0);
 
     memset (loss_bitmap_, 0, sizeof (loss_bitmap_));
     loss_window_size_ = loss_window_size;
     loss_seqno_ = seqno;
 
     total_packets_ = 0;
     lost_packets_ = 0;
     loss_index_ = 0;
     loss_missed_hellos_ = 0;
 
     link_quality_ = 0;
     nb_link_quality_ = 0;
     etx_ = 0;
   }
 
   inline double link_quality () { return link_quality_; }
   inline double nb_link_quality() { return nb_link_quality_; }
   inline double etx() { return etx_; }
 
   inline double& next_hello () { return next_hello_; }
 
   inline double& hello_ival () { return hello_ival_; }
 
   inline void update_link_quality (double nb_link_quality) {
     // update link quality information
     nb_link_quality_ = nb_link_quality;
 
     // calculate the new etx value
     double mult = link_quality_ * nb_link_quality_;
     switch (parameter_.link_quality()) {
     case OLSR_BEHAVIOR_ETX:
       etx_ = (mult < 0.01) ? 100.0 : 1.0 / mult;
       break;
 
     case OLSR_BEHAVIOR_ML:
       etx_ = mult;
       break;
 
     case OLSR_BEHAVIOR_NONE:
     default:
       etx_ = 0.0;
       break;
     }
   }
 
   inline void process_packet (bool received, double htime) {
     // Code extracted from olsrd implementation
     unsigned char mask = 1 << (loss_index_ & 7);
     int index = loss_index_ >> 3;
     if (received) {
       // packet not lost
       if ((loss_bitmap_[index] & mask) != 0) {
         // but the packet that we replace was lost
         // => decrement packet loss
         loss_bitmap_[index] &= ~mask;
         lost_packets_--;
       }
       hello_ival_ = htime;
       next_hello_ = CURRENT_TIME + hello_ival_;
     }
     else { // if (!received)
       // packet lost
       if ((loss_bitmap_[index] & mask ) == 0) {
         // but the packet that we replace was not lost
         // => increment packet loss
         loss_bitmap_[index] |= mask;
         lost_packets_++;
       }
     }
     // move to the next packet
     // wrap around at the end of the packet loss window
     loss_index_ = (loss_index_ + 1) % (loss_window_size_);
     // count the total number of handled packets up to the window size
     if (total_packets_ < loss_window_size_)
       total_packets_++;
 
     // calculate the new link quality
     //
     // start slowly: receive the first packet => link quality = 1 / n
     //               (n = window size)
     link_quality_ = (double)(total_packets_ - lost_packets_) /
                      (double)(loss_window_size_);
 
     // calculate the new etx value
     double mult = link_quality_ * nb_link_quality_;
     switch (parameter_.link_quality()) {
     case OLSR_BEHAVIOR_ETX:
       etx_ = (mult < 0.01) ? 100.0 : 1.0 / mult;
       break;
 
     case OLSR_BEHAVIOR_ML:
       etx_ = mult;
       break;
 
     case OLSR_BEHAVIOR_NONE:
     default:
       etx_ = 0.0;
       break;
     }
   }
 
   inline void receive (u_int16_t seqno, double htime) {
     while (seqno != loss_seqno_) {
       // have already considered all lost HELLO messages?
       if (loss_missed_hellos_ == 0)
         process_packet (false, htime);
       else
         // if not, then decrement the number of lost HELLOs
         loss_missed_hellos_--;
       // set loss_seqno_ to the next expected sequence number
       loss_seqno_ = (loss_seqno_ + 1) % (OLSR_MAX_SEQ_NUM + 1);
     }
     // we have received a packet, otherwise this function would not
     // have been called
     process_packet (true, htime);
     // (re-)initialize
     loss_missed_hellos_ = 0;
     loss_seqno_ = (seqno + 1) % (OLSR_MAX_SEQ_NUM + 1);
   }
 
   inline void packet_timeout () {
     process_packet (false, 0.0);
     // memorize that we've counted the packet, so that we do not
     // count it a second time later
     loss_missed_hellos_++;
     // Schedule next timeout...
     next_hello_ = CURRENT_TIME + hello_ival_;
   }
 
   /// Link delay extension
   double link_delay_;
   double nb_link_delay_;
   double recv1_ [CAPPROBE_MAX_ARRAY];
   double recv2_ [CAPPROBE_MAX_ARRAY];
 
   inline void link_delay_init () {
 
     link_delay_ = 4;
     nb_link_delay_ = 6;
     for (int i = 0; i < CAPPROBE_MAX_ARRAY; i++)
       recv1_ [i] = recv2_ [i] = -1;
   }
 
   inline void link_delay_computation (OLSR_pkt* pkt) {
     double c_alpha = parameter_.c_alpha();
     int i;
 
     if (pkt->sn() % CAPPROBE_MAX_ARRAY % 2 == 0) {
       i = ((pkt->sn() % CAPPROBE_MAX_ARRAY) - 1) / 2;
       recv2_[i] = CURRENT_TIME - pkt->send_time();
       if (recv1_[i] > 0) {
         double disp = recv2_[i] - recv1_[i];
         // if (disp > 0 && recv1_[i] - disp > 0) {
         //  double delay = recv1_[i] - disp;
         //  link_delay_ = (link_delay_ == 1) ? delay : c_alpha * delay +
         //                 (1 - c_alpha) * (link_delay_);
         // }
         if (disp > 0)
           link_delay_ = /*(link_delay_ == 1) ? disp : */c_alpha * disp +
                          (1 - c_alpha) * (link_delay_);
       }
       recv1_[i] = -1;
       recv2_[i] = -1;
     } else {
       i = pkt->sn() % CAPPROBE_MAX_ARRAY / 2;
 
       recv1_[i] = CURRENT_TIME - pkt->send_time();
       recv2_[i] = 0;
     }
 
   }
 
   inline double link_delay() { return link_delay_; }
   inline double nb_link_delay() { return nb_link_delay_; }
 
   inline void update_link_delay (double nb_link_delay) {
     nb_link_delay_ = nb_link_delay;
   }
 
   inline void mac_failed () {
     /// Link quality extension
     memset (loss_bitmap_, 0, sizeof (loss_bitmap_));
 
     total_packets_ = 0;
     lost_packets_ = 0;
     loss_index_ = 0;
     loss_missed_hellos_ = 0;
 
     /// Link delay extension
     link_delay_ = 10;
     nb_link_delay_ = 10;
     for (int i = 0; i < CAPPROBE_MAX_ARRAY; i++)
       recv1_ [i] = recv2_ [i] = -1;
   }
 
 } OLSR_link_tuple;
 
 /// A Neighbor Tuple.
 typedef struct OLSR_nb_tuple {
   /// Main address of a neighbor node.
   nsaddr_t nb_main_addr_;
   /// Neighbor Type and Link Type at the four less significative digits.
   u_int8_t status_;
   /// A value between 0 and 7 specifying the node's willingness to carry traffic on behalf of other nodes.
   u_int8_t willingness_;
 
   inline nsaddr_t&  nb_main_addr()  { return nb_main_addr_; }
   inline u_int8_t&  status()  { return status_; }
   inline u_int8_t&  willingness()  { return willingness_; }
 } OLSR_nb_tuple;
 
 /// A 2-hop Tuple.
 typedef struct OLSR_nb2hop_tuple {
   /// Main address of a neighbor.
   nsaddr_t  nb_main_addr_;
   /// Main address of a 2-hop neighbor with a symmetric link to nb_main_addr.
   nsaddr_t  nb2hop_addr_;
   /// Time at which this tuple expires and must be removed.
   double    time_;
 
   inline nsaddr_t&  nb_main_addr()  { return nb_main_addr_; }
   inline nsaddr_t&  nb2hop_addr()  { return nb2hop_addr_; }
 
   /// Link quality extension
   double link_quality_;
   double nb_link_quality_;
   double etx_;
 
   inline double  link_quality() { return link_quality_; }
   inline double  nb_link_quality() { return nb_link_quality_; }
   inline double  etx() { return etx_; }
 
   inline void update_link_quality (double link_quality, double nb_link_quality) {
     // update link quality information
     link_quality_ = link_quality;
     nb_link_quality_ = nb_link_quality;
 
     // calculate the new etx value
     double mult = link_quality_ * nb_link_quality_;
     switch (parameter_.link_quality()) {
     case OLSR_BEHAVIOR_ETX:
       etx_ = (mult < 0.01) ? 100.0 : 1.0 / mult;
       break;
 
     case OLSR_BEHAVIOR_ML:
       etx_ = mult;
       break;
 
     case OLSR_BEHAVIOR_NONE:
     default:
       etx_ = 0.0;
       break;
     }
   }
 
   /// Link delay extension
   double  link_delay_;
   double nb_link_delay_;
 
   inline double  link_delay() { return link_delay_; }
   inline double  nb_link_delay() { return nb_link_delay_; }
 
   inline void update_link_delay (double link_delay, double nb_link_delay) {
     link_delay_ = link_delay;
     nb_link_delay_ = nb_link_delay;
   }
 
   inline double&  time()    { return time_; }
 } OLSR_nb2hop_tuple;
 
 /// An MPR-Selector Tuple.
 typedef struct OLSR_mprsel_tuple {
   /// Main address of a node which have selected this node as a MPR.
   nsaddr_t  main_addr_;
   /// Time at which this tuple expires and must be removed.
   double    time_;
 
   inline nsaddr_t&  main_addr()  { return main_addr_; }
   inline double&    time()    { return time_; }
 } OLSR_mprsel_tuple;
 
 /// The type "list of interface addresses"
 typedef std::vector<nsaddr_t> addr_list_t;
 
 /// A Duplicate Tuple
 typedef struct OLSR_dup_tuple {
   /// Originator address of the message.
   nsaddr_t  addr_;
   /// Message sequence number.
   u_int16_t  seq_num_;
   /// Indicates whether the message has been retransmitted or not.
   bool    retransmitted_;
   /// List of interfaces which the message has been received on.
   addr_list_t  iface_list_;
   /// Time at which this tuple expires and must be removed.
   double    time_;
 
   inline nsaddr_t&  addr()    { return addr_; }
   inline u_int16_t&  seq_num()  { return seq_num_; }
   inline bool&    retransmitted()  { return retransmitted_; }
   inline addr_list_t&  iface_list()  { return iface_list_; }
   inline double&    time()    { return time_; }
 } OLSR_dup_tuple;
 
 /// A Topology Tuple
 typedef struct OLSR_topology_tuple {
   /// Main address of the destination.
   nsaddr_t  dest_addr_;
   /// Main address of a node which is a neighbor of the destination.
   nsaddr_t  last_addr_;
   /// Sequence number.
   u_int16_t  seq_;
   /// Time at which this tuple expires and must be removed.
   double    time_;
 
   inline nsaddr_t&  dest_addr()  { return dest_addr_; }
   inline nsaddr_t&  last_addr()  { return last_addr_; }
 
   /// Link quality extension
   double  link_quality_;
   double nb_link_quality_;
   double etx_;
 
   inline double  link_quality() { return link_quality_; }
   inline double  nb_link_quality() { return nb_link_quality_; }
   inline double  etx() { return etx_; }
 
   inline void update_link_quality (double link_quality, double nb_link_quality) {
     // update link quality information
     link_quality_ = link_quality;
     nb_link_quality_ = nb_link_quality;
 
     // calculate the new etx value
     double mult = link_quality_ * nb_link_quality_;
     switch (parameter_.link_quality()) {
     case OLSR_BEHAVIOR_ETX:
       etx_ = (mult < 0.01) ? 100.0 : 1.0 / mult;
       break;
 
     case OLSR_BEHAVIOR_ML:
       etx_ = mult;
       break;
 
     case OLSR_BEHAVIOR_NONE:
     default:
       etx_ = 0.0;
       break;
     }
   }
 
   /// Link delay extension
   double  link_delay_;
   double nb_link_delay_;
 
   inline double  link_delay() { return link_delay_; }
   inline double  nb_link_delay() { return nb_link_delay_; }
 
   inline void update_link_delay (double link_delay, double nb_link_delay) {
     link_delay_ = link_delay;
     nb_link_delay_ = nb_link_delay;
   }
 
   inline u_int16_t&  seq()    { return seq_; }
   inline double&    time()    { return time_; }
 } OLSR_topology_tuple;
 
 
 typedef std::set<nsaddr_t>      mprset_t;  ///< MPR Set type.
 typedef std::vector<OLSR_mprsel_tuple*>    mprselset_t;  ///< MPR Selector Set type.
 typedef std::vector<OLSR_link_tuple*>    linkset_t;  ///< Link Set type.
 typedef std::vector<OLSR_nb_tuple*>    nbset_t;  ///< Neighbor Set type.
 typedef std::vector<OLSR_nb2hop_tuple*>    nb2hopset_t;  ///< 2-hop Neighbor Set type.
 typedef std::vector<OLSR_topology_tuple*>  topologyset_t;  ///< Topology Set type.
 typedef std::vector<OLSR_dup_tuple*>    dupset_t;  ///< Duplicate Set type.
 typedef std::vector<OLSR_iface_assoc_tuple*>  ifaceassocset_t;///< Interface Association Set type.
 
 #endif

