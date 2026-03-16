// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <deque>
#include <string>

#include <omnetpp.h>

#include "inet/common/geometry/common/Coord.h"
#include "inet/mobility/contract/IMobility.h"

/**
 * OmnetMetricsServer — live OMNeT→ROS2 network metrics bridge.
 *
 * Listens on a TCP port and, at each update interval, sends a single
 * ASCII snapshot line to any connected client:
 *
 *   <simtime_s> <distance_m> <rssi_dbm> <snir_db> <per> <radio_distance_m>
 *
 * RSSI and SNIR are derived from a free-space path-loss model using the
 * configured tx power and carrier frequency.  PER is estimated from a
 * sliding window of radio reception events subscribed via INET signals.
 * radio_distance_m is the FSPL-inverted distance estimate derived solely
 * from the RSSI value (no Gazebo position data used).
 *
 * The server accepts exactly one client at a time.  If the client
 * disconnects it is replaced by the next one that connects.
 */
class OmnetMetricsServer : public omnetpp::cSimpleModule, public omnetpp::cListener
{
  protected:
    // ── parameters ────────────────────────────────────────────────────
    int         port            = 5556;
    double      updateInterval  = 0.1;   // seconds
    std::string ugvModulePath;           // e.g. "ugv"
    std::string uavModulePath;           // e.g. "uav"
    std::string radioModulePath;         // e.g. "wlan[0].radio"  (relative to ugv)
    double      txPowerDbm      = -4.0;  // 20 mW default for IEEE 802.11g
    double      carrierFreqHz   = 2.4e9; // 2.4 GHz
    double      noisePowerDbm   = -90.0; // thermal noise floor

    // ── runtime state ─────────────────────────────────────────────────
    inet::IMobility* ugvMobility = nullptr;
    inet::IMobility* uavMobility = nullptr;

    // Packet delivery sliding window (true=received, false=dropped)
    static constexpr int PER_WINDOW = 30;
    std::deque<bool> perWindow;

    // Timer
    omnetpp::cMessage* metricsTimer = nullptr;

    // TCP server / client sockets (opaque pointers to SOCKET)
    void* serverSockPtr = nullptr;
    void* clientSockPtr = nullptr;

    // Cached signal IDs
    static const omnetpp::simsignal_t packetSentToUpperSignal;
    static const omnetpp::simsignal_t packetDroppedSignal;

  protected:
    // ── OMNeT++ lifecycle ──────────────────────────────────────────────
    virtual int  numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(omnetpp::cMessage* msg) override;
    virtual void finish() override;

    // ── cListener ─────────────────────────────────────────────────────
    // Receives packetSentToUpper / packetDropped (cObject* variant)
    virtual void receiveSignal(omnetpp::cComponent* src,
                               omnetpp::simsignal_t  signal,
                               omnetpp::cObject*     obj,
                               omnetpp::cObject*     details) override;

  private:
    // ── socket helpers ────────────────────────────────────────────────
    void startServer();
    void tryAcceptClient();
    void sendMetricsLine();
    void closeClientSocket();
    void closeServerSocket();

    // ── metric computation ────────────────────────────────────────────
    double computeDistance() const;
    double computeRssiDbm(double distanceM) const;
    double computeSnirDb(double distanceM) const;
    double computePer() const;
    double computeRadioDistance(double rssiDbm) const;
};
