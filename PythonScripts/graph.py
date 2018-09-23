# Python3 Program to print BFS traversal
# from a given source vertex. BFS(int s)
# traverses vertices reachable from s.
from collections import defaultdict
import json 
# This class represents a directed graph
# using adjacency list representation
class Graph:
 
    # Constructor
    def __init__(self):
 
        # default dictionary to store graph
        self.graph = defaultdict(list)
 
    # function to add an edge to graph
    def addEdge(self,u,v):
        self.graph[u].append(v)
    
    def findLeafNodes(self):
        nodes = []
        for i in self.graph:
            if len(self.graph[i]) == 1:
                nodes.append(i)

        print(nodes)

    # Function to print a BFS of graph
    def BFS(self, s):
 
        visited = {}
        # Mark all the vertices as not visited
        for n in self.graph:
            visited[n] = False
        
 
        # Create a queue for BFS
        queue = []
        queue2 = []
        # Mark the source node as 
        # visited and enqueue it
        queue.append(s)
        queue2.append(s)
        visited[s] = True
 
        while queue:
            while queue:
     
                # Dequeue a vertex from 
                # queue and print it
                s = queue.pop(0)
                print (s, end = " ")
     
                # Get all adjacent vertices of the
                # dequeued vertex s. If a adjacent
                # has not been visited, then mark it
                # visited and enqueue it
                for i in self.graph[s]:
                    if visited[i] == False:
                        queue2.append(i)
                        visited[i] = True
            print ("NEXT LEVEL \n ")
            queue = queue2
            queue2 = []
 
# Driver code
 
# Create a graph given in
# the above diagram
jsonData = json.load(open("output_7415.json","r"))
g = Graph()
for edge in jsonData['edges']:
  n1, n2 = edge['nodes']
  g.addEdge(n1, n2)
  g.addEdge(n2, n1)


print ("Following is Breadth First Traversal"
                  " (starting from vertex 2)")
# g.BFS(398686)
g.findLeafNodes()
# This code is contributed by Neelam Yadav