// Copyright (C) 2021 Tom R. Dial
#include "router.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>

using ::std::cerr;
using ::std::endl;

namespace {

// Limits governing the number of replicas to use.
const int kMinReplicaCount = 1;
const int kMaxReplicaCount = 256;

// The minimum weight. Specifying a weight of 0.0, however, will cause the
// number of replicas to be rounded up to at least one.
const double kMinWeight = 0.0f;

// The maximum weight is 16x, which seems reasonable in a world in which
// vendors provide 1x containers to 16x containers. Research may indicate
// that we would like to have greater weights. Use this for now.
const double kMaxWeight = 16.0f;

// Compute the number of replicas for a host.
size_t ComputeReplicaCount(int base_replica_count, double weight) {
  const size_t unadj_replicas =
      static_cast<size_t>(static_cast<double>(base_replica_count) * weight);
  return ((unadj_replicas < 1) ? 1 : unadj_replicas);
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

  // Compute total number of replicas for the host.
  const size_t replicas = ComputeReplicaCount(default_replica_count_, weight);

  // Create a standard library hash function that maps strings => size_t
  std::hash<std::string> hash_func;

  // Create new record for the host. We store the object in a map so that
  // we may look it up by name, and we also store weak references to the
  // entry in the replica index so that we can find the host given the
  // replica ID.
  HostInfo* info = new HostInfo();
  info->name = host;
  info->weight = weight;

  // Store the mapping from host names => info structures.
  hosts_[host] = info;

  // Add N replicas (determined above in the computation. We guarantee
  // that we will add the exact number of replicas by handling the
  // admittedly unlikely occurrence of a collision.
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
  }

  return true;
}

// Remove a host from the routing table.
bool Router::RemoveHost(const std::string& host) {
  auto it = hosts_.find(host);
  if (it == hosts_.end()) {
    return false;
  }

  // Erase all replicas.
  for (auto id : it->second->replicas) {
    replica_index_.erase(id);
  }

  // The host record was dynamically allocated; delete .
  delete it->second;

  // Erase the host entry completely.
  hosts_.erase(it);

  return true;
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
