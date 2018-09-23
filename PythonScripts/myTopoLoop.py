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

def _tree_edges(n,r):
    # helper function for trees
    # yields edges in rooted tree at 0 with n nodes and branching ratio r
    nodes=iter(range(n))
    parents=[next(nodes)] # stack of max length r
    while parents:
        source=parents.pop(0)
        for i in range(r):
            try:
                target=next(nodes)
                parents.append(target)
                yield source,target
            except StopIteration:
                break


jsonData = json.load(open("output_7415.json","r"))

G = nx.DiGraph()

for edge in jsonData['edges']:
  n1, n2 = edge['nodes']
  G.add_edge(n1, n2)
  G.add_edge(n2, n1)
T = tree = nx.bfs_tree(G, 398686)

T
# def bfs(graph, start):
#     visited, queue = set(), [start]
#     while queue:
#         vertex = queue.pop(0)
#         if vertex not in visited:
#             visited.add(vertex)
#             queue.extend(graph[vertex] - visited)
#     return visited


