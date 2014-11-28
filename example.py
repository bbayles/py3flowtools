#! /usr/bin/env python 

import flowtools

set = flowtools.FlowSet( "-" ) # Read from stdin

for flow in set:
    print "%s %s" % ( flow.srcaddr, flow.dstaddr ) 
    print " ", repr( flow.getID() )
    print " ", repr( flow.getID( 1 ) )
    
