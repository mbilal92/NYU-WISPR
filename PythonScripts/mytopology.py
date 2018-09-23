# from mininet.net import Mininet
# from mininet.cli import CLI
# from mininet.log import setLogLevel,  info
# from mininet.node import Node
import sys
sys.setrecursionlimit(5000)
import random
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
from mininet.node import RemoteController,  OVSSwitch
from mininet.link import Intf
from mininet.link import TCLink
from networkx.readwrite import json_graph
import json

class MinimalTopo( Topo ):
    "Minimal topology with a single switch and two hosts"

# def runMinimalTopo():
#   "Bootstrap a Mininet network using the Minimal Topology"
# G = nx.Graph()
def runMinimalTopo():
  "Bootstrap a Mininet network using the Minimal Topology"

  jsonData = json.load(open("output_7415.json", "r"))

  # G = nx.read_gpickle("mendocino.gpickle")
  # G = json_graph.node_link_graph(jsonData)
  G = nx.Graph()

  topo = MinimalTopo()
  net = Mininet(
      topo=topo, 
      controller=lambda name: RemoteController( name,  ip='127.0.0.1' ), 
      switch=OVSSwitch, 
      autoSetMacs=True, 
      build=False,
      link=TCLink )


  # nx.draw_networkx(G)
  # nx.draw(G, pos=nx.pydot_layout(G),  **options)
  # plt.show()
  switches = []
  switchCounter = 1
  interfaceCounter = 1
  h_count=0
  s_count=0


  name_switch = {}
  leafNodes = [113666, 209019,  113670,  370696,  396300,  396303,  396308,  301083,  352284,  301090,  301095,  416263,  139313,  308284,  333886,  308293,  197816,  417877,  454745,  348261,  201833,  406653,  440451,  516518,  480396,  413891,  510148,  421043,  124087,  304312,  311485,  362690,  124100,  337094,  510151,  510152,  510160,  413906,  421077,  116954,  418018,  262694,  395499,  418029,  249070,  300283,  384253,  417035,  307486,  307490,  417059,  307495,  307500,  307508,  502071,  145720,  365883,  127292,  421258,  379787,  398676,  108886,  398685,  301118,  260326,  498147,  417854,  197012,  587133,  303489,  428267,  354695,  370068,  420254,  310688,  420268,  420272,  266675,  420277,  420281,  405950,  394701,  394704,  325082,  299484,  383455,  299493,  417875,  416247,  416250,  504494,  370091,  397830,  306701,  409113,  306718,  365093,  449287,  423486,  409151,  381024,  339524,  302685,  521317,  526948,  485998,  302704,  137843,  486013,  508552,  410733,  419472,  412314,  265887,  393896,  363464,  422686,  419539,  356312,  361176,  415452,  305886,  305894,  408300,  305903,  331504,  377147,  364283,  199423,  125696,  287495,  422671,  107281,  269075,  422679,  118559,  404268,  269102,  397104,  375604,  132919,  301900,  485197,  195410,  53079,  415576,  309088,  121699,  360293,  360297,  418668,  374639,  415608,  171371,  381831,  381835,  246669,  402589,  253857,  377762,  385960,  191406,  117737,  305088,  499650,  305093,  230656,  305104,  356307,  198616,  488409,  363487,  198626,  363493,  349165,  421871,  166568,  407538,  440312]
  # brokenLinks = [143337, 247470, 114469, 117765]
  brokenLinks = [198617, 307487, 306714, 305110]
  # for edge in G.edges():
  print len(jsonData['edges'])

  ed = 0
  for edge in jsonData['edges']:
      n1,  n2 = edge['nodes']
      G.add_edge(n1,  n2)
      # print n1,  n2
      if n1 not in name_switch:
        name = 's' + str(n1)
        name_switch[n1] = name
        # h1 = net.addSwitch(name)
        h1 = net.addSwitch(name,  protocols='OpenFlow13')
        if n1 in leafNodes or n1 == 398686:
          _intf = Intf( "veth" + str(interfaceCounter),  node=h1 )
          _intf = Intf( "veth" + str(interfaceCounter + 2),  node=h1 )  
          interfaceCounter += 4
          print name, "veth" + str(interfaceCounter), "veth" + str(interfaceCounter+2)
        switchCounter += 1
        # interfaceCounter += 4
        s_count+=1
      else:
        h1 = net.getNodeByName(name_switch[n1])    

      if n2 not in name_switch:
        name = 's' + str(n2)
        name_switch[n2] = name
        # h2 = net.addSwitch(name)
        h2 = net.addSwitch(name,  protocols='OpenFlow13')
        if n2 in leafNodes or n2 == 398686:
          _intf = Intf( "veth" + str(interfaceCounter), node = h2)
          _intf = Intf( "veth" + str(interfaceCounter + 2), node = h2)  
          interfaceCounter += 4
          print name, "veth" + str(interfaceCounter), "veth" + str(interfaceCounter+2)
        switchCounter += 1
        # interfaceCounter += 4
        s_count+=1
      else:
        h2 = net.getNodeByName(name_switch[n2])   

      # if n1 not in name_switch:
      #   name = 's' + str(switchCounter)
      #   name_switch[n1] = name
      #   h1 = net.addSwitch(name,  protocols='OpenFlow13')
      #   switchCounter += 1
      #   interfaceCounter += 4
      #   s_count+=1
      # else:
      #   h1 = net.getNodeByName(name_switch[n1])    

      # if n2 not in name_switch:
      #   name = 's' + str(switchCounter)
      #   name_switch[n2] = name
      #   h2 = net.addSwitch(name,  protocols='OpenFlow13')
      #   switchCounter += 1
      #   interfaceCounter += 4
      #   s_count+=1
      # else:
      #   h2 = net.getNodeByName(name_switch[n2])   


          # switches.append(s)
          # _intf = Intf( "veth" + str(interfaceCounter),  node=s )
          # _intf = Intf( "veth" + str(interfaceCounter + 2),  node=s )  
      # if n1 in brokenLinks and n2 in brokenLinks:
      #   net.addLink(h1, h2, loss=5, delay='500ms')
      #   print "brokenLink"
      # else:
        # net.addLink(h1, h2)
      net.addLink(h1, h2)
      # if switchCounter > 300:
      #   break

  h1 = net.addHost('h1')
  net.addLink(net.getNodeByName("s398686"), h1)
  #net.setConfig
  # leafNodes = [113666,  113670,  370696,  396300,  396303,  396308,  301083,  352284,  301090,  301095,  416263,  139313,  308284,  333886,  308293,  197816,  417877,  454745,  348261,  201833,  209019,  406653,  440451,  516518,  480396,  413891,  510148,  421043,  124087,  304312,  311485,  362690,  124100,  337094,  510151,  510152,  510160,  413906,  421077,  116954,  418018,  262694,  395499,  418029,  249070,  300283,  384253,  417035,  307486,  307490,  417059,  307495,  307500,  307508,  502071,  145720,  365883,  127292,  421258,  379787,  398676,  108886,  398685,  301118,  260326,  498147,  417854,  197012,  587133,  303489,  428267,  354695,  370068,  420254,  310688,  420268,  420272,  266675,  420277,  420281,  405950,  394701,  394704,  325082,  299484,  383455,  299493,  417875,  416247,  416250,  504494,  370091,  397830,  306701,  409113,  306718,  365093,  449287,  423486,  409151,  381024,  339524,  302685,  521317,  526948,  485998,  302704,  137843,  486013,  508552,  410733,  419472,  412314,  265887,  393896,  363464,  422686,  419539,  356312,  361176,  415452,  305886,  305894,  408300,  305903,  331504,  377147,  364283,  199423,  125696,  287495,  422671,  107281,  269075,  422679,  118559,  404268,  269102,  397104,  375604,  132919,  301900,  485197,  195410,  53079,  415576,  309088,  121699,  360293,  360297,  418668,  374639,  415608,  171371,  381831,  381835,  246669,  402589,  253857,  377762,  385960,  191406,  117737,  305088,  499650,  305093,  230656,  305104,  356307,  198616,  488409,  363487,  198626,  363493,  349165,  421871,  166568,  407538,  440312]
  # LEVEL = []
  # LEVEL.append([407518, 398685, 399481, 510950, 398676, 376285, 300303, 247470, 413068, 306695, 305083, 300286, 414667, 379787, 418633, 408340, 422681, 363491, 308312, 417879, 418679, 345102, 362669, 419510]) 
  # LEVEL.append([408306, 395501, 397105, 411525, 404268, 508555, 486016, 510152, 510149, 587133, 505281, 384298, 377147, 359504, 301900, 299496, 301894, 172975, 143337, 229855, 247469, 260326, 246669, 409860, 305894, 305094, 308293, 305890, 308288, 307487, 303488, 268299, 198620, 300283, 299484, 417864, 417068, 417053, 416233, 421043, 407538, 416373, 405942, 409958, 440312, 422679, 421080, 428267, 365089, 365093, 363493, 362691, 363487, 306714, 307515, 304315, 307507, 307510, 417877, 417875, 420275, 419474, 336291, 355505, 348261, 349165, 376155, 365824, 363464, 434734, 423483, 413108])
  # LEVEL.append([408300, 394701, 396300, 393896, 395499, 394704, 397104, 396308, 396303, 412310, 410733, 419539, 508552, 508557, 486807, 486013, 484411, 499650, 521325, 510148, 510151, 504467, 504494, 386681, 359506, 359499, 296299, 299491, 301092, 301095, 303492, 171371, 166574, 117765, 133721, 145720, 222635, 249070, 253857, 398634, 305093, 307492, 311487, 308284, 307486, 301887, 305086, 262694, 269092, 197015, 199418, 198617, 418668, 420254, 416263, 416247, 417854, 416254, 414648, 416237, 433232, 438053, 415576, 405950, 408352, 420281, 421077, 364285, 365883, 370696, 362690, 305110, 306718, 304312, 301118, 307504, 308304, 307508, 420277, 420272, 421871, 420268, 419472, 336294, 342733, 376402, 354703, 376958, 269075, 374645, 454745, 423477, 423486, 422686, 413906, 413090])   
  # LEVEL.append([412314, 409113, 499768, 510160, 487610, 480397, 485210, 520521, 526950, 516518, 502071, 384256, 356307, 359493, 287495, 299493, 301090, 304290, 166568, 118564, 118559, 116155, 114469, 125726, 132919, 230656, 400234, 397830, 307490, 309088, 311485, 310688, 301083, 305088, 303489, 266675, 261892, 197816, 194611, 197012, 200221, 198616, 415452, 417057, 416250, 417035, 449287, 425228, 423651, 409151, 364283, 305104, 302704, 306698, 306701, 307502, 337094, 333886, 331506, 339524, 356312, 375604, 372482, 370068, 373993, 355500, 377762, 380183, 381028, 374639, 422671, 413891])
  # LEVEL.append([499738, 488409, 480396, 486000, 521317, 526948, 383455, 384253, 360291, 360297, 305886, 304285, 115482, 115329, 112125, 116954, 123290, 113670, 115276, 113666, 126509, 402589, 401036, 265887, 193010, 195410, 203436, 205030, 417059, 440416, 420421, 421252, 307495, 305903, 307500, 331504, 325082, 371687, 361176, 381145, 352284, 354695, 269102, 381835, 381024, 381831])
  # LEVEL.append([498147, 485998, 485197, 360293, 302685, 53079, 137843, 117737, 136912, 124087, 125691, 124100, 121699, 114480, 126502, 404245, 191406, 201833, 199426, 212224, 440451, 417217, 418029, 370091, 385960, 421258])
  # LEVEL.append([139313, 127292, 108886, 107281, 125696, 406653, 198626, 199423, 209019, 418018, 414011])
 
  # newLInks = open("newLinks,txt", "w")
  # i = 5
  # while i > 1:
  #   for j in range(0, 10):
  #     nn1 = random.choice(LEVEL[i])
  #     nn2 = random.choice(LEVEL[i-1])

  #     while nn1 in leafNodes:
  #       nn1 = random.choice(LEVEL[i])

  #     while nn2 in leafNodes:
  #       nn2 = random.choice(LEVEL[i-1])
  #     newLInks.write("s" + str(nn1)+ " s" + str(nn2) + "\n")
  #     net.addLink(net.getNodeByName("s" + str(nn1)),  net.getNodeByName("s" + str(nn2)))
  #   i-=1


  newLInks = open("newLinks.txt", "r")
  for line in newLInks:
      fields = line.split(" ")
      nn1 = fields[0].strip()
      nn2 = fields[1].strip()
      net.addLink(net.getNodeByName(nn1),  net.getNodeByName(nn2))
      print fields

  h_count = 2
  for node in leafNodes:
    h1 = net.addHost('h' + str(h_count))
    net.addLink(net.getNodeByName("s" + str(node) ), h1)
    h_count+=1


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

# print s_count, w_c_count, r_count, w_m_count
# nx.draw(G,  nx.get_node_attributes(G,  'pos'),  node_color=colors,  with_labels=False,  weight=0.25,  node_size=size)
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




  # my_nodes = ['172.18.90.1',  '10.6.8.1',   '172.18.72.3',  '172.18.72.2',  '172.18.190.252',  '172.18.190.253',  '172.18.91.2',  '172.18.101.2',  '172.18.92.3',  '172.18.60.1',  '172.18.33.3',  '172.18.70.1',  '172.18.221.2',  '172.18.221.3',  '10.5.195.1',  '172.18.220.1',  '172.18.34.3',  '172.18.14.3',  '172.18.11.3',  '172.18.162.2',  '172.18.20.1',  '172.18.101.3',  '172.18.15.2',  '172.18.200.1',  '172.18.34.2',  '172.18.10.1',  '172.18.202.2',  '172.18.40.1',  '172.18.160.1',  '172.18.14.5',  '172.18.122.2',  '172.18.33.2',  '172.18.15.3',  '172.18.11.2',  '172.18.122.3',  '172.18.12.3',  '172.18.12.2',  '172.18.34.4',  '172.18.202.3',  '172.18.30.1',   '172.18.151.2',  '172.18.151.3',  '172.18.75.2',  '172.18.75.3',  '172.18.161.3',  '172.18.161.2',  '172.18.193.2',  '172.18.193.3',  '172.18.92.2',  '172.18.50.1',  '172.18.152.3',  '172.18.152.2',  '172.18.102.2',  '172.18.102.3',  '172.18.30.250',  '172.18.120.1',  '172.18.91.3',  '172.18.162.3',  '192.168.234.1']
  # name_switch = {}
  # node_types = ['cpe', 'wireless-bridge-master', 'wireless-bridge-client', 'router', 'switch']
  
  # distance = {}
  # distanceVal = []
  # R = 6373.0

  # for node in my_nodes:
  #   lat1, lng1 =  G.node[node]['pos']
  #   lat1 = radians(lat1)
  #   lng1 = radians(lng1)
  #   for node2 in my_nodes:
  #     if node == node2:
  #       continue
  #     lat2, lng2 =  G.node[node2]['pos']
  #     lat2 = radians(lat2)
  #     lng2 = radians(lng2)
  #     dlon = lng2 - lng1
  #     dlat = lat2 - lat1

  #     a = sin(dlat / 2)**2 + cos(lat1) * cos(lat2) * sin(dlon / 2)**2
  #     c = 2 * atan2(sqrt(a),  sqrt(1 - a))

  #     distance = R * c

  #     distanceVal.append(distance)
  #     # distance[node, node2] = distance


  
  # arr = numpy.array(distanceVal)
  # print "values", distanceVal
  # print "mean",  numpy.mean(arr)
  # print "std", numpy.std(arr)
  # print max(distanceVal),  min(distanceVal)