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

from ryu.base import app_manager
from ryu.controller import ofp_event
from ryu.controller.handler import CONFIG_DISPATCHER, MAIN_DISPATCHER
from ryu.controller.handler import set_ev_cls
from ryu.ofproto import ofproto_v1_3
from ryu.lib.packet import packet
from ryu.lib.packet import ethernet
from ryu.lib.packet import ipv4
from ryu.lib.packet import ether_types
from ryu.lib.packet import gre
from ryu.lib.packet import vlan



class SimpleSwitch13(app_manager.RyuApp):
    OFP_VERSIONS = [ofproto_v1_3.OFP_VERSION]

    def __init__(self, *args, **kwargs):
        super(SimpleSwitch13, self).__init__(*args, **kwargs)
        self.mac_to_port = {}

    @set_ev_cls(ofp_event.EventOFPSwitchFeatures, CONFIG_DISPATCHER)
    def switch_features_handler(self, ev):
        datapath = ev.msg.datapath
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser

        # install table-miss flow entry
        #
        # We specify NO BUFFER to max_len of the output action due to
        # OVS bug. At this moment, if we specify a lesser number, e.g.,
        # 128, OVS will send Packet-In with invalid buffer_id and
        # truncated packet data. In that case, we cannot output packets
        # correctly.  The bug has been fixed in OVS v2.1.0.
        match = parser.OFPMatch()
        actions = [parser.OFPActionOutput(ofproto.OFPP_CONTROLLER,
                                          ofproto.OFPCML_NO_BUFFER)]
        self.add_flow(datapath, 0, match, actions)

    def add_flow(self, datapath, priority, match, actions, buffer_id=None):
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser

        inst = [parser.OFPInstructionActions(ofproto.OFPIT_APPLY_ACTIONS,
                                             actions)]
        if buffer_id:
            mod = parser.OFPFlowMod(datapath=datapath, buffer_id=buffer_id,
                                    priority=priority, match=match,
                                    instructions=inst)
        else:
            mod = parser.OFPFlowMod(datapath=datapath, priority=priority,
                                    match=match, instructions=inst)
        datapath.send_msg(mod)

    @set_ev_cls(ofp_event.EventOFPPacketIn, MAIN_DISPATCHER)
    def _packet_in_handler(self, ev):
        # If you hit this you might want to increase
        # the "miss_send_length" of your switch
        if ev.msg.msg_len < ev.msg.total_len:
            self.logger.debug("packet truncated: only %s of %s bytes",
                              ev.msg.msg_len, ev.msg.total_len)
        msg = ev.msg
        datapath = msg.datapath
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser
        in_port = msg.match['in_port']

        pkt = packet.Packet(msg.data)
        eth = pkt.get_protocols(ethernet.ethernet)[0]

        if eth.ethertype == ether_types.ETH_TYPE_LLDP:
            # ignore lldp packet
            return
        dst = eth.dst
        src = eth.src

        dpid = datapath.id
        self.mac_to_port.setdefault(dpid, {})

        self.logger.info("packet in %s %s %s %s", dpid, src, dst, in_port)

        # learn a mac address to avoid FLOOD next time.      

        ip = pkt.get_protocol(vlan.vlan)
        if ip is not None:
            print ip
            print ip.ethertype
            print ip.vid

        if dst in self.mac_to_port[dpid]:
            out_port = self.mac_to_port[dpid][dst]
        else:
            self.mac_to_port[dpid][src] = in_port
            out_port = ofproto.OFPP_FLOOD

        # print self.mac_to_port
        actions = [parser.OFPActionOutput(out_port)]

        # install a flow to avoid packet_in next time
        if out_port != ofproto.OFPP_FLOOD and in_port != 2 :
            match = parser.OFPMatch(in_port=in_port, eth_dst=dst, eth_src=src, eth_type=0x0806)
            if msg.buffer_id != ofproto.OFP_NO_BUFFER:
                self.add_flow(datapath, 1, match, actions, msg.buffer_id)
            else:
                self.add_flow(datapath, 1, match, actions)

            # i = 1
            # while i < 7:
            #     print i
            #     match2 = parser.OFPMatch(in_port=2,eth_dst=dst, eth_type=0x0800, vlan_vid=i)
            #     if msg.buffer_id != ofproto.OFP_NO_BUFFER:
            #         self.add_flow(datapath, 2, match2, actions, msg.buffer_id)
            #     else:
            #         self.add_flow(datapath, 2, match2, actions)
            #     i += 1

            if in_port != 2 :
                print "TCP FLOWS"
                match = parser.OFPMatch(in_port=in_port, eth_dst=dst, eth_type=0x0800)
                actions2 = [parser.OFPActionOutput(1)]
                if msg.buffer_id != ofproto.OFP_NO_BUFFER:
                    self.add_flow(datapath, 1, match, actions2, msg.buffer_id)
                else:
                    self.add_flow(datapath, 1, match, actions2)
                
                match2 = parser.OFPMatch(in_port=2, eth_dst=dst, eth_type=0x0800)
                if msg.buffer_id != ofproto.OFP_NO_BUFFER:
                    self.add_flow(datapath, 1, match2, actions, msg.buffer_id)
                else:
                    self.add_flow(datapath, 1, match2, actions)

                match = parser.OFPMatch(in_port=in_port, eth_dst=dst, eth_type=0x8100, vlan_vid=4098)
                actions2 = [parser.OFPActionOutput(1)]
                if msg.buffer_id != ofproto.OFP_NO_BUFFER:
                    self.add_flow(datapath, 1, match, actions2, msg.buffer_id)
                else:
                    self.add_flow(datapath, 1, match, actions2)
                
                match2 = parser.OFPMatch(in_port=2, eth_dst=dst, eth_type=0x8100, vlan_vid=4098)
                if msg.buffer_id != ofproto.OFP_NO_BUFFER:
                    self.add_flow(datapath, 1, match2, actions, msg.buffer_id)
                else:
                    self.add_flow(datapath, 1, match2, actions)

            #     #vlan_vid
            #     #vlan_tci
            #     # match = parser.OFPMatch(in_port=in_port, eth_dst=dst,eth_type=0x8100, vlan_vid=0x1000/0x1000)
            #     # actions2 = [parser.OFPActionOutput(1)]
            #     # if msg.buffer_id != ofproto.OFP_NO_BUFFER:
            #     #     self.add_flow(datapath, 2, match, actions2, msg.buffer_id)
            #     # else:
            #     #     self.add_flow(datapath, 2, match, actions2)
                     
            #     match2 = parser.OFPMatch(in_port=2, eth_dst=dst, eth_type=0x0800, vlan_tci=9/0x0fff)
            #     if msg.buffer_id != ofproto.OFP_NO_BUFFER:
            #         self.add_flow(datapath, 2, match2, actions, msg.buffer_id)
            #     else:
            #         self.add_flow(datapath, 2, match2, actions)

            #     match2 = parser.OFPMatch(in_port=2, eth_dst=dst, eth_type=0x8100, vlan_tci=9/0x0fff)
            #     if msg.buffer_id != ofproto.OFP_NO_BUFFER:
            #         self.add_flow(datapath, 2, match2, actions, msg.buffer_id)
            #         return
            #     else:
            #         self.add_flow(datapath, 2, match2, actions)



        data = None
        if msg.buffer_id == ofproto.OFP_NO_BUFFER:
            data = msg.data

        out = parser.OFPPacketOut(datapath=datapath, buffer_id=msg.buffer_id,
                                  in_port=in_port, actions=actions, data=data)
        datapath.send_msg(out)
