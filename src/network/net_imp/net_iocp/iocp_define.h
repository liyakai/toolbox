#pragma once

using ACCEPTEX = LPFN_ACCEPTEX;  // AcceptEx º¯ÊýÖ¸Õë,Ïê¼ûMSDN

// MSDN: The number of bytes reserved for the local address information. 
// This value must be at least 16 bytes more than the maximum address length for the transport protocol in use.
constexpr std::size_t ACCEPTEX_ADDR_SIZE = sizeof(sockaddr_in) + 16;


