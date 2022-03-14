#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include <algorithm>
#include <deque>
#include <functional>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "router.h"

using namespace std;

string MakeRandomKey() {
  stringstream ss;
  for (int i = 0; i < 8; ++i) {
    char ch = 'A' + (rand() % 26);
    ss << ch;
  }
  return ss.str();
}

int main(int argc, char* argv[]) {
  // Seed RNG for our simulation
  srand(time(NULL));

  const int kDefaultReplicaCount = 128;
  Router router(kDefaultReplicaCount);

  // Add hosts with with mostly the same weights.
  router.AddHost("srv-01", 4.0);
  router.AddHost("srv-02", 2.0);
  router.AddHost("srv-03", 2.0);
  router.AddHost("srv-04", 2.0);
  router.AddHost("srv-05", 2.0);
  router.AddHost("srv-06", 2.0);

  cout << "-------" << endl;

  // Map to track routing stats.
  map<string, int> hist;

  // Simulate routing a bunch of requests, tracking the routing choices.
  const int REQUEST_COUNT = 100000;
  for (int i = 0; i < REQUEST_COUNT; ++i) {
    const string key(MakeRandomKey());
    const string host = router.Route(key);
    hist[host]++;
  }

  cout << "-------" << endl;

  // Print "histogram", % utilization of each host
  cout << "Histogram: " << endl;
  for (auto it : hist) {
    double load = ((double)it.second) / ((double)REQUEST_COUNT) * 100.0;
    cout << "server: " << it.first << ", load: " << load << endl;
  }

  return 0;
}
