/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Projekt: Porównanie pojemności sieci Wi-Fi (n/ac/ax)
 * Przystosowano dla NS-3.47 z obsługą wyboru pasma 2.4/5GHz
 */

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-flow-classifier.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("wifi-videoconf-capacity");

class SimulationHelper 
{
public:
    static OnOffHelper CreateOnOffHelper(InetSocketAddress socketAddress, DataRate dataRate, int packetSize, uint8_t tid, Time start, Time stop) 
    {
        OnOffHelper onOffHelper ("ns3::UdpSocketFactory", socketAddress);
        onOffHelper.SetAttribute ("DataRate", DataRateValue (dataRate));
        onOffHelper.SetAttribute ("PacketSize", UintegerValue (packetSize));
        onOffHelper.SetAttribute ("StartTime", TimeValue (start));
        onOffHelper.SetAttribute ("StopTime", TimeValue (stop));
        onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
        onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
        
        // TOS dla QoS w NS-3.47
        onOffHelper.SetAttribute ("Tos", UintegerValue (tid << 5)); 
        
        return onOffHelper;
    }

    static void PopulateArpCache () 
    {
        Ptr<ArpCache> arp = CreateObject<ArpCache> ();
        arp->SetAliveTimeout (Seconds (3600 * 24 * 365));
        for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); ++i) 
        {	
            Ptr<Ipv4L3Protocol> ip = (*i)->GetObject<Ipv4L3Protocol> ();
            ObjectVectorValue interfaces;
            ip->GetAttribute ("InterfaceList", interfaces);
            for (ObjectVectorValue::Iterator j = interfaces.Begin (); j != interfaces.End (); j++) 
            {		
                Ptr<Ipv4Interface> ipIface = (*j).second->GetObject<Ipv4Interface> ();
                Ptr<NetDevice> device = ipIface->GetDevice ();
                Mac48Address addr = Mac48Address::ConvertFrom (device->GetAddress ());
                for (uint32_t k = 0; k < ipIface->GetNAddresses (); k++) 
                {			
                    Ipv4Address ipAddr = ipIface->GetAddress (k).GetLocal();		
                    if (ipAddr == Ipv4Address::GetLoopback ()) continue;
                    ArpCache::Entry *entry = arp->Add (ipAddr);
                    Ipv4Header ipv4Hdr;
                    ipv4Hdr.SetDestination (ipAddr);
                    Ptr<Packet> p = Create<Packet> (100);  
                    entry->MarkWaitReply (ArpCache::Ipv4PayloadHeaderPair (p, ipv4Hdr));
                    entry->MarkAlive (addr);
                }
            }
        }
        for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); ++i) 
        {
            Ptr<Ipv4L3Protocol> ip = (*i)->GetObject<Ipv4L3Protocol> ();
            ObjectVectorValue interfaces;
            ip->GetAttribute ("InterfaceList", interfaces);
            for (ObjectVectorValue::Iterator j = interfaces.Begin (); j != interfaces.End (); j++)
            {
                Ptr<Ipv4Interface> ipIface = (*j).second->GetObject<Ipv4Interface> ();
                ipIface->SetAttribute ("ArpCache", PointerValue (arp));
            }
        }
    }
};

int main (int argc, char *argv[])
{
    std::string wifiStandard = "ac";
    double band = 5.0; // Domyślnie 5 GHz
    uint32_t nVideo = 5;
    uint32_t nBackground = 0;
    double videoMbps = 2.0; 
    double bgMbps = 10.0;   
    float simTime = 30.0;
    float calcStart = 20.0;
    uint32_t seed = 1;

    CommandLine cmd;
    cmd.AddValue ("standard", "Standard Wi-Fi: n, ac, ax", wifiStandard);
    cmd.AddValue ("band", "Pasmo w GHz: 2.4 lub 5.0", band);
    cmd.AddValue ("nVideo", "Liczba stacji wideo", nVideo);
    cmd.AddValue ("nBackground", "Liczba stacji tla", nBackground);
    cmd.AddValue ("vRate", "Predkosc wideo [Mbps]", videoMbps);
    cmd.AddValue ("bgRate", "Predkosc tla [Mbps]", bgMbps);
    cmd.AddValue ("simTime", "Czas symulacji [s]", simTime);
    cmd.AddValue ("seed", "Seed", seed);
    cmd.Parse (argc, argv);

    // Blokada dla nieistniejącej konfiguracji ac na 2.4GHz
    if (wifiStandard == "ac" && band < 4.0) {
        NS_FATAL_ERROR("Standard 802.11ac nie jest obsługiwany w paśmie 2.4 GHz!");
    }

    Time simulationTime = Seconds (simTime);
    ns3::RngSeedManager::SetSeed (seed);

    uint32_t nSTA = nVideo + nBackground;
    NodeContainer sta;
    sta.Create (nSTA + 1);

    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (0.0, 0.0, 0.0)); // AP
    for (uint32_t i = 0; i < nSTA; i++)
        positionAlloc->Add (Vector (5.0, 0.0, 0.0)); // STAs w odległości 5m
    mobility.SetPositionAllocator (positionAlloc);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (sta);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    YansWifiPhyHelper phy;
    phy.SetChannel (channel.Create ());

    // USTAWIENIE PASMA W NS-3.47 (ChannelSettings)
    // Format: {Numer kanału, Szerokość w MHz, Pasmo, Indeks PHY}
    // Wartości '0' sprawiają, że NS-3 automatycznie dobiera optymalny kanał i maksymalną szerokość dla danego standardu
    if (band < 4.0) {
        phy.Set ("ChannelSettings", StringValue ("{0, 0, BAND_2_4GHZ, 0}")); 
    } else {
        phy.Set ("ChannelSettings", StringValue ("{0, 0, BAND_5GHZ, 0}")); 
    }
    WifiHelper wifi;
    WifiMacHelper mac;

    if (wifiStandard == "n") {
        wifi.SetStandard (WIFI_STANDARD_80211n);
    } else if (wifiStandard == "ac") {
        wifi.SetStandard (WIFI_STANDARD_80211ac);
    } else if (wifiStandard == "ax") {
        wifi.SetStandard (WIFI_STANDARD_80211ax);
    }

    mac.SetType ("ns3::AdhocWifiMac", "Ssid", SsidValue (Ssid ("JOWS-Project")));
    NetDeviceContainer staDevices = wifi.Install (phy, mac, sta);

    // Ograniczenie czasu życia pakietu w kolejce Video do 150ms (zgodnie z metodologią)
    Config::Set ("/NodeList/*/DeviceList/*/Mac/VI_Txop/Queue/MaxDelay", TimeValue (MilliSeconds (150)));

    InternetStackHelper stack;
    stack.Install (sta);

    Ipv4AddressHelper address;
    address.SetBase ("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer staIf = address.Assign (staDevices);

    uint32_t apIndex = nSTA; 
    Ipv4Address apAddress = staIf.GetAddress(apIndex);

    PacketSinkHelper sink_VI ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), 1005));
    sink_VI.Install (sta.Get(apIndex));
    PacketSinkHelper sink_BE ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), 1000));
    sink_BE.Install (sta.Get(apIndex));

    for (uint32_t i = 0; i < nSTA; i++) 
    {
        if (i < nVideo) {
            SimulationHelper::CreateOnOffHelper(InetSocketAddress (apAddress, 1005), 
                DataRate (videoMbps * 1000000), 1472, 5, Seconds(0.0), simulationTime).Install(sta.Get(i));
        } else {
            SimulationHelper::CreateOnOffHelper(InetSocketAddress (apAddress, 1000), 
                DataRate (bgMbps * 1000000), 1472, 0, Seconds(1.0), simulationTime).Install(sta.Get(i));
        }
    }

    FlowMonitorHelper flowmon_helper;
    Ptr<FlowMonitor> monitor = flowmon_helper.InstallAll ();
    monitor->SetAttribute ("StartTime", TimeValue (Seconds (calcStart)));

    SimulationHelper::PopulateArpCache ();
    Simulator::Stop (simulationTime);
    Simulator::Run ();

    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon_helper.GetClassifier ());
    std::map< FlowId, FlowMonitor::FlowStats > stats = monitor->GetFlowStats();

    std::cout << "\n--- PROJEKT JOWS: ANALIZA POJEMNOSCI SIECI ---" << std::endl;
    std::cout << "Standard: " << wifiStandard << " | Pasmo: " << band << " GHz" << std::endl;
    std::cout << "Stacje Video: " << nVideo << " | Stacje tla: " << nBackground << std::endl;

    for (std::map< FlowId, FlowMonitor::FlowStats >::iterator flow = stats.begin (); flow != stats.end (); flow++)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (flow->first);
        if (t.destinationPort == 1005) { // Logujemy tylko ruch VIDEO
            std::cout << "FlowID: " << flow->first << " (VIDEO, " << t.sourceAddress << " -> AP)\n";
            if (flow->second.rxPackets > 0) {
                double duration = (simulationTime - Seconds (calcStart)).GetSeconds ();
                std::cout << "  Throughput:  " << flow->second.rxBytes * 8.0 / duration / 1000000 << " Mb/s\n";
                std::cout << "  Mean Delay:  " << (double)flow->second.delaySum.GetMilliSeconds() / flow->second.rxPackets << " ms\n";
                std::cout << "  Mean Jitter: " << (double)flow->second.jitterSum.GetMilliSeconds() / (flow->second.rxPackets - 1) << " ms\n";
                std::cout << "  Packet Loss: " << (double)flow->second.lostPackets * 100 / (flow->second.txPackets) << " %\n";
            }
        }
    }

    Simulator::Destroy ();
    return 0;
}