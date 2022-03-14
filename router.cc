#include "router.h"
//#include "fibonacci_hash.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>

using ::std::cerr;
using ::std::endl;

namespace {

// Limits governing the number of slots to use for normal-weighted hosts.
const int kMinReplicaCount = 1;
const int kMaxReplicaCount = 256;
// const int kNormalWeightReplicas = 128;

// The minimum weight. Specifying a weight of 0.0, however, will cause the
// number of slots to be rounded up to at least one.
const double kMinWeight = 0.0f;

// The maximum weight is 16x, which seems reasonable in a world in which
// vendors provide 1x containers to 16x containers. Research may indicate
// that we would like to have greater weights. Use this for now.
const double kMaxWeight = 16.0f;

// Compute the number of slots for a host.
size_t ComputeSlotCount(int normal_weight_slot_count, double weight) {
  const double normal_slot_count = normal_weight_slot_count;
  const double unadjusted_slots = normal_slot_count * weight;
  const size_t integer_slots = (size_t)(unadjusted_slots);
  return ((integer_slots < 1) ? 1 : integer_slots);
}

std::string MakeReplicaName(const std::string& name, size_t replica) {
  std::stringstream ss;
  ss << name << "_" << replica;
  return ss.str();
}

}  // namespace

Router::Router(int default_replica_count)
    : default_replica_count_(default_replica_count) {
  if ((default_replica_count < kMinReplicaCount) ||
      (default_replica_count > kMaxReplicaCount)) {
    throw std::domain_error("default replica count must be 1 <= N <= 256");
  }
}

Router::~Router() {
  // Delete structures allocated for hosts.
  // The replicas_ index are essentially "weak references" and thus
  // no action must be taken there.
  for (auto host : hosts_) {
    delete host.second;
  }
}

// Add a host to the routing table with specified weight.
// Returns 'true' if host was added.
// Returns 'false' if host already existed in routing table.
bool Router::AddHost(const std::string& host, double weight) {
  // Reject request to add already-added host.
  if (hosts_.find(host) != hosts_.end()) {
    return false;
  }

  // Reject invalid weights.
  if ((weight < kMinWeight) || (weight > kMaxWeight)) {
    return false;
  }

  // Compute total number of slots for the host.
  const size_t replicas = ComputeSlotCount(default_replica_count_, weight);
  cerr << "replicas: " << replicas << endl;

  // Create a standard library hash function that maps strings => size_t
  std::hash<std::string> hash_func;

  // Create new record for the host. We'll fill in the slots member in
  // in the loop below.
  HostInfo* info = new HostInfo();
  info->name = host;
  info->weight = weight;

  // Store the mapping from host names => info structures.
  hosts_[host] = info;

  size_t replica_id = 0;
  while (info->replicas.size() < replicas) {
    ++replica_id;
    const std::string replica_name = MakeReplicaName(host, replica_id);
    const size_t replica_hash = hash_func(replica_name);
    if (replica_index_.find(replica_hash) != replica_index_.end()) {
      // Hash collision, retry.
      continue;
    }
    info->replica_name = replica_name;
    info->replicas.push_back(replica_hash);
    replica_index_[replica_hash] = info;
    cerr << "replica_name: " << replica_name
         << ", replica_hash: " << replica_hash << endl;
  }

  return true;
}

// Remove a host from the routing table.
void Router::RemoveHost(const std::string& host) {
  cerr << "TODO(tdial): Implement" << endl;
}

// Route a user key to a host.
// Returns empty string if no routing available.
std::string Router::Route(const std::string& key) {
  // Standard library's hash function.
  std::hash<std::string> hash_func;

  // Compute hash for key using std::hash
  const size_t key_hash = hash_func(key);

  // Search in map for next nearest host.
  auto it1 = replica_index_.lower_bound(key_hash);
  if (it1 != replica_index_.end()) {
    return it1->second->name;
  }

  // Not found, must have wrapped around. Search starting at 0.
  auto it2 = replica_index_.lower_bound(0);
  if (it2 != replica_index_.end()) {
    return it2->second->name;
  }

  // Not found, map must be empty.
  return "";
}

void Router::Debug() {
  cerr << "replica_index_.size() = " << replica_index_.size() << endl;
}
