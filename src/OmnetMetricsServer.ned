package uav_ugv;

//
// OmnetMetricsServer — live OMNeT→ROS2 network metrics bridge.
//
// Listens on a TCP port and periodically sends one ASCII snapshot line
// to a connected client (e.g. the ROS2 omnet_metrics_bridge node):
//
//   <simtime_s> <distance_m> <rssi_dbm> <snir_db> <per>
//
// RSSI and SNIR are computed via a free-space path-loss model.
// PER is estimated from a sliding window of radio reception events.
//
simple OmnetMetricsServer
{
    parameters:
        int    port           = default(5556);
        double updateInterval @unit(s) = default(0.1s);
        string ugvModule      = default("^.ugv");
        string uavModule      = default("^.uav");
        // Path to the UGV radio module, relative to the ugvModule.
        // Change to "LoRaNic.radio" for LoRa configs.
        string radioModulePath = default("wlan[0].radio");
        // Transmit power of the UAV radio (dBm).
        double txPowerDbm     = default(13.0);    // ~20 mW (802.11g)
        // Carrier frequency (Hz) — must match the radio medium config.
        double carrierFreqHz  = default(2.4e9);   // 2.4 GHz WiFi default
        // Thermal + receiver noise floor (dBm).
        double noisePowerDbm  = default(-90.0);
        @display("i=block/server");
        @class(OmnetMetricsServer);
}
