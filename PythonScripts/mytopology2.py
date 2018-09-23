# from mininet.net import Mininet
# from mininet.cli import CLI
# from mininet.log import setLogLevel, info
# from mininet.node import Node
from math import sin, cos, sqrt, atan2, radians
import sys
sys.setrecursionlimit(5000)
import numpy

import matplotlib
matplotlib.use('Agg')

try:
    import matplotlib.pyplot as plt
except:
    raise
import networkx as nx

from mininet.cli import CLI
from mininet.log import setLogLevel
from mininet.net import Mininet
from mininet.topo import Topo
from mininet.node import RemoteController, OVSSwitch
from mininet.link import Intf

class MinimalTopo( Topo ):
    "Minimal topology with a single switch and two hosts"

def runMinimalTopo():
  "Bootstrap a Mininet network using the Minimal Topology"
# G = nx.Graph()
  G = nx.read_gpickle("mendocino.gpickle")
  # net = Mininet( )

  device_types = ['bts', 'switch-stp', 'cpe', 'wireless-bridge-client', 'wireless-bridge-master', 'wifi-ap', 'router', 'switch-dumb', 'relay-client']

  colors = []
  size = []


  topo = MinimalTopo()
  net = Mininet(
      topo=topo,
      controller=lambda name: RemoteController( name, ip='127.0.0.1' ),
      switch=OVSSwitch,
      autoSetMacs=True,
      build=False )

  # class LinuxRouter( Node ):
  #     "A Node with IP forwarding enabled."

  #     def config( self, **params ):
  #         super( LinuxRouter, self).config( **params )
  #         # Enable forwarding on the router
  #         self.cmd( 'sysctl net.ipv4.ip_forward=1' )

  #     def terminate( self ):
  #         self.cmd( 'sysctl net.ipv4.ip_forward=0' )
  #         super( LinuxRouter, self ).terminate()

  s_count = 0
  r_count = 0
  w_m_count = 0
  w_c_count = 0
  options = {
      'node_color': 'black',
      'node_size': 50,
      'line_color': 'grey',
      'linewidths': 0,
      'width': 0.1,
  }

  # nx.draw_networkx(G)
  # nx.draw(G,pos=nx.pydot_layout(G), **options)
  # plt.show()
  switches = []
  switchCounter = 1
  interfaceCounter = 1
  h_count=0
  s_count=0

  # print G.node["10.6.86.200"]
  indegree = {}
  my_nodes = ['172.18.90.1', '10.6.8.1',  '172.18.72.3', '172.18.72.2', '172.18.190.252', '172.18.190.253', '172.18.91.2', '172.18.101.2', '172.18.92.3', '172.18.60.1', '172.18.33.3', '172.18.70.1', '172.18.221.2', '172.18.221.3', '10.5.195.1', '172.18.220.1', '172.18.34.3', '172.18.14.3', '172.18.11.3', '172.18.162.2', '172.18.20.1', '172.18.101.3', '172.18.15.2', '172.18.200.1', '172.18.34.2', '172.18.10.1', '172.18.202.2', '172.18.40.1', '172.18.160.1', '172.18.14.5', '172.18.122.2', '172.18.33.2', '172.18.15.3', '172.18.11.2', '172.18.122.3', '172.18.12.3', '172.18.12.2', '172.18.34.4', '172.18.202.3', '172.18.30.1',  '172.18.151.2', '172.18.151.3', '172.18.75.2', '172.18.75.3', '172.18.161.3', '172.18.161.2', '172.18.193.2', '172.18.193.3', '172.18.92.2', '172.18.50.1', '172.18.152.3', '172.18.152.2', '172.18.102.2', '172.18.102.3', '172.18.30.250', '172.18.120.1', '172.18.91.3', '172.18.162.3', '192.168.234.1']
  R = 6373.0
  name_switch = {}

  for node in my_nodes:
    lat1,lng1 =  G.node[node]['pos']
    lat1 = radians(lat1)
    lng1 = radians(lng1)
    if node not in indegree:
      indegree[node] = 0

    for node2 in my_nodes:
      if node == node2:
        continue
      lat2,lng2 =  G.node[node2]['pos']
      lat2 = radians(lat2)
      lng2 = radians(lng2)
      dlon = lng2 - lng1
      dlat = lat2 - lat1

      a = sin(dlat / 2)**2 + cos(lat1) * cos(lat2) * sin(dlon / 2)**2
      c = 2 * atan2(sqrt(a), sqrt(1 - a))
      if node2 not in indegree:  
        indegree[node2] = 0          

      distance = R * c
      if distance >= 10 and (indegree[node] < 2 or indegree[node2] < 2):
        if node not in name_switch:
          name = 's' + str(switchCounter)
          name_switch[node] = name
          h1 = net.addSwitch(name, protocols='OpenFlow13')
          _intf = Intf( "veth" + str(interfaceCounter), node=h1 )
          _intf = Intf( "veth" + str(interfaceCounter + 2), node=h1 )  
          switchCounter += 1
          interfaceCounter += 4
          s_count+=1
        else:
          h1 = net.getNodeByName(name_switch[node])    

        if node2 not in name_switch:
          name = 's' + str(switchCounter)
          name_switch[node2] = name
          h2 = net.addSwitch(name, protocols='OpenFlow13')
          _intf = Intf( "veth" + str(interfaceCounter), node=h2 )
          _intf = Intf( "veth" + str(interfaceCounter + 2), node=h2 )  
          switchCounter += 1
          interfaceCounter += 4
          s_count+=1
        else:
          h2 = net.getNodeByName(name_switch[node2])   


            # switches.append(s)
            # _intf = Intf( "veth" + str(interfaceCounter), node=s )
            # _intf = Intf( "veth" + str(interfaceCounter + 2), node=s )  
        net.addLink(h1,h2)


        indegree[node] += 1
        indegree[node2] += 1
        if switchCounter > 300:
          break

  h1 = net.addHost('h1')
  net.addLink(net.getNodeByName("s6"),h1)
  h2 = net.addHost('h2')
  net.addLink(net.getNodeByName("s52"),h2)

  # for node in G.nodes():
  #     lat,lng =  G.node[node]['pos']
      # if lat == 0 or lng == 0:
      #     G.remove_node(node)
      #     continue
      # nodetype = G.node[node]['type']
      # if nodetype not in device_types:
      #   device_types.append(nodetype)
      # # if nodetype == 'cpe':
      # #    # G.remove_node(node)
      # #     # colors.append('b')
      # #     # size.append(5)
      # #    continue
      # if nodetype == 'cpe' or nodetype == 'wireless-bridge-master' or nodetype == 'wireless-bridge-client' or nodetype == 'router' or 'switch' in nodetype:
      #     # s = net.addSwitch('s' + str(switchCounter), protocols='OpenFlow13')
      #     # s = net.addSwitch(node, protocols='OpenFlow13')
      #     # switches.append(s)
      #     # _intf = Intf( "veth" + str(interfaceCounter), node=s )
      #     # _intf = Intf( "veth" + str(interfaceCounter + 2), node=s )
      #     # switchCounter += 1
      #     # interfaceCounter += 4
      #     # net.addSwitch(node)
      #     s_count+=1
      # else:
      #     # net.addHost(node, ip = node)
      #     h_count+=1

  # print h_count, s_count
  # print device_types


  # for edge in G.edges():
  #     n1, n2 = edge
  #     # print n1, n2
  #     nodetype1 = G.node[n1]['type']
  #     nodetype2 = G.node[n2]['type']
  #     if nodetype1 in node_types and nodetype2 in node_types:

  #       if n1 not in name_switch:
  #         name = 's' + str(switchCounter)
  #         name_switch[n1] = name
  #         h1 = net.addSwitch(name, protocols='OpenFlow13')
  #         switchCounter += 1
  #         interfaceCounter += 4
  #         s_count+=1
  #       else:
  #         h1 = net.getNodeByName(name_switch[n1])    

  #       if n2 not in name_switch:
  #         name = 's' + str(switchCounter)
  #         name_switch[n2] = name
  #         h2 = net.addSwitch(name, protocols='OpenFlow13')
  #         switchCounter += 1
  #         interfaceCounter += 4
  #         s_count+=1
  #       else:
  #         h2 = net.getNodeByName(name_switch[n2])   


  #           # switches.append(s)
  #           # _intf = Intf( "veth" + str(interfaceCounter), node=s )
  #           # _intf = Intf( "veth" + str(interfaceCounter + 2), node=s )  
  #       net.addLink(h1,h2)
  #       if switchCounter > 300:
  #         break

  # print name_switch

  net.build()
  net.start()

  CLI( net )
  net.stop()

    # break
# net.start()
# CLI( net )

# print s_count,w_c_count,r_count,w_m_count
# nx.draw(G, nx.get_node_attributes(G, 'pos'), node_color=colors, with_labels=False, weight=0.25, node_size=size)
# print len(G.nodes())
# print len(G.edges())
# # print G
# # nx.draw_networkx(G)
# plt.show()
# plt.savefig("path.png")

if __name__ == '__main__':
    setLogLevel( 'info' )
    runMinimalTopo()

# Allows the file to be imported using `mn --custom <filename> --topospo minimal`
topos = {
    'minimal': MinimalTopo
}