# Copyright (C) 2011 Nippon Telegraph and Telephone Corporation.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
An OpenFlow 1.0 L2 learning switch implementation.
"""


from ryu.base import app_manager
from ryu.controller import ofp_event
from ryu.controller.handler import MAIN_DISPATCHER
from ryu.controller.handler import set_ev_cls
from ryu.ofproto import ofproto_v1_0,ofproto_v1_2,ofproto_v1_3,ofproto_v1_4
from ryu.lib.mac import haddr_to_bin
from ryu.lib.packet import packet
from ryu.lib.packet import ethernet
from ryu.lib.packet import tcp #,gre
from ryu.lib.packet import in_proto as inet
from ryu.lib.packet import ether_types
from ryu.topology import event, switches
# from ryu.topology.api import get_switch, get_link, get_host
from ryu.topology.api import *

import networkx as nx
from ryu.lib import hub
from array import array

class SimpleSwitch(app_manager.RyuApp):
    OFP_VERSIONS = [ofproto_v1_0.OFP_VERSION, ofproto_v1_2.OFP_VERSION,
                    ofproto_v1_3.OFP_VERSION, ofproto_v1_4.OFP_VERSION]

    def __init__(self, *args, **kwargs):
        super(SimpleSwitch, self).__init__(*args, **kwargs)
        self.mac_to_port = {}
        self.net = nx.DiGraph()
        self.topology_api_app = self
        self.nodes = {}
        self.links = {}
        self.no_of_nodes = 0
        self.no_of_links = 0
        self.i=0
        # self.monitor_thread = hub.spawn(self._monitor)

    def add_flow(self, datapath, in_port, dst, actions):
        ofproto = datapath.ofproto

        match = datapath.ofproto_parser.OFPMatch(
            in_port=in_port, dl_dst=haddr_to_bin(dst))

        mod = datapath.ofproto_parser.OFPFlowMod(
            datapath=datapath, match=match, cookie=0,
            command=ofproto.OFPFC_ADD, idle_timeout=0, hard_timeout=0,
            priority=ofproto.OFP_DEFAULT_PRIORITY,
            flags=ofproto.OFPFF_SEND_FLOW_REM, actions=actions)
        datapath.send_msg(mod)
    
    def _monitor(self):
        while True:
            print "Nodes"
            print self.net.nodes()
            
            print "Edges"
            print self.net.edges()

            print "Host"
            # host = get_all_host(self.topology_api_app)
            # for h in host:
            #     print h

            # try:
            #     import matplotlib.pyplot as plt
            #     # plt.ion()
            #     nx.draw_networkx(self.net)
            #     # plt.plot()
            #     plt.savefig("/home/ubuntu/NYU/myscripts/Bilal_path.png")
            #     plt.clf()
            # except:
            #     print "Error"
            #     raise

            # hub.sleep(5)


    @set_ev_cls(ofp_event.EventOFPPacketIn, MAIN_DISPATCHER)
    def _packet_in_handler(self, ev):
        msg = ev.msg
        datapath = msg.datapath
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser
        in_port = msg.match['in_port']
 
        pkt = packet.Packet(msg.data)
        eth = pkt.get_protocol(ethernet.ethernet)
        # print ":hahahaha",eth.ethertype , "\n"
        # if eth.ethertype == ether_types.ETH_TYPE_LLDP:
        #     # ignore lldp packet
        #     return
 
        dst = eth.dst
        src = eth.src
        dpid = datapath.id
        self.mac_to_port.setdefault(dpid, {})
        #print "nodes"
        #print self.net.nodes()
        #print "edges"
        #print self.net.edges()
        self.logger.info("packet in %s %s %s %s", dpid, src, dst, in_port)

        if src not in self.net:
            self.net.add_node(src)
            self.net.add_edge(dpid,src,{'port':in_port})
            self.net.add_edge(src,dpid)

        # learn a mac address to avoid FLOOD next time.
        self.mac_to_port[dpid][src] = in_port
        # print self.net
        if dst in self.mac_to_port[dpid]:

            actions1 = [datapath.ofproto_parser.OFPActionOutput(3)]
            self.add_flow(datapath, in_port, dst, actions1)

            out_port = self.mac_to_port[dpid][dst]
            actions2 = [datapath.ofproto_parser.OFPActionOutput(out_port)]
            self.add_flow(datapath, 3, dst, actions2)


        else:
            out_port = ofproto.OFPP_FLOOD

            actions1 = [datapath.ofproto_parser.OFPActionOutput(out_port)]

            # install a flow to avoid packet_in next time
            if out_port != ofproto.OFPP_FLOOD:
                self.add_flow(datapath, in_port, dst, actions1)

        data = None
        if msg.buffer_id == ofproto.OFP_NO_BUFFER:
            data = msg.data

        out = datapath.ofproto_parser.OFPPacketOut(
            datapath=datapath, buffer_id=msg.buffer_id, in_port=in_port,
            actions=actions1, data=data)
        datapath.send_msg(out)

    # @set_ev_cls(ofp_event.EventOFPPortStatus, MAIN_DISPATCHER)
    # def _port_status_handler(self, ev):
    #     msg = ev.msg
    #     reason = msg.reason
    #     port_no = msg.desc.port_no
    #     print self.net.edges()
    #     print self.net.nodes()
    #     ofproto = msg.datapath.ofproto
    #     if reason == ofproto.OFPPR_ADD:
    #         self.logger.info("port added %s", port_no)
    #     elif reason == ofproto.OFPPR_DELETE:
    #         self.logger.info("port deleted %s", port_no)
    #     elif reason == ofproto.OFPPR_MODIFY:
    #         self.logger.info("port modified %s", port_no)
    #     else:
    #         self.logger.info("Illeagal port state %s %s", port_no, reason)
    
    def ls(self,obj):
        print("\n".join([x for x in dir(obj) if x[0] != "_"]))
    
    # @set_ev_cls(event.EventSwitchEnter)
    # def get_topology_data(self, ev):
    #     print "switch Enter"
    #     switch_list = get_switch(self.topology_api_app, None)

    #     switches=[switch.dp.id for switch in switch_list]
    #     for switch in switch_list:
    #       self.ls(switch)
    #       print switch
    #     links_list = get_link(self.topology_api_app, None)
    #     links=[(link.src.dpid,link.dst.dpid,{'port':link.src.port_no}) for link in links_list]
    #     self.net.add_edges_from(links)
    #     links=[(link.dst.dpid,link.src.dpid,{'port':link.dst.port_no}) for link in links_list]
    #     print links
    #     print switches
    #     # self.net.add_nodes_from(switches)
    #     self.net.add_edges_from(links)

