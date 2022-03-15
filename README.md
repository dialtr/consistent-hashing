# consistent-hashing

## About
The **Router** class implements a stable mechanism for routing keys to
hosts in a way that minimizes "reshuffles" when hosts become unavailable.
It is implemented in terms of a consistent hash ring in which potentially
many replicas of each host are used in order to ensure a relatively even
distribution of load to each host.

In addition to supporting a user-specified count of replicas (the number
replicas used by default) Router supports weighting hosts with a floating
point "weight" value. The actual number of replicas added to the ring is
the product of the weight and the default replica count. As least one
host replica is placed on the logical ring even if the weight is set to
zero.

## Building and running the demo from source.

```
  # Fetch source
  git clone git@github.com:dialtr/consistent-hashing.git

  # Build
  cd consistent-hashing
  make
  
  # Run the demo
  ./demo
```

## Usage lifecycle

```C++
  // Construct with default replica count.
  Router router(100);

  // Add host(s)
  router.AddHost("foo", 1.0);
  router.AddHost("bar", 1.0);
  router.AddHost("baz", 2.0);

  // Router request key string to a host
  auto route = router.Route("some-key");

  // Remove host upon detecting it is out of service.
  router.RemoveHost("foo");
```

## Authors
Copyright (C) 2021 Tom R. Dial 

