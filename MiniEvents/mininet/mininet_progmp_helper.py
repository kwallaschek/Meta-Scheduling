#!/usr/bin/python

"""
mininet_progmp_helper.py: Simple example of MPTCP in Mininet to test ProgMP Scheduler.

Check https://progmp.net for more details.

"""
import time
import json
import os
from time import sleep
import argparse
from progmp import ProgMP

from mininet.cli import CLI
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.link import TCLink
from mininet.log import setLogLevel
from minisched import scheduler
from mininet.topo import SingleSwitchTopo
from mininet.log import setLogLevel, info, debug
from mininet.net import Mininet
from mininet.node import OVSController, DefaultController, Host, OVSKernelSwitch
from mininet.link import TCLink, Intf, Link


class Minievents(Mininet):
    def __init__(self, topo=None, switch=OVSKernelSwitch, host=Host,
                 controller=DefaultController, link=Link, intf=Intf,
                 build=True, xterms=False, cleanup=False, ipBase='10.0.0.0/8',
                 inNamespace=False,
                 autoSetMacs=False, autoStaticArp=False, autoPinCpus=False,
                 listenPort=None, waitConnected=False, events_file=None):
        super(Minievents, self).__init__(topo=topo, switch=switch, host=host, controller=controller,
                                         link=link, intf=intf, build=build, xterms=xterms, cleanup=cleanup,
                                         ipBase=ipBase, inNamespace=inNamespace, autoSetMacs=autoSetMacs,
                                         autoStaticArp=autoStaticArp, autoPinCpus=autoPinCpus,
                                         listenPort=listenPort,
                                         waitConnected=waitConnected)
      
        self.scheduler = scheduler(time.time, time.sleep)
        if events_file:
            json_events = json.load(open(events_file))
            self.load_events(json_events)

    def load_events(self, json_events):
        # event type to function correspondence
        event_type_to_f = {'editLink': self.editLink, 'iperf': self.iperf, 'ping': self.ping, 'stop': self.stop}
        for event in json_events:
            debug("processing event: time {time}, type {type}, params {params}\n".format(**event))
            event_type = event['type']
            self.scheduler.enter(event['time'], 1, event_type_to_f[event_type], kwargs=event['params'])

    # EVENT COMMANDS
    def delLink(self, src, dst):
        # TODO This code should be tested
        info('{time}:deleting link from {src} to {dst}\n'.format(time=time.time(), src=src, dst=dst))
        n1, n2 = self.get(src, dst)
        intf_pairs = n1.connectionsTo(n2)
        for intf_pair in intf_pairs:
            n1_intf, n2_intf = intf_pair
            info('{time}:deleting link from {intf1} and {intf2}\n'.format(time=time.time(), intf1=n1_intf.name,
                                                                          intf2=n2_intf.name))
            n1_intf.link.delete()
            self.links.remove(n1_intf.link)
            del n1.intfs[n1.ports[n1_intf]]
            del n1.ports[n1_intf]
            del n1.nameToIntf[n1_intf.name]

            n2_intf.delete()
            del n2.intfs[n2.ports[n2_intf]]
            del n2.ports[n2_intf]
            del n2.nameToIntf[n2_intf.name]

    def editLink(self, **kwargs):
        """
        Command to edit the properties of a link between src and dst.
        :param kwargs: named arguments
            src: name of the source node.
            dst: name of the destination node.
            bw: bandwidth in Mbps.
            loss: packet loss ratio percentage.
            delay: delay in ms.
        """
        n1, n2 = self.get(kwargs['src'], kwargs['dst'])
        intf_pairs = n1.connectionsTo(n2)
        info('***editLink event at t={time}: {args}\n'.format(time=time.time(), args=kwargs))
        for intf_pair in intf_pairs:
            n1_intf, n2_intf = intf_pair
            n1_intf.config(**kwargs)
            n2_intf.config(**kwargs)


    def iperf(self, **kwargs):
        """
        Command to start a transfer between src and dst.
        :param kwargs: named arguments
            src: name of the source node.
            dst: name of the destination node.
            protocol: tcp or udp (default tcp).
            duration: duration of the transfert in seconds (default 10s).
            bw: for udp, bandwidth to send at in bits/sec (default 1 Mbit/sec)

        """
        kwargs.setdefault('protocol', 'TCP')
        kwargs.setdefault('duration', 10)
        kwargs.setdefault('bw', 100000)
        info('***iperf event at t={time}: {args}\n'.format(time=time.time(), args=kwargs))
        
        if not os.path.exists("output"):
            os.makedirs("output")
        server_output = "output/iperf-{protocol}-server-{src}-{dst}.txt".format(**kwargs)
        client_output = "output/iperf-{protocol}-client-{src}-{dst}.txt".format(**kwargs)
        info('output filenames: {client} {server}\n'.format(client=client_output, server=server_output))

        client, server = self.get(kwargs['src'], kwargs['dst'])
        iperf_server_cmd = ''
        iperf_client_cmd = ''

 ### Needed for mptcp: -n 2
        server.sendCmd('ifconfig h1-eth0 10.0.0.1')
        sleep(.5)
        server.waiting = False
        server.sendCmd('ifconfig h1-eth1 11.0.0.1')
        server.waiting = False
        sleep(.5)
        client.sendCmd('ifconfig h2-eth0 10.0.0.2')
        client.waiting = False
        sleep(.5)
        client.sendCmd('ifconfig h2-eth1 11.0.0.2')
        client.waiting = False
        sleep(.5)
        if kwargs['protocol'].upper() == 'UDP':
             iperf_server_cmd = 'iperf -u -s -i 1'
             iperf_client_cmd = 'iperf -u -t {duration} -c {server_ip} -b {bw}'.format(server_ip=server.IP(), **kwargs)


        elif kwargs['protocol'].upper() == 'TCP':
             iperf_server_cmd = 'iperf -s -i 1'
             iperf_client_cmd = 'iperf -t {duration} -c {server_ip}'.format(server_ip=server.IP(), **kwargs)
        else :
            raise Exception( 'Unexpected protocol:{protocol}'.format(**kwargs))

        server.sendCmd('{cmd} &>{output} &'.format(cmd=iperf_server_cmd, output=server_output))
        info('iperf server command: {cmd} -s -i 1 &>{output} &\n'.format(cmd=iperf_server_cmd,
                                                                                output=server_output))
        # This is a patch to allow sendingCmd while iperf is running in background.CONS: we can not know when
        # iperf finishes and get their output
        server.waiting = False

        if kwargs['protocol'].lower() == 'tcp':
            while 'Connected' not in client.cmd(
                            'sh -c "echo A | telnet -e A %s 5001"' % server.IP()):
                info('Waiting for iperf to start up...\n')
                sleep(.5)

        info('iperf client command: {cmd} &>{output} &\n'.format(
            cmd = iperf_client_cmd, output=client_output))
        client.sendCmd('{cmd} &>{output} &'.format(
            cmd = iperf_client_cmd, output=client_output))
        # This is a patch to allow sendingCmd while iperf is running in background.CONS: we can not know when
        # iperf finishes and get their output
        client.waiting = False

    def ping(self, **kwargs):
        """
        Command to send pings between src and dst.
        :param kwargs: named arguments
            src: name of the source node.
            dst: name of the destination node.
            interval: time between ping packet transmissions.
            count: number of ping packets.
        """
        kwargs.setdefault('count', 3)
        kwargs.setdefault('interval', 1.0)
        info('***ping event at t={time}: {args}\n'.format(time=time.time(), args=kwargs))
        
        if not os.path.exists("output"):
            os.makedirs("output")
        output = "output/ping-{src}-{dst}.txt".format(**kwargs)
        info('output filename: {output}\n'.format(output=output))

        src, dst = self.get(kwargs['src'], kwargs['dst'])
        ping_cmd = 'ping -c {count} -i {interval} {dst_ip}'.format(dst_ip=dst.IP(), **kwargs)

        info('ping command: {cmd} &>{output} &\n'.format(
            cmd = ping_cmd, output=output))
        src.sendCmd('{cmd} &>{output} &'.format(
            cmd = ping_cmd, output=output))
        # This is a patch to allow sendingCmd while ping is running in background.CONS: we can not know when
        # ping finishes and get its output
        src.waiting = False

    def start(self):
        super(Minievents, self).start()
        CLI(self) if self.scheduler.empty() else self.scheduler.run()
class StaticTopo(Topo):
    "Simple topo with 2 hosts and 'number_of_paths' paths"
    def build(self, number_of_paths = 2, loss = 0):
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')
        
        
        for i in range(0, number_of_paths): 
            s = self.addSwitch('s' + str(i))

            self.addLink(h1, s, bw=100, delay="20ms", loss=float(loss))
            self.addLink(h2, s, bw=100, delay="20ms", loss=float(loss))
           

def runExperiment(number_of_paths, with_cli, loss):
    net = Minievents(topo=StaticTopo(number_of_paths, loss), link=TCLink, controller=OVSController, events_file="./testEvents.json")
    setLogLevel('info')   
    net.start()
    h1 = net.get('h1')
    h2 = net.get('h2')


    # there is probably a better way, but somehow we have to configure
    # the IP adresses
    for i in range(0, number_of_paths):
        h1.cmd('ifconfig h1-eth' + str(i) + ' 1' + str(i) + '.0.0.1')
        h2.cmd('ifconfig h2-eth' + str(i) + ' 1' + str(i) + '.0.0.2')
        
    
    # heat network to avoid intial packet artefacts
    for i in range(number_of_paths):
        h1.cmd("ping 1" + str(i) + ".0.0.2 -c 4")
  
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--file", help="The file which contains the scheduler", required=True)
    parser.add_argument("-n", "--number_of_subflows", help="The number of subflows")
    parser.add_argument("-d", "--debug", help="Running in debug mode", default=False)
    parser.add_argument("-c", "--cli", help="Waiting in command line interface", default=False)
    parser.add_argument("-l", "--loss", help="Loss rate", default=0)
    args = parser.parse_args()
    
    setLogLevel('warning')
    
    if args.debug:
        os.system('sysctl -w net.mptcp.mptcp_debug=1')
    else:
        os.system('sysctl -w net.mptcp.mptcp_debug=0')

    os.system('sysctl -w net.mptcp.mptcp_enabled=1')
    os.system('sysctl -w net.mptcp.mptcp_scheduler=rbs')
    os.system('sysctl -w net.mptcp.mptcp_path_manager=fullmesh')
    
    schedulerName = ProgMP.getSchedulerName(args.file)
    if schedulerName is None:
        print "Scheduler file makes some trouble..."
        exit()
        
    with open(args.file, "r") as src:
        schedProgStr = src.read()
        
    try:
        ProgMP.loadScheduler(schedProgStr)
    except:
        print "Scheduler loading error."
        exit()
    
    print "now setting sched", schedulerName
    ProgMP.setDefaultScheduler(schedulerName)
   
    if args.number_of_subflows:
        number_of_paths = [int(args.number_of_subflows)]
    else:
        number_of_paths = [1, 2, 3]
    
    for paths in number_of_paths:
        print "Running experiments with ", paths, "subflows"
        runExperiment(paths, args.cli, args.loss)
    ProgMP.setDefaultScheduler("simple")
    ProgMP.removeScheduler(schedulerName)
