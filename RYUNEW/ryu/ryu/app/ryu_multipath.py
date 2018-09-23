from ryu.base import app_manager
from ryu.controller import mac_to_port
from ryu.controller import ofp_event
from ryu.controller.handler import CONFIG_DISPATCHER, MAIN_DISPATCHER
from ryu.controller.handler import set_ev_cls
from ryu.ofproto import ofproto_v1_3
from ryu.lib.mac import haddr_to_bin
from ryu.lib.packet import packet
from ryu.lib.packet import arp
from ryu.lib.packet import ethernet
from ryu.lib.packet import ipv4
from ryu.lib.packet import ipv6
from ryu.lib.packet import tcp
from ryu.lib.packet import udp
from ryu.lib.packet import icmp
from ryu.lib.packet import vlan
from ryu.lib.packet import ether_types
from ryu.lib import mac, ip
from ryu.topology.api import get_switch, get_link
from ryu.app.wsgi import ControllerBase
from ryu.topology import event

from collections import defaultdict
from operator import itemgetter

import networkx as nx
import os
import random
import time
from itertools import islice


# Cisco Reference bandwidth = 1 Gbps
REFERENCE_BW = 10000000

PATH_INSTALLED = False
DEFAULT_BW = 10000000

MAX_PATHS = 10
MAX_GROUP = 0x2

class ProjectController(app_manager.RyuApp):
    OFP_VERSIONS = [ofproto_v1_3.OFP_VERSION]

    def __init__(self, *args, **kwargs):
        super(ProjectController, self).__init__(*args, **kwargs)
        self.mac_to_port = {}
        self.topology_api_app = self
        self.datapath_list = {}
        self.arp_table = {}
        self.switches = []
        self.hosts = {}
        self.multipath_group_ids = {}
        self.group_ids = []
        self.adjacency = defaultdict(dict)
        self.bandwidths = defaultdict(lambda: defaultdict(lambda: DEFAULT_BW))
        self.G = nx.Graph()
        self.timeDict = {}

    def get_paths(self, src, dst):
        '''
        Get all paths from src to dst using DFS algorithm    
        '''
        if src == dst:
            # host target is on the same switch
            return [[src]]
        # paths = []
        # stack = [(src, [src])]
        # while stack:
        #     (node, path) = stack.pop()
        #     for next in set(self.adjacency[node].keys()) - set(path):
        #         if next is dst:
        #             paths.append(path + [next])
        #             if len(paths) >= MAX_PATHS:
        #                 break
        #                 stack = []
        #         else:
        #             stack.append((next, path + [next]))
        
        print "Available paths from ", src, " to ", dst, " : ",
        # paths = [p for p in nx.shortest_simple_paths(self.G, src, dst)]
        paths = list(islice(nx.shortest_simple_paths(self.G, src, dst), MAX_PATHS))
        # paths = list(islice(nx.edge_disjoint_paths(self.G, src, dst), MAX_PATHS))
        
        print paths
        return paths

    def get_link_cost(self, s1, s2):
        '''
        Get the link cost between two switches 
        '''
        e1 = self.adjacency[s1][s2]
        e2 = self.adjacency[s2][s1]
        bl = min(self.bandwidths[s1][e1], self.bandwidths[s2][e2])
        ew = REFERENCE_BW/bl
        return ew

    def get_path_cost(self, path):
        '''
        Get the path cost
        '''
        cost = 0
        for i in range(len(path) - 1):
            cost += self.get_link_cost(path[i], path[i+1])
        return cost

    def get_optimal_paths(self, src, dst):
        '''
        Get the n-most optimal paths according to MAX_PATHS
        '''
        paths = self.get_paths(src, dst)
        paths_count = len(paths) if len(
            paths) < MAX_PATHS else MAX_PATHS
        return sorted(paths, key=lambda x: self.get_path_cost(x))[0:(paths_count)]

    def add_ports_to_paths(self, paths, first_port, last_port):
        '''
        Add the ports that connects the switches for all paths
        '''
        paths_p = []
        for path in paths:
            p = {}
            in_port = first_port
            for s1, s2 in zip(path[:-1], path[1:]):
                out_port = self.adjacency[s1][s2]
                p[s1] = (in_port, out_port)
                in_port = self.adjacency[s2][s1]
            p[path[-1]] = (in_port, last_port)
            paths_p.append(p)
        return paths_p

    def generate_openflow_gid(self):
        '''
        Returns a random OpenFlow group id
        '''
        n = random.randint(0, 2**32)
        while n in self.group_ids:
            n = random.randint(0, 2**32)
        return n

    def install_paths(self, src, first_port, dst, last_port, ip_src, ip_dst, src_mac ,dst_mac):
        computation_start = time.time()
        # paths = self.get_optimal_paths(src, dst)
        paths = self.get_paths(src, dst)
        pw = []

        for path in paths:
            pw.append(self.get_path_cost(path))
            # print path, "cost = ", pw[len(pw) - 1]
        sum_of_pw = sum(pw) * 1.0
        paths_with_ports = self.add_ports_to_paths(paths, first_port, last_port)

        # print "paths_with_ports", paths_with_ports
       
        firstPath = True

        vlan = 0x1001
        mirrIorvlan = 0x1101
        for path in paths_with_ports:
            for node in path:
                    dp = self.datapath_list[node]
                    print "DPID:", dp.id
                    ofp = dp.ofproto
                    parser = dp.ofproto_parser


                    in_port = path[node][0]
                    out_port = path[node][1]
                    actions = [parser.OFPActionOutput(out_port)]
                    actions_reverse = [parser.OFPActionOutput(in_port)]
                    actions2 = [parser.OFPActionOutput(1)]                          

                    if firstPath:
                        match_arp = parser.OFPMatch(in_port=in_port, eth_type=0x0806, arp_spa=ip_src, arp_tpa=ip_dst)
                        match_arp2 = parser.OFPMatch(in_port=out_port, eth_type=0x0806, arp_spa=ip_dst, arp_tpa=ip_src)
                        self.add_flow(dp, 1, match_arp, actions)
                        self.add_flow(dp, 1, match_arp2, actions_reverse)

                    if node == src:
                        match = parser.OFPMatch(in_port=in_port, ipv4_src=ip_src, ipv4_dst=ip_dst, eth_type=0x0800)
                        self.add_flow(dp, 2, match, actions2)

                        match4 = parser.OFPMatch(in_port=2, ipv4_src=ip_dst, ipv4_dst=ip_src, eth_type=0x0800)
                        self.add_flow(dp, 2, match4, actions_reverse)

                        match_arp3 = parser.OFPMatch(in_port=2, eth_type=0x0800, ipv4_src=ip_dst, ipv4_dst=ip_src, ip_proto=1)
                        self.add_flow(dp, 10, match_arp3, actions_reverse)
                        # match2 = parser.OFPMatch(in_port=2, eth_dst=dst_mac, eth_src=src_mac, eth_type=0x8100, vlan_vid=vlan)
                        # self.add_flow(dp, 3, match2, actions)
                        vlanGroup = vlan 
                        for x in xrange(0x0, MAX_GROUP+1):
                            match2 = parser.OFPMatch(in_port=2, eth_dst=dst_mac, eth_src=src_mac, eth_type=0x8100, vlan_vid=vlanGroup)
                            self.add_flow(dp, 3, match2, actions)
                            match3 = parser.OFPMatch(in_port=out_port, eth_dst=src_mac, eth_src=dst_mac, eth_type=0x8100, vlan_vid=vlanGroup)
                            self.add_flow(dp, 3, match3, actions2)
                            vlanGroup = vlanGroup + 0x10


                    elif node == dst:
                        match4 = parser.OFPMatch(in_port=2, ipv4_src=ip_src, ipv4_dst=ip_dst, eth_type=0x0800)
                        self.add_flow(dp, 2, match4, actions)

                        match = parser.OFPMatch(in_port=out_port, ipv4_src=ip_dst, ipv4_dst=ip_src, eth_type=0x0800)
                        self.add_flow(dp, 2, match, actions2)

                        # match_arp3 = parser.OFPMatch(in_port=2, eth_type=0x0800,eth_dst=dst_mac, eth_src=src_mac, ipv4_src=ip_dst, ipv4_dst=ip_src, ip_proto=1)
                        # self.add_flow(dp, 10, match_arp3, actions)
                        # match_arp3 = parser.OFPMatch(in_port=2, eth_type=0x0800,eth_dst=src_mac, eth_src=dst_mac, ipv4_src=ip_src, ipv4_dst=ip_dst, ip_proto=1)
                        # self.add_flow(dp, 10, match_arp3, actions)
                        # match3 = parser.OFPMatch(in_port=in_port, eth_dst=dst_mac, eth_src=src_mac, eth_type=0x8100, vlan_vid=vlan)
                        # self.add_flow(dp, 3, match3, actions2)

                        vlanGroup = vlan 
                        for x in xrange(0x0, MAX_GROUP+1):
                            match3 = parser.OFPMatch(in_port=in_port, eth_dst=dst_mac, eth_src=src_mac, eth_type=0x8100, vlan_vid=vlanGroup)
                            self.add_flow(dp, 3, match3, actions2)
                            match2 = parser.OFPMatch(in_port=2, eth_dst=src_mac, eth_src=dst_mac, eth_type=0x8100, vlan_vid=vlanGroup)
                            self.add_flow(dp, 3, match2, actions_reverse)

                            vlanGroup = vlanGroup + 0x10


                    else:
                        # match = parser.OFPMatch(in_port=in_port, eth_dst=dst_mac, eth_src=src_mac, eth_type=0x8100, vlan_vid=vlan)
                        # self.add_flow(dp, 3, match, actions)
                        vlanGroup = vlan 
                        for x in xrange(0x0, MAX_GROUP+1):
                            match = parser.OFPMatch(in_port=in_port, eth_dst=dst_mac, eth_src=src_mac, eth_type=0x8100, vlan_vid=vlanGroup)
                            self.add_flow(dp, 3, match, actions)

                            match = parser.OFPMatch(in_port=out_port, eth_dst=src_mac, eth_src=dst_mac, eth_type=0x8100, vlan_vid=vlanGroup)
                            self.add_flow(dp, 3, match, actions_reverse)

                            vlanGroup = vlanGroup + 0x10



                        # print node, in_port, out_port , "vlan"
            MirriorPath = random.choice(paths_with_ports)
            if len(paths_with_ports) > 1 :               
                while MirriorPath == path:
                    MirriorPath = random.choice(paths_with_ports)

            print "original Patj ", path
            print "MirriorPath", MirriorPath

            for node2 in MirriorPath:

                    dp = self.datapath_list[node2]
                    ofp = dp.ofproto
                    parser = dp.ofproto_parser

                    in_port = MirriorPath[node2][0]
                    out_port = MirriorPath[node2][1]
                    actions = [parser.OFPActionOutput(out_port)]
                    actions2 = [parser.OFPActionOutput(1)]
                    actions_reverse = [parser.OFPActionOutput(in_port)]

                    if node2 == src:
                        vlanGroup = mirrIorvlan 
                        for x in xrange(0x0, MAX_GROUP+1):
                            match2 = parser.OFPMatch(in_port=2, eth_dst=dst_mac, eth_src=src_mac, eth_type=0x8100, vlan_vid=vlanGroup)
                            self.add_flow(dp, 3, match2, actions)

                            match3 = parser.OFPMatch(in_port=out_port, eth_dst=src_mac, eth_src=dst_mac, eth_type=0x8100, vlan_vid=vlanGroup)
                            self.add_flow(dp, 3, match3, actions2)
                            
                            vlanGroup = vlanGroup + 0x10

                    elif node2 == dst:
                        vlanGroup = mirrIorvlan 
                        for x in xrange(0x0, MAX_GROUP+1):
                            match3 = parser.OFPMatch(in_port=in_port, eth_dst=dst_mac, eth_src=src_mac, eth_type=0x8100, vlan_vid=vlanGroup)
                            self.add_flow(dp, 3, match3, actions2)

                            match2 = parser.OFPMatch(in_port=2, eth_dst=src_mac, eth_src=dst_mac, eth_type=0x8100, vlan_vid=vlanGroup)
                            self.add_flow(dp, 3, match2, actions_reverse)
                            vlanGroup = vlanGroup + 0x10
                    else:
                        vlanGroup = mirrIorvlan 
                        for x in xrange(0x0, MAX_GROUP+1):
                            match = parser.OFPMatch(in_port=in_port, eth_dst=dst_mac, eth_src=src_mac, eth_type=0x8100, vlan_vid=vlanGroup)
                            self.add_flow(dp, 3, match, actions)

                            match = parser.OFPMatch(in_port=out_port, eth_dst=src_mac, eth_src=dst_mac, eth_type=0x8100, vlan_vid=vlanGroup)
                            self.add_flow(dp, 3, match, actions_reverse)
                            vlanGroup = vlanGroup + 0x10

            vlan += 1
            mirrIorvlan += 1
            # print vlan, hex(vlan)
            firstPath = False

        print "Path installation finished in ", time.time() - computation_start 
        return paths_with_ports[0][src][1]

    def add_flow(self, datapath, priority, match, actions, buffer_id=None):
        # print "Adding flow ", match, actions
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

    @set_ev_cls(ofp_event.EventOFPSwitchFeatures, CONFIG_DISPATCHER)
    def _switch_features_handler(self, ev):
        # print "switch_features_handler is called"
        datapath = ev.msg.datapath
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser

        match = parser.OFPMatch()
        actions = [parser.OFPActionOutput(ofproto.OFPP_CONTROLLER,
                                          ofproto.OFPCML_NO_BUFFER)]
        self.add_flow(datapath, 0, match, actions)

    @set_ev_cls(ofp_event.EventOFPPortDescStatsReply, MAIN_DISPATCHER)
    def port_desc_stats_reply_handler(self, ev):
        switch = ev.msg.datapath
        for p in ev.msg.body:
            self.bandwidths[switch.id][p.port_no] = p.curr_speed

    @set_ev_cls(ofp_event.EventOFPPacketIn, MAIN_DISPATCHER)
    def _packet_in_handler(self, ev):
        msg = ev.msg
        datapath = msg.datapath
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser
        in_port = msg.match['in_port']
        global PATH_INSTALLED


        pkt = packet.Packet(msg.data)
        eth = pkt.get_protocol(ethernet.ethernet)
        arp_pkt = pkt.get_protocol(arp.arp)
                
        # ip = pkt.get_protocol(icmp.icmp)
        # if ip is not None:
        #     print ip

        # avoid broadcast from LLDP
        if eth.ethertype == 35020:
            return

        if pkt.get_protocol(ipv6.ipv6):  # Drop the IPV6 Packets.
            match = parser.OFPMatch(eth_type=eth.ethertype)
            actions = []
            self.add_flow(datapath, 1, match, actions)
            return None

        dst = eth.dst
        src = eth.src
        dpid = datapath.id
            
        if (src == "00:50:56:fe:c0:ed" and dst == "00:0c:29:af:d3:52") or (src == "00:0c:29:af:d3:52" and dst == "00:50:56:fe:c0:ed"):
            match = parser.OFPMatch(eth_dst=dst, eth_src=src)
            actions = []
            self.add_flow(datapath, 1, match, actions)
            return None


        # self.logger.info("packet in %s %s %s %s", dpid, src, dst, in_port)

        # ip = pkt.get_protocol(ipv4.ipv4)
        # if ip is not None:
        #     print ip

        # ip = pkt.get_protocol(vlan.vlan)
        # if ip is not None:
        #     print ip

        if src not in self.hosts:
            self.hosts[src] = (dpid, in_port)
        # print self.hosts

        out_port = ofproto.OFPP_FLOOD

        if arp_pkt:
            src_ip = arp_pkt.src_ip
            dst_ip = arp_pkt.dst_ip
            if ( dst_ip in self.timeDict and src_ip in self.timeDict[dst_ip]) or ( src_ip in self.timeDict and dst_ip in self.timeDict[src_ip]):
                # print "Path Alread Done"
                return
            else:
                if arp_pkt.opcode == arp.ARP_REPLY:
                    self.arp_table[src_ip] = src
                    h1 = self.hosts[src]
                    h2 = self.hosts[dst]
                    out_port = self.install_paths(h1[0], h1[1], h2[0], h2[1], src_ip, dst_ip, src ,dst)
                    # self.install_paths(h2[0], h2[1], h1[0], h1[1], dst_ip, src_ip, dst, src) # reverse
                    self.timeDict[src_ip]= {}
                    self.timeDict[src_ip][dst_ip]= out_port
                    self.timeDict[dst_ip]= {}
                    self.timeDict[dst_ip][src_ip]= out_port
                elif arp_pkt.opcode == arp.ARP_REQUEST:
                    if dst_ip in self.arp_table:
                        self.arp_table[src_ip] = src
                        dst_mac = self.arp_table[dst_ip]
                        h1 = self.hosts[src]
                        h2 = self.hosts[dst_mac]
                        out_port = self.install_paths(h1[0], h1[1], h2[0], h2[1], src_ip, dst_ip, src, dst_mac)
                        # self.install_paths(h2[0], h2[1], h1[0], h1[1], dst_ip, src_ip, dst_mac, src) # reverse
                        self.timeDict[src_ip]= {}
                        self.timeDict[src_ip][dst_ip]= out_port
                        self.timeDict[dst_ip]= {}
                        self.timeDict[dst_ip][src_ip]= out_port
            actions = [parser.OFPActionOutput(out_port)]

            data = None
            if msg.buffer_id == ofproto.OFP_NO_BUFFER:
                data = msg.data

            out = parser.OFPPacketOut(
                datapath=datapath, buffer_id=msg.buffer_id, in_port=in_port,
                actions=actions, data=data)
            datapath.send_msg(out)
        else:
            self.logger.info("packet in %s %s %s %s", dpid, src, dst, in_port)
            print pkt


    @set_ev_cls(event.EventSwitchEnter)
    def switch_enter_handler(self, ev):
        switch = ev.switch.dp
        ofp_parser = switch.ofproto_parser
        # print "switch_add"
        if switch.id not in self.switches:
            self.switches.append(switch.id)
            self.datapath_list[switch.id] = switch

            # Request port/link descriptions, useful for obtaining bandwidth
            req = ofp_parser.OFPPortDescStatsRequest(switch)
            switch.send_msg(req)
            self.G.add_node(switch.id)
            if len(self.G) >= 349:
                print "chlo g, done scene"

    @set_ev_cls(event.EventSwitchLeave, MAIN_DISPATCHER)
    def switch_leave_handler(self, ev):
        print ev
        switch = ev.switch.dp.id
        if switch in self.switches:
            self.switches.remove(switch)
            del self.datapath_list[switch]
            if switch in self.adjacency:
                del self.adjacency[switch]
            self.G.remove_node(switch)

    @set_ev_cls(event.EventLinkAdd, MAIN_DISPATCHER)
    def link_add_handler(self, ev):
        s1 = ev.link.src
        s2 = ev.link.dst
        self.adjacency[s1.dpid][s2.dpid] = s1.port_no
        self.adjacency[s2.dpid][s1.dpid] = s2.port_no
        self.G.add_edge(s1.dpid, s2.dpid)
        # print self.adjacency
    @set_ev_cls(event.EventLinkDelete, MAIN_DISPATCHER)
    def link_delete_handler(self, ev):
        s1 = ev.link.src
        s2 = ev.link.dst
        # Exception handling if switch already deleted
        try:
            del self.adjacency[s1.dpid][s2.dpid]
            del self.adjacency[s2.dpid][s1.dpid]
            
        except KeyError:
            pass
