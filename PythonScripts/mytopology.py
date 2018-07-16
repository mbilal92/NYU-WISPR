# from mininet.net import Mininet
# from mininet.cli import CLI
# from mininet.log import setLogLevel, info
# from mininet.node import Node
import sys
sys.setrecursionlimit(5000)

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
from networkx.readwrite import json_graph
import json
class MinimalTopo( Topo ):
    "Minimal topology with a single switch and two hosts"

# def runMinimalTopo():
#   "Bootstrap a Mininet network using the Minimal Topology"
# G = nx.Graph()
def runMinimalTopo():
  "Bootstrap a Mininet network using the Minimal Topology"

  jsonData = json.load(open("output_7415.json","r"))


  # G = nx.read_gpickle("mendocino.gpickle")
  # G = json_graph.node_link_graph(jsonData)

  topo = MinimalTopo()
  net = Mininet(
      topo=topo,
      controller=lambda name: RemoteController( name, ip='127.0.0.1' ),
      switch=OVSSwitch,
      autoSetMacs=True,
      build=False )


  # nx.draw_networkx(G)
  # nx.draw(G,pos=nx.pydot_layout(G), **options)
  # plt.show()
  switches = []
  switchCounter = 1
  interfaceCounter = 1
  h_count=0
  s_count=0


  name_switch = {}

  # for edge in G.edges():
  for edge in jsonData['edges']:
      n1, n2 = edge['nodes']
      # print n1, n2
      if n1 not in name_switch:
        name = 's' + str(n1)
        name_switch[n1] = name
        h1 = net.addSwitch(name, protocols='OpenFlow13')
        # _intf = Intf( "veth" + str(interfaceCounter), node=h1 )
        # _intf = Intf( "veth" + str(interfaceCounter + 2), node=h1 )  
        switchCounter += 1
        interfaceCounter += 4
        s_count+=1
      else:
        h1 = net.getNodeByName(name_switch[n1])    

      if n2 not in name_switch:
        name = 's' + str(n2)
        name_switch[n2] = name
        h2 = net.addSwitch(name, protocols='OpenFlow13')
        # _intf = Intf( "veth" + str(interfaceCounter), node=h2 )
        # _intf = Intf( "veth" + str(interfaceCounter + 2), node=h2 )  
        switchCounter += 1
        interfaceCounter += 4
        s_count+=1
      else:
        h2 = net.getNodeByName(name_switch[n2])   

      # if n1 not in name_switch:
      #   name = 's' + str(switchCounter)
      #   name_switch[n1] = name
      #   h1 = net.addSwitch(name, protocols='OpenFlow13')
      #   switchCounter += 1
      #   interfaceCounter += 4
      #   s_count+=1
      # else:
      #   h1 = net.getNodeByName(name_switch[n1])    

      # if n2 not in name_switch:
      #   name = 's' + str(switchCounter)
      #   name_switch[n2] = name
      #   h2 = net.addSwitch(name, protocols='OpenFlow13')
      #   switchCounter += 1
      #   interfaceCounter += 4
      #   s_count+=1
      # else:
      #   h2 = net.getNodeByName(name_switch[n2])   


          # switches.append(s)
          # _intf = Intf( "veth" + str(interfaceCounter), node=s )
          # _intf = Intf( "veth" + str(interfaceCounter + 2), node=s )  
      net.addLink(h1,h2)
      # if switchCounter > 300:
      #   break

  print name_switch

  net.build()
  net.start()

  CLI( net )
  net.stop()


if __name__ == '__main__':
    setLogLevel( 'info' )
    runMinimalTopo()

# Allows the file to be imported using `mn --custom <filename> --topospo minimal`
topos = {
    'minimal': MinimalTopo
}
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

# if __name__ == '__main__':
#     setLogLevel( 'info' )
#     runMinimalTopo()

# # Allows the file to be imported using `mn --custom <filename> --topospo minimal`
# topos = {
#     'minimal': MinimalTopo
# }




  # my_nodes = ['172.18.90.1', '10.6.8.1',  '172.18.72.3', '172.18.72.2', '172.18.190.252', '172.18.190.253', '172.18.91.2', '172.18.101.2', '172.18.92.3', '172.18.60.1', '172.18.33.3', '172.18.70.1', '172.18.221.2', '172.18.221.3', '10.5.195.1', '172.18.220.1', '172.18.34.3', '172.18.14.3', '172.18.11.3', '172.18.162.2', '172.18.20.1', '172.18.101.3', '172.18.15.2', '172.18.200.1', '172.18.34.2', '172.18.10.1', '172.18.202.2', '172.18.40.1', '172.18.160.1', '172.18.14.5', '172.18.122.2', '172.18.33.2', '172.18.15.3', '172.18.11.2', '172.18.122.3', '172.18.12.3', '172.18.12.2', '172.18.34.4', '172.18.202.3', '172.18.30.1',  '172.18.151.2', '172.18.151.3', '172.18.75.2', '172.18.75.3', '172.18.161.3', '172.18.161.2', '172.18.193.2', '172.18.193.3', '172.18.92.2', '172.18.50.1', '172.18.152.3', '172.18.152.2', '172.18.102.2', '172.18.102.3', '172.18.30.250', '172.18.120.1', '172.18.91.3', '172.18.162.3', '192.168.234.1']
  # name_switch = {}
  # node_types = ['cpe','wireless-bridge-master','wireless-bridge-client','router','switch']
  
  # distance = {}
  # distanceVal = []
  # R = 6373.0

  # for node in my_nodes:
  #   lat1,lng1 =  G.node[node]['pos']
  #   lat1 = radians(lat1)
  #   lng1 = radians(lng1)
  #   for node2 in my_nodes:
  #     if node == node2:
  #       continue
  #     lat2,lng2 =  G.node[node2]['pos']
  #     lat2 = radians(lat2)
  #     lng2 = radians(lng2)
  #     dlon = lng2 - lng1
  #     dlat = lat2 - lat1

  #     a = sin(dlat / 2)**2 + cos(lat1) * cos(lat2) * sin(dlon / 2)**2
  #     c = 2 * atan2(sqrt(a), sqrt(1 - a))

  #     distance = R * c

  #     distanceVal.append(distance)
  #     # distance[node,node2] = distance


  
  # arr = numpy.array(distanceVal)
  # print "values",distanceVal
  # print "mean", numpy.mean(arr)
  # print "std",numpy.std(arr)
  # print max(distanceVal), min(distanceVal)