#!/usr/bin/python

"""
A simple minimal topology script for Mininet.

Based in part on examples in the [Introduction to Mininet] page on the Mininet's
project wiki.

[Introduction to Mininet]: https://github.com/mininet/mininet/wiki/Introduction-to-Mininet#apilevels

"""

from mininet.cli import CLI
from mininet.log import setLogLevel
from mininet.net import Mininet
from mininet.topo import Topo
from mininet.node import RemoteController, OVSSwitch
from mininet.link import Intf

class MinimalTopo( Topo ):
    "Minimal topology with a single switch and two hosts"


switches = []

def runMinimalTopo():
    "Bootstrap a Mininet network using the Minimal Topology"

    topo = MinimalTopo()
    net = Mininet(
        topo=topo,
        controller=lambda name: RemoteController( name, ip='127.0.0.1' ),
        switch=OVSSwitch,
        autoSetMacs=True,
        build=False )

    switchCounter = 1
    interfaceCounter = 1

    while switchCounter < 2:
            s = net.addSwitch('s' + str(switchCounter) , protocols='OpenFlow13')
            switches.append(s)
            _intf = Intf( "veth" + str(interfaceCounter), node=s )
            _intf = Intf( "veth" + str(interfaceCounter + 2), node=s )
            switchCounter += 1
            interfaceCounter += 4
    for switch in switches:
        print switch.dpid
    h1 = net.addHost('h1')
    # h2 = net.addHost('h2')

    # net.addLink(switches[1-1], switches[2-1])
    # net.addLink(switches[1-1], switches[3-1])
    # net.addLink(switches[1-1], switches[4-1])

    # net.addLink(switches[2-1], switches[5-1])
    # net.addLink(switches[5-1], switches[7-1])

    # net.addLink(switches[3-1], switches[7-1])

    # net.addLink(switches[4-1], switches[6-1])
    # net.addLink(switches[6-1], switches[7-1])


    _intf = Intf( "eth0", node=switches[1-1])

    # while switchCounter < 15:
    #     s = net.addSwitch('s' + str(switchCounter))
    #     switches.append(s)
    #     _intf = Intf( "veth" + str(interfaceCounter), node=s )
    #     _intf = Intf( "veth" + str(interfaceCounter + 2), node=s )
    #     switchCounter += 1
    #     interfaceCounter += 4


    # h1 = net.addHost('h1')
    # h2 = net.addHost('h2')
    # # h3 = net.addHost('h3')

    # i = 0
    # while i < 5:
    #     net.addLink(switches[i], switches[i+1])
    # i = 5
    # while i < 8:
    #     net.addLink(switches[i], switches[i+1])

    # i = 9
    # while i < 13:
    #     net.addLink(switches[i], switches[i+1])

    # net.addLink(switches[0], switches[5])
    # net.addLink(switches[4], switches[8])
    # net.addLink(switches[4], switches[13])
    # net.addLink(switches[1], switches[6])
    # net.addLink(switches[6], switches[11])

    net.addLink(h1, switches[0])
    # net.addLink(h2, switches[6])

    # net.addLink(h2, s1)
    
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