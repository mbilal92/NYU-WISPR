#!/usr/bin/python

from mininet.net import Mininet
from mininet.node import Controller
from mininet.node import RemoteController, OVSSwitch
from mininet.cli import CLI
from mininet.link import Intf
from mininet.log import setLogLevel, info
from mininet.topo import Topo


class MinimalTopo( Topo ):
    "Minimal topology with a single switch and two hosts"
def myNetwork():
    topo = MinimalTopo()

    net = Mininet(
        topo=topo,
        controller=lambda name: RemoteController( name, ip='127.0.0.1' ),
        switch=OVSSwitch,
        autoSetMacs=True)

    # info( '*** Adding controller\n' )
    # net.addController(name='c0')

    info( '*** Add switches\n')
    s1 = net.addSwitch('s1')
    Intf( 'eth0', node=s1 )

    info( '*** Add hosts\n')
    h1 = net.addHost('h1')
    h2 = net.addHost('h2')

    info( '*** Add links\n')
    net.addLink(h1, s1)
    net.addLink(h2, s1)
    net.build()
    info( '*** Starting network\n')
    net.start()
    h1.cmdPrint('dhclient '+h1.defaultIntf().name + ' -v')
    CLI(net)
    net.stop()

if __name__ == '__main__':
    setLogLevel( 'info' )
    myNetwork()