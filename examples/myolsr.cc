#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/animation-interface.h"
#include "ns3/hd-olsr-helper.h"
#include "ns3/olsr-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/hd-olsr-routing-protocol.h"
#include "ns3/olsr-routing-protocol.h"

#include <ctime>

using namespace ns3;

int mpr_total_num = 0;

NS_LOG_COMPONENT_DEFINE ("myolsr");
/*
static void
print_position(NodeContainer nodes)
{   
    std::cout << "print_position" << std::endl;
    // iterate our nodes and print their position.
    for (NodeContainer::Iterator j = nodes.Begin ();j != nodes.End (); ++j)
    {
        Ptr<Node> object = *j;
        Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
        NS_ASSERT (position != 0);
        Vector pos = position->GetPosition ();
        std::cout << pos << std::endl;
    }
}
*/
/*
static void 
print_loc_vel(Ptr<Node> n,Ptr<OutputStreamWrapper> stream)
{
    std::ostream* os = stream->GetStream ();
    std::string nodename = std::to_string(n->GetId());
    Ptr<MobilityModel> mobility = n->GetObject<MobilityModel> ();
    Vector3D pos = mobility->GetPosition();
    Vector3D vel = mobility->GetVelocity();
    *os << nodename << " " << Simulator::Now().GetSeconds() << " " << pos << " " << vel << std::endl;
}
*/

static void
print_simulator_time()
{   
    const std::string& t = std::to_string(Simulator::Now().GetSeconds());
    std::string msg = "Now simulate time is " + t;
    NS_LOG_UNCOND(msg);
}

static void
print_mpr(NodeContainer nodes, std::string filename, bool hd)
{   
    //Ptr<OutputStreamWrapper> mprStream = Create<OutputStreamWrapper>(filename,std::ios::out);
    int SumOfMpr = 0;
    for (NodeContainer::Iterator j = nodes.Begin ();j != nodes.End (); ++j)
    {
        Ptr<Node> node = *j;
        if(hd){
            Ptr<ns3::hdolsr::RoutingProtocol> rp = node->GetObject<ns3::hdolsr::RoutingProtocol> ();
            if(rp)
            {
                //rp->PrintMprSet(mprStream);
                SumOfMpr += rp->GetMprNums();
            }
        }
        else{
            Ptr<ns3::olsr::RoutingProtocol> rp = node->GetObject<ns3::olsr::RoutingProtocol> ();
            if(rp)
            {
                //rp->PrintMprSet(mprStream);
                SumOfMpr += rp->GetMprNums();
            }
        }
        
    }
    //std::cout << SumOfMpr << std::endl;
    mpr_total_num += SumOfMpr;
}

int
main(int argc, char *argv[])
{
    bool verbose = false;
    bool tracing = false;

    uint32_t nAdhoc = 100;
    double sTime = 300;
    bool hd = false;
    double distance = 500.0;
    double HelloInterval = 1.0;
    double TcInterval = 2.0;

    CommandLine cmd;
    cmd.AddValue ("nAdhoc", "Number of wifi STA devices", nAdhoc);
    cmd.AddValue ("sTime", "Simulate time (seconds)", sTime);
    cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
    cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
    cmd.AddValue ("hd", "using hd olsr protocol", hd);
    cmd.AddValue ("distance", "tansmission distance", distance);
    cmd.AddValue ("hello", "hello and hd interval", HelloInterval);
    cmd.AddValue ("tc", "Tc interval", TcInterval);

    cmd.Parse (argc,argv);

    if (verbose)
    {
        LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
        LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
    }

    NodeContainer wifiAdhocNodes;
    wifiAdhocNodes.Create(nAdhoc);

    WifiHelper wifi;
    
    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
    
    YansWifiChannelHelper channel;
    channel.AddPropagationLoss("ns3::RangePropagationLossModel","MaxRange",DoubleValue(distance));
    channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    //channel.SetPropagationDelay("ns3::RandomPropagationDelayModel");
    
    phy.SetChannel(channel.Create());
    
    NetDeviceContainer wifiAdhocDevices = wifi.Install(phy,mac,wifiAdhocNodes);

    MobilityHelper mobility;
/*
    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                    "MinX", DoubleValue (100.0),
                                    "MinY", DoubleValue (100.0),
                                    "DeltaX", DoubleValue (100.0),
                                    "DeltaY", DoubleValue (100.0),
                                    "GridWidth", UintegerValue (10),
                                    "LayoutType", StringValue ("RowFirst"));

    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
			                    "Mode", StringValue ("Time"),
                                "Time", StringValue ("2s"),
                                "Speed", StringValue ("ns3::UniformRandomVariable[Min=20.0|Max=30.0]"),
			                    "Bounds", RectangleValue (Rectangle (0.0, 1200, 0.0, 1200)));
*/
    //ns3::UniformRandomVariable[Min=2.0|Max=4.0]
    //ns3::ConstantRandomVariable[Constant=100.0]
    
    mobility.SetMobilityModel ("ns3::GaussMarkovMobilityModel",
                                "Bounds", BoxValue (Box (0, 3000, 0, 3000, 0, 1000)),
                                "TimeStep", TimeValue (Seconds (3)),
                                "Alpha", DoubleValue (0.85),
                                "MeanVelocity", StringValue ("ns3::UniformRandomVariable[Min=10|Max=20]"),
                                "MeanDirection", StringValue ("ns3::UniformRandomVariable[Min=0|Max=6.283185307]"),
                                "MeanPitch", StringValue ("ns3::UniformRandomVariable[Min=0.05|Max=0.05]"),
                                "NormalVelocity", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=1.0|Bound=10.0]"),
                                "NormalDirection", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.2|Bound=0.4]"),
                                "NormalPitch", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.02|Bound=0.04]"));
    mobility.SetPositionAllocator ("ns3::RandomBoxPositionAllocator",
                                    "X", StringValue ("ns3::UniformRandomVariable[Min=0|Max=3000]"),
                                    "Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max=3000]"),
                                    "Z", StringValue ("ns3::UniformRandomVariable[Min=0|Max=1000]"));

    mobility.Install (wifiAdhocNodes);

    NS_LOG_INFO ("Enabling OLSR Routing.");

    OlsrHelper olsr;
    HDOlsrHelper hdolsr;
    
    //Ipv4StaticRoutingHelper staticRouting;
    Ipv4ListRoutingHelper list;
    //list.Add (staticRouting, 0);

    Ptr<Node> node = wifiAdhocNodes.Get(0);
    Ptr<MobilityModel> mo = node->GetObject<MobilityModel> ();
    PointerValue pt;
    mo->GetAttribute("MeanVelocity",pt);
    Ptr<UniformRandomVariable> rv = pt.Get<UniformRandomVariable> ();
    double minSpeed = rv->GetMin();
    double maxSpeed = rv->GetMax();
    NS_LOG_INFO ("minspeed="<<minSpeed<<"maxspeed"<<maxSpeed);

    if(hd)
    {   
        hdolsr.Set("HelloInterval",TimeValue(Seconds(HelloInterval)));
        hdolsr.Set("TcInterval",TimeValue(Seconds(TcInterval)));
        hdolsr.Set("HdInterval",TimeValue(Seconds(HelloInterval)));
        hdolsr.Set("MinSpeed",DoubleValue(minSpeed));
        hdolsr.Set("MaxSpeed",DoubleValue(maxSpeed));
        hdolsr.Set("MaxRange",DoubleValue(distance));
        list.Add (hdolsr, 10);
    }
    else
    {   
        olsr.Set("HelloInterval",TimeValue(Seconds(HelloInterval)));
        olsr.Set("TcInterval",TimeValue(Seconds(TcInterval)));
        list.Add (olsr, 10);
    }
    
    InternetStackHelper stack;
    stack.SetRoutingHelper(list);
    stack.Install(wifiAdhocNodes);
/*
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("routes.tr",std::ios::out);
    Ptr<OutputStreamWrapper> neighborStream = Create<OutputStreamWrapper>("neighbors.tr",std::ios::out);
    olsr.PrintRoutingTableAllAt(Seconds(15.0),routingStream);
    olsr.PrintNeighborCacheAllAt(Seconds(15.0),neighborStream);
*/
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0","255.255.255.0");

    Ipv4InterfaceContainer interface;
    interface = address.Assign(wifiAdhocDevices);

    uint16_t port = 9;
    if(!hd){
        DataRate rate = DataRate("100kbps");

        OnOffHelper onoff1 ("ns3::UdpSocketFactory",InetSocketAddress (interface.GetAddress (20), port));
        onoff1.SetConstantRate (rate);
        onoff1.SetAttribute("OffTime",StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
        onoff1.SetAttribute("PacketSize",UintegerValue(1024));

        ApplicationContainer onOffApp1 = onoff1.Install (wifiAdhocNodes.Get (10));
        onOffApp1.Start (Seconds(0));
        onOffApp1.Stop (Seconds (sTime));


        OnOffHelper onoff2 ("ns3::UdpSocketFactory",InetSocketAddress (interface.GetAddress (40), port));
        onoff2.SetConstantRate (rate);
        onoff1.SetAttribute("OffTime",StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
        onoff2.SetAttribute("PacketSize",UintegerValue(1024));

        ApplicationContainer onOffApp2 = onoff2.Install (wifiAdhocNodes.Get (30));
        onOffApp2.Start (Seconds(0));
        onOffApp2.Stop (Seconds (sTime));

        OnOffHelper onoff3 ("ns3::UdpSocketFactory",InetSocketAddress (interface.GetAddress (60), port));
        onoff3.SetConstantRate (rate);
        onoff1.SetAttribute("OffTime",StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
        onoff3.SetAttribute("PacketSize",UintegerValue(1024));

        ApplicationContainer onOffApp3 = onoff3.Install (wifiAdhocNodes.Get (50));
        onOffApp3.Start (Seconds(0));
        onOffApp3.Stop (Seconds (sTime));

        // Create packet sinks to receive these packets
        PacketSinkHelper sink ("ns3::UdpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));
        NodeContainer sinks = NodeContainer (wifiAdhocNodes.Get (20),wifiAdhocNodes.Get (40),wifiAdhocNodes.Get (60));
        ApplicationContainer sinkApps = sink.Install (sinks);
        sinkApps.Start (Seconds(0));
        sinkApps.Stop (Seconds (sTime));
    }
    else{
        /*
        for(int i=0;i<(int)nAdhoc;i++){
            double f = 3;
            switch (nAdhoc)
            {
            case 60:
                f = 5;
                break;
            case 80:
                f = 3.75;
                break;
            case 100:
                f = 3;
                break;
            case 120:
                f = 2.5;
                break;
            
            default: f = 2.5;
                break;
            }
            UdpClientHelper udpclient1(interface.GetAddress(((i+1)%nAdhoc)),port);
            udpclient1.SetAttribute("MaxPackets",UintegerValue(100000));
            udpclient1.SetAttribute("Interval",TimeValue(MilliSeconds(80)));
            udpclient1.SetAttribute("PacketSize",UintegerValue(1024));
            ApplicationContainer udpc1 = udpclient1.Install(wifiAdhocNodes.Get(i));
            udpc1.Start(Seconds(f*(double)i));
            udpc1.Stop(Seconds(f*(double)i+f));
        
            UdpServerHelper udpserver1(port);
            ApplicationContainer udps1 = udpserver1.Install(wifiAdhocNodes.Get((i+1)%nAdhoc));
            udps1.Start(Seconds(f*(double)i));
            udps1.Stop(Seconds(f*(double)i+f));
        */
        UdpClientHelper udpclient1(interface.GetAddress(20),port);
        udpclient1.SetAttribute("MaxPackets",UintegerValue(100000));
        udpclient1.SetAttribute("Interval",TimeValue(MilliSeconds(80)));
        udpclient1.SetAttribute("PacketSize",UintegerValue(1024));
        ApplicationContainer udpc1 = udpclient1.Install(wifiAdhocNodes.Get(10));
        udpc1.Start(Seconds(0));
        udpc1.Stop(Seconds(sTime));
    
        UdpServerHelper udpserver1(port);
        ApplicationContainer udps1 = udpserver1.Install(wifiAdhocNodes.Get(20));
        udps1.Start(Seconds(0));
        udps1.Stop(Seconds(sTime));

        UdpClientHelper udpclient2(interface.GetAddress(40),port);
        udpclient2.SetAttribute("MaxPackets",UintegerValue(50000));
        udpclient2.SetAttribute("Interval",TimeValue(MilliSeconds(80)));
        udpclient2.SetAttribute("PacketSize",UintegerValue(1024));
        ApplicationContainer udpc2 = udpclient2.Install(wifiAdhocNodes.Get(30));
        udpc2.Start(Seconds(0));
        udpc2.Stop(Seconds(sTime));
    
        UdpServerHelper udpserver2(port);
        ApplicationContainer udps2 = udpserver2.Install(wifiAdhocNodes.Get(40));
        udps2.Start(Seconds(0));
        udps2.Stop(Seconds(sTime));

        UdpClientHelper udpclient3(interface.GetAddress(60),port);
        udpclient3.SetAttribute("MaxPackets",UintegerValue(50000));
        udpclient3.SetAttribute("Interval",TimeValue(MilliSeconds(80)));
        udpclient3.SetAttribute("PacketSize",UintegerValue(1024));
        ApplicationContainer udpc3 = udpclient3.Install(wifiAdhocNodes.Get(50));
        udpc3.Start(Seconds(0));
        udpc3.Stop(Seconds(sTime));
    
        UdpServerHelper udpserver3(port);
        ApplicationContainer udps3 = udpserver3.Install(wifiAdhocNodes.Get(60));
        udps3.Start(Seconds(0));
        udps3.Stop(Seconds(sTime));
        
    }
    

    
    
//----------------------------------------------------------------------------------
/*
    UdpClientHelper udpclient2(interface.GetAddress(39),port);
    udpclient2.SetAttribute("MaxPackets",UintegerValue(50000));
    udpclient2.SetAttribute("Interval",TimeValue(MilliSeconds(100)));
    udpclient2.SetAttribute("PacketSize",UintegerValue(1024));
    ApplicationContainer udpc2 = udpclient2.Install(wifiAdhocNodes.Get(30));
    udpc2.Start(Seconds(5.0));
    udpc2.Stop(Seconds(sTime));
 
    UdpServerHelper udpserver2(port);
    ApplicationContainer udps2 = udpserver2.Install(wifiAdhocNodes.Get(39));
    udps2.Start(Seconds(5.0));
    udps2.Stop(Seconds(sTime));
*/
//-----------------------------------------------------------------------------------
/*
    UdpClientHelper udpclient3(interface.GetAddress(79),port);
    udpclient3.SetAttribute("MaxPackets",UintegerValue(50000));
    udpclient3.SetAttribute("Interval",TimeValue(MilliSeconds(100)));
    udpclient3.SetAttribute("PacketSize",UintegerValue(1024));
    ApplicationContainer udpc3 = udpclient3.Install(wifiAdhocNodes.Get(70));
    udpc3.Start(Seconds(5.0));
    udpc3.Stop(Seconds(sTime));
 
    UdpServerHelper udpserver3(port);
    ApplicationContainer udps3 = udpserver3.Install(wifiAdhocNodes.Get(79));
    udps3.Start(Seconds(5.0));
    udps3.Stop(Seconds(sTime));
*/
    //NS_LOG_UNCOND("schedule print_position");
    //Simulator::Schedule(Seconds(20),&print_position,wifiAdhocNodes);

    Simulator::Stop (Seconds (sTime));

    for(int i=0;i<=int(sTime);i+=50){
        Simulator::Schedule(Seconds(double(i)),&print_simulator_time);
        Simulator::Schedule(Seconds(double(i)),&print_mpr,wifiAdhocNodes,"mpr.tr",hd);
    }
    /*
    Ptr<OutputStreamWrapper> pos_vel = Create<OutputStreamWrapper>("link.tr",std::ios::out);
    for(int i=0;i<(int)nAdhoc;i++){
        for(int j=100*1000;j<=200*1000;j+=10){
            Simulator::Schedule(MilliSeconds(j),&print_loc_vel,wifiAdhocNodes.Get(i),pos_vel);
        }
    }
    */
    if (tracing == true)
    {
        AsciiTraceHelper ascii;
        //stack.EnablePcapIpv4All("myolsr");
        stack.EnablePcapIpv4("myolsr",wifiAdhocNodes.Get(79));
        //stack.EnableAsciiIpv4All(ascii.CreateFileStream ("myolsr.tr"));
        
    }
    //AnimationInterface anim("myolsr2.xml");
    //anim.SetMaxPktsPerTraceFile(INT64_MAX);
    clock_t starttime,endtime;
    starttime = clock();

    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    //NS_LOG_UNCOND("simulate start");
    Simulator::Run ();
    endtime = clock();
    std::cout << "The simluator time is:" <<(double)(endtime - starttime) / CLOCKS_PER_SEC << " s" << std::endl;

    
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());

    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

    uint32_t totalRx =0;
    uint32_t totalTx =0;
    uint32_t totalDrop =0;
    uint32_t totalLost = 0;
    double delay = 0;
    double count =0;
    double hopCount = 0;

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {

        //Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
/**
   (t.destinationAddress == "10.1.1.10") ||
    (t.destinationAddress == "10.1.1.40") ||
    (t.destinationAddress == "10.1.1.80")
*/
        if (true)
        {
            if (i->second.rxPackets != 0)
            {
                totalRx +=  i->second.rxPackets;
                totalTx +=  i->second.txPackets;
                totalLost += i->second.lostPackets;
                hopCount += (double)(i->second.timesForwarded);
                delay += i->second.delaySum.GetSeconds();
                count++;

                for (uint32_t j=0; j < i->second.packetsDropped.size() ; j++){
                    totalDrop += i->second.packetsDropped[j];
                }
            }
        }
    }
    double hc = (hopCount / (double)(totalRx) );
    double de = delay/totalRx;

    std::cout << "mpr_total_num:" << mpr_total_num << std::endl;
    std::cout << "totalTx:" << totalTx << " totalRx:" << totalRx << " DeliveryRate:" << (double)totalRx/(double)totalTx*100 << "%" << std::endl;
    std::cout << "hopCount:" << hc << " delay:" << de << " delay/hop:" << de/hc << std::endl;

    Simulator::Destroy ();

    //std::cout << "The run time is:" << (double)clock() /CLOCKS_PER_SEC<< " s" << std::endl;
    return 0;
}
