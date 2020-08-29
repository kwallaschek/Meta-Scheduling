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
    net = Minievents(topo=StaticTopo(number_of_paths, loss), link=TCLink, controller=OVSController, events_file="testEvents.json")
    net.start()
   #
  
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
