#ifndef INCLUDE_ROUTER_H_
#define INCLUDE_ROUTER_H_

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

class Router {
 public:
  // Construct router specifying default replica count for each host.
  // The actual number of replicas used is computed by multiplying
  // the weight by the default count.
  explicit Router(int default_replica_count);

  ~Router();

  // Add a host to the routing table with specified weight.
  bool AddHost(const std::string& host, double weight);

  // Remove a host from the routing table.
  void RemoveHost(const std::string& host);

  // Route a user key to a host.
  // Returns empty string if no routing available.
  std::string Route(const std::string& key);

  // Dump internal table to stderr
  void Debug();

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
