#pragma once
#include <stdint.h>
#include <string>
#include "tools/log.h"
#include "exceptions.h"
#include "utility.h"

// 定义 OSPF日志接口
#define OspfLogTrace(LogFormat, ...)     LogTrace(LogFormat, ## __VA_ARGS__)
#define OspfLogDebug(LogFormat, ...)     LogDebug(LogFormat, ## __VA_ARGS__)
#define OspfLogInfo(LogFormat, ...)      LogInfo(LogFormat, ## __VA_ARGS__)
#define OspfLogWarn(LogFormat, ...)      LogWarn(LogFormat, ## __VA_ARGS__)
#define OspfLogError(LogFormat, ...)     LogError(LogFormat, ## __VA_ARGS__)
#define OspfLogFatal(LogFormat, ...)     LogFatal(LogFormat, ## __VA_ARGS__)
#define OspfLogAssert(assertion)        \
 do {                                   \
     if (!(assertion)) {                \
     OspfLogFatal(#assertion);            \
     }                                  \
 } while (0)
#define OspfLogUnreachable()                     \
 do {                                   \
    OspfLogFatal("Internal fatal error: unreachable code reached");   \
    /* exit(1);     unreached: keep the compiler happy */    \
 } while (0)

/**
 * OSPF Types
 */
struct OspfTypes
{

    /**
     * The OSPF version.
     */
    enum Version {V2 = 2, V3 = 3};

    /**
     * The type of an OSPF packet.
     */
    typedef uint16_t Type;

    /**
     * Router ID.
     */
    typedef uint32_t RouterID;

    /**
     * Area ID.
     */
    typedef uint32_t AreaID;

    /**
     * Link Type
     */
    enum LinkType
    {
        PointToPoint,
        BROADCAST,
        NBMA,
        PointToMultiPoint,
        VirtualLink
    };

    /**
     * Authentication type: OSPFv2 standard header.
     */
    typedef uint16_t AuType;
    static const AuType NULL_AUTHENTICATION = 0;
    static const AuType SIMPLE_PASSWORD = 1;
    static const AuType CRYPTOGRAPHIC_AUTHENTICATION = 2;

    /**
     * Area Type
     */
    enum AreaType
    {
        NORMAL,     // Normal Area
        STUB,       // Stub Area
        NSSA,       // Not-So-Stubby Area
    };

    /**
     * Routing Entry Type.
     */
    enum VertexType
    {
        Router,
        Network
    };

    /**
     * NSSA Translator Role.
     */
    enum NSSATranslatorRole
    {
        ALWAYS,
        CANDIDATE
    };

    /**
     * NSSA Translator State.
     */
    enum NSSATranslatorState
    {
        ENABLED,
        ELECTED,
        DISABLED,
    };

    /**
     * The AreaID for the backbone area.
     */
    static const AreaID BACKBONE = 0;

    /**
     * An opaque handle that identifies a peer.
     */
    typedef uint32_t PeerID;

    /**
     * An opaque handle that identifies a neighbour.
     */
    typedef uint32_t NeighbourID;

    /**
     * The IP protocol number used by OSPF.
     */
    static const uint16_t IP_PROTOCOL_NUMBER = 89;

    /**
     * An identifier meaning all peers. No single peer can have this
     * identifier.
     */
    static const PeerID ALLPEERS = 0;

    /**
     * An identifier meaning all neighbours. No single neighbour can
     * have this identifier.
     */
    static const NeighbourID ALLNEIGHBOURS = 0;

    /**
     * An interface ID that will never be allocated OSPFv3 only.
     */
    static const uint32_t UNUSED_INTERFACE_ID = 0;

    /**
     *
     * The maximum time between distinct originations of any particular
     * LSA.  If the LS age field of one of the router's self-originated
     * LSAs reaches the value LSRefreshTime, a new instance of the LSA
     * is originated, even though the contents of the LSA (apart from
     * the LSA header) will be the same.  The value of LSRefreshTime is
     * set to 30 minutes.
     */
    static const uint32_t LSRefreshTime = 30 * 60;

    /**
     * The minimum time between distinct originations of any particular
     * LSA.  The value of MinLSInterval is set to 5 seconds.
     */
    static const uint32_t MinLSInterval = 5;

    /**
     * For any particular LSA, the minimum time that must elapse
     * between reception of new LSA instances during flooding. LSA
     * instances received at higher frequencies are discarded. The
     * value of MinLSArrival is set to 1 second.
     */
    static const uint32_t MinLSArrival = 1;

    /**
     * The maximum age that an LSA can attain. When an LSA's LS age
     * field reaches MaxAge, it is reflooded in an attempt to flush the
     * LSA from the routing domain. LSAs of age MaxAge
     * are not used in the routing table calculation.   The value of
     * MaxAge is set to 1 hour.
     */
    static const uint32_t MaxAge = 60 * 60;

    /**
     * When the age of an LSA in the link state database hits a
     * multiple of CheckAge, the LSA's checksum is verified.  An
     * incorrect checksum at this time indicates a serious error.  The
     * value of CheckAge is set to 5 minutes.
     */
    static const uint32_t CheckAge = 5 * 60;

    /*
     * The maximum time dispersion that can occur, as an LSA is flooded
     * throughout the AS.  Most of this time is accounted for by the
     * LSAs sitting on router output queues (and therefore not aging)
     * during the flooding process.  The value of MaxAgeDiff is set to
     * 15 minutes.
     */
    static const int32_t MaxAgeDiff = 15 * 60;

    /*
     * The metric value indicating that the destination described by an
     * LSA is unreachable. Used in summary-LSAs and AS-external-LSAs as
     * an alternative to premature aging. It is
     * defined to be the 24-bit binary value of all ones: 0xffffff.
     */
    static const uint32_t LSInfinity = 0xffffff;

    /*
     * The Destination ID that indicates the default route.  This route
     * is used when no other matching routing table entry can be found.
     * The default destination can only be advertised in AS-external-
     * LSAs and in stub areas' type 3 summary-LSAs.  Its value is the
     * IP address 0.0.0.0. Its associated Network Mask is also always
     * 0.0.0.0.
     */
    static const uint32_t DefaultDestination = 0;

    /*
     * The value used for LS Sequence Number when originating the first
     * instance of any LSA.
     */
    static const int32_t InitialSequenceNumber = 0x80000001;

    /*
     * The maximum value that LS Sequence Number can attain.
     */
    static const int32_t MaxSequenceNumber = 0x7fffffff;
};
