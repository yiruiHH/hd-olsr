#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/hd-olsr-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("hd-olsr");


int
main(int argc, char *argv[])
{
    uint32_t nAdhoc = 3;
    double sTime = 20;
    bool tracing = true;

    NodeContainer wifiAdhocNodes;
    wifiAdhocNodes.Create(nAdhoc);

    WifiHelper wifi;
    
    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
    
    YansWifiChannelHelper channel;
    channel.AddPropagationLoss("ns3::RangePropagationLossModel","MaxRange",DoubleValue(250));
    channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    
    phy.SetChannel(channel.Create());
    
    NetDeviceContainer wifiAdhocDevices = wifi.Install(phy,mac,wifiAdhocNodes);

    MobilityHelper mobility;

    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                    "MinX", DoubleValue (250.0),
                                    "MinY", DoubleValue (250.0),
                                    "DeltaX", DoubleValue (100.0),
                                    "DeltaY", DoubleValue (100.0),
                                    "GridWidth", UintegerValue (10),
                                    "LayoutType", StringValue ("RowFirst"));

    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
			                    "Mode", StringValue ("Time"),
                                "Time", StringValue ("0.1s"),
                                "Speed", StringValue ("ns3::UniformRandomVariable[Min=10.0|Max=20.0]"),
			                    "Bounds", RectangleValue (Rectangle (0.0, 1500, 0.0, 1500)));

    //ns3::UniformRandomVariable[Min=2.0|Max=4.0]
    //ns3::ConstantRandomVariable[Constant=100.0]

    mobility.Install (wifiAdhocNodes);

    NS_LOG_INFO ("Enabling HD OLSR Routing.");
    HDOlsrHelper olsr;
    Ipv4StaticRoutingHelper staticRouting;
    Ipv4ListRoutingHelper list;
    list.Add (staticRouting, 0);
    list.Add (olsr, 10);

    InternetStackHelper stack;
    stack.SetRoutingHelper(list);
    stack.Install(wifiAdhocNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0","255.255.255.0");

    Ipv4InterfaceContainer interface;
    interface = address.Assign(wifiAdhocDevices);


    UdpClientHelper udpclient(interface.GetAddress(2),9);
    udpclient.SetAttribute("MaxPackets",UintegerValue(100));
    udpclient.SetAttribute("Interval",TimeValue(MilliSeconds(10)));
    udpclient.SetAttribute("PacketSize",UintegerValue(1400));
    ApplicationContainer udpc = udpclient.Install(wifiAdhocNodes.Get(0));
    udpc.Start(Seconds(5));
    udpc.Stop(Seconds(15));

    UdpServerHelper udpserver(9);
    ApplicationContainer udps = udpserver.Install(wifiAdhocNodes.Get(2));
    udps.Start(Seconds(4));
    udps.Stop(Seconds(16));

    Simulator::Stop (Seconds (sTime));


    if (tracing == true)
    {
        AsciiTraceHelper ascii;
        //stack.EnablePcapIpv4All("myolsr");
        stack.EnablePcapIpv4("hdolsr",wifiAdhocNodes.Get(0));
        stack.EnableAsciiIpv4All(ascii.CreateFileStream ("hdolsr.tr"));
        
    }

    
    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}
