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

    # def build( self ):
        # Create two hosts.
        # h1 = self.addHost( 'h1' )
        # h2 = self.addHost( 'h2' )

        # h3 = self.addHost( 'h3' )
        # # Create a switch
        # s1 = self.addSwitch( 's1' )
        # s2 = self.addSwitch( 's2' )
        # s3 = self.addSwitch( 's3' )

        # # _intf = Intf( "veth1", node=self.net.get('s1') )
        # self.addLink( s1, s2 )
        # # self.addLink( s3, s2 )

        # # # Add links between the switch and each host
        # self.addLink( h1, s1 )
        # self.addLink( h2, s1 )
        # self.addLink( h3, s2 )
        # self.addLink( h3, s2 )

def runMinimalTopo():
    "Bootstrap a Mininet network using the Minimal Topology"

    # Create an instance of our topology
    topo = MinimalTopo()

    # Create a network based on the topology using OVS and controlled by
    # a remote controller.
    net = Mininet(
        topo=topo,
        controller=lambda name: RemoteController( name, ip='127.0.0.1' ),
        switch=OVSSwitch,
        autoSetMacs=True,
        build=False )

        
    s1 = net.addSwitch('s1')
    _intf = Intf( "veth1", node=s1 )
    _intf = Intf( "veth3", node=s1 )
        # Delete old tunnel if still exists*//*
    # s1.cmd('sudo ip tun del s1-gre1')
    # s1.cmd('sudo ip li ad s1-gre1 type gretap local 1.1.1.1 remote 2.2.2.1')
    # s1.cmd('sudo ip li se dev s1-gre1 up')
    # Intf( 's1-gre1', node=s1 )


    s2 = net.addSwitch('s2')
    _intf = Intf( "veth5", node=s2 )
    _intf = Intf( "veth7", node=s2 )

    s3 = net.addSwitch('s3')
    _intf = Intf( "veth9", node=s3 )
    _intf = Intf( "veth11", node=s3 )

    s4 = net.addSwitch('s4')
    _intf = Intf( "veth13", node=s4 )
    _intf = Intf( "veth15", node=s4 )

    s5 = net.addSwitch('s5')
    _intf = Intf( "veth17", node=s5 )
    _intf = Intf( "veth19", node=s5 )

    # _intf = Intf( "eth0", node=s4 )

    # s3 = net.addSwitch('s3', protocols='OpenFlow13')
    # _intf = Intf( "veth17", node=s3 )
    # _intf = Intf( "veth19", node=s3 )

    # Intf( 's3-gre1', node=s3 )

    h1 = net.addHost('h1')
    h2 = net.addHost('h2')
    # h3 = net.addHost('h3')

    net.addLink(s1, s2)
    net.addLink(s2, s4)
    net.addLink(s1, s3)
    net.addLink(s3, s4)
    net.addLink(s4, s5)
    net.addLink(s3, s5)
    net.addLink(s1, s5)

    net.addLink(h1, s1)
    # net.addLink(h2, s1)
    net.addLink(h2, s4)

    # net.addLink(h2, s1)
    
    net.build()
    # switch = net.switches[ 2 ]
    # _intf = Intf( "veth5", node=switch )
    # Actually start the network
    net.start()
    
    # h1.cmdPrint('dhclient '+h1.defaultIntf().name + '-v')
    # Drop the user in to a CLI so user can run commands.
    CLI( net )

    # After the user exits the CLI, shutdown the network.
    net.stop()

if __name__ == '__main__':
    # This runs if this file is executed directly
    setLogLevel( 'info' )
    runMinimalTopo()

# Allows the file to be imported using `mn --custom <filename> --topospo minimal`
topos = {
    'minimal': MinimalTopo
}