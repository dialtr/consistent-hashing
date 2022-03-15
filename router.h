// Copyright (C) 2021 Tom R. Dial
#ifndef INCLUDE_ROUTER_H_
#define INCLUDE_ROUTER_H_

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

// The **Router** class implements a stable mechanism for routing keys to
// hosts in a way that minimizes "reshuffles" when hosts become unavailable.
// It is implemented in terms of a consistent hash ring in which potentially
// many replicas of each host are used in order to ensure a relatively even
// distribution of load to each host.
//
// In addition to supporting a user-specified count of replicas (the number
// replicas used by default) Router supports weighting hosts with a floating
// point "weight" value. The actual number of replicas added to the ring is
// the product of the weight and the default replica count. As least one
// host replica is placed on the logical ring even if the weight is set to
// zero.
//
// Typical lifecycle:
//
//   // Construct with default replica count.
//   Router router(100);
//
//   // Add host(s)
//   router.AddHost("foo", 1.0);
//   router.AddHost("bar", 1.0);
//   router.AddHost("baz", 2.0);
//
//   // Router request key string to a host
//   auto route = router.Route("some-key");
//
//   // Remove host upon detecting it is out of service.
//   router.RemoveHost("foo");
//  

class Router {
 public:
  // Construct router specifying default replica count for each host.
  // The actual number of replicas used is computed by multiplying
  // the weight by the default count.
  explicit Router(int default_replica_count);

  ~Router();

  // Add a host to the routing table with specified weight.
	// Returns 'true' on success.
  // Returns 'false' if the host has already been added.
  bool AddHost(const std::string& host, double weight);

  // Remove a host from the routing table.
	// Returns 'true' on success.
	// Returns 'false' if no host by the specified name existed.
  bool RemoveHost(const std::string& host);

  // Route a user key to a host.
  // Returns empty string if no routing available.
  std::string Route(const std::string& key);

 private:
  // No copy / assignment allowed.
  Router(const Router&);
  Router& operator=(const Router&);

  // Number of replicas used for each host when weight is 1.0.
  // Weight multiplies this. At least one replica is always used.
  int default_replica_count_;

  // For each host, we keep track of its weight and replica IDs
  struct HostInfo {
    std::string name;
    std::string replica_name;
    double weight;
    std::vector<size_t> replicas;
  };

  // Keep track of all registered hosts.
  std::unordered_map<std::string, HostInfo*> hosts_;

  // Index to map replica IDs to host records.
  std::map<size_t, HostInfo*> replica_index_;
};

#endif  // INCLUDE_ROUTER_H_
